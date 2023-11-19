#!/bin/sh --
set -ex
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "" ""

FILE_DEPENDENCIES="$(find ../hc/src -name "*.c" -print -o -name "*.h" -print)"

recipe_start
mkdir "./$RECIPE_NAME"
recipe_finish
