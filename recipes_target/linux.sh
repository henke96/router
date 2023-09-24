#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh ../hc/bootstrap/xz.sh ../recipes_host/llvm.sh ../recipes_host/bc.sh ../recipes_host/flex.sh ../recipes_host/bison.sh ../recipes_host/elfutils.sh ./linux-firmware.sh ./init.sh ./router.sh ./hostapd.sh"

URL="https://www.kernel.org/pub/linux/kernel/v6.x/linux-6.1.48.tar.xz"
SHA256="c606cbd0353e677df6fae73cc16ba3c9244b98372ed7771d551024016f55ac31"
FILE_DEPENDENCIES="files/linux/initramfs files/linux/.config files/linux/regulatory.db files/hostapd/hostapd.conf"

recipe_start
mkdir ../$RECIPE_NAME

export KBUILD_BUILD_TIMESTAMP="1970-01-01" KBUILD_BUILD_USER="@" KBUILD_BUILD_HOST="@" ARCH=x86_64
export HOSTCFLAGS="-I$SCRIPT_DIR/../recipes_host/elfutils/include" HOSTLDFLAGS="-Wl,-rpath,$SCRIPT_DIR/../recipes_host/elfutils/lib -L$SCRIPT_DIR/../recipes_host/elfutils/lib"
llvm_tools="CC=clang LD=ld.lld AR=llvm-ar NM=llvm-nm STRIP=llvm-strip OBJCOPY=llvm-objcopy OBJDUMP=llvm-objdump READELF=llvm-readelf"
cp ../files/linux/.config .config

# Set reproducible timestamps for initramfs.
touch -t 197001010000.00 ../linux-firmware/iwlwifi-cc-a0-72.ucode
touch -t 197001010000.00 ../files/linux/regulatory.db
touch -t 197001010000.00 ../hostapd/hostapd
touch -t 197001010000.00 ../files/hostapd/hostapd.conf
touch -t 197001010000.00 ../init/$ARCH/init.elf
touch -t 197001010000.00 ../init/$ARCH/debug.init.elf
touch -t 197001010000.00 ../router/$ARCH/router.elf
touch -t 197001010000.00 ../router/$ARCH/debug.router.elf

# Build release image.
export DEBUG_PREFIX=
make -j "$NUM_CPUS" $llvm_tools HOSTCC="${CC:-cc}"
mv arch/x86/boot/bzImage "../$RECIPE_NAME/"

# Build debug image.
touch usr/gen_init_cpio
export DEBUG_PREFIX=debug.
make -j "$NUM_CPUS" $llvm_tools HOSTCC="${CC:-cc}"
mv arch/x86/boot/bzImage "../$RECIPE_NAME/debug.bzImage"
recipe_finish

