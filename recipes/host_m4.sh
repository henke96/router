#!/bin/sh --
set -ex
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh" ""

pkg="m4-1.4.19"
URL="https://ftp.gnu.org/gnu/m4/$pkg.tar.gz"
SHA512="f5dd0f02fcae65a176a16af9a8e1747c26e9440c6c224003ba458d3298b777a75ffb189aee9051fb0c4840b2a48278be4a51d959381af0b1d627570f478c58f2"

recipe_start
rm -rf "./$pkg"; gzip -d -c "$DOWNLOAD" | tar xf -; cd "./$pkg"

./configure --prefix="$SCRIPT_DIR/$RECIPE_NAME" --disable-dependency-tracking --disable-nls --without-libiconv-prefix --without-libintl-prefix --without-dmalloc --without-libsigsegv-prefix
make -j "$NUM_CPUS" install

cd ..; rm -rf "./$pkg"
recipe_finish
