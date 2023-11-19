#!/bin/sh --
set -ex
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh" ""

pkg="perl-5.38.0"
URL="https://www.cpan.org/src/5.0/$pkg.tar.gz"
SHA512="8118acacc4ea3103a433b15c1c868f4e3f9ff6147ffadcfff322c80444f69e8f4ed5b8e849073ecd35cb3ca36d16c6a6b510d2902292a9da996809baec17444a"

recipe_start
rm -rf "./$pkg"; gzip -d -c "$DOWNLOAD" | tar xf -; cd "./$pkg"

./Configure -des -Dprefix="$SCRIPT_DIR/$RECIPE_NAME"
make -j "$NUM_CPUS" install

cd ..; rm -rf "./$pkg"
recipe_finish