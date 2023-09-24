#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh ../hc/bootstrap/xz.sh ../recipes_host/rsync.sh"

URL="https://www.kernel.org/pub/linux/kernel/v6.x/linux-6.1.48.tar.xz"
SHA256="c606cbd0353e677df6fae73cc16ba3c9244b98372ed7771d551024016f55ac31"

recipe_start
export ARCH=x86_64
make -j "$NUM_CPUS" headers_install INSTALL_HDR_PATH="$SCRIPT_DIR/$RECIPE_NAME/$ARCH" HOSTCC="${CC:-cc}"
recipe_finish

