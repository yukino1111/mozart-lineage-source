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

MOZART_LIBINIT_BUILD_EPOCH := $(strip $(MOZART_BUILD_DATETIME))
ifeq ($(MOZART_LIBINIT_BUILD_EPOCH),)
MOZART_LIBINIT_BUILD_EPOCH := $(strip $(BUILD_DATETIME))
endif
ifeq ($(MOZART_LIBINIT_BUILD_EPOCH),)
MOZART_LIBINIT_BUILD_EPOCH := $(shell if [ -f "$(OUT_DIR)/build_date.txt" ]; then cat "$(OUT_DIR)/build_date.txt"; fi)
endif
MOZART_LIBINIT_BUILD_NUMBER := $(shell if [ -n "$(MOZART_LIBINIT_BUILD_EPOCH)" ]; then date -u -d @"$(MOZART_LIBINIT_BUILD_EPOCH)" +%Y%m%d; else date -u +%Y%m%d; fi)

LOCAL_CFLAGS += \
    -DMOZART_BUILD_NUMBER=\"$(MOZART_LIBINIT_BUILD_NUMBER)\"

LOCAL_C_INCLUDES := \
    system/core/base/include \
    system/core/init

LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_TAGS := optional

include $(BUILD_STATIC_LIBRARY)
