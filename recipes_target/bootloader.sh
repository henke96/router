#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../recipes_host/llvm.sh ./hc.sh ./linux.sh"

FILE_DEPENDENCIES="$(find files/bootloader -name "*.c" -print -o -name "*.h" -print)"

recipe_start
export NO_AARCH64=1
export FLAGS="-I$SCRIPT_DIR/linux"
../../hc/tools/build/efi.sh ../files/bootloader bootloader
mkdir ../$RECIPE_NAME
mv ../files/bootloader/x86_64 ../$RECIPE_NAME/
recipe_finish

