#!/bin/sh
set -e
script_dir="$(dirname "$0")"
root_dir="$script_dir/../.."
linux_out=$(echo $root_dir/linux/linux*/arch/x86/boot)

FLAGS="-I$linux_out -Os" "$root_dir/hc/tools/build/efi.sh" "$script_dir" bootloader
