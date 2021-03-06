## Building Linux
1. Install dependencies: sudo apt install make flex bison libelf-dev libssl-dev
2. Check linux.config for what version of Linux and clang/lld to use.
3. Download Linux from https://www.kernel.org/pub/linux/kernel/ and extract it under this folder.
   Alternatively, clone https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git and checkout the correct version.
4. Enter the folder and run:
```
cp ../linux.config .config
KBUILD_BUILD_TIMESTAMP="@" KBUILD_BUILD_USER="@" KBUILD_BUILD_HOST="@" ARCH=x86_64 LLVM=1 make -j <number-of-cores>
```
If your LLVM binaries have a suffix (ex: clang-14), use LLVM=-14 instead.
