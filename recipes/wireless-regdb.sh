#!/bin/sh --
set -eax
cd -- "${0%/*}/"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/xz.sh" ""

pkg="wireless-regdb-2023.09.01"
URL="https://www.kernel.org/pub/software/network/wireless-regdb/$pkg.tar.xz"
SHA512="e88b4ea8b40f916f4e6c7fea8ea1b9fc7c6ece755d6ce24a8f43f66840f68e9971938707b0d94c87a2aa00dfe7f5734df469e0e088a82fe544a031b1d7422596"

recipe_start
rm -rf "./$pkg"; xz -d -c "$DOWNLOAD" | tar xf - "$pkg/regulatory.db"
mv "./$pkg" "./$RECIPE_NAME"
recipe_finish
