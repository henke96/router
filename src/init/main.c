#include "hc/hc.h"
#include "hc/util.c"
#include "hc/libc/small.c"
#include "hc/linux/linux.h"
#include "hc/linux/sys.c"
#include "hc/linux/util.c"
#include "hc/linux/helpers/_start.c"

static int32_t initialise(void) {
    if (sys_mount("", "/proc", "proc", 0, NULL) < 0) return -1;

    // Disable memory overcommit.
    int32_t fd = sys_openat(-1, "/proc/sys/vm/overcommit_memory", O_WRONLY, 0);
    if (fd < 0) return -2;
    int64_t num = sys_write(fd, "2", 1);
    sys_close(fd);
    if (num != 1) return -3;

    // Panic if we run out of memory anyway.
    fd = sys_openat(-1, "/proc/sys/vm/panic_on_oom", O_WRONLY, 0);
    if (fd < 0) return -4;
    num = sys_write(fd, "2", 1);
    sys_close(fd);
    if (num != 1) return -5;
    return 0;
}

int32_t main(int32_t argc, char **argv) {
    int32_t status = initialise();
    if (status < 0) {
        char buffer[util_INT32_MAX_CHARS];
        char *numStr = util_intToStr(&buffer[util_INT32_MAX_CHARS], status);
        sys_writev(STDOUT_FILENO, (struct iovec[3]) {
            { .iov_base = "Failed to initialise (", .iov_len = 22 },
            { .iov_base = numStr, .iov_len = (int64_t)(&buffer[util_INT32_MAX_CHARS] - numStr) },
            { .iov_base = ")\n", .iov_len = 2 }
        }, 3);
        goto halt;
    }

    struct clone_args args = {
        .flags = CLONE_VM | CLONE_VFORK,
        .exit_signal = SIGCHLD
    };
    status = sys_clone3(&args);
    if (status == 0) {
        const char *newArgv[] = { "/bin/router", NULL };
        sys_execveat(-1, newArgv[0], &newArgv[0], (const char **)util_getEnvp(argc, argv), 0);
        return 1; // Let _start exit the child.
    }
    if (status < 0) goto halt;

    // Wait for children.
    for (;;) {
        struct rusage rusage;
        int32_t pid = sys_wait4(-1, &status, 0, &rusage);
        if (pid < 0) goto halt;

        char pidBuffer[util_INT32_MAX_CHARS];
        char statusBuffer[util_INT32_MAX_CHARS];
        char maxRssBuffer[util_INT64_MAX_CHARS];
        char *pidStr = util_intToStr(&pidBuffer[util_INT32_MAX_CHARS], pid);
        char *statusStr = util_intToStr(&statusBuffer[util_INT32_MAX_CHARS], status);
        char *maxRssStr = util_intToStr(&maxRssBuffer[util_INT64_MAX_CHARS], rusage.ru_maxrss);
        sys_writev(STDOUT_FILENO, (struct iovec[7]) {
            { .iov_base = "Pid ", .iov_len = 4 },
            { .iov_base = pidStr, .iov_len = (int64_t)(&pidBuffer[util_INT32_MAX_CHARS] - pidStr) },
            { .iov_base = " exited (status=", .iov_len = 16 },
            { .iov_base = statusStr, .iov_len = (int64_t)(&statusBuffer[util_INT32_MAX_CHARS] - statusStr) },
            { .iov_base = ", maxRss=", .iov_len = 9 },
            { .iov_base = maxRssStr, .iov_len = (int64_t)(&maxRssBuffer[util_INT64_MAX_CHARS] - maxRssStr) },
            { .iov_base = ")\n", .iov_len = 2 }
        }, 7);
    }

    halt:
    if (sys_umount2("/", 0) < 0) sys_write(STDOUT_FILENO, "Failed to umount /\n", 20);
    sys_sync();
    sys_reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_HALT, NULL);
    return 0;
}