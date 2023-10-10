#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh"

URL="https://ftp.gnu.org/gnu/mtools/mtools-4.0.43.tar.gz"
SHA256="8866666fa06906ee02c709f670ae6361c5ac2008251ed825c43d321c06775718"

recipe_start
./configure --prefix="$SCRIPT_DIR/$RECIPE_NAME"
make -j "$NUM_CPUS" install
recipe_finish
