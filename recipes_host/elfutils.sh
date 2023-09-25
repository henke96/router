#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh ./bzip2.sh ./m4.sh ./zlib.sh"

URL="https://sourceware.org/elfutils/ftp/0.189/elfutils-0.189.tar.bz2"
SHA256="39bd8f1a338e2b7cd4abc3ff11a0eddc6e690f69578a57478d8179b4148708c8"

recipe_start
./configure --prefix="$SCRIPT_DIR/$RECIPE_NAME" --disable-dependency-tracking --disable-demangler --disable-nls --without-valgrind --without-bzlib --without-lzma --without-zstd --without-libiconv-prefix --without-libintl-prefix --disable-debuginfod --disable-libdebuginfod --disable-symbol-versioning \
CFLAGS="-Wno-error -I$SCRIPT_DIR/zlib/include" LDFLAGS="-L$SCRIPT_DIR/zlib/lib -Wl,-rpath=$SCRIPT_DIR/$RECIPE_NAME/lib -Wl,-rpath=$SCRIPT_DIR/zlib/lib"
make -j "$NUM_CPUS" install
recipe_finish
