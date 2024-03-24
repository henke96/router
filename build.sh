#!/bin/sh --
set -e
script_dir="$(cd -- "${0%/*}/" && pwd)"

"$script_dir/recipes/bootloader"

rm -rf "$OUT/out"
mkdir -p "$OUT/out/efi/boot"
cp "$OUT/bootloader/x86_64-windows-gnu_bootloader.efi" "$OUT/out/efi/boot/bootx64.efi"
cp "$OUT/bootloader/debug_x86_64-windows-gnu_bootloader.efi" "$OUT/out/efi/boot/debug_bootx64.efi"
