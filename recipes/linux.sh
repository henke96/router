#!/bin/sh --
set -e
script_dir="$(cd -- "$(dirname -- "$0")" && pwd)"
. "$script_dir/../hc/bootstrap/recipe.sh"

URL="https://www.kernel.org/pub/linux/kernel/v6.x/linux-6.1.48.tar.xz"
SHA256="c606cbd0353e677df6fae73cc16ba3c9244b98372ed7771d551024016f55ac31"
DEPENDENCIES="../hc/bootstrap/make.sh ../hc/bootstrap/xz.sh ./llvm.sh ./bc.sh ./flex.sh ./bison.sh ./elfutils.sh ./linux-firmware.sh"
FILE_DEPENDENCIES="files/linux/initramfs files/linux/.config files/linux/regulatory.db ../src/init/x86_64/init.elf ../src/router/x86_64/router.elf"

recipe_start
cp ../files/linux/.config .config
KBUILD_BUILD_TIMESTAMP="@" KBUILD_BUILD_USER="@" KBUILD_BUILD_HOST="@" ARCH=x86_64 LLVM=1 HOSTCFLAGS="-I$script_dir/elfutils/include" HOSTLDFLAGS="-rpath $script_dir/elfutils/lib -L$script_dir/elfutils/lib" make -j$NUMCPUS
mkdir ../linux
mv arch/x86/boot/bzImage ../linux/
recipe_finish
