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

# Can be `debug` or `release`.
export BUILD_TYPE="${BUILD_TYPE:-debug}"

# Build everything for initramfs.
src/init/build.sh
src/router/build.sh

# Build Linux.
(
    for dir in linux/linux*; do
        case "$dir" in
            linux/linux-firmware*) continue;;
            linux/linux.config) continue;;
            *)
                cd "$dir"
                export LINUX_FIRMWARE=$(echo ../linux-firmware*) # For initramfs building.
                if test "$BUILD_TYPE" = "debug"; then
                    export DEBUG_PREFIX="debug."
                fi
                KBUILD_BUILD_TIMESTAMP="@" KBUILD_BUILD_USER="@" KBUILD_BUILD_HOST="@" ARCH=x86_64 LLVM=${LLVM:-1} make -j$NUMCPUS
                break;;
        esac
    done
)

# Build bootloader.
src/bootloader/build.sh

# Create disk.
dd if=/dev/zero of=disk.img bs=1048576 count=8

# Create disk filesystem.
mkfs.fat -F 12 -i 0 -n ROUTER disk.img

# Create loop device for disk.
dev=$(udisksctl loop-setup -f disk.img | sed -E 's/^Mapped file .+ as ([^.]+).*$/\1/')

# Mount disk.
mnt=$(udisksctl mount -b "$dev" 2>&1 | sed -E 's/^Mounted .+ at ([^.]+).*$/\1/' | sed -E 's/.+ already mounted at `(.+)'\''.*$/\1/')

# Install OS.
mkdir -p "$mnt/efi/boot"
cp src/bootloader/bootloader.efi "$mnt/efi/boot/bootx64.efi"

# Check if creating installer image or not.
if test $# -ge 1; then
    # Yes, create install file.
    echo -n "$1" > "$mnt/install"
fi

# Generate a wireguard key. TODO: remove
mkdir -p "$mnt/config/wg"
dd if=/dev/urandom of="$mnt/config/wg/key" bs=32 count=1
