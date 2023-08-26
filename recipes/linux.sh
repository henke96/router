#!/bin/sh --
set -e
script_dir="$(cd -- "$(dirname -- "$0")" && pwd)"
. "$script_dir/../hc/bootstrap/recipe.sh"

URL="https://www.kernel.org/pub/linux/kernel/v6.x/linux-6.1.27.tar.xz"
SHA256="c2b74b96dd3d0cc9f300914ef7c4eef76d5fac9de6047961f49e69447ce9f905"
DEPENDENCIES="../hc/bootstrap/make.sh ../hc/bootstrap/xz.sh ./llvm.sh ./bc.sh ./flex.sh ./bison.sh ./elfutils.sh ./linux-firmware.sh"
FILE_DEPENDENCIES="files/linux/initramfs files/linux/.config files/linux/regulatory.db ../src/init/x86_64/init.elf ../src/router/x86_64/router.elf"

recipe_start
cp ../files/linux/.config .config
KBUILD_BUILD_TIMESTAMP="@" KBUILD_BUILD_USER="@" KBUILD_BUILD_HOST="@" ARCH=x86_64 LLVM=1 HOSTCFLAGS="-I$script_dir/elfutils/include" HOSTLDFLAGS="-rpath $script_dir/elfutils/lib -L$script_dir/elfutils/lib" make -j$NUMCPUS
mkdir ../linux
mv arch/x86/boot/bzImage ../linux/
recipe_finish
