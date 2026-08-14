/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cstdint>
#include <new>

#include <ui/GraphicBuffer.h>

// Android 6 exposed GraphicBuffer constructors without a layer-count or
// requestor-name argument. Huawei's camera stack still calls those entry
// points, while current libui provides layout-compatible replacements.
extern "C" void graphic_buffer_ctor_size_legacy(
        void* buffer, uint32_t width, uint32_t height, int32_t format,
        uint32_t usage) asm("_ZN7android13GraphicBufferC1Ejjij");

extern "C" void graphic_buffer_ctor_size_legacy(
        void* buffer, uint32_t width, uint32_t height, int32_t format,
        uint32_t usage) {
    new (buffer) android::GraphicBuffer(
            width, height, static_cast<android::PixelFormat>(format), usage,
            "mozart-camera");
}

extern "C" void graphic_buffer_ctor_handle_legacy(
        void* buffer, uint32_t width, uint32_t height, int32_t format,
        uint32_t usage, uint32_t stride, native_handle_t* handle,
        bool keep_ownership)
        asm("_ZN7android13GraphicBufferC1EjjijjP13native_handleb");

extern "C" void graphic_buffer_ctor_handle_legacy(
        void* buffer, uint32_t width, uint32_t height, int32_t format,
        uint32_t usage, uint32_t stride, native_handle_t* handle,
        bool keep_ownership) {
    new (buffer) android::GraphicBuffer(
            width, height, static_cast<android::PixelFormat>(format), 1,
            usage, stride, handle, keep_ownership);
}

extern "C" int32_t graphic_buffer_lock_legacy(
        void* buffer, uint32_t usage, void** vaddr)
        asm("_ZN7android13GraphicBuffer4lockEjPPv");

extern "C" int32_t graphic_buffer_lock_legacy(
        void* buffer, uint32_t usage, void** vaddr) {
    return static_cast<android::GraphicBuffer*>(buffer)->lock(
            usage, vaddr, nullptr, nullptr);
}
