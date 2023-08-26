#!/bin/sh
set -e
script_dir="$(cd -- "$(dirname -- "$0")" && pwd)"
root_dir="$script_dir/../.."

export NO_AARCH64=1
FLAGS="-I$root_dir/recipes/linux" "$root_dir/hc/tools/build/efi.sh" "$script_dir" bootloader
