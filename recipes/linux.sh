#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh ../hc/bootstrap/xz.sh ./host_llvm.sh ./host_bc.sh ./host_flex.sh ./host_bison.sh ./host_elfutils.sh ./linux-firmware.sh ./init.sh ./router.sh ./hostapd.sh"

URL="https://www.kernel.org/pub/linux/kernel/v6.x/linux-6.6.tar.xz"
SHA256="d926a06c63dd8ac7df3f86ee1ffc2ce2a3b81a2d168484e76b5b389aba8e56d0"
FILE_DEPENDENCIES="files/linux/initramfs files/linux/.config files/linux/regulatory.db"

recipe_start
mkdir ../$RECIPE_NAME

export KBUILD_BUILD_TIMESTAMP="1970-01-01" KBUILD_BUILD_USER="@" KBUILD_BUILD_HOST="@" ARCH=x86_64
export HOSTCFLAGS="-I$SCRIPT_DIR/host_elfutils/include" HOSTLDFLAGS="-Wl,-rpath,$SCRIPT_DIR/host_elfutils/lib -L$SCRIPT_DIR/host_elfutils/lib"
llvm_tools="CC=clang LD=ld.lld AR=llvm-ar NM=llvm-nm STRIP=llvm-strip OBJCOPY=llvm-objcopy OBJDUMP=llvm-objdump READELF=llvm-readelf"
cp ../files/linux/.config .config

# Set reproducible timestamps for initramfs.
touch -t 197001010000.00 ../linux-firmware/iwlwifi-cc-a0-72.ucode
touch -t 197001010000.00 ../files/linux/regulatory.db
touch -t 197001010000.00 ../hostapd/hostapd
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

