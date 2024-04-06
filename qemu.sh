#!/bin/sh
# Usage: ./qemu.sh [MACVTAP_INTERFACE]
set -e
script_dir="$(cd -- "${0%/*}/" && pwd)"

test -n "$OUT" || { echo "Please set OUT"; exit 1; }
NUM_CPUS="${NUM_CPUS:-1}"

cleanup() {
    set +e
    ip link del qemu1
    ip link del qemu2
    ip link del qemu3
    ip link del qemu4
    ip link del qemu5
    ip link del qemu6
    ip link del qemu7
    ip link del qemu8
}

add_nic() {
    ip tuntap add mode tap qemu$1
    ip link set qemu$1 up
    qemu_args="$qemu_args -netdev tap,id=net$1,ifname=qemu$1,script=no,downscript=no -device e1000,netdev=net$1"
}

trap cleanup EXIT
trap "" INT # Make sure cleanup gets run on Ctrl-C.

# NIC 1, optionally a macvtap.
if test -n "$1"; then
    ip link add link $1 name qemu1 type macvtap
    qemu_args="-netdev tap,fd=3,id=net1 -device e1000,netdev=net1,mac=$(cat /sys/class/net/qemu1/address) 3<>/dev/tap$(cat /sys/class/net/qemu1/ifindex)"
else
    ip tuntap add mode tap qemu1
    qemu_args="-netdev tap,id=net1,ifname=qemu1,script=no,downscript=no -device e1000,netdev=net1"
fi
ip link set qemu1 up

add_nic 2
add_nic 3
add_nic 4
add_nic 5
add_nic 6
add_nic 7
add_nic 8

dd if=/dev/zero of="$OUT/disk1.img" bs=1048576 count=1000
mformat -i "$OUT/disk1.img" -F -N 0 -v DISK1 ::
mcopy -i "$OUT/disk1.img" -s "$OUT/out/efi" ::/
if test -d "$OUT/devtools"; then
    mcopy -i "$OUT/disk1.img" -s "$OUT/devtools/router.tar" ::/
    mcopy -i "$OUT/disk1.img" -s "$OUT/devtools/devtools.tar" ::/
fi
if test -d "$OUT/downloads"; then
    mcopy -i "$OUT/disk1.img" -s "$OUT/downloads/out" ::/
fi

test -f "$OUT/disk2.img" || dd if=/dev/zero of="$OUT/disk2.img" bs=1048576 count=10000

qemu-system-x86_64 \
-bios /usr/share/qemu/OVMF.fd \
-cpu host \
-smp "$NUM_CPUS" \
-m "${NUM_CPUS}G" \
-drive format=raw,file=$OUT/disk1.img \
-drive format=raw,file=$OUT/disk2.img \
-enable-kvm $qemu_args
