/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cerrno>
#include <cstdint>

#include <hardware/gralloc.h>
#include <hardware/hardware.h>

namespace {

const gralloc_module_t* get_gralloc_module() {
    const hw_module_t* module = nullptr;
    if (hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &module) != 0) {
        return nullptr;
    }
    return reinterpret_cast<const gralloc_module_t*>(module);
}

}  // namespace

// Android 6 exposed GraphicBufferMapper's gralloc0 register/unregister
// methods. Modern libui removed them, but mozart's B217 gralloc HAL still
// implements the original callbacks used by Huawei's camera stack.
extern "C" int32_t graphic_buffer_mapper_register_legacy(
        void*, const native_handle_t* handle)
        asm("_ZN7android19GraphicBufferMapper14registerBufferEPK13native_handle");

extern "C" int32_t graphic_buffer_mapper_register_legacy(
        void*, const native_handle_t* handle) {
    const gralloc_module_t* module = get_gralloc_module();
    if (module == nullptr || module->registerBuffer == nullptr) {
        return -ENODEV;
    }
    return module->registerBuffer(module, handle);
}

extern "C" int32_t graphic_buffer_mapper_unregister_legacy(
        void*, const native_handle_t* handle)
        asm("_ZN7android19GraphicBufferMapper16unregisterBufferEPK13native_handle");

extern "C" int32_t graphic_buffer_mapper_unregister_legacy(
        void*, const native_handle_t* handle) {
    const gralloc_module_t* module = get_gralloc_module();
    if (module == nullptr || module->unregisterBuffer == nullptr) {
        return -ENODEV;
    }
    return module->unregisterBuffer(module, handle);
}

struct LegacyRect {
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
};

extern "C" int32_t graphic_buffer_mapper_lock_legacy(
        void*, const native_handle_t* handle, uint32_t usage,
        const LegacyRect& bounds, void** vaddr)
        asm("_ZN7android19GraphicBufferMapper4lockEPK13native_handlejRKNS_4RectEPPv");

extern "C" int32_t graphic_buffer_mapper_lock_legacy(
        void*, const native_handle_t* handle, uint32_t usage,
        const LegacyRect& bounds, void** vaddr) {
    const gralloc_module_t* module = get_gralloc_module();
    if (module == nullptr || module->lock == nullptr) {
        return -ENODEV;
    }
    return module->lock(module, handle, usage, bounds.left, bounds.top,
            bounds.right - bounds.left, bounds.bottom - bounds.top, vaddr);
}

extern "C" int32_t graphic_buffer_mapper_unlock_legacy(
        void*, const native_handle_t* handle)
        asm("_ZN7android19GraphicBufferMapper6unlockEPK13native_handle");

extern "C" int32_t graphic_buffer_mapper_unlock_legacy(
        void*, const native_handle_t* handle) {
    const gralloc_module_t* module = get_gralloc_module();
    if (module == nullptr || module->unlock == nullptr) {
        return -ENODEV;
    }
    return module->unlock(module, handle);
}
