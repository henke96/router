#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../recipes_host/llvm.sh ../hc/bootstrap/make.sh"

URL="https://musl.libc.org/releases/musl-1.2.4.tar.gz"
SHA256="7a35eae33d5372a7c0da1188de798726f68825513b7ae3ebe97aaaa52114f039"

recipe_start
arch=x86_64
export CC=clang AR=llvm-ar RANLIB=llvm-ranlib
export CFLAGS="-target $arch-unknown-linux-musl"
export LIBCC="$("$CC" $CFLAGS --print-libgcc-file-name)"
./configure --prefix= --target=$arch
make -j "$NUM_CPUS" install DESTDIR="$SCRIPT_DIR/$RECIPE_NAME/$arch"
recipe_finish
