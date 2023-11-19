#!/bin/sh --
set -ex
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh ./host_bzip2.sh ./host_m4.sh" "./host_zlib.sh"

pkg="elfutils-0.189"
URL="https://sourceware.org/elfutils/ftp/0.189/$pkg.tar.bz2"
SHA512="93a877e34db93e5498581d0ab2d702b08c0d87e4cafd9cec9d6636dfa85a168095c305c11583a5b0fb79374dd93bc8d0e9ce6016e6c172764bcea12861605b71"
FILE_DEPENDENCIES="files/elfutils/dummylib.c files/elfutils/libintl.h"

recipe_start
rm -rf "./$pkg"; bzip2 -d -c "$DOWNLOAD" | tar xf -; cd "./$pkg"

# Trick configure script to avoid useless dependencies.
cc -shared -o ./libfts.so ../files/elfutils/dummylib.c
cp ./libfts.so ./libargp.so
cp ./libfts.so ./libobstack.so

./configure --prefix="$SCRIPT_DIR/$RECIPE_NAME" --disable-dependency-tracking --disable-demangler --disable-nls --without-valgrind --without-bzlib --without-lzma --without-zstd --without-libiconv-prefix --without-libintl-prefix --disable-debuginfod --disable-libdebuginfod --disable-symbol-versioning \
CFLAGS="-Wno-error -I$SCRIPT_DIR/host_zlib/include" LDFLAGS="-L`pwd` -L$SCRIPT_DIR/host_zlib/lib"
cp ../files/elfutils/libintl.h ./libelf/libintl.h
touch ./lib/libeu.a
make -C libelf -j "$NUM_CPUS" install

cd ..; rm -rf "./$pkg"
recipe_finish
