#!/bin/sh --
set -e
script_dir="$(cd -- "$(dirname -- "$0")" && pwd)"
. "$script_dir/../hc/bootstrap/recipe.sh"

URL="https://www.kernel.org/pub/linux/kernel/v6.x/linux-6.1.48.tar.xz"
SHA256="c606cbd0353e677df6fae73cc16ba3c9244b98372ed7771d551024016f55ac31"
DEPENDENCIES="../hc/bootstrap/make.sh ../hc/bootstrap/xz.sh ../recipes_host/llvm.sh ../recipes_host/bc.sh ../recipes_host/flex.sh ../recipes_host/bison.sh ../recipes_host/elfutils.sh ./linux-firmware.sh ./init.sh ./router.sh"
FILE_DEPENDENCIES="files/linux/initramfs files/linux/.config files/linux/regulatory.db files/linux/build.sh"

recipe_start
cp ../files/linux/.config .config
export DEBUG_PREFIX=debug.
. ../files/linux/build.sh
mkdir ../linux-debug
mv arch/x86/boot/bzImage ../linux-debug/
recipe_finish
