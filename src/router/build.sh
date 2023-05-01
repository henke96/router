#!/bin/sh
set -e
script_dir="$(dirname "$0")"
root_dir="$script_dir/../.."
FLAGS="-Os" "$root_dir/hc/tools/build/elf.sh" "$script_dir" router
