#!/bin/sh --
set -e
script_dir="$(cd -- "$(dirname -- "$0")" && pwd)"
. "$script_dir/../hc/bootstrap/recipe.sh"

DEPENDENCIES="../recipes_host/llvm.sh"
FILE_DEPENDENCIES="$(find "$script_dir/files/init" -name "*.c" -print -o -name "*.h" -print)"

recipe_start
export NO_AARCH64=1 NO_RISCV64=1
../../hc/tools/build/elf.sh ../files/router router
mkdir ../router
mv ../files/router/x86_64 ../router/
recipe_finish
