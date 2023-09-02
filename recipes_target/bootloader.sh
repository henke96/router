#!/bin/sh --
set -e
script_dir="$(cd -- "$(dirname -- "$0")" && pwd)"
. "$script_dir/../hc/bootstrap/recipe.sh"

DEPENDENCIES="../recipes_host/llvm.sh ./linux.sh"
FILE_DEPENDENCIES="$(find "$script_dir/files/bootloader" -name "*.c" -print -o -name "*.h" -print)"

recipe_start
export NO_AARCH64=1
export FLAGS="-I$script_dir/linux"
../../hc/tools/build/efi.sh ../files/bootloader bootloader
mkdir ../bootloader
mv ../files/bootloader/x86_64 ../bootloader/
recipe_finish
