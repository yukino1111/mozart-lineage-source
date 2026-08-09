#!/bin/bash

set -euo pipefail

script_dir="$(cd "$(dirname "$0")" && pwd)"
symbol_map="${SYMBOL_MAP:-${script_dir}/audio_legacy_symbols.map}"

if [ "$#" -eq 0 ]; then
    echo "usage: $0 ELF_FILE..." >&2
    exit 2
fi

if [ ! -f "${symbol_map}" ]; then
    echo "symbol map not found: ${symbol_map}" >&2
    exit 2
fi

for elf_file in "$@"; do
    if [ ! -f "${elf_file}" ]; then
        echo "ELF file not found: ${elf_file}" >&2
        exit 2
    fi

    original_size="$(stat -c %s "${elf_file}")"

    while read -r old_symbol new_symbol; do
        [ -n "${old_symbol}" ] || continue

        old_count="$(LC_ALL=C grep -aFo "${old_symbol}" "${elf_file}" 2>/dev/null | wc -l || true)"
        new_count="$(LC_ALL=C grep -aFo "${new_symbol}" "${elf_file}" 2>/dev/null | wc -l || true)"

        if [ "${old_count}" -eq 0 ]; then
            if [ "${new_count}" -eq 0 ]; then
                echo "neither symbol exists in ${elf_file}: ${old_symbol}" >&2
                exit 1
            fi
            continue
        fi

        if [ "${#old_symbol}" -ne "${#new_symbol}" ]; then
            echo "replacement changes string length: ${old_symbol} -> ${new_symbol}" >&2
            exit 1
        fi

        OLD_SYMBOL="${old_symbol}" NEW_SYMBOL="${new_symbol}" \
            perl -0pi -e 's/\Q$ENV{OLD_SYMBOL}\E/$ENV{NEW_SYMBOL}/g' "${elf_file}"
    done < "${symbol_map}"

    if [ "$(stat -c %s "${elf_file}")" -ne "${original_size}" ]; then
        echo "ELF size changed unexpectedly: ${elf_file}" >&2
        exit 1
    fi

    if readelf -Ws "${elf_file}" 2>/dev/null | awk '$7 == "UND" { print $8 }' | grep -q '_60$'; then
        echo "legacy ICU symbols remain unresolved in ${elf_file}" >&2
        exit 1
    fi

    echo "patched and verified: ${elf_file}"
done
