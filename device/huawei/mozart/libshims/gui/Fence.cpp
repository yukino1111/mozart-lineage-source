/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cstdint>
#include <unistd.h>

// Android 6 exported android::Fence::~Fence(), while newer libui keeps the
// equivalent unique_fd cleanup inline.  Huawei's camera HAL still references
// the old symbol.  Both layouts place the owned fd immediately after the
// LightRefBase refcount, so provide the missing cleanup entry point.
extern "C" void mozart_legacy_fence_destructor(void* fence)
        asm("_ZN7android5FenceD1Ev");

extern "C" void mozart_legacy_fence_destructor(void* fence) {
    auto* fd = reinterpret_cast<int*>(
            static_cast<char*>(fence) + sizeof(int32_t));
    if (*fd >= 0) {
        close(*fd);
        *fd = -1;
    }
}
