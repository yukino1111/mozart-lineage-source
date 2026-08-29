/*
 * Copyright 2026 yukino1111
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "MozartHwc2"

#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <mutex>
#include <new>
#include <string>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <thread>
#include <unistd.h>

#include <hardware/fb.h>
#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>
#include <hwc2onfbadapter/HWC2OnFbAdapter.h>
#include <log/log.h>

namespace {

constexpr char kVsyncEventPath[] = "/sys/class/graphics/fb0/vsync_event";
constexpr char kFramebufferPath[] = "/dev/graphics/fb0";
constexpr unsigned long kHisiFbVsyncCtrl = _IOW('M', 0x02, unsigned int);

template <typename Function>
hwc2_function_pointer_t asFunctionPointer(Function function) {
    return reinterpret_cast<hwc2_function_pointer_t>(function);
}

class MozartHwc2 final : public android::HWC2OnFbAdapter {
  public:
    explicit MozartHwc2(framebuffer_device_t* framebuffer)
        : HWC2OnFbAdapter(framebuffer), mOriginalGetFunction(getFunction) {
        hwc2_device::getCapabilities = hookGetCapabilities;
        getFunction = hookGetFunction;
        mVsyncThread = std::thread(&MozartHwc2::vsyncLoop, this);
    }

    void initializeCommon(const hw_module_t* module) {
        common.tag = HARDWARE_DEVICE_TAG;
        common.version = HWC_DEVICE_API_VERSION_2_0;
        common.module = const_cast<hw_module_t*>(module);
        common.close = hookClose;
    }

  private:
    static MozartHwc2& cast(hwc2_device_t* device) {
        return *static_cast<MozartHwc2*>(device);
    }

    static MozartHwc2& cast(hw_device_t* device) {
        return *reinterpret_cast<MozartHwc2*>(device);
    }

    static void hookGetCapabilities(hwc2_device_t* /*device*/, uint32_t* outCount,
                                    int32_t* /*outCapabilities*/) {
        // LOS 18.1 is built without sync framework support. Advertising the
        // fb adapter's unreliable-fence capability makes Composer reject a
        // standalone HWC2 module before it can use that legacy configuration.
        *outCount = 0;
    }

    static hwc2_function_pointer_t hookGetFunction(hwc2_device_t* device, int32_t descriptor) {
        switch (descriptor) {
            case HWC2_FUNCTION_REGISTER_CALLBACK:
                return asFunctionPointer(hookRegisterCallback);
            case HWC2_FUNCTION_SET_VSYNC_ENABLED:
                return asFunctionPointer(hookSetVsyncEnabled);
            default: {
                auto& hwc = cast(device);
                return hwc.mOriginalGetFunction(device, descriptor);
            }
        }
    }

    static int32_t hookRegisterCallback(hwc2_device_t* device, int32_t descriptor,
                                        hwc2_callback_data_t callbackData,
                                        hwc2_function_pointer_t pointer) {
        auto& hwc = cast(device);
        if (descriptor != HWC2_CALLBACK_VSYNC) {
            auto original = reinterpret_cast<HWC2_PFN_REGISTER_CALLBACK>(
                    hwc.mOriginalGetFunction(device, HWC2_FUNCTION_REGISTER_CALLBACK));
            return original ? original(device, descriptor, callbackData, pointer)
                            : HWC2_ERROR_UNSUPPORTED;
        }

        std::lock_guard<std::mutex> lock(hwc.mMutex);
        hwc.mVsyncCallback = reinterpret_cast<HWC2_PFN_VSYNC>(pointer);
        hwc.mVsyncCallbackData = callbackData;
        return HWC2_ERROR_NONE;
    }

    static int32_t hookSetVsyncEnabled(hwc2_device_t* device, hwc2_display_t display,
                                       int32_t enabled) {
        auto& hwc = cast(device);
        if (display != getDisplayId()) {
            return HWC2_ERROR_BAD_DISPLAY;
        }
        if (enabled != HWC2_VSYNC_ENABLE && enabled != HWC2_VSYNC_DISABLE) {
            return HWC2_ERROR_BAD_PARAMETER;
        }

        {
            std::lock_guard<std::mutex> lock(hwc.mMutex);
            hwc.mVsyncEnabled = enabled == HWC2_VSYNC_ENABLE;
        }
        hwc.mCondition.notify_all();
        return HWC2_ERROR_NONE;
    }

    static int hookClose(hw_device_t* device) {
        auto& hwc = cast(device);
        hwc.stopVsyncThread();
        static_cast<android::HWC2OnFbAdapter&>(hwc).close();
        return 0;
    }

    static int64_t monotonicNow() {
        timespec timestamp{};
        clock_gettime(CLOCK_MONOTONIC, &timestamp);
        return static_cast<int64_t>(timestamp.tv_sec) * 1000000000LL + timestamp.tv_nsec;
    }

    void stopVsyncThread() {
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mStopping = true;
            mVsyncEnabled = false;
        }
        mCondition.notify_all();
        if (mVsyncThread.joinable()) {
            mVsyncThread.join();
        }
    }

    bool waitUntilEnabled() {
        std::unique_lock<std::mutex> lock(mMutex);
        mCondition.wait(lock, [this] { return mVsyncEnabled || mStopping; });
        return !mStopping;
    }

    bool callbacksEnabled() {
        std::lock_guard<std::mutex> lock(mMutex);
        return mVsyncEnabled && !mStopping;
    }

    void dispatchVsync(int64_t timestamp) {
        HWC2_PFN_VSYNC callback = nullptr;
        hwc2_callback_data_t callbackData = nullptr;
        {
            std::lock_guard<std::mutex> lock(mMutex);
            if (!mVsyncEnabled || mStopping) {
                return;
            }
            callback = mVsyncCallback;
            callbackData = mVsyncCallbackData;
        }
        if (callback) {
            callback(callbackData, getDisplayId(), timestamp);
        }
    }

    bool runKernelVsync(int framebufferFd, int eventFd) {
        unsigned int enable = 1;
        if (ioctl(framebufferFd, kHisiFbVsyncCtrl, &enable) != 0) {
            ALOGW("kernel VSYNC enable failed: %s", strerror(errno));
            return false;
        }

        bool usable = true;
        while (callbacksEnabled()) {
            char event[64] = {};
            if (lseek(eventFd, 0, SEEK_SET) < 0) {
                usable = false;
                break;
            }
            const ssize_t length = read(eventFd, event, sizeof(event) - 1);
            int64_t timestamp = 0;
            if (length <= 0 || std::sscanf(event, "VSYNC=%" SCNd64, &timestamp) != 1) {
                usable = false;
                break;
            }
            dispatchVsync(timestamp);
        }

        enable = 0;
        if (ioctl(framebufferFd, kHisiFbVsyncCtrl, &enable) != 0) {
            ALOGW("kernel VSYNC disable failed: %s", strerror(errno));
        }
        return usable;
    }

    void runTimerVsync() {
        const auto period = std::chrono::nanoseconds(getInfo().vsync_period_ns);
        auto nextVsync = std::chrono::steady_clock::now() + period;

        while (callbacksEnabled()) {
            std::unique_lock<std::mutex> lock(mMutex);
            if (mCondition.wait_until(lock, nextVsync,
                                      [this] { return !mVsyncEnabled || mStopping; })) {
                return;
            }
            lock.unlock();
            dispatchVsync(monotonicNow());
            nextVsync += period;
        }
    }

    void vsyncLoop() {
        prctl(PR_SET_NAME, "mozart-vsync", 0, 0, 0);

        const int framebufferFd = open(kFramebufferPath, O_RDWR | O_CLOEXEC);
        const int eventFd = open(kVsyncEventPath, O_RDONLY | O_CLOEXEC);
        bool useKernelVsync = framebufferFd >= 0 && eventFd >= 0;
        if (!useKernelVsync) {
            ALOGW("kernel VSYNC unavailable, using timer fallback: %s", strerror(errno));
        }

        while (waitUntilEnabled()) {
            if (useKernelVsync) {
                useKernelVsync = runKernelVsync(framebufferFd, eventFd);
                if (!useKernelVsync) {
                    ALOGW("kernel VSYNC failed, switching permanently to timer fallback");
                }
            } else {
                runTimerVsync();
            }
        }

        if (eventFd >= 0) {
            ::close(eventFd);
        }
        if (framebufferFd >= 0) {
            ::close(framebufferFd);
        }
    }

    hwc2_function_pointer_t (*mOriginalGetFunction)(hwc2_device_t*, int32_t);
    std::thread mVsyncThread;
    std::mutex mMutex;
    std::condition_variable mCondition;
    HWC2_PFN_VSYNC mVsyncCallback{nullptr};
    hwc2_callback_data_t mVsyncCallbackData{nullptr};
    bool mVsyncEnabled{false};
    bool mStopping{false};
};

