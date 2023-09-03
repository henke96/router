#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh ./m4.sh"

URL="https://github.com/westes/flex/releases/download/v2.6.4/flex-2.6.4.tar.gz"
SHA256="e87aae032bf07c26f85ac0ed3250998c37621d95f8bd748b31f15b33c45ee995"

recipe_start
./configure --prefix="$SCRIPT_DIR/$RECIPE_NAME" --disable-dependency-tracking --disable-nls --disable-libfl --without-libiconv-prefix --without-libintl-prefix
make -j "$NUM_CPUS" install
recipe_finish
