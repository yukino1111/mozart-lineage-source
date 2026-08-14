/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <algorithm>
#include <cstdint>
#include <cstring>

#include <system/camera_metadata.h>
#include <utils/Errors.h>

/* Android 6 CameraMetadata is a five-byte object; Android 9+ is not ABI-compatible. */
struct LegacyCameraMetadata {
    camera_metadata_t* buffer;
    bool locked;
};

static_assert(offsetof(LegacyCameraMetadata, buffer) == 0, "legacy mBuffer offset");
static_assert(offsetof(LegacyCameraMetadata, locked) == 4, "legacy mLocked offset");

static int32_t resizeIfNeeded(LegacyCameraMetadata* metadata,
                              size_t extraEntries, size_t extraData) {
    if (metadata->buffer == nullptr) {
        metadata->buffer = allocate_camera_metadata(extraEntries * 2, extraData * 2);
        return metadata->buffer != nullptr ? android::OK : android::NO_MEMORY;
    }
    const size_t entryCount = get_camera_metadata_entry_count(metadata->buffer);
    const size_t entryCapacity = get_camera_metadata_entry_capacity(metadata->buffer);
    const size_t dataCount = get_camera_metadata_data_count(metadata->buffer);
    const size_t dataCapacity = get_camera_metadata_data_capacity(metadata->buffer);
    const size_t newEntryCapacity = entryCount + extraEntries > entryCapacity
            ? (entryCount + extraEntries) * 2 : entryCapacity;
    const size_t newDataCapacity = dataCount + extraData > dataCapacity
            ? (dataCount + extraData) * 2 : dataCapacity;
    if (newEntryCapacity == entryCapacity && newDataCapacity == dataCapacity) {
        return android::OK;
    }
    camera_metadata_t* replacement =
            allocate_camera_metadata(newEntryCapacity, newDataCapacity);
    if (replacement == nullptr) return android::NO_MEMORY;
    const int32_t result = append_camera_metadata(replacement, metadata->buffer);
    if (result != android::OK) {
        free_camera_metadata(replacement);
        return result;
    }
    free_camera_metadata(metadata->buffer);
    metadata->buffer = replacement;
    return android::OK;
}

static int32_t updateImpl(LegacyCameraMetadata* metadata, uint32_t tag,
                          const void* data, size_t dataCount) {
    if (metadata->locked) return android::INVALID_OPERATION;
    const int type = get_local_camera_metadata_tag_type(tag, metadata->buffer);
    if (type < 0) return android::BAD_VALUE;
    if (metadata->buffer != nullptr) {
        const uintptr_t bufferAddress = reinterpret_cast<uintptr_t>(metadata->buffer);
        const uintptr_t dataAddress = reinterpret_cast<uintptr_t>(data);
        const size_t bufferSize = get_camera_metadata_size(metadata->buffer);
        if (dataAddress > bufferAddress && dataAddress < bufferAddress + bufferSize) {
            return android::INVALID_OPERATION;
        }
    }
    int32_t result = resizeIfNeeded(metadata, 1,
            calculate_camera_metadata_entry_data_size(type, dataCount));
    if (result != android::OK) return result;
    camera_metadata_entry_t entry = {};
    result = find_camera_metadata_entry(metadata->buffer, tag, &entry);
    if (result == android::NAME_NOT_FOUND) {
        return add_camera_metadata_entry(metadata->buffer, tag, data, dataCount);
    }
    if (result == android::OK) {
        return update_camera_metadata_entry(metadata->buffer, entry.index,
                                             data, dataCount, nullptr);
    }
    return result;
}

extern "C" void camera_metadata_ctor_legacy(LegacyCameraMetadata*)
        asm("_ZN7android14CameraMetadataC1Ev");
extern "C" void camera_metadata_ctor_legacy(LegacyCameraMetadata* metadata) {
    metadata->buffer = nullptr; metadata->locked = false;
}
extern "C" void camera_metadata_clear_legacy(LegacyCameraMetadata*)
        asm("_ZN7android14CameraMetadata5clearEv");
