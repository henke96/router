#!/bin/sh --
set -ex
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "./host_llvm.sh" "./hc.sh"

FILE_DEPENDENCIES="$(find ../init -name "*.c" -print -o -name "*.h" -print)"

recipe_start
mkdir "./$RECIPE_NAME"

export NO_AARCH64=1 NO_RISCV64=1
../hc/tools/build/elf.sh ../init init
mv ../init/x86_64 "./$RECIPE_NAME/"
recipe_finish
