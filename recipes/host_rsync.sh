#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh"

URL="https://download.samba.org/pub/rsync/src/rsync-3.2.7.tar.gz"
SHA256="4e7d9d3f6ed10878c58c5fb724a67dacf4b6aac7340b13e488fb2dc41346f2bb"

recipe_start
./configure --prefix="$SCRIPT_DIR/$RECIPE_NAME" --disable-debug --disable-md2man --disable-locale --disable-openssl --disable-xxhash --disable-zstd --disable-iconv-open --disable-iconv --disable-acl-support --disable-xattr-support --disable-lz4 --without-rrsync
make -j "$NUM_CPUS" install
recipe_finish
