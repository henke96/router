## Prepare Linux Build
1. Install dependencies: sudo apt install make flex bison libelf-dev
2. Check linux.config for what version of Linux and LLVM to use.
3. Download Linux from https://www.kernel.org/pub/linux/kernel/ and extract it under this folder.
   Alternatively, clone https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git and checkout the correct version.
4. Enter the folder and run `cp ../linux.config .config`
5. Download Linux firmware from https://www.kernel.org/pub/linux/kernel/firmware/ and extract under this folder.
   Alternatively, clone https://git.kernel.org/pub/scm/linux/kernel/git/firmware/linux-firmware.git.
6. If your LLVM tools have a suffix, like clang-15, you need to run `LLVM=-15 ./build_disk.sh` later when building the disk image.
