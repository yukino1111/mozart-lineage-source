#
# Copyright (C) 2025 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/product_launched_with_m.mk)

# Avoid the legacy libhidl one-second getService delay; this product ships a complete VINTF manifest.
PRODUCT_ENFORCE_VINTF_MANIFEST_OVERRIDE := true

# Inherit device configurations.
$(call inherit-product, $(LOCAL_PATH)/device.mk)

# Keep stock-style detection checks from finding Lineage addon/recovery helpers.
PRODUCT_DISABLE_LINEAGE_BACKUPTOOL := true

# Inherit some common LineageOS stuff.
$(call inherit-product, vendor/lineage/config/common_full_tablet_wifionly.mk)

# Device identifier.
PRODUCT_DEVICE := mozart
PRODUCT_NAME := lineage_mozart
PRODUCT_BRAND := huawei
PRODUCT_MANUFACTURER := Huawei
PRODUCT_MODEL := MediaPad M2 8.0

PRODUCT_CHARACTERISTICS := tablet
PRODUCT_GMS_CLIENTID_BASE := android-huawei

MOZART_BUILD_ID ?= PQ3A.190801.002
MOZART_FINGERPRINT_ID ?= PQ3A.190801
MOZART_BUILD_EPOCH ?= $(MOZART_BUILD_DATETIME)
MOZART_BUILD_NUMBER ?= $(shell if [ -n "$(MOZART_BUILD_EPOCH)" ]; then date -u -d @"$(MOZART_BUILD_EPOCH)" +%Y%m%d; else date -u +%Y%m%d; fi)
MOZART_BUILD_DESC := lineage_mozart-user 9 $(MOZART_BUILD_ID) $(MOZART_BUILD_NUMBER) release-keys

PRODUCT_BUILD_PROP_OVERRIDES += \
    BUILD_DISPLAY_ID="$(MOZART_BUILD_DESC)" \
    BUILD_ID=$(MOZART_BUILD_ID) \
    BUILD_NUMBER=$(MOZART_BUILD_NUMBER) \
    PRIVATE_BUILD_DESC="$(MOZART_BUILD_DESC)" \
    TARGET_DEVICE=hi3635

BUILD_FINGERPRINT := huawei/mozart/hi3635:9/$(MOZART_FINGERPRINT_ID)/$(MOZART_BUILD_NUMBER):user/release-keys
