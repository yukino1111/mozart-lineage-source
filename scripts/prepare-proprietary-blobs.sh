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
    copybit="$VENDOR_PROPRIETARY/$libdir/hw/copybit.hi3635.so"
    gralloc="$VENDOR_PROPRIETARY/$libdir/hw/gralloc.hi3635.so"
    mali="$VENDOR_PROPRIETARY/vendor/$libdir/egl/libGLES_mali.so"

    for required in "$source_ion" "$copybit" "$gralloc" "$mali"; do
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

camera_config_server="$VENDOR_PROPRIETARY/vendor/bin/HwCamCfgSvr"
gps_daemon="$VENDOR_PROPRIETARY/bin/glgps4752"
camera_algo="$VENDOR_PROPRIETARY/system/lib/libcamera_algo.so"

for required in "$camera_config_server" "$gps_daemon" "$camera_algo"; do
    if [[ ! -f "$required" ]]; then
        echo "error: missing proprietary blob: $required" >&2
        exit 4
    fi
done

# Android 11 moved ProcessCallStack out of libutils. libshim_gui retains the
# diagnostic-only Marshmallow symbols used by Huawei's camera config server.
add_needed "$camera_config_server" libshim_gui.so

# Huawei's camera algorithm allocates a GraphicBuffer with the Android 6
# 136-byte object size. Android 11's 32-bit object is 152 bytes; enlarge the
# single allocation immediate before the compatibility constructor runs.
# Keep a full instruction context so an unrelated 0x88 immediate is never
# modified, and accept an already-prepared blob for idempotence.
perl -0777 -pi -e '
    BEGIN {
        $old = pack("H*", "4ff4807400e001248820f2f780ed");
        $new = pack("H*", "4ff4807400e001249820f2f780ed");
    }
    $old_count = () = /\Q$old\E/g;
    $new_count = () = /\Q$new\E/g;
    die "unexpected GraphicBuffer allocation signature\n"
        unless ($old_count == 1 && $new_count == 0) ||
               ($old_count == 0 && $new_count == 1);
    s/\Q$old\E/$new/ if $old_count == 1;
' "$camera_algo"

add_needed "$camera_algo" libshim_camera_legacy.so
add_needed "$camera_algo" libshim_gui.so

# BoringSSL removed the SSLv3-specific entry point. TLS_method has equivalent
# negotiation semantics for this client and its shorter name can safely reuse
# the existing dynamic string slot without changing ELF segment layout.
if readelf -Ws "$gps_daemon" | grep -F 'UND SSLv3_client_method' >/dev/null; then
    perl -0pi -e \
        's/SSLv3_client_method/TLS_method\x00\x00\x00\x00\x00\x00\x00\x00\x00/g' \
        "$gps_daemon"
fi
if ! readelf -Ws "$gps_daemon" | grep -F 'UND TLS_method' >/dev/null; then
    echo "error: failed to prepare TLS compatibility symbol in $gps_daemon" >&2
    exit 4
fi

validate_load_segments "$camera_config_server"
validate_load_segments "$gps_daemon"
validate_load_segments "$camera_algo"

audio_symbol_patcher="$ANDROID_TOP/device/huawei/mozart/tools/patch_legacy_icu_symbols.sh"
if [[ ! -x "$audio_symbol_patcher" ]]; then
    echo "error: missing audio compatibility tool: $audio_symbol_patcher" >&2
    exit 4
fi
"$audio_symbol_patcher" \
    "$VENDOR_PROPRIETARY/lib/hw/audio.primary.hi3635.so" \
    "$VENDOR_PROPRIETARY/lib64/hw/audio.primary.hi3635.so" \
    "$VENDOR_PROPRIETARY/lib/libhuaweiprocessing.so"

echo "prepared legacy mozart graphics, camera, GNSS and private libion blobs"
