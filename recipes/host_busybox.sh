#!/bin/sh --
set -eax
cd -- "${0%/*}/"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh ./host_bzip2.sh" ""

pkg="busybox-1.36.1"
URL="https://busybox.net/downloads/$pkg.tar.bz2"
SHA512="8c0c754c9ae04b5e6b23596283a7d3a4ef96225fe179f92d6f6a99c69c0caa95b1aa56c267f52d7c807f6cc69e1f0b7dd29a8ac624098f601738f8c0c57980d4"
FILE_DEPENDENCIES="files/host_busybox/.config"

recipe_start
rm -rf "./$pkg"; bzip2 -d -c "$DOWNLOAD" | tar xf -; cd "./$pkg"

cp ../files/host_busybox/.config .
make -j "$NUM_CPUS" HOSTCC="$CC" CC="$CC"
make -j "$NUM_CPUS" install HOSTCC="$CC" CC="$CC"

cd ..; rm -rf "./$pkg"
recipe_finish
