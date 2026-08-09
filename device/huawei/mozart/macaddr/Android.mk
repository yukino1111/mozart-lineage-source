#
# Copyright (C) 2026 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := mac_addr_normalization
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true

LOCAL_SRC_FILES := mac_addr_normalization.cpp

LOCAL_SHARED_LIBRARIES := \
    liblog

LOCAL_CFLAGS := \
    -Wall \
    -Werror \
    -Wextra

include $(BUILD_EXECUTABLE)
