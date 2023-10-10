#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh"

URL="https://musl.libc.org/releases/musl-1.2.4.tar.gz"
SHA256="7a35eae33d5372a7c0da1188de798726f68825513b7ae3ebe97aaaa52114f039"

recipe_start
arch=x86_64
make -j "$NUM_CPUS" install-headers ARCH=$arch prefix= DESTDIR="$SCRIPT_DIR/$RECIPE_NAME/$arch"
recipe_finish
