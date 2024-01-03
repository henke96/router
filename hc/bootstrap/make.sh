#!/bin/sh --
set -eax
SCRIPT_DIR="$(cd -- "${0%/*}/" && pwd)"
. "$SCRIPT_DIR/../tools/shell/recipe.sh"
recipe_init ""

pkg="make-4.4"
URL="https://ftp.gnu.org/gnu/make/$pkg.tar.gz"
SHA512="4be73f494295dcfa10034531b0d920cfdb5438bc20625f863f5c878549c140e1e67195162580c53060c3c11c67a2c739c09051f02cdd283e5aa9ebcd68975a1f"

recipe_start
rm -rf "./$pkg"; gzip -d -c "$DOWNLOAD" | tar xf -; cd "./$pkg"

./configure --prefix="$RECIPE_OUT" --disable-dependency-tracking --without-libiconv-prefix --without-libintl-prefix --without-guile --without-customs --without-dmalloc
./build.sh
mkdir -p "$RECIPE_OUT/bin"
mv ./make "$RECIPE_OUT/bin/"

cd ..; rm -rf "./$pkg"
recipe_finish
