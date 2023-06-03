#!/bin/sh
set -e
script_dir="$(dirname "$0")"
root_dir="$script_dir/../.."

export NO_AARCH64=1 NO_RISCV64=1
"$root_dir/hc/tools/build/elf.sh" "$script_dir" router
