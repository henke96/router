#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh ./host_m4.sh"

URL="https://ftp.gnu.org/gnu/bison/bison-3.8.2.tar.gz"
SHA256="06c9e13bdf7eb24d4ceb6b59205a4f67c2c7e7213119644430fe82fbd14a0abb"

recipe_start
./configure --prefix="$SCRIPT_DIR/$RECIPE_NAME" --disable-dependency-tracking --disable-nls --disable-yacc --without-libiconv-prefix --without-libintl-prefix --without-libreadline-prefix --without-libtextstyle-prefix
make -j "$NUM_CPUS" install
recipe_finish
