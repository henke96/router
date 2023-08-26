#!/bin/sh --
set -e
script_dir="$(cd -- "$(dirname -- "$0")" && pwd)"
. "$script_dir/../hc/bootstrap/recipe.sh"

URL="https://ftp.gnu.org/gnu/bc/bc-1.06.tar.gz"
SHA256="4ef6d9f17c3c0d92d8798e35666175ecd3d8efac4009d6457b5c99cea72c0e33"
DEPENDENCIES="../hc/bootstrap/make.sh ./flex.sh"

recipe_start
./configure --prefix="$script_dir/bc" --disable-dependency-tracking --disable-nls --disable-yacc --without-libiconv-prefix --without-libintl-prefix --without-libreadline-prefix --without-libtextstyle-prefix
make -j "$NUM_CPUS" install
recipe_finish
