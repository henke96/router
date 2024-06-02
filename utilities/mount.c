#include "hc/hc.h"
#include "hc/compilerRt/mem.c"
#include "hc/linux/linux.h"
#include "hc/linux/sys.c"
#include "hc/linux/helpers/_start.c"

int32_t start(int32_t argc, char **argv, hc_UNUSED char **envp) {
    if (argc == 4 && sys_mount(argv[1], argv[2], argv[3], MS_NOATIME, NULL) == 0) return 0;
    if (argc == 2 && sys_umount2(argv[1], 0) == 0) return 0;
    sys_write(1, hc_STR_COMMA_LEN(
        "Mounting: mount /dev/sda1 /mnt vfat\n"
        "Unmounting: mount /mnt\n"
    ));
    return 1;
}
