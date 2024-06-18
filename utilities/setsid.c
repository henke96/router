#include "hc/hc.h"
#include "hc/debug.h"
#include "hc/util.c"
#include "hc/compilerRt/mem.c"
#include "hc/linux/linux.h"
#include "hc/linux/sys.c"
#include "hc/linux/debug.c"
#include "hc/linux/helpers/_start.c"

int32_t start(int32_t argc, char **argv, char **envp) {
    if (argc < 2) {
        sys_write(1, hc_STR_COMMA_LEN("Usage: setsid PROGRAM [ARG]...\n"));
        return 1;
    }

    if (sys_setsid() < 0) return 1;
    if (sys_ioctl(0, TIOCSCTTY, 0) < 0) return 1;

    sys_execveat(AT_FDCWD, argv[1], (const char *const *)&argv[1], (const char *const *)envp, 0);
    return 1;
}
