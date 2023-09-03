#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh

recipe_init "../recipes_host/llvm.sh ./hc.sh"

FILE_DEPENDENCIES="$(find files/router -name "*.c" -print -o -name "*.h" -print)"

recipe_start
export NO_AARCH64=1 NO_RISCV64=1
../../hc/tools/build/elf.sh ../files/router router
mkdir ../router
mv ../files/router/x86_64 ../router/
recipe_finish
