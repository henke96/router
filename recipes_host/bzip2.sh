#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh"

URL="https://sourceware.org/pub/bzip2/bzip2-1.0.8.tar.gz"
SHA256="ab5a03176ee106d3f0fa90e381da478ddae405918153cca248e682cd0c4a2269"

recipe_start
make -j "$NUM_CPUS" install PREFIX="$SCRIPT_DIR/$RECIPE_NAME" CC="${CC:-cc}"
recipe_finish
