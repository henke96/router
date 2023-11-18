#!/bin/sh --
set -ex
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh" ""

URL="https://musl.libc.org/releases/musl-1.2.4.tar.gz"
SHA512="498ec5d7941194a8806f4d42f0f6d218c862996ef1398b737d0d06995e0b7a6574b240a48088f6b84016b14b2776fe463f829dcb11149cdfc1023d496b235c55"

recipe_start
arch=x86_64
make -j "$NUM_CPUS" install-headers ARCH=$arch prefix= DESTDIR="$SCRIPT_DIR/$RECIPE_NAME/$arch"
recipe_finish
