#!/bin/sh --
# Usage: ./build_disk.sh [INSTALL_DEV]
#
# INSTALL_DEV Allows creating an installer image that will install to the
#             provided device name, or promt the user if empty string.
set -e
script_dir="$(cd -- "$(dirname -- "$0")" && pwd)"

# Can be `debug` or `release`.
export BUILD_TYPE="${BUILD_TYPE:-debug}"

if test "$BUILD_TYPE" = "debug"; then
    disk_recipe=disk-debug
elif test "$BUILD_TYPE" = "release"; then
    disk_recipe=disk
else
    echo "Invalid BUILD_TYPE"
    exit 1
fi
"$script_dir/recipes_target/$disk_recipe.sh"
cp "$script_dir/recipes_target/$disk_recipe/disk.img" "$script_dir/"

export PATH="$script_dir/recipes_host/mtools/bin:$PATH"
# Check if creating installer image or not.
if test $# -ge 1; then
    # Yes, create install file.
    echo -n "$1" | mcopy -i "$script_dir/disk.img" - ::/install
fi
