#!/bin/sh
set -e
script_dir="$(dirname $0)"
flags="$(cat $script_dir/flags)"
CC="${CC:-clang}"
LD="${LD:-lld}"
"$CC" -I$script_dir/src $flags -target wasm32-unknown-unknown-unknown -fuse-ld="$LD" -Wl,--gc-sections "$@"
