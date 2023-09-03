#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
export DEBUG_PREFIX=debug.
. files/linux/base_recipe.sh
