#!/bin/sh
# Usage: ./create_disk.sh [INSTALL_COMMAND]
set -e

cleanup() {
    set +e
    umount mnt/
    losetup -d $dev
}

dd if=/dev/zero of=disk.img bs=1048576 count=16

dev="$(losetup --show -f disk.img)"
trap cleanup EXIT

# Create partitions and filesystems.
parted -s $dev \
mklabel gpt \
mkpart Primary 2048s 100% set 1 esp on

mkfs -t fat -F 16 ${dev}p1

# Mount the disk.
mkdir -p mnt/
mount ${dev}p1 mnt/
echo "Disk device is: $dev"

# Install kernel.
mkdir -p mnt/EFI/BOOT
cp linux/linux*/arch/x86/boot/bzImage mnt/EFI/BOOT/BOOTX64.EFI

# Install directories.
mkdir -p mnt/dev
mkdir -p mnt/proc
mkdir -p mnt/bin

# Install binaries.
cp src/init/release.bin mnt/bin/init
cp src/router/release.bin mnt/bin/router

if test -n "$1"
then
    eval "$1"
else
    echo "No install command given, entering shell. Use Ctrl-D when done."
    $SHELL
fi
