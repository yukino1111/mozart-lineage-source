LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := Cromite
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_TAGS := optional
LOCAL_PRODUCT_MODULE := true
LOCAL_SRC_FILES := cromite/Cromite.apk
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_MODULE_TARGET_ARCH := arm64
LOCAL_PREBUILT_JNI_LIBS_arm64 := \
    cromite/lib/arm64-v8a/libchrome.so \
    cromite/lib/arm64-v8a/libchrome_crashpad_handler.so
LOCAL_OVERRIDES_PACKAGES := Jelly
# Android 9's generic prebuilt rule rewrites compressed JNI entries even for
# PRESIGNED apps, which invalidates Cromite's APK Signature Scheme v2 block.
LOCAL_REPLACE_PREBUILT_APK_INSTALLED := $(LOCAL_PATH)/cromite/Cromite.apk
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := CromiteSystemWebView
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_TAGS := optional
LOCAL_PRODUCT_MODULE := true
LOCAL_SRC_FILES := cromite/CromiteSystemWebView.apk
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_MULTILIB := both
LOCAL_MODULE_TARGET_ARCH := arm arm64
LOCAL_PREBUILT_JNI_LIBS_arm := @lib/armeabi-v7a/libwebviewchromium.so
LOCAL_PREBUILT_JNI_LIBS_arm64 := @lib/arm64-v8a/libwebviewchromium.so
LOCAL_REQUIRED_MODULES := \
    libwebviewchromium_loader \
    libwebviewchromium_plat_support
LOCAL_OVERRIDES_PACKAGES := webview
include $(BUILD_PREBUILT)
