#!/usr/bin/env bash
set -euo pipefail

PATCH_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ANDROID_TOP="${1:-/android/lineage16-mozart}"

if [[ ! -d "$ANDROID_TOP/.repo" ]]; then
    echo "error: $ANDROID_TOP does not look like an Android repo checkout" >&2
    exit 2
fi

apply_once() {
    local repo="$1"
    local patch="$2"

    if [[ ! -d "$ANDROID_TOP/$repo" ]]; then
        echo "error: missing Android repository: $repo" >&2
        exit 3
    fi

    if git -C "$ANDROID_TOP/$repo" apply -R --check "$patch" >/dev/null 2>&1; then
        echo "skip: already applied: $repo/${patch#$PATCH_ROOT/}"
        return
    fi

    echo "apply: $repo/${patch#$PATCH_ROOT/}"
    git -C "$ANDROID_TOP/$repo" apply --check "$patch"
    git -C "$ANDROID_TOP/$repo" apply "$patch"
}

apply_kirin930_patch() {
    local repo="$1"
    local patch_rel="$2"

    apply_once "$repo" "$PATCH_ROOT/patches/upstream/kirin930/$patch_rel"
}

bash "$PATCH_ROOT/scripts/install-device-tree.sh" "$ANDROID_TOP"

# Apply imported work as its original mail-style patch, then apply the local
# compatibility work by topic. This keeps third-party authorship visible and
# makes every patch below the single applied copy of that change.
apply_once \
    "build/make" \
    "$PATCH_ROOT/patches/build/make/mozart-release-ota-build-tools.patch"

apply_once \
    "frameworks/av" \
    "$PATCH_ROOT/patches/frameworks/av/img-msvdx-decoder-framerate-compat.patch"
apply_once \
    "frameworks/av" \
    "$PATCH_ROOT/patches/frameworks/av/legacy-mozart-camera-recording.patch"

apply_kirin930_patch \
    "frameworks/base" \
    "frameworks/base/Hardware-bitmaps-support-workaround.patch"
apply_once \
    "frameworks/base" \
    "$PATCH_ROOT/patches/frameworks/base/packageinstaller-webview-compat.patch"
apply_once \
    "frameworks/base" \
    "$PATCH_ROOT/patches/frameworks/base/gnss-geofence-native-timeout.patch"
apply_once \
    "frameworks/base" \
    "$PATCH_ROOT/patches/frameworks/base/legacy-mozart-runtime-compat.patch"

apply_once \
    "frameworks/native" \
    "$PATCH_ROOT/patches/frameworks/native/surfaceflinger-powerdown-lcd-on-off.patch"
apply_once \
    "frameworks/native" \
    "$PATCH_ROOT/patches/frameworks/native/legacy-huawei-camera-abi.patch"

apply_kirin930_patch \
    "hardware/broadcom/wlan" \
    "hardware/broadcom/wlan/WifiHAL-Do-not-error-check-on-initialization.patch"

apply_kirin930_patch \
    "hardware/interfaces" \
    "hardware/interfaces/Audio-skip-setMasterVolume-if-not-implement.patch"
apply_once \
    "hardware/interfaces" \
    "$PATCH_ROOT/patches/hardware/interfaces/legacy-private-sensor-type-compat.patch"
apply_once \
    "hardware/interfaces" \
    "$PATCH_ROOT/patches/hardware/interfaces/hwc2onfbadapter-hisi-dss-overlay-fallback.patch"
apply_once \
    "hardware/interfaces" \
    "$PATCH_ROOT/patches/hardware/interfaces/legacy-huawei-camera-hal.patch"
apply_once \
    "hardware/interfaces" \
    "$PATCH_ROOT/patches/hardware/interfaces/health-board-battery-property-hook.patch"

apply_once \
    "kernel/huawei/mozart" \
    "$PATCH_ROOT/patches/kernel/huawei/mozart/disable-debug-info.patch"
apply_once \
    "kernel/huawei/mozart" \
    "$PATCH_ROOT/patches/kernel/huawei/mozart/arm64-nt-arm-system-call-regset.patch"
apply_once \
    "kernel/huawei/mozart" \
    "$PATCH_ROOT/patches/kernel/huawei/mozart/legacy-mozart-b217-runtime.patch"
apply_once \
    "kernel/huawei/mozart" \
    "$PATCH_ROOT/patches/kernel/huawei/mozart/cpufreq-keep-scaling-min-freq-writable.patch"

apply_once \
    "packages/apps/Camera2" \
    "$PATCH_ROOT/patches/packages/apps/Camera2/legacy-mozart-camera-behavior.patch"

apply_kirin930_patch \
    "system/bt" \
    "system/bt/Hci-dont-crash-if-some-checks-fail.patch"

apply_kirin930_patch \
    "system/core" \
    "system/core/Support-mkbootimg-0xffb88000-as-tags-offset.patch"
apply_once \
    "system/core" \
    "$PATCH_ROOT/patches/system/core/init-user-permissive-selinux.patch"

apply_once \
    "vendor/huawei/mozart" \
    "$PATCH_ROOT/patches/vendor/huawei/mozart/restore-emui31-gpu-omx-vendor-paths.patch"
apply_once \
    "vendor/huawei/mozart" \
    "$PATCH_ROOT/patches/vendor/huawei/mozart/preserve-stock-mac-normalization-helper.patch"
apply_once \
    "vendor/huawei/mozart" \
    "$PATCH_ROOT/patches/vendor/huawei/mozart/legacy-mozart-camera-vendor.patch"

apply_once \
    "vendor/lineage" \
    "$PATCH_ROOT/patches/vendor/lineage/disable-backuptool-and-hudson-fetch.patch"

BLOB_CACHE="$PATCH_ROOT/proprietary-blobs/huawei/mozart"
if [[ -d "$BLOB_CACHE" ]] && [[ -n "$(find "$BLOB_CACHE" -type f -print -quit)" ]]; then
    "$PATCH_ROOT/scripts/extract-proprietary-blobs.sh" "$ANDROID_TOP"
else
    echo "note: proprietary blob cache is absent"
    echo "      run scripts/extract-proprietary-blobs.sh with a stock /system extraction before building"
fi

echo "local mozart patches are applied"
