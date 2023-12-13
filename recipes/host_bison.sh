#!/bin/sh --
set -eax
cd -- "${0%/*}/"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh ../hc/bootstrap/xz.sh ./host_m4.sh" ""

pkg="bison-3.8.2"
URL="https://ftp.gnu.org/gnu/bison/$pkg.tar.xz"
SHA512="d4d23af6671406e97257892f90651b67f2ab95219831372be032190b7156c10a3435d457857e677445df8b2327aacccc15344acbbc3808a6f332a93cce23b444"

recipe_start
rm -rf "./$pkg"; xz -d -c "$DOWNLOAD" | tar xf -; cd "./$pkg"

./configure --prefix="$SCRIPT_DIR/$RECIPE_NAME" --disable-dependency-tracking --disable-nls --disable-yacc --without-libiconv-prefix --without-libintl-prefix --without-libreadline-prefix --without-libtextstyle-prefix
make -j "$NUM_CPUS" install

cd ..; rm -rf "./$pkg"
recipe_finish
