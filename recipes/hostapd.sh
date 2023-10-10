#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "./host_llvm.sh ../hc/bootstrap/make.sh ./musl.sh ./libnl3.sh"

URL="https://w1.fi/releases/hostapd-2.10.tar.gz"
SHA256="206e7c799b678572c2e3d12030238784bc4a9f82323b0156b4c9466f1498915d"
FILE_DEPENDENCIES="files/hostapd/.config"

recipe_start
cp ../files/hostapd/.config hostapd/.config
arch=x86_64
export CC=clang AR=llvm-ar RANLIB=llvm-ranlib
export CFLAGS="-target $arch-unknown-linux-musl --sysroot $SCRIPT_DIR/musl/$arch -I$SCRIPT_DIR/linux-headers/$arch/include"
export LDFLAGS="-target $arch-unknown-linux-musl --sysroot $SCRIPT_DIR/musl/$arch -static -L$SCRIPT_DIR/libnl3/lib"
make -C hostapd -j "$NUM_CPUS" install BINDIR= DESTDIR="$SCRIPT_DIR/hostapd" LIBNL_INC="$SCRIPT_DIR/libnl3/include/libnl3"
llvm-objcopy --strip-sections "$SCRIPT_DIR/hostapd/hostapd"
recipe_finish
