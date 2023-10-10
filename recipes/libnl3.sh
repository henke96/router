#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "./host_llvm.sh ./host_bison.sh ./host_flex.sh ../hc/bootstrap/make.sh ./musl.sh ./linux-headers.sh"

URL="https://github.com/thom311/libnl/releases/download/libnl3_8_0/libnl-3.8.0.tar.gz"
SHA256="bb726c6d7a08b121978d73ff98425bf313fa26a27a331d465e4f1d7ec5b838c6"

recipe_start
arch=x86_64
export CC=clang AR=llvm-ar RANLIB=llvm-ranlib
export CFLAGS="-target $arch-unknown-linux-musl --sysroot $SCRIPT_DIR/musl/$arch -I$SCRIPT_DIR/linux-headers/$arch/include"
./configure --prefix= --host=$arch --disable-dependency-tracking --disable-debug --enable-shared=no
make -j "$NUM_CPUS" install DESTDIR="$SCRIPT_DIR/$RECIPE_NAME"
recipe_finish
