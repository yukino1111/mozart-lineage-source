#
# Copyright (C) 2025 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := libshim_log.cpp
LOCAL_MODULE := libshim_log
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
    camera_legacy.c \
    ../../../../external/compiler-rt/lib/builtins/fixdfdi.c \
    ../../../../external/compiler-rt/lib/builtins/fixunsdfdi.c
LOCAL_MODULE := libshim_camera_legacy
LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := 32
LOCAL_C_INCLUDES := external/compiler-rt/lib/builtins
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
    gui/ISensorServer.cpp \
    gui/SensorManager.cpp
LOCAL_SRC_FILES_32 := \
    gui/CameraMetadata.cpp \
    gui/Fence.cpp \
    gui/GraphicBuffer.cpp \
    gui/GraphicBufferMapper.cpp \
    utils/ProcessCallStack.cpp
LOCAL_SHARED_LIBRARIES := libbase libbinder libcamera_client libcamera_metadata libsensor libcutils libhardware libhidlbase libsync libui libnativeloader libgui libutils liblog
LOCAL_MODULE := libshim_gui
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
# glgps4752 is 64-bit while the legacy camera stack consumes the 32-bit shim.
# CameraMetadata has a 32-bit-only object layout, so only the sensor bridge is
# built for arm64.
LOCAL_MULTILIB := both
include $(BUILD_SHARED_LIBRARY)
