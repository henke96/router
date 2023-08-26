#!/bin/sh --
set -e
script_dir="$(cd -- "$(dirname -- "$0")" && pwd)"
. "$script_dir/../hc/bootstrap/recipe.sh"

URL="https://github.com/westes/flex/releases/download/v2.6.4/flex-2.6.4.tar.gz"
SHA256="e87aae032bf07c26f85ac0ed3250998c37621d95f8bd748b31f15b33c45ee995"
DEPENDENCIES="../hc/bootstrap/make.sh ./m4.sh"

recipe_start
./configure --prefix="$script_dir/flex" --disable-dependency-tracking --disable-nls --disable-libfl --without-libiconv-prefix --without-libintl-prefix
make -j "$NUM_CPUS" install
recipe_finish
