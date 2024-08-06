#include "hc/hc.h"
#include "hc/debug.h"
#include "hc/util.c"
#include "hc/compilerRt/mem.c"
#include "hc/linux/linux.h"
#include "hc/linux/sys.c"
#include "hc/linux/debug.c"
#include "hc/linux/util.c"
#include "hc/linux/helpers/_start.c"
#include "hc/linux/helpers/sys_clone3_func.c"

static char buffer[65536] hc_ALIGNED(16);

struct run_args {
    const char **argv;
    const char **envp;
};
static char run_stack[4096] hc_ALIGNED(16);
static int32_t run_execErrno;
static noreturn void run(void *arg) {
    struct run_args *args = arg;
    run_execErrno = 0;
    run_execErrno = -sys_execveat(AT_FDCWD, args->argv[0], &args->argv[0], &args->envp[0], 0);
    sys_exit_group(0);
}

int32_t start(int32_t argc, char **argv, hc_UNUSED char **envp) {
    if (argc != 2) {
        sys_write(1, hc_STR_COMMA_LEN("Usage: devsetup DEVTOOLS_PATH\n"));
        return 1;
    }
    char *devtoolsPath = argv[1];
    int64_t devtoolsPathLen = util_cstrLen(devtoolsPath);
    #define MAX_DEVTOOLS_PATH_LEN (PATH_MAX - (int64_t)hc_STR_LEN("/devtools.tar\0"))
    if (devtoolsPathLen > MAX_DEVTOOLS_PATH_LEN) return 1;

    struct clone_args cloneArgs = {
        .flags = CLONE_VM | CLONE_VFORK,
        .exit_signal = SIGCHLD,
        .stack = &run_stack[0],
        .stack_size = sizeof(run_stack)
    };
    static_assert(sizeof(buffer) > MAX_DEVTOOLS_PATH_LEN + hc_STR_LEN("/devtools.tar\0"), "Buffer too small");

    // Extract devtools.tar and source.tar, if not already extracted.
    int32_t devtoolsPid = -1;
    if (sys_faccessat(AT_FDCWD, "devtools", 0) != 0) {
        hc_MEMCPY(&buffer[0], devtoolsPath, (uint64_t)devtoolsPathLen);
        hc_MEMCPY(&buffer[devtoolsPathLen], hc_STR_COMMA_LEN("/devtools.tar\0"));
        struct run_args devtoolsArgs = {
            .argv = &((const char *[]) { "untar", &buffer[0], NULL })[0],
            .envp = &((const char *[]) { NULL })[0]
        };
        devtoolsPid = sys_clone3_func(&cloneArgs, sizeof(cloneArgs), run, &devtoolsArgs);
        if (devtoolsPid < 0 || run_execErrno != 0) return 1;
    }

    int32_t sourcePid = -1;
    if (sys_faccessat(AT_FDCWD, "source", 0) != 0) {
        hc_MEMCPY(&buffer[devtoolsPathLen], hc_STR_COMMA_LEN("/source.tar\0"));
        struct run_args sourceArgs = {
            .argv = &((const char *[]) { "untar", &buffer[0], NULL })[0],
            .envp = &((const char *[]) { NULL })[0]
        };
        sourcePid = sys_clone3_func(&cloneArgs, sizeof(cloneArgs), run, &sourceArgs);
        if (sourcePid < 0 || run_execErrno != 0) return 1;
    }

    while (devtoolsPid >= 0 || sourcePid >= 0) {
        int32_t status = 0;
        int32_t pid = sys_wait4(-1, &status, 0, NULL);
        if (pid < 0 || status != 0) return 1;

        if (pid == devtoolsPid) devtoolsPid = -1;
        else if (pid == sourcePid) sourcePid = -1;
        else return 1;
    }

    // Start devtools/bin/sh
    if (sys_setsid() < 0) return 1;
    if (sys_ioctl(0, TIOCSCTTY, 0) < 0) return 1;

    static_assert(
        sizeof(buffer) > (
            hc_STR_LEN("DOWNLOADS=") + PATH_MAX +
            hc_STR_LEN("NUM_CPUS=") + util_UINT64_MAX_CHARS + hc_STR_LEN("\0")
        ),
        "Buffer too small"
    );
    char *pos = &buffer[sizeof(buffer)];
    pos = hc_MEMCPY(pos - hc_STR_LEN("/downloads\0"), hc_STR_COMMA_LEN("/downloads\0"));
    pos = hc_MEMCPY(pos - devtoolsPathLen, devtoolsPath, (uint64_t)devtoolsPathLen);
    pos = hc_MEMCPY(pos - hc_STR_LEN("DOWNLOADS="), hc_STR_COMMA_LEN("DOWNLOADS="));
    char *downloadsEnv = pos;
    *--pos = '\0';
    pos = util_uintToStr(pos, (uint64_t)util_getCpuCount());
    char *numCpusEnv = hc_MEMCPY(pos - hc_STR_LEN("NUM_CPUS="), hc_STR_COMMA_LEN("NUM_CPUS="));
    const char *shArgv[] = { "devtools/bin/sh", NULL };
    const char *shEnvp[] = { "HOME=/", "TERM=linux", "PATH=/devtools/bin", downloadsEnv, numCpusEnv, NULL };
    sys_execveat(AT_FDCWD, shArgv[0], &shArgv[0], &shEnvp[0], 0);
    return 1;
}
