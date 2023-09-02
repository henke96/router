#!/bin/sh --
set -e
script_dir="$(cd -- "$(dirname -- "$0")" && pwd)"
. "$script_dir/../hc/bootstrap/recipe.sh"

URL="https://sourceware.org/elfutils/ftp/0.189/elfutils-0.189.tar.bz2"
SHA256="39bd8f1a338e2b7cd4abc3ff11a0eddc6e690f69578a57478d8179b4148708c8"
DEPENDENCIES="../hc/bootstrap/make.sh ./bzip2.sh ./m4.sh ./zlib.sh"

recipe_start
./configure --prefix="$script_dir/elfutils" --disable-dependency-tracking --disable-demangler --disable-nls --without-valgrind --without-bzlib --without-lzma --without-zstd --without-libiconv-prefix --without-libintl-prefix --disable-debuginfod --disable-libdebuginfod --disable-symbol-versioning CFLAGS="-Wno-error -I$script_dir/zlib/include" LDFLAGS="-L$script_dir/zlib/lib"
make -j "$NUM_CPUS" install
recipe_finish
