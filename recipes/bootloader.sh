#!/bin/sh --
set -ex
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "./host_llvm.sh" "./hc.sh ./linux.sh"

FILE_DEPENDENCIES="$(find ../bootloader -name "*.c" -print -o -name "*.h" -print)"

recipe_start
export NO_AARCH64=1
export FLAGS="-I$SCRIPT_DIR/linux"
../hc/tools/build/efi.sh ../bootloader bootloader
mkdir ./$RECIPE_NAME
mv ../bootloader/x86_64 ./$RECIPE_NAME/
recipe_finish

