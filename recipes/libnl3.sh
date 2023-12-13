#!/bin/sh --
set -eax
cd -- "${0%/*}/"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh ./host_llvm.sh ./host_bison.sh ./host_flex.sh" "./musl.sh ./linux-headers.sh"

pkg="libnl-3.8.0"
URL="https://github.com/thom311/libnl/releases/download/libnl3_8_0/$pkg.tar.gz"
SHA512="a8da7e8464c5cab2807cd86f83fca53bc77f7c3bdefe43880db7be9caf5ba6bfca1b7e778b906c12fda1cf7e6f413f88cc7ad1dcc42fe8c050a8d28a6e656c02"

recipe_start
rm -rf "./$pkg"; gzip -d -c "$DOWNLOAD" | tar xf -; cd "./$pkg"

arch=x86_64
export CC=clang AR=llvm-ar RANLIB=llvm-ranlib
export CFLAGS="-target $arch-unknown-linux-musl --sysroot $SCRIPT_DIR/musl/$arch -I$SCRIPT_DIR/linux-headers/$arch"
./configure --prefix= --host=$arch --disable-dependency-tracking --disable-debug --enable-shared=no
make -j "$NUM_CPUS" install DESTDIR="$SCRIPT_DIR/$RECIPE_NAME"

cd ..; rm -rf "./$pkg"
recipe_finish
