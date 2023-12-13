#!/bin/sh --
set -eax
cd -- "${0%/*}/"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh ../hc/bootstrap/xz.sh" ""

pkg="perl-5.38.2"
URL="https://www.cpan.org/src/5.0/$pkg.tar.xz"
SHA512="0ca51e447c7a18639627c281a1c7ae6662c773745ea3c86bede46336d5514ecc97ded2c61166e1ac15635581489dc596368907aa3a775b34db225b76d7402d10"

recipe_start
rm -rf "./$pkg"; xz -d -c "$DOWNLOAD" | tar xf -; cd "./$pkg"

./Configure -des -Dman1dir=none -Dman3dir=none -Dprefix="$SCRIPT_DIR/$RECIPE_NAME"
make -j "$NUM_CPUS" install

cd ..; rm -rf "./$pkg"
recipe_finish
