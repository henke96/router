#!/bin/sh
# Usage: ./build_disk.sh [INSTALL_DEV]
#
# INSTALL_DEV Allows creating an installer image that will install to the
#             provided device name, or promt the user if empty string.
set -e

cleanup() {
    set +e
    umount "$mnt"
    udisksctl loop-delete -b "$dev"
}
trap cleanup EXIT

# Build everything for initramfs.
src/init/build.sh
src/router/build.sh

# Build Linux.
(cd linux/linux* && KBUILD_BUILD_TIMESTAMP="@" KBUILD_BUILD_USER="@" KBUILD_BUILD_HOST="@" ARCH=x86_64 LLVM=${LLVM:-1} make -j$NUMCPUS)

# Build bootloader.
src/bootloader/build.sh

# Create disk.
dd if=/dev/zero of=disk.img bs=1048576 count=8

# Create disk filesystem.
mkfs.fat -F 12 -i 0 -n ROUTER disk.img

# Create loop device for disk.
dev=$(udisksctl loop-setup -f disk.img | sed -E 's/^Mapped file .+ as ([^.]+).*$/\1/')

# Mount disk.
mnt=$(udisksctl mount -b "$dev" | sed -E 's/^Mounted .+ at ([^.]+).*$/\1/')

# Install kernel.
mkdir -p "$mnt/EFI/BOOT"
cp src/bootloader/bootloader.efi "$mnt/EFI/BOOT/BOOTX64.EFI"

# Check if creating installer image or not.
if test $# -ge 1; then
    # Yes, create install file.
    echo -n "$1" > "$mnt/install"
else
    # No, generate a wireguard key.
    dd if=/dev/urandom of="$mnt/wgkey" bs=32 count=1
fi
