#!/bin/sh --
set -e
script_dir="$(cd -- "$(dirname -- "$0")" && pwd)"
. "$script_dir/../hc/bootstrap/recipe.sh"

URL="https://sourceware.org/pub/bzip2/bzip2-1.0.8.tar.gz"
SHA256="ab5a03176ee106d3f0fa90e381da478ddae405918153cca248e682cd0c4a2269"
DEPENDENCIES="../hc/bootstrap/make.sh"

recipe_start
make -j "$NUM_CPUS" install PREFIX="$script_dir/bzip2"
recipe_finish
