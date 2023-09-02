#!/bin/sh --
set -e
script_dir="$(cd -- "$(dirname -- "$0")" && pwd)"
. "$script_dir/../hc/bootstrap/recipe.sh"

URL="https://www.zlib.net/zlib-1.3.tar.gz"
SHA256="ff0ba4c292013dbc27530b3a81e1f9a813cd39de01ca5e0f8bf355702efa593e"
DEPENDENCIES="../hc/bootstrap/make.sh"

recipe_start
./configure --prefix="$script_dir/zlib"
make -j "$NUM_CPUS" install
recipe_finish
