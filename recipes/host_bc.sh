#!/bin/sh --
set -ex
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh" "./host_flex.sh"

URL="https://ftp.gnu.org/gnu/bc/bc-1.06.tar.gz"
SHA512="a61fa22689bccd843e5f99bfc0ec1fe5577536522392067e4e35bae0914250c1f7d198cfd8bfc4489b969cde0fb200c90028d0be06e99a99afa61f3ce841bf4e"

recipe_start
./configure --prefix="$SCRIPT_DIR/$RECIPE_NAME" --disable-dependency-tracking --disable-nls --disable-yacc --without-libiconv-prefix --without-libintl-prefix --without-libreadline-prefix --without-libtextstyle-prefix
make -j "$NUM_CPUS" install
recipe_finish
