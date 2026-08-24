/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LOG_TAG "ThermalHAL-hi3635"
#include <log/log.h>

#include <hardware/hardware.h>
#include <hardware/thermal.h>

#define MAX_PATH_LENGTH 128
#define MAX_TYPE_LENGTH 32
#define CPU_COUNT 8
#define PROC_STAT_PATH "/proc/stat"
#define CPU_ONLINE_PATH "/sys/devices/system/cpu/cpu%u/online"
#define THERMAL_DIR "/sys/devices/virtual/thermal"
#define THERMAL_ZONE_PREFIX "thermal_zone"

static const char *cpu_names[CPU_COUNT] = {
    "cpu0", "cpu1", "cpu2", "cpu3", "cpu4", "cpu5", "cpu6", "cpu7",
};

struct temperature_sensor {
    const char *name;
    enum temperature_type type;
    char path[MAX_PATH_LENGTH];
};

static struct temperature_sensor sensors[] = {
    { "cluster0", DEVICE_TEMPERATURE_CPU, "" },
    { "cluster1", DEVICE_TEMPERATURE_CPU, "" },
    { "gpu", DEVICE_TEMPERATURE_GPU, "" },
};

static pthread_once_t sensors_once = PTHREAD_ONCE_INIT;

static int read_string(const char *path, char *value, size_t size) {
    FILE *file = fopen(path, "re");
    if (file == NULL) {
        return -errno;
    }

    if (fgets(value, size, file) == NULL) {
        int error = ferror(file) && errno != 0 ? -errno : -EIO;
        fclose(file);
        return error;
    }
    fclose(file);

    value[strcspn(value, "\r\n")] = '\0';
    return 0;
}

static void find_temperature_sensors(void) {
    DIR *directory = opendir(THERMAL_DIR);
    if (directory == NULL) {
        ALOGE("Failed to open %s: %s", THERMAL_DIR, strerror(errno));
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        if (strncmp(entry->d_name, THERMAL_ZONE_PREFIX,
                    strlen(THERMAL_ZONE_PREFIX)) != 0) {
            continue;
        }

        char path[MAX_PATH_LENGTH];
        char type[MAX_TYPE_LENGTH];
        snprintf(path, sizeof(path), "%s/%s/type", THERMAL_DIR, entry->d_name);
        if (read_string(path, type, sizeof(type)) != 0) {
            continue;
        }

        for (size_t i = 0; i < sizeof(sensors) / sizeof(sensors[0]); ++i) {
            if (strcmp(type, sensors[i].name) == 0) {
                snprintf(sensors[i].path, sizeof(sensors[i].path), "%s/%s/temp",
                         THERMAL_DIR, entry->d_name);
                break;
            }
        }
    }
    closedir(directory);
}

static float read_temperature(const struct temperature_sensor *sensor) {
    if (sensor->path[0] == '\0') {
        return UNKNOWN_TEMPERATURE;
    }

    FILE *file = fopen(sensor->path, "re");
    if (file == NULL) {
        ALOGE("Failed to open %s: %s", sensor->path, strerror(errno));
        return UNKNOWN_TEMPERATURE;
    }

    float temperature;
    if (fscanf(file, "%f", &temperature) != 1) {
        ALOGE("Failed to read %s", sensor->path);
        temperature = UNKNOWN_TEMPERATURE;
    }
    fclose(file);
    return temperature;
}

static ssize_t get_temperatures(thermal_module_t *module, temperature_t *list,
                                size_t size) {
    (void)module;
    pthread_once(&sensors_once, find_temperature_sensors);

    const size_t sensor_count = sizeof(sensors) / sizeof(sensors[0]);
    if (list == NULL) {
        return sensor_count;
    }

    size_t output_count = size < sensor_count ? size : sensor_count;
    for (size_t i = 0; i < output_count; ++i) {
        list[i] = (temperature_t) {
            .type = sensors[i].type,
            .name = sensors[i].name,
            // The hi3635 thermal driver exports CPU/GPU values in degrees Celsius.
            .current_value = read_temperature(&sensors[i]),
            .throttling_threshold = UNKNOWN_TEMPERATURE,
            .shutdown_threshold = UNKNOWN_TEMPERATURE,
            .vr_throttling_threshold = UNKNOWN_TEMPERATURE,
        };
    }

    return sensor_count;
}

static ssize_t get_cpu_usages(thermal_module_t *module, cpu_usage_t *list) {
    (void)module;
    if (list == NULL) {
        return CPU_COUNT;
    }

    long ticks_per_second = sysconf(_SC_CLK_TCK);
    if (ticks_per_second <= 0) {
        return -EINVAL;
    }

    for (unsigned int cpu = 0; cpu < CPU_COUNT; ++cpu) {
        list[cpu] = (cpu_usage_t) {
            .name = cpu_names[cpu],
            .active = 0,
            .total = 0,
            .is_online = cpu == 0,
        };

        if (cpu != 0) {
            char path[MAX_PATH_LENGTH];
            int online;
            snprintf(path, sizeof(path), CPU_ONLINE_PATH, cpu);
            FILE *online_file = fopen(path, "re");
            if (online_file != NULL) {
                list[cpu].is_online = fscanf(online_file, "%d", &online) == 1 && online != 0;
                fclose(online_file);
            }
        }
    }

    FILE *stat_file = fopen(PROC_STAT_PATH, "re");
    if (stat_file == NULL) {
        return -errno;
    }

    char line[512];
    while (fgets(line, sizeof(line), stat_file) != NULL) {
        char name[16];
        unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
        if (sscanf(line, "%15s %llu %llu %llu %llu %llu %llu %llu %llu",
                   name, &user, &nice, &system, &idle, &iowait, &irq, &softirq,
                   &steal) != 9 || strncmp(name, "cpu", 3) != 0 || name[3] == '\0') {
            continue;
        }

        char *end;
        unsigned long cpu = strtoul(name + 3, &end, 10);
        if (*end != '\0' || cpu >= CPU_COUNT) {
            continue;
        }

        unsigned long long active = user + nice + system + irq + softirq + steal;
        unsigned long long total = active + idle + iowait;
        list[cpu].active = active * 1000ULL / (unsigned long long)ticks_per_second;
        list[cpu].total = total * 1000ULL / (unsigned long long)ticks_per_second;
    }
    fclose(stat_file);
    return CPU_COUNT;
}

static ssize_t get_cooling_devices(thermal_module_t *module,
                                   cooling_device_t *list, size_t size) {
    (void)module;
    (void)list;
    (void)size;
    return 0;
}

static struct hw_module_methods_t thermal_module_methods = {
    .open = NULL,
};

thermal_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = THERMAL_HARDWARE_MODULE_API_VERSION_0_1,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = THERMAL_HARDWARE_MODULE_ID,
        .name = "HiSilicon hi3635 Thermal HAL",
        .author = "The LineageOS Project",
        .methods = &thermal_module_methods,
    },
    .getTemperatures = get_temperatures,
    .getCpuUsages = get_cpu_usages,
    .getCoolingDevices = get_cooling_devices,
};
