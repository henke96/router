#!/bin/sh --
set -eax
cd -- "${0%/*}/"
. ../hc/bootstrap/recipe.sh
recipe_init "./host_llvm.sh" "./hc.sh"

FILE_DEPENDENCIES="$(find ../init -name "*.c" -print -o -name "*.h" -print)"

recipe_start
export NO_AARCH64=1 NO_RISCV64=1
../hc/tools/build/elf.sh ../init init
mkdir "./$RECIPE_NAME"
mv ../init/x86_64 "./$RECIPE_NAME/"
recipe_finish
