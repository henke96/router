#!/bin/sh --
set -eax
cd -- "${0%/*}/"
. ../hc/bootstrap/recipe.sh

recipe_init "./host_llvm.sh" "./hc.sh"

FILE_DEPENDENCIES="$(find ../router -name "*.c" -print -o -name "*.h" -print)"

recipe_start
export NO_AARCH64=1 NO_RISCV64=1
../hc/tools/build/elf.sh ../router router
mkdir "./$RECIPE_NAME"
mv ../router/x86_64 "./$RECIPE_NAME/"
recipe_finish
