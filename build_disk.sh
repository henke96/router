#!/bin/sh --
# Usage: ./build_disk.sh [INSTALL_DEV]
#
# INSTALL_DEV Allows creating an installer image that will install to the
#             provided device name, or promt the user if empty string.
set -e
script_dir="$(cd -- "$(dirname -- "$0")" && pwd)"

"$script_dir/recipes/disk.sh"
cp "$script_dir/recipes/disk/disk.img" "$script_dir/"
cp "$script_dir/recipes/disk/debug.disk.img" "$script_dir/"

export PATH="$script_dir/recipes/host_mtools/bin:$PATH"
# Check if creating installer image or not.
if test $# -ge 1; then
    # Yes, create install file.
    echo -n "$1" | mcopy -i "$script_dir/disk.img" - ::/install
fi