int openDevice(const hw_module_t* module, const char* name, hw_device_t** outDevice) {
    if (!name || std::strcmp(name, HWC_HARDWARE_COMPOSER) != 0 || !outDevice) {
        return -EINVAL;
    }

    const hw_module_t* grallocModule = nullptr;
    int error = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &grallocModule);
    if (error != 0) {
        ALOGE("failed to load gralloc: %s", strerror(-error));
        return error;
    }

    framebuffer_device_t* framebuffer = nullptr;
    error = framebuffer_open(grallocModule, &framebuffer);
    if (error != 0) {
        ALOGE("failed to open framebuffer: %s", strerror(-error));
        return error;
    }

    auto* hwc = new (std::nothrow) MozartHwc2(framebuffer);
    if (!hwc) {
        framebuffer_close(framebuffer);
        return -ENOMEM;
    }
    hwc->initializeCommon(module);
    *outDevice = &hwc->common;
    ALOGI("loaded CLIENT composer with hi3635 kernel VSYNC support");
    return 0;
}

hw_module_methods_t moduleMethods = {
        .open = openDevice,
};

}  // namespace

extern "C" hwc_module_t HAL_MODULE_INFO_SYM = {
        .common = {
                .tag = HARDWARE_MODULE_TAG,
                .version_major = 3,
                .version_minor = 0,
                .id = HWC_HARDWARE_MODULE_ID,
                .name = "Mozart HWC2 module",
                .author = "yukino1111",
                .methods = &moduleMethods,
        },
};
