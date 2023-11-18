#!/bin/sh --
set -ex
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh" ""

URL="https://sourceware.org/pub/bzip2/bzip2-1.0.8.tar.gz"
SHA512="083f5e675d73f3233c7930ebe20425a533feedeaaa9d8cc86831312a6581cefbe6ed0d08d2fa89be81082f2a5abdabca8b3c080bf97218a1bd59dc118a30b9f3"

recipe_start
make -j "$NUM_CPUS" install PREFIX="$SCRIPT_DIR/$RECIPE_NAME"
recipe_finish
