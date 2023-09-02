#!/bin/sh --
set -e
script_dir="$(cd -- "$(dirname -- "$0")" && pwd)"
root_dir="$script_dir/../../.."

export LINK_LIBDL=1 LINK_LIBLOG=1
"$root_dir/tools/build/androidelf.sh" "$script_dir" hello elf