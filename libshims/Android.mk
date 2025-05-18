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
