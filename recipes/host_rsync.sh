#!/bin/sh --
set -ex
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh" ""

URL="https://download.samba.org/pub/rsync/src/rsync-3.2.7.tar.gz"
SHA512="c2afba11a352fd88133f9e96e19d6df80eb864450c83eced13a7faa23df947bccf2ef093f2101df6ee30abff4cbbd39ac802e9aa5f726e42c9caff274fad8377"

recipe_start
./configure --prefix="$SCRIPT_DIR/$RECIPE_NAME" --disable-debug --disable-md2man --disable-locale --disable-openssl --disable-xxhash --disable-zstd --disable-iconv-open --disable-iconv --disable-acl-support --disable-xattr-support --disable-lz4 --without-rrsync
make -j "$NUM_CPUS" install
recipe_finish
