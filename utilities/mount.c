#include "hc/hc.h"
#include "hc/compilerRt/mem.c"
#include "hc/linux/linux.h"
#include "hc/linux/sys.c"
#include "hc/linux/helpers/_start.c"

int32_t start(int32_t argc, char **argv, hc_UNUSED char **envp) {
    if (argc < 4 || sys_mount(argv[1], argv[2], argv[3], MS_NOATIME, NULL) < 0) {
        sys_write(STDOUT_FILENO, hc_STR_COMMA_LEN("Usage: mount DEVICE MOUNTPOINT FSTYPE\n"));
        return 1;
    }
    return 0;
}
