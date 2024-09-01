#include "hc/hc.h"
#include "hc/debug.h"
#include "hc/util.c"
#include "hc/compilerRt/mem.c"
#include "hc/linux/linux.h"
#include "hc/linux/sys.c"
#include "hc/linux/debug.c"
#include "hc/linux/util.c"
#include "hc/linux/helpers/_start.c"

int32_t start(int32_t argc, char **argv, hc_UNUSED char **envp) {
    if (argc < 2 || argc > 3) {
        sys_write(1, hc_STR_COMMA_LEN("Usage: bootstrap PATH [DEVICE]\n"));
        return 1;
    }

    // TODO: resolve absolute path
    char *path = argv[1];
    int64_t pathLen = util_cstrLen(path);
    if (pathLen > (PATH_MAX - (int64_t)hc_STR_LEN("/downloads\0"))) return 1;

    if (sys_setsid() < 0) return 1;
    if (sys_ioctl(0, TIOCSCTTY, 0) < 0) return 1;
    if (argc == 3 && sys_mount(argv[2], path, "iso9660", MS_RDONLY, NULL) < 0) return 1;

    static char buffer[
        hc_STR_LEN("PATH=\0") + PATH_MAX +
        hc_STR_LEN("DOWNLOADS=\0") + PATH_MAX +
        hc_STR_LEN("NUM_CPUS=\0") + util_UINT64_MAX_CHARS +
        PATH_MAX + hc_STR_LEN("/bin/sh\0")
    ];
    char *pos = &buffer[sizeof(buffer)];
    pos = hc_MEMCPY(pos - hc_STR_LEN("/bin\0"), hc_STR_COMMA_LEN("/bin\0"));
    pos = hc_MEMCPY(pos - pathLen, path, (uint64_t)pathLen);
    pos = hc_MEMCPY(pos - hc_STR_LEN("PATH="), hc_STR_COMMA_LEN("PATH="));
    char *pathEnv = pos;
    pos = hc_MEMCPY(pos - hc_STR_LEN("/downloads\0"), hc_STR_COMMA_LEN("/downloads\0"));
    pos = hc_MEMCPY(pos - pathLen, path, (uint64_t)pathLen);
    pos = hc_MEMCPY(pos - hc_STR_LEN("DOWNLOADS="), hc_STR_COMMA_LEN("DOWNLOADS="));
    char *downloadsEnv = pos;
    *--pos = '\0';
    pos = util_uintToStr(pos, (uint64_t)util_getCpuCount());
    pos = hc_MEMCPY(pos - hc_STR_LEN("NUM_CPUS="), hc_STR_COMMA_LEN("NUM_CPUS="));
    char *numCpusEnv = pos;
    pos = hc_MEMCPY(pos - hc_STR_LEN("/bin/sh\0"), hc_STR_COMMA_LEN("/bin/sh\0"));
    pos = hc_MEMCPY(pos - pathLen, path, (uint64_t)pathLen);
    char *shPath = pos;
    const char *shArgv[] = { shPath, NULL };
    const char *shEnvp[] = { "HOME=/", "TERM=linux", pathEnv, downloadsEnv, numCpusEnv, NULL };
    sys_execveat(-1, shArgv[0], &shArgv[0], &shEnvp[0], 0);
    return 1;
}
