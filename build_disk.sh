#!/bin/sh
# Usage: ./build_disk.sh [INSTALL_DEV]
#
# INSTALL_DEV Allows creating an installer image that will install to the
#             provided device name, or promt the user if empty string.
set -e

cleanup() {
    set +e
    rm -r initramfs
    umount mnt/
    losetup -d $dev
}
trap cleanup EXIT

TYPE="${TYPE:-release}"
LLVM="${LLVM:-1}"
cc="clang$LLVM"
objcopy="llvm-objcopy$LLVM"
if test "$LLVM" -eq 1; then
    cc="clang"
    objcopy="llvm-objcopy"
fi

# Build everything for initramfs.
CC=$cc OBJCOPY=$objcopy src/init/build.sh
CC=$cc OBJCOPY=$objcopy src/router/build.sh

# Create initramfs.
mkdir -p initramfs/proc
mkdir -p initramfs/sys
mkdir -p initramfs/mnt
mkdir -p initramfs/dev
mknod initramfs/dev/console c 5 1  # Needed by `console_on_rootfs()` in Linux.

cp src/init/$TYPE.bin initramfs/init
cp src/router/$TYPE.bin initramfs/router

# Build Linux.
(cd linux/linux* && KBUILD_BUILD_TIMESTAMP="@" KBUILD_BUILD_USER="@" KBUILD_BUILD_HOST="@" ARCH=x86_64 LLVM=$LLVM make -j)

# Build bootloader.
CC=$cc OBJCOPY=$objcopy src/bootloader/build.sh

# Create disk and loop device.
dd if=/dev/zero of=disk.img bs=1048576 count=8
dev="$(losetup --show -f disk.img)"

# Create disk filesystem.
mkfs.fat -F 12 -i 0 -n ROUTER $dev

# Mount the disk.
mkdir -p mnt
mount $dev mnt/
echo "Disk device is: $dev"

# Install kernel.
mkdir -p mnt/EFI/BOOT
cp src/bootloader/bootloader.efi mnt/EFI/BOOT/BOOTX64.EFI

# Check if creating installer image or not.
if test $# -ge 1; then
    # Yes, create install file.
    echo -n "$1" > mnt/install
else
    # No, generate a wireguard key.
    dd if=/dev/urandom of=mnt/wgkey bs=32 count=1
fi

echo "Entering shell. Use Ctrl-D when done."
$SHELL
