#!/bin/sh
script_dir="$(dirname $0)"
linux_out=$(echo $script_dir/../../linux/linux*/arch/x86/boot)
flags="-Wl,-subsystem,efi_application -I$linux_out -Os -s"
"$script_dir/../../hc/cc_pe.sh" $flags -S -o "$script_dir/bootloader.efi.s" "$script_dir/main.c"
"$script_dir/../../hc/cc_pe.sh" $flags -o "$script_dir/bootloader.efi" "$script_dir/main.c"
