#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh ./host_bzip2.sh ./host_m4.sh ./host_zlib.sh"

URL="https://sourceware.org/elfutils/ftp/0.189/elfutils-0.189.tar.bz2"
SHA512="93a877e34db93e5498581d0ab2d702b08c0d87e4cafd9cec9d6636dfa85a168095c305c11583a5b0fb79374dd93bc8d0e9ce6016e6c172764bcea12861605b71"

recipe_start
./configure --prefix="$SCRIPT_DIR/$RECIPE_NAME" --disable-dependency-tracking --disable-demangler --disable-nls --without-valgrind --without-bzlib --without-lzma --without-zstd --without-libiconv-prefix --without-libintl-prefix --disable-debuginfod --disable-libdebuginfod --disable-symbol-versioning \
CFLAGS="-DFNM_EXTMATCH=0 -Wno-error -I$SCRIPT_DIR/host_zlib/include" LDFLAGS="-L$SCRIPT_DIR/host_zlib/lib -Wl,-rpath=$SCRIPT_DIR/$RECIPE_NAME/lib -Wl,-rpath=$SCRIPT_DIR/host_zlib/lib"
make -j "$NUM_CPUS" install
recipe_finish
