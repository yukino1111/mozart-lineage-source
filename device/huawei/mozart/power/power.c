/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "PowerHAL-hi3635"

#include <errno.h>
#include <fcntl.h>
#include <hardware/hardware.h>
#include <hardware/power.h>
#include <log/log.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_INTERACTION_MS 1500
#define LAUNCH_BOOST_MS 1500
#define MAX_BOOST_MS 3000
#define LINEAGE_POWER_HINT_CPU_BOOST 0x00000110

static const char *cpu0_min =
        "/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq";
static const char *cpu4_min =
        "/sys/devices/system/cpu/cpu4/cpufreq/scaling_min_freq";
static const char *cpu0_boostpulse =
        "/sys/devices/system/cpu/cpu0/cpufreq/interactive/boostpulse";
static const char *gpu_min = "/sys/class/devfreq/gpufreq/min_freq";
static const char *ddr_min = "/sys/class/devfreq/ddrfreq/min_freq";
static const char *hmp_up = "/sys/kernel/hmp/up_threshold";
static const char *hmp_down = "/sys/kernel/hmp/down_threshold";

static pthread_mutex_t boost_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t boost_cond;
static pthread_t boost_thread;
static struct timespec boost_deadline;
static bool boost_active;
static bool boost_thread_started;
static bool low_power_mode;

static void write_value(const char *path, const char *value) {
    int fd = open(path, O_WRONLY | O_CLOEXEC);
    if (fd < 0) {
        ALOGE("open %s failed: %s", path, strerror(errno));
        return;
    }

    size_t length = strlen(value);
    ssize_t written = write(fd, value, length);
    if (written != (ssize_t)length) {
        ALOGE("write %s failed: %s", path,
              written < 0 ? strerror(errno) : "short write");
    }
    close(fd);
}

static void set_interaction_boost(bool enabled) {
    if (enabled) {
        // Use B217-supported operating points for short UI interactions. The
        // 600 MHz GPU point reduces SystemUI tail latency while the kernel
        // thermal limits remain authoritative; no maximum is changed.
        write_value(cpu0_min, "1516800");
        write_value(cpu4_min, "2016000");
        write_value(gpu_min, "600000000");
        write_value(ddr_min, "667000000");
        write_value(hmp_up, "300");
        write_value(hmp_down, "150");
    } else {
        write_value(cpu0_min, "403200");
        write_value(cpu4_min, "1017600");
        write_value(gpu_min, "288000000");
        write_value(ddr_min, "120000000");
        write_value(hmp_up, "978");
        write_value(hmp_down, "672");
    }
}

static int compare_timespec(const struct timespec *left,
                            const struct timespec *right) {
    if (left->tv_sec != right->tv_sec) {
        return left->tv_sec < right->tv_sec ? -1 : 1;
    }
    if (left->tv_nsec != right->tv_nsec) {
        return left->tv_nsec < right->tv_nsec ? -1 : 1;
    }
    return 0;
}

static void *boost_worker(void *unused) {
    (void)unused;

    pthread_mutex_lock(&boost_lock);
    for (;;) {
        while (!boost_active) {
            pthread_cond_wait(&boost_cond, &boost_lock);
        }

        int result = pthread_cond_timedwait(&boost_cond, &boost_lock,
                                            &boost_deadline);
        if (result == ETIMEDOUT) {
            struct timespec now;
            clock_gettime(CLOCK_MONOTONIC, &now);
            if (boost_active && compare_timespec(&now, &boost_deadline) >= 0) {
                boost_active = false;
                pthread_mutex_unlock(&boost_lock);
                set_interaction_boost(false);
                pthread_mutex_lock(&boost_lock);
            }
        }
    }
}

static void request_boost(int duration_ms) {
    if (duration_ms <= 0) {
        duration_ms = DEFAULT_INTERACTION_MS;
    } else if (duration_ms > MAX_BOOST_MS) {
        duration_ms = MAX_BOOST_MS;
    }

    struct timespec deadline;
    clock_gettime(CLOCK_MONOTONIC, &deadline);
    deadline.tv_sec += duration_ms / 1000;
    deadline.tv_nsec += (duration_ms % 1000) * 1000000L;
    if (deadline.tv_nsec >= 1000000000L) {
        ++deadline.tv_sec;
        deadline.tv_nsec -= 1000000000L;
    }

    pthread_mutex_lock(&boost_lock);
    if (low_power_mode) {
        pthread_mutex_unlock(&boost_lock);
        return;
    }
    if (!boost_active || compare_timespec(&deadline, &boost_deadline) > 0) {
        boost_deadline = deadline;
    }
    boost_active = true;
    pthread_cond_signal(&boost_cond);
    pthread_mutex_unlock(&boost_lock);

    set_interaction_boost(true);
}

static void cancel_boost(void) {
    pthread_mutex_lock(&boost_lock);
    bool was_active = boost_active;
    boost_active = false;
    pthread_cond_signal(&boost_cond);
    pthread_mutex_unlock(&boost_lock);

    if (was_active) {
        set_interaction_boost(false);
    }
}

static void power_init(struct power_module *module) {
    (void)module;

    pthread_condattr_t attr;
    pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    pthread_cond_init(&boost_cond, &attr);
    pthread_condattr_destroy(&attr);

    if (pthread_create(&boost_thread, NULL, boost_worker, NULL) == 0) {
        pthread_detach(boost_thread);
        boost_thread_started = true;
    } else {
        ALOGE("failed to start interaction boost worker");
    }
}

static void power_set_interactive(struct power_module *module, int on) {
    (void)module;
    if (!on && boost_thread_started) {
        cancel_boost();
    }
}

static void power_hint(struct power_module *module, power_hint_t hint,
                       void *data) {
    (void)module;
    if (!boost_thread_started) {
        return;
    }

    switch (hint) {
        case POWER_HINT_INTERACTION:
            // The B217 power HAL pulses the little cluster on every real
            // touch. Android limits identical user-activity hints to 10 Hz.
            write_value(cpu0_boostpulse, "1");
            request_boost(data != NULL ? *(int32_t *)data
                                       : DEFAULT_INTERACTION_MS);
            break;
        case POWER_HINT_LAUNCH:
            if (data != NULL && *(int32_t *)data != 0) {
                request_boost(LAUNCH_BOOST_MS);
            }
            break;
        case POWER_HINT_LOW_POWER: {
            bool enabled = data != NULL && *(int32_t *)data != 0;
            pthread_mutex_lock(&boost_lock);
            low_power_mode = enabled;
            pthread_mutex_unlock(&boost_lock);
            if (enabled) {
                cancel_boost();
            }
            break;
        }
        default:
            if ((uint32_t)hint == LINEAGE_POWER_HINT_CPU_BOOST) {
                // Lineage passes this duration in microseconds.
                int duration_us = data != NULL ? *(int32_t *)data : 0;
                request_boost(duration_us / 1000);
            }
            break;
    }
}

static struct hw_module_methods_t power_module_methods = {
    .open = NULL,
};

struct power_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = POWER_MODULE_API_VERSION_0_2,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = POWER_HARDWARE_MODULE_ID,
        .name = "HiSilicon hi3635 Power HAL",
        .author = "The LineageOS Project",
        .methods = &power_module_methods,
    },
    .init = power_init,
    .setInteractive = power_set_interactive,
    .powerHint = power_hint,
};
