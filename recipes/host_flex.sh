#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh ./host_m4.sh"

URL="https://github.com/westes/flex/releases/download/v2.6.4/flex-2.6.4.tar.gz"
SHA512="e9785f3d620a204b7d20222888917dc065c2036cae28667065bf7862dfa1b25235095a12fd04efdbd09bfd17d3452e6b9ef953a8c1137862ff671c97132a082e"

recipe_start
./configure --prefix="$SCRIPT_DIR/$RECIPE_NAME" --disable-dependency-tracking --disable-nls --disable-libfl --without-libiconv-prefix --without-libintl-prefix
make -j "$NUM_CPUS" install
recipe_finish
