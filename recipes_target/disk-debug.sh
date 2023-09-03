#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
DEBUG_PREFIX=debug.
DEBUG_SUFFIX=-debug
. files/disk/base_recipe.sh
