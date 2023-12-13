#!/bin/sh --
set -e
script_dir="$(cd -- "${0%/*}/" && pwd)"

"$script_dir/recipes/bootloader.sh"
mkdir -p "$script_dir/out/efi/boot"
cp "$script_dir/recipes/bootloader/x86_64/bootloader.efi" "$script_dir/out/efi/boot/bootx64.efi"
cp "$script_dir/recipes/bootloader/x86_64/debug.bootloader.efi" "$script_dir/out/efi/boot/debug.bootx64.efi"
