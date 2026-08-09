#!/usr/bin/env bash
set -euo pipefail

PATCH_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ANDROID_TOP="${1:-/android/lineage18.1-mozart}"
VENDOR_PROPRIETARY="$ANDROID_TOP/vendor/huawei/mozart/proprietary"
PATCHELF="${PATCHELF:-patchelf}"

if [[ ! -d "$VENDOR_PROPRIETARY" ]]; then
    echo "error: missing mozart proprietary tree: $VENDOR_PROPRIETARY" >&2
    exit 2
fi

if ! command -v "$PATCHELF" >/dev/null 2>&1; then
    echo "error: patchelf is required to prepare the legacy blobs" >&2
    exit 3
fi
if ! command -v readelf >/dev/null 2>&1; then
    echo "error: readelf is required to validate the prepared legacy blobs" >&2
    exit 3
fi

replace_needed() {
    local binary="$1"
    local old="$2"
    local new="$3"

    if "$PATCHELF" --print-needed "$binary" | grep -Fxq "$new"; then
        return
    fi

    if ! "$PATCHELF" --print-needed "$binary" | grep -Fxq "$old"; then
        echo "error: $binary requires neither $old nor $new" >&2
        exit 4
    fi

    "$PATCHELF" --replace-needed "$old" "$new" "$binary"
}

add_needed() {
    local binary="$1"
    local library="$2"

    if ! "$PATCHELF" --print-needed "$binary" | grep -Fxq "$library"; then
        "$PATCHELF" --add-needed "$library" "$binary"
    fi
}

validate_load_segments() {
    local binary="$1"
    local type offset vaddr align
    local found=0

    while read -r type offset vaddr align; do
        found=1
        if (( offset % align != vaddr % align )); then
            echo "error: misaligned PT_LOAD in $binary: offset=$offset vaddr=$vaddr align=$align" >&2
            exit 5
        fi
    done < <(readelf -lW "$binary" | awk '$1 == "LOAD" { print $1, $2, $3, $NF }')

    if (( found == 0 )); then
        echo "error: no PT_LOAD segments found in $binary" >&2
        exit 5
    fi
}

for libdir in lib lib64; do
    source_ion="$VENDOR_PROPRIETARY/$libdir/libion.so"
    mozart_ion="$VENDOR_PROPRIETARY/$libdir/libion_mozart.so"
    gralloc="$VENDOR_PROPRIETARY/$libdir/hw/gralloc.hi3635.so"
    mali="$VENDOR_PROPRIETARY/vendor/$libdir/egl/libGLES_mali.so"

    for required in "$source_ion" "$gralloc" "$mali"; do
        if [[ ! -f "$required" ]]; then
            echo "error: missing proprietary blob: $required" >&2
            echo "run scripts/extract-proprietary-blobs.sh first if necessary" >&2
            exit 4
        fi
    done

    install -m 0644 "$source_ion" "$mozart_ion"
    "$PATCHELF" --set-soname libion_mozart.so "$mozart_ion"
    replace_needed "$gralloc" libion.so libion_mozart.so
    add_needed "$mali" libutilscallstack.so
    validate_load_segments "$mozart_ion"
    validate_load_segments "$gralloc"
    validate_load_segments "$mali"
done

echo "prepared legacy mozart gralloc, Mali and private libion blobs"
