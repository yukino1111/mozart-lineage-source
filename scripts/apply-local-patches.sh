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

# Apply the complete public-project state used by the verified B217 user build.
# The smaller historical patches remain under patches/ for attribution, while
# this generated final stack is the single source of truth for reproduction.
final_patches=(
    "build/make|build-make.patch"
    "frameworks/av|frameworks-av.patch"
    "frameworks/base|frameworks-base.patch"
    "frameworks/native|frameworks-native.patch"
    "hardware/broadcom/wlan|hardware-broadcom-wlan.patch"
    "hardware/interfaces|hardware-interfaces.patch"
    "kernel/huawei/mozart|kernel-huawei-mozart.patch"
    "lineage-sdk|lineage-sdk.patch"
    "packages/apps/Camera2|packages-apps-Camera2.patch"
    "system/bt|system-bt.patch"
    "system/core|system-core.patch"
    "vendor/huawei/mozart|vendor-huawei-mozart.patch"
    "vendor/lineage|vendor-lineage.patch"
)

for entry in "${final_patches[@]}"; do
    repo="${entry%%|*}"
    patch="${entry#*|}"
    apply_once "$repo" "$PATCH_ROOT/patches/final/$patch"
done

BLOB_CACHE="$PATCH_ROOT/proprietary-blobs/huawei/mozart"
if [[ -d "$BLOB_CACHE" ]] && [[ -n "$(find "$BLOB_CACHE" -type f -print -quit)" ]]; then
    "$PATCH_ROOT/scripts/extract-proprietary-blobs.sh" "$ANDROID_TOP"
else
    echo "note: proprietary blob cache is absent"
    echo "      run scripts/extract-proprietary-blobs.sh with a stock /system extraction before building"
fi

echo "local mozart patches are applied"
