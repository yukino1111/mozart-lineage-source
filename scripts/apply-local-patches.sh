#!/usr/bin/env bash
set -euo pipefail

PATCH_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ANDROID_TOP="${1:-/android/lineage18.1-mozart}"

if [[ ! -d "$ANDROID_TOP/.repo" ]]; then
    echo "error: $ANDROID_TOP does not look like an Android repo checkout" >&2
    exit 2
fi

bash "$PATCH_ROOT/scripts/install-device-tree.sh" "$ANDROID_TOP"

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

apply_once \
    "build/make" \
    "$PATCH_ROOT/patches/build/make/lineage18-mozart-build-ota.patch"

apply_once \
    "frameworks/av" \
    "$PATCH_ROOT/patches/frameworks/av/legacy-audio-version-table.patch"

apply_once \
    "frameworks/base" \
    "$PATCH_ROOT/patches/frameworks/base/legacy-mali-egl-main-thread.patch"

apply_once \
    "frameworks/base" \
    "$PATCH_ROOT/patches/frameworks/base/legacy-install-media-gnss-stability.patch"

apply_once \
    "frameworks/native" \
    "$PATCH_ROOT/patches/frameworks/native/0001-frameworks-native-backport-Region-FatVector-revert.patch"

apply_once \
    "frameworks/native" \
    "$PATCH_ROOT/patches/frameworks/native/0002-surfaceflinger-control-the-legacy-framebuffer-power-.patch"

apply_once \
    "hardware/broadcom/wlan" \
    "$PATCH_ROOT/patches/hardware/broadcom/wlan/legacy-bcmdhd-wifi-hal.patch"

apply_once \
    "hardware/interfaces" \
    "$PATCH_ROOT/patches/hardware/interfaces/0001-composer-restore-the-2.1-passthrough-implementation.patch"

apply_once \
    "hardware/interfaces" \
    "$PATCH_ROOT/patches/hardware/interfaces/0002-wifi-validate-legacy-interface-handles.patch"

apply_once \
    "hardware/interfaces" \
    "$PATCH_ROOT/patches/hardware/interfaces/0003-light-synchronize-the-mozart-framebuffer-state.patch"

apply_once \
    "hardware/interfaces" \
    "$PATCH_ROOT/patches/hardware/interfaces/legacy-private-sensor-types.patch"

apply_once \
    "hardware/lineage/interfaces" \
    "$PATCH_ROOT/patches/hardware/lineage/interfaces/legacy-gnss-nmea-copy.patch"

apply_once \
    "hardware/libhardware" \
    "$PATCH_ROOT/patches/hardware/libhardware/legacy-mozart-gralloc-path.patch"

apply_once \
    "kernel/huawei/mozart" \
    "$PATCH_ROOT/patches/kernel/huawei/mozart/0001-mozart-disable-kernel-debug-information.patch"

apply_once \
    "kernel/huawei/mozart" \
    "$PATCH_ROOT/patches/kernel/huawei/mozart/0002-arm64-ptrace-add-NT_ARM_SYSTEM_CALL-regset.patch"

apply_once \
    "kernel/huawei/mozart" \
    "$PATCH_ROOT/patches/kernel/huawei/mozart/0003-ion-require-the-CMA-heap-device.patch"

apply_once \
    "kernel/huawei/mozart" \
    "$PATCH_ROOT/patches/kernel/huawei/mozart/0004-proc-actually-make-proc_fd_permission-thread-friendl.patch"

apply_once \
    "kernel/huawei/mozart" \
    "$PATCH_ROOT/patches/kernel/huawei/mozart/legacy-fde-aes-compat.patch"

apply_once \
    "packages/apps/Bluetooth" \
    "$PATCH_ROOT/patches/packages/apps/Bluetooth/legacy-huawei-disable-scs.patch"

apply_once \
    "packages/modules/NetworkStack" \
    "$PATCH_ROOT/patches/packages/modules/NetworkStack/legacy-kernel-tcp-info.patch"

apply_once \
    "system/bt" \
    "$PATCH_ROOT/patches/system/bt/legacy-huawei-disable-scs.patch"

apply_once \
    "system/core" \
    "$PATCH_ROOT/patches/system/core/legacy-first-stage-mount.patch"

apply_once \
    "system/core" \
    "$PATCH_ROOT/patches/system/core/legacy-dm-uevent-compat.patch"

apply_once \
    "system/sepolicy" \
    "$PATCH_ROOT/patches/system/sepolicy/legacy-fde-data-mirror-policy.patch"

apply_once \
    "system/vold" \
    "$PATCH_ROOT/patches/system/vold/legacy-fde-data-mirror-unmount.patch"

apply_once \
    "system/tools/mkbootimg" \
    "$PATCH_ROOT/patches/system/tools/mkbootimg/legacy-boot-addresses.patch"

apply_once \
    "vendor/huawei/mozart" \
    "$PATCH_ROOT/patches/vendor/huawei/mozart/lineage18-vendor-layout.patch"

apply_once \
    "vendor/lineage" \
    "$PATCH_ROOT/patches/vendor/lineage/disable-recovery-backuptool.patch"

"$PATCH_ROOT/scripts/prepare-proprietary-blobs.sh" "$ANDROID_TOP"

echo "LineageOS 18.1 mozart patches are applied"
