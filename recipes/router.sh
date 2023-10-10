#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh

recipe_init "./host_llvm.sh ./hc.sh"

FILE_DEPENDENCIES="$(find ../router -name "*.c" -print -o -name "*.h" -print)"

recipe_start
export NO_AARCH64=1 NO_RISCV64=1
../../hc/tools/build/elf.sh ../../router router
mkdir ../router
mv ../../router/x86_64 ../router/
recipe_finish
