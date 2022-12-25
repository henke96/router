#!/bin/sh
script_dir="$(dirname $0)"
"$script_dir/../../hc/examples/helper.sh" "$script_dir"

cp "$script_dir/$BUILD_TYPE.bin" "$script_dir/app.bin"
