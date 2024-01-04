#!/bin/sh --
set -e
script_dir="$(cd -- "${0%/*}/" && pwd)"

export OUT="$script_dir/hc-out/router-recipes"
mkdir -p "$OUT"
"$script_dir/recipes/bootloader"

mkdir -p "$script_dir/out/efi/boot"
cp "$OUT/bootloader-out/x86_64/bootloader.efi" "$script_dir/out/efi/boot/bootx64.efi"
cp "$OUT/bootloader-out/x86_64/debug-bootloader.efi" "$script_dir/out/efi/boot/debug-bootx64.efi"
