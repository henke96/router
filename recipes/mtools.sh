#!/bin/sh --
set -e
script_dir="$(cd -- "$(dirname -- "$0")" && pwd)"
. "$script_dir/../hc/bootstrap/recipe.sh"

URL="https://ftp.gnu.org/gnu/mtools/mtools-4.0.43.tar.gz"
SHA256="8866666fa06906ee02c709f670ae6361c5ac2008251ed825c43d321c06775718"
DEPENDENCIES="../hc/bootstrap/make.sh"

recipe_start
./configure --prefix="$script_dir/mtools"
make -j "$NUM_CPUS" install
recipe_finish