extern "C" void camera_metadata_clear_legacy(LegacyCameraMetadata* metadata) {
    if (!metadata->locked && metadata->buffer != nullptr) {
        free_camera_metadata(metadata->buffer); metadata->buffer = nullptr;
    }
}
extern "C" void camera_metadata_dtor_legacy(LegacyCameraMetadata*)
        asm("_ZN7android14CameraMetadataD1Ev");
extern "C" void camera_metadata_dtor_legacy(LegacyCameraMetadata* metadata) {
    metadata->locked = false; camera_metadata_clear_legacy(metadata);
}
extern "C" camera_metadata_t* camera_metadata_release_legacy(LegacyCameraMetadata*)
        asm("_ZN7android14CameraMetadata7releaseEv");
extern "C" camera_metadata_t* camera_metadata_release_legacy(LegacyCameraMetadata* metadata) {
    if (metadata->locked) return nullptr;
    camera_metadata_t* buffer = metadata->buffer; metadata->buffer = nullptr; return buffer;
}
extern "C" void camera_metadata_acquire_legacy(LegacyCameraMetadata*, camera_metadata_t*)
        asm("_ZN7android14CameraMetadata7acquireEP15camera_metadata");
extern "C" void camera_metadata_acquire_legacy(LegacyCameraMetadata* metadata,
                                                 camera_metadata_t* buffer) {
    if (metadata->locked) return;
    camera_metadata_clear_legacy(metadata); metadata->buffer = buffer;
}
extern "C" const camera_metadata_t* camera_metadata_get_and_lock_legacy(LegacyCameraMetadata*)
        asm("_ZNK7android14CameraMetadata10getAndLockEv");
extern "C" const camera_metadata_t* camera_metadata_get_and_lock_legacy(
        LegacyCameraMetadata* metadata) {
    metadata->locked = true; return metadata->buffer;
}
extern "C" int32_t camera_metadata_unlock_legacy(LegacyCameraMetadata*,
                                                   const camera_metadata_t*)
        asm("_ZN7android14CameraMetadata6unlockEPK15camera_metadata");
extern "C" int32_t camera_metadata_unlock_legacy(LegacyCameraMetadata* metadata,
                                                   const camera_metadata_t* buffer) {
    if (!metadata->locked) return android::INVALID_OPERATION;
    if (buffer != metadata->buffer) return android::BAD_VALUE;
    metadata->locked = false; return android::OK;
}
extern "C" camera_metadata_entry_t camera_metadata_find_legacy(LegacyCameraMetadata*, uint32_t)
        asm("_ZN7android14CameraMetadata4findEj");
extern "C" camera_metadata_entry_t camera_metadata_find_legacy(
        LegacyCameraMetadata* metadata, uint32_t tag) {
    camera_metadata_entry_t entry = {};
    if (!metadata->locked && metadata->buffer != nullptr) {
        find_camera_metadata_entry(metadata->buffer, tag, &entry);
    }
    return entry;
}

#define DEFINE_LEGACY_UPDATE(name, mangled, type)                              \
    extern "C" int32_t name(LegacyCameraMetadata*, uint32_t, const type*,     \
                            uint32_t) asm(mangled);                            \
    extern "C" int32_t name(LegacyCameraMetadata* metadata, uint32_t tag,     \
                            const type* data, uint32_t count) {                \
        return updateImpl(metadata, tag, data, count);                         \
    }
DEFINE_LEGACY_UPDATE(camera_metadata_update_u8_legacy,
        "_ZN7android14CameraMetadata6updateEjPKhj", uint8_t)
DEFINE_LEGACY_UPDATE(camera_metadata_update_i32_legacy,
        "_ZN7android14CameraMetadata6updateEjPKij", int32_t)
DEFINE_LEGACY_UPDATE(camera_metadata_update_float_legacy,
        "_ZN7android14CameraMetadata6updateEjPKfj", float)
DEFINE_LEGACY_UPDATE(camera_metadata_update_i64_legacy,
        "_ZN7android14CameraMetadata6updateEjPKxj", int64_t)
