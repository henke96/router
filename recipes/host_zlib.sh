#!/bin/sh --
set -ex
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh" ""

URL="https://www.zlib.net/zlib-1.3.tar.gz"
SHA512="185795044461cd78a5545250e06f6efdb0556e8d1bfe44e657b509dd6f00ba8892c8eb3febe65f79ee0b192d6af857f0e0055326d33a881449f3833f92e5f8fb"

recipe_start
./configure --prefix="$SCRIPT_DIR/$RECIPE_NAME"
make -j "$NUM_CPUS" install
recipe_finish
