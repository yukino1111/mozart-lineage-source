#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ANDROID_TOP="${1:-/android/lineage16-mozart}"
SOURCE_TREE="$REPO_ROOT/device/huawei/mozart"
TARGET_TREE="$ANDROID_TOP/device/huawei/mozart"

if [[ ! -d "$ANDROID_TOP/.repo" ]]; then
    echo "error: $ANDROID_TOP does not look like an Android repo checkout" >&2
    exit 2
fi

if [[ ! -d "$SOURCE_TREE" ]]; then
    echo "error: maintained device source is missing: $SOURCE_TREE" >&2
    exit 3
fi

if [[ ! -d "$TARGET_TREE/.git" ]]; then
    echo "error: pinned kirin930 device baseline is missing: $TARGET_TREE" >&2
    echo "copy local_manifests/mozart.xml into .repo/local_manifests and run repo sync first" >&2
    exit 3
fi

if ! command -v rsync >/dev/null 2>&1; then
    echo "error: rsync is required to install the maintained device source" >&2
    exit 3
fi

check_prebuilt() {
    local name="$1"
    local expected="$2"
    local path="$TARGET_TREE/rootdir/sbin/$name"
    local actual

    if [[ ! -f "$path" ]]; then
        echo "error: pinned device baseline is missing proprietary prebuilt: rootdir/sbin/$name" >&2
        exit 4
    fi

    actual="$(sha1sum "$path" | awk '{print $1}')"
    if [[ "$actual" != "$expected" ]]; then
        echo "error: unexpected sha1 for rootdir/sbin/$name" >&2
        echo "expected: $expected" >&2
        echo "actual:   $actual" >&2
        exit 4
    fi
}

# The unchanged health daemon comes from the pinned upstream device baseline.
# The B217 oeminfo and TEE daemons are installed by the proprietary extraction
# script and are deliberately excluded from this public repository.
check_prebuilt hw_healthd 6cad7ff3470a05df2bccef2489ba96d07286052d

rsync -a --delete \
    --exclude='/.git/' \
    --exclude='/patches/' \
    --exclude='/rootdir/sbin/' \
    --exclude='/prebuilt/cromite/*.apk' \
    --exclude='/prebuilt/cromite/lib/' \
    "$SOURCE_TREE/" "$TARGET_TREE/"

echo "installed maintained device source: device/huawei/mozart"
