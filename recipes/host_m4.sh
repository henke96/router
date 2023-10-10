#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh"

URL="https://ftp.gnu.org/gnu/m4/m4-1.4.19.tar.gz"
SHA256="3be4a26d825ffdfda52a56fc43246456989a3630093cced3fbddf4771ee58a70"

recipe_start
./configure --prefix="$SCRIPT_DIR/$RECIPE_NAME" --disable-dependency-tracking --disable-nls --without-libiconv-prefix --without-libintl-prefix --without-dmalloc --without-libsigsegv-prefix
make -j "$NUM_CPUS" install
recipe_finish
