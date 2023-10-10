#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh"

URL="https://www.zlib.net/zlib-1.3.tar.gz"
SHA256="ff0ba4c292013dbc27530b3a81e1f9a813cd39de01ca5e0f8bf355702efa593e"

recipe_start
./configure --prefix="$SCRIPT_DIR/$RECIPE_NAME"
make -j "$NUM_CPUS" install
recipe_finish
