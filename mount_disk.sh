#!/bin/sh
set -e

cleanup() {
    set +e
    umount "$mnt"
    udisksctl loop-delete -b "$dev"
}

trap cleanup EXIT

# Create loop device for disk.
dev=$(udisksctl loop-setup -f disk.img | sed -E 's/^Mapped file .+ as ([^.]+).*$/\1/')

# Mount disk.
mnt=$(udisksctl mount -b "$dev" | sed -E 's/^Mounted .+ at ([^.]+).*$/\1/')

echo "Entering shell, use Ctrl-D when done."
(cd "$mnt" && $SHELL)
