## Prepare Linux Build
1. Install dependencies: sudo apt install make flex bison libelf-dev
2. Check linux.config for what version of Linux and LLVM to use.
3. Download Linux from https://www.kernel.org/pub/linux/kernel/ and extract it under this folder.
   Alternatively, clone https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git and checkout the correct version.
4. Enter the folder and run `cp ../linux.config .config`
5. If your LLVM tools have a suffix, like clang-14, you need to set `LLVM=-14` later when building the disk image.
