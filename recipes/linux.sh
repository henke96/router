#!/bin/sh --
set -ex
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh ../hc/bootstrap/xz.sh ./host_llvm.sh ./host_bc.sh ./host_flex.sh ./host_bison.sh ./host_elfutils.sh ./host_perl.sh" "./linux-firmware.sh ./init.sh ./router.sh ./hostapd.sh"

URL="https://www.kernel.org/pub/linux/kernel/v6.x/linux-6.6.tar.xz"
SHA512="458b2c34d46206f9b4ccbac54cc57aeca1eaecaf831bc441e59701bac6eadffc17f6ce24af6eadd0454964e843186539ac0d63295ad2cc32d112b60360c39a35"
FILE_DEPENDENCIES="files/linux/initramfs files/linux/.config files/linux/regulatory.db"

recipe_start
mkdir ../$RECIPE_NAME

export KBUILD_BUILD_TIMESTAMP="1970-01-01" KBUILD_BUILD_USER="@" KBUILD_BUILD_HOST="@" ARCH=x86_64
export HOSTCFLAGS="-I$SCRIPT_DIR/host_elfutils/include" HOSTLDFLAGS="-Wl,-rpath,$SCRIPT_DIR/host_elfutils/lib -L$SCRIPT_DIR/host_elfutils/lib"
llvm_tools="CC=clang LD=ld.lld"
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
make -j "$NUM_CPUS" $llvm_tools HOSTCC="$CC"
mv arch/x86/boot/bzImage "../$RECIPE_NAME/"

# Build debug image.
touch usr/gen_init_cpio
export DEBUG_PREFIX=debug.
make -j "$NUM_CPUS" $llvm_tools HOSTCC="$CC"
mv arch/x86/boot/bzImage "../$RECIPE_NAME/debug.bzImage"
recipe_finish

