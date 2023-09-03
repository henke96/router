#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init ""

FILE_DEPENDENCIES="$(find ../hc/src -name "*.c" -print -o -name "*.h" -print)"

recipe_start
cd ..
mv temp hc
cd hc
recipe_finish
