#!/usr/bin/env bash
set -euo pipefail

PATCH_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ANDROID_TOP="${1:-/android/lineage16-mozart}"
SOURCE_ROOT="${2:-$PATCH_ROOT/proprietary-blobs/huawei/mozart}"
LIST_FILE="$PATCH_ROOT/proprietary-files/mozart-emui31-graphics.txt"
CACHE_ROOT="$PATCH_ROOT/proprietary-blobs/huawei/mozart"
VENDOR_PROPRIETARY="$ANDROID_TOP/vendor/huawei/mozart/proprietary"
STRICT_SHA1="${STRICT_SHA1:-0}"

if [[ ! -d "$ANDROID_TOP/.repo" ]]; then
    echo "error: $ANDROID_TOP does not look like an Android repo checkout" >&2
    exit 2
fi

if [[ ! -d "$ANDROID_TOP/vendor/huawei/mozart" ]]; then
    echo "error: missing vendor/huawei/mozart in $ANDROID_TOP" >&2
    exit 2
fi

if [[ ! -d "$SOURCE_ROOT" ]]; then
    echo "error: proprietary blob source is missing: $SOURCE_ROOT" >&2
    echo "pass a stock /system extraction as the second argument, or place files under:" >&2
    echo "  $CACHE_ROOT/proprietary" >&2
    exit 3
fi

find_blob() {
    local rel="$1"
    local candidate

    for candidate in \
        "$SOURCE_ROOT/proprietary/$rel" \
        "$SOURCE_ROOT/system/$rel" \
        "$SOURCE_ROOT/$rel"; do
        if [[ -f "$candidate" ]]; then
            printf '%s\n' "$candidate"
            return 0
        fi
    done

    return 1
}

trim() {
    local value="$1"
    value="${value#"${value%%[![:space:]]*}"}"
    value="${value%"${value##*[![:space:]]}"}"
    printf '%s\n' "$value"
}

copied=0

while IFS= read -r raw_line || [[ -n "$raw_line" ]]; do
    line="$(trim "${raw_line%%#*}")"
    [[ -z "$line" ]] && continue

    rel="${line%%;*}"
    meta="${line#"$rel"}"
    expected_sha1=""

    if [[ "$meta" =~ sha1=([0-9a-fA-F]+) ]]; then
        expected_sha1="${BASH_REMATCH[1],,}"
    fi

    if ! src="$(find_blob "$rel")"; then
        echo "error: missing proprietary blob: $rel" >&2
        exit 3
    fi

    if [[ -n "$expected_sha1" ]]; then
        actual_sha1="$(sha1sum "$src" | awk '{print $1}')"
        if [[ "$actual_sha1" != "$expected_sha1" ]]; then
            message="sha1 mismatch for $rel: expected $expected_sha1, got $actual_sha1"
            if [[ "$STRICT_SHA1" == "1" ]]; then
                echo "error: $message" >&2
                exit 4
            fi
            echo "warning: $message" >&2
        fi
    fi

    dst="$VENDOR_PROPRIETARY/$rel"
    cache="$CACHE_ROOT/proprietary/$rel"

    install -D -m 0644 "$src" "$dst"
    if [[ "$src" != "$cache" ]]; then
        install -D -m 0644 "$src" "$cache"
    fi

    echo "copy: $rel"
    copied=$((copied + 1))
done < "$LIST_FILE"

echo "copied $copied proprietary blobs"
