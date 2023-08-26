#!/bin/sh --
# Usage: ./build_disk.sh [INSTALL_DEV]
#
# INSTALL_DEV Allows creating an installer image that will install to the
#             provided device name, or promt the user if empty string.
set -e
script_dir="$(cd -- "$(dirname -- "$0")" && pwd)"

# Can be `debug` or `release`.
export BUILD_TYPE="${BUILD_TYPE:-debug}"

# Build LLVM.
"$script_dir/recipes/llvm.sh"
export LLVM="$script_dir/recipes/llvm"

# Build everything for initramfs.
"$script_dir/src/init/build.sh"
"$script_dir/src/router/build.sh"

# Build Linux.
"$script_dir/recipes/linux.sh"

# Build bootloader.
"$script_dir/src/bootloader/build.sh"

# Create disk.
export PATH="$script_dir/recipes/mtools/bin:$PATH"
dd if=/dev/zero of="$script_dir/disk.img" bs=1048576 count=8
mformat -i "$script_dir/disk.img" -N 0 -v ROUTER ::
mmd -i "$script_dir/disk.img" ::/efi
mmd -i "$script_dir/disk.img" ::/efi/boot
mcopy -i "$script_dir/disk.img" "$script_dir/src/bootloader/x86_64/bootloader.efi" ::/efi/boot/bootx64.efi

# Check if creating installer image or not.
if test $# -ge 1; then
    # Yes, create install file.
    echo -n "$1" | mcopy -i "$script_dir/disk.img" - ::/install
fi

# Generate a wireguard key. TODO: remove
mmd -i "$script_dir/disk.img" ::/config
mmd -i "$script_dir/disk.img" ::/config/wg
dd if=/dev/urandom bs=32 count=1 | mcopy -i "$script_dir/disk.img" - ::/config/wg/key
