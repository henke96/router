#!/bin/sh --
set -e
script_dir="$(cd -- "${0%/*}/" && pwd)"

"$script_dir/recipes/bootloader_x86_64"

rm -rf "$OUT/out"
mkdir -p "$OUT/out/efi/boot"
cp "$OUT/bootloader_x86_64/x86_64-windows-gnu_bootloader.efi" "$OUT/out/efi/boot/bootx64.efi"
cp "$OUT/bootloader_x86_64/debug_x86_64-windows-gnu_bootloader.efi" "$OUT/out/efi/boot/debug_bootx64.efi"

if test -z "$NO_SOURCE"; then
    "$script_dir/recipes/source"
    cp "$OUT/source/source.tar" "$OUT/out/"
fi

if test -z "$NO_DOWNLOADS"; then
    "$script_dir/recipes/downloads"
    cp -r "$OUT/downloads/out" "$OUT/out/downloads"
fi

if test -z "$NO_DEVTOOLS"; then
    "$script_dir/recipes/devtools_x86_64"
    cp "$OUT/devtools_x86_64/devtools.tar" "$OUT/out/"
fi
