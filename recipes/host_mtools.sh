#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh"

URL="https://ftp.gnu.org/gnu/mtools/mtools-4.0.43.tar.gz"
SHA512="e99643298846facf81e2a4f8dc7b780f03aa28dafdf790cb745a368a617375d12327675b12c3da87584c20f41024481326b976a1a9996685c7d2ce07395c18f8"

recipe_start
./configure --prefix="$SCRIPT_DIR/$RECIPE_NAME"
make -j "$NUM_CPUS" install
recipe_finish
