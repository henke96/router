#!/bin/sh --
set -e
script_dir="$(cd -- "$(dirname -- "$0")" && pwd)"
. "$script_dir/../hc/bootstrap/recipe.sh"

URL="https://ftp.gnu.org/gnu/m4/m4-1.4.19.tar.gz"
SHA256="3be4a26d825ffdfda52a56fc43246456989a3630093cced3fbddf4771ee58a70"
DEPENDENCIES="../hc/bootstrap/make.sh"

recipe_start
./configure --prefix="$script_dir/m4" --disable-dependency-tracking --disable-nls --without-libiconv-prefix --without-libintl-prefix --without-dmalloc --without-libsigsegv-prefix
make -j "$NUM_CPUS" install
recipe_finish
