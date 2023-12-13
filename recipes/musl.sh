#!/bin/sh --
set -eax
cd -- "${0%/*}/"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh ./host_llvm.sh" ""

pkg="musl-1.2.4"
URL="https://musl.libc.org/releases/$pkg.tar.gz"
SHA512="498ec5d7941194a8806f4d42f0f6d218c862996ef1398b737d0d06995e0b7a6574b240a48088f6b84016b14b2776fe463f829dcb11149cdfc1023d496b235c55"

recipe_start
rm -rf "./$pkg"; gzip -d -c "$DOWNLOAD" | tar xf -; cd "./$pkg"

arch=x86_64
export CC=clang AR=llvm-ar RANLIB=llvm-ranlib
export CFLAGS="-target $arch-unknown-linux-musl"
export LIBCC="$("$CC" $CFLAGS --print-libgcc-file-name)"
./configure --prefix= --target=$arch
make -j "$NUM_CPUS" install DESTDIR="$SCRIPT_DIR/$RECIPE_NAME/$arch"

cd ..; rm -rf "./$pkg"
recipe_finish
