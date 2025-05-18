#
# Copyright (C) 2025 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libinit_huawei_hi3635
LOCAL_SRC_FILES := init_huawei_hi3635.cpp

LOCAL_SHARED_LIBRARIES := libbase

LOCAL_C_INCLUDES := \
    system/core/base/include \
    system/core/init

LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_TAGS := optional

include $(BUILD_STATIC_LIBRARY)
