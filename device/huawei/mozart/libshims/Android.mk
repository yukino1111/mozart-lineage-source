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
    gui/ISensorServer.cpp \
    gui/SensorManager.cpp
LOCAL_SHARED_LIBRARIES := libbase libbinder libsensor libcutils libhardware libhidlbase libsync libui libnativeloader libgui libutils liblog
LOCAL_MODULE := libshim_gui
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
include $(BUILD_SHARED_LIBRARY)
