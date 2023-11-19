#!/bin/sh --
set -ex
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh ./host_m4.sh" ""

pkg="bison-3.8.2"
URL="https://ftp.gnu.org/gnu/bison/$pkg.tar.gz"
SHA512="bdf21dc063d12cc3fe742d3716cf44c3ddc642e1b236825460f6b9a74c4412b891cd9b8f7e139b02e7897176cd0be202bf4838179c355d89fdfb128cf3649971"

recipe_start
rm -rf "./$pkg"; gzip -d -c "$DOWNLOAD" | tar xf -; cd "./$pkg"

./configure --prefix="$SCRIPT_DIR/$RECIPE_NAME" --disable-dependency-tracking --disable-nls --disable-yacc --without-libiconv-prefix --without-libintl-prefix --without-libreadline-prefix --without-libtextstyle-prefix
make -j "$NUM_CPUS" install

cd ..; rm -rf "./$pkg"
recipe_finish
