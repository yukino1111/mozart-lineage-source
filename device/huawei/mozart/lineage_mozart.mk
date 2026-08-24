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

# Use the matching Cromite browser and System WebView pair. Their prebuilt
# modules override Jelly and the stale AOSP WebView packages.
PRODUCT_PACKAGES += \
    Cromite \
    CromiteSystemWebView

# Device identifier.
PRODUCT_DEVICE := mozart
PRODUCT_NAME := lineage_mozart
PRODUCT_BRAND := HUAWEI
PRODUCT_MANUFACTURER := HUAWEI
PRODUCT_MODEL := HUAWEI M2-801W

PRODUCT_CHARACTERISTICS := tablet
PRODUCT_GMS_CLIENTID_BASE := android-huawei

MOZART_BUILD_ID := HUAWEIM2-801W
MOZART_BUILD_NUMBER := C233B217
MOZART_BUILD_DISPLAY_ID := M2-801WV100R001C233B217
MOZART_BUILD_DESC := M2-user 6.0 $(MOZART_BUILD_ID) $(MOZART_BUILD_NUMBER) release-keys

PRODUCT_BUILD_PROP_OVERRIDES += \
    BUILD_DISPLAY_ID="$(MOZART_BUILD_DISPLAY_ID)" \
    BUILD_ID=$(MOZART_BUILD_ID) \
    BUILD_NUMBER=$(MOZART_BUILD_NUMBER) \
    PRIVATE_BUILD_DESC="$(MOZART_BUILD_DESC)" \
    TARGET_DEVICE=HWMozart

BUILD_FINGERPRINT := HUAWEI/M2/HWMozart:6.0/$(MOZART_BUILD_ID)/$(MOZART_BUILD_NUMBER):user/release-keys
