#!/bin/sh --
set -e
script_dir="$(cd -- "${0%/*}/" && pwd)"
root_dir="$script_dir/../../.."

export FLAGS="-Wl,--no-entry $FLAGS"
"$root_dir/tools/build/wasm.sh" "$script_dir" openGl

"$root_dir/tools/htmlPacker/htmlPacker.sh" "$script_dir" _start.html openGl
