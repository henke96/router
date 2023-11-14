#include "hc/hc.h"
#include "hc/debug.h"
#include "hc/math.c"
#include "hc/util.c"
#include "hc/compiler_rt/libc.c"
#include "hc/linux/linux.h"
#include "hc/linux/sys.c"
#include "hc/linux/debug.c"
#include "hc/linux/util.c"
#include "hc/linux/helpers/_start.c"
#include "hc/linux/helpers/sys_clone3_func.c"

#define buffer_DEVNAME_OFFSET 3084
static char buffer[4096] hc_ALIGNED(16);

static int32_t initialise(void) {
    if (sys_mount("", "/proc", "proc", 0, NULL) < 0) return -1;

    // Disable memory overcommit.
    int32_t fd = sys_openat(-1, "/proc/sys/vm/overcommit_memory", O_WRONLY, 0);
    if (fd < 0) return -2;
    int64_t num = sys_write(fd, "2", 1);
    if (sys_close(fd) != 0) return -3;
    if (num != 1) return -4;

    // Panic if we run out of memory anyway.
    fd = sys_openat(-1, "/proc/sys/vm/panic_on_oom", O_WRONLY, 0);
    if (fd < 0) return -5;
    num = sys_write(fd, hc_STR_COMMA_LEN("2"));
    if (sys_close(fd) != 0) return -6;
    if (num != 1) return -7;

    if (sys_mount("", "/sys", "sysfs", 0, NULL) < 0) return -8;
    if (sys_mount("", "/dev", "devtmpfs", 0, NULL) < 0) return -9;
    return 0;
}

// Stores name of device at `&buffer[buffer_DEVNAME_OFFSET]`.
static int32_t iterateDevices(uint32_t volumeId) {
    int32_t devFd = sys_openat(-1, "/dev", O_RDONLY, 0);
    if (devFd < 0) return -1;

    for (;;) {
        int64_t read = sys_getdents64(devFd, &buffer[0], 3072); // Leave 1024 bytes at the end.
        if (read == 0) break;
        if (read < 0) return -2;

        for (
            struct linux_dirent64 *current = (void *)&buffer[0];
            current != (void *)&buffer[read];
            current = (void *)current + current->d_reclen
        ) {
            struct statx statx;
            statx.stx_rdev_major = 0;
            if (sys_statx(devFd, &current->d_name[0], 0, 0, &statx) < 0) return -3;
            if (statx.stx_rdev_major == 0) continue;

            int64_t nameLen = util_cstrLen(&current->d_name[0]);
            char *buffer2 = &buffer[3072];

            // Check if it's a block device.
            hc_MEMMOVE(&buffer2[17], &current->d_name[0], (uint64_t)nameLen + 1);
            hc_MEMCPY(&buffer2[0], hc_STR_COMMA_LEN("/sys/class/block/"));
            if (sys_faccessat(-1, &buffer2[0], 0) != 0) continue;

            hc_MEMCPY(&buffer2[10], hc_STR_COMMA_LEN("  /dev")); // The slash ends up at `buffer_DEVNAME_OFFSET` (3072 + 10 + 2).

            // Read fat volume id and label to verify it's the correct one.
            int32_t fd = sys_openat(-1, &buffer2[12], O_RDONLY, 0);
            if (fd < 0) return -4;

            char fatBuffer[4] hc_ALIGNED(4);
            if (sys_pread64(fd, &fatBuffer[0], sizeof(fatBuffer), 0x27) != sizeof(fatBuffer)) return -5;
            if (sys_close(fd) != 0) return -6;

            // TODO: Make sure it's actually FAT, and check label as well.
            if (*(uint32_t *)&fatBuffer[0] != volumeId) continue;

            if (sys_close(devFd) != 0) return -7;
            return 0; // Found.
        }
    }
    if (sys_close(devFd) != 0) return -8;
    return 1; // Not found.
}

static int32_t mountDisk(void) {
    int32_t fd = sys_openat(-1, "/mnt/config/disk/dev", O_RDONLY, 0);
    if (fd == -ENOENT) return 0;
    if (fd < 0) return -1;
    int32_t numRead = 0;
    for (;;) {
        int32_t read = (int32_t)sys_read(fd, &buffer[numRead], (int32_t)sizeof(buffer) - numRead);
        if (read == 0) break;
        if (read < 0) return -2;
        numRead += read;
    }
    buffer[numRead] = '\0';
    if (sys_close(fd) != 0) return -3;
    if (sys_mkdirat(-1, "/disk", 0755) < 0) return -4;
    int32_t status;
    for (;;) {
        status = sys_mount(&buffer[0], "/disk", "ext4", MS_NOATIME, NULL);
        if (status == 0) break;
        if (status != -ENOENT) {
            debug_printNum("Failed to mount disk (", status, ")\n");
            return -5;
        }
        if (sys_clock_nanosleep(CLOCK_MONOTONIC, 0, &(struct timespec) { .tv_nsec = 100000000 }, NULL) != 0) return -6;
    }
    return 0;
}

static noreturn void router(hc_UNUSED void *arg) {
    const char *newArgv[] = { "/router", NULL };
    const char *newEnvp[] = { NULL };
    sys_execveat(-1, newArgv[0], &newArgv[0], &newEnvp[0], 0);
    sys_exit_group(1);
}

int32_t start(hc_UNUSED int32_t argc, hc_UNUSED char **argv, hc_UNUSED char **envp) {
    bool cleanExit = false;
    int32_t status = initialise();
    if (status < 0) {
        debug_printNum("Failed to initialise (", status, ")\n");
        goto halt;
    }

    // Parse volume id from bootloader.
    int32_t cmdlineFd = sys_openat(-1, "/proc/cmdline", O_RDONLY, 0);
    if (cmdlineFd < 0) goto halt;
    if (sys_read(cmdlineFd, &buffer[0], 8) != 8) goto halt;
    if (sys_close(cmdlineFd) != 0) goto halt;

    uint64_t volumeId;
    if (util_hexToUint(&buffer[0], 8, &volumeId) <= 0) goto halt;

    // Find and mount the boot filesystem.
    while ((status = iterateDevices((uint32_t)volumeId)) == 1) {
        // Try max 10 times per second.
        if (sys_clock_nanosleep(CLOCK_MONOTONIC, 0, &(struct timespec) { .tv_nsec = 100000000 }, NULL) != 0) goto halt;
    }
    if (status < 0) {
        debug_printNum("Failed to find boot filesystem (", status, ")\n");
        goto halt;
    }
    status = sys_mount(&buffer[buffer_DEVNAME_OFFSET], "/mnt", "msdos", MS_NOATIME, NULL);
    if (status == -EROFS) {
        status = sys_mount(&buffer[buffer_DEVNAME_OFFSET], "/mnt", "msdos", MS_RDONLY | MS_NOATIME, NULL);
    }
    if (status != 0) goto halt;

    status = mountDisk();
    if (status < 0) {
        debug_printNum("Failed mounting disk (", status, ")\n");
        goto halt_umount;
    }

    struct clone_args args = {
        .flags = CLONE_VM | CLONE_VFORK,
        .exit_signal = SIGCHLD,
        .stack = &buffer[0],
        .stack_size = sizeof(buffer)
    };
    status = sys_clone3_func(&args, sizeof(args), router, NULL);
    if (status < 0) goto halt_umount;

    // Wait for children.
    for (;;) {
        struct rusage rusage;
        rusage.ru_maxrss = 0;
        int32_t pid = sys_wait4(-1, &status, 0, &rusage);
        if (pid < 0) goto halt_umount;

        #define PID_END util_INT32_MAX_CHARS
        #define STATUS_END (PID_END + util_INT32_MAX_CHARS)
        #define MAXRSS_END (STATUS_END + util_INT64_MAX_CHARS)
        char *pidStr = util_intToStr(&buffer[PID_END], pid);
        char *statusStr = util_intToStr(&buffer[STATUS_END], status);
        char *maxRssStr = util_intToStr(&buffer[MAXRSS_END], rusage.ru_maxrss);
        struct iovec_const iov[] = {
            { hc_STR_COMMA_LEN("Pid ") },
            { pidStr, (int64_t)(&buffer[PID_END] - pidStr) },
            { hc_STR_COMMA_LEN(" exited (status=") },
            { statusStr, (int64_t)(&buffer[STATUS_END] - statusStr) },
            { hc_STR_COMMA_LEN(", maxRss=") },
            { maxRssStr, (int64_t)(&buffer[MAXRSS_END] - maxRssStr) },
            { hc_STR_COMMA_LEN(")\n") }
        };
        sys_writev(STDOUT_FILENO, &iov[0], hc_ARRAY_LEN(iov));

        // Currently we only have the router process, so shutdown if it exits.
        if (status == 0) cleanExit = true;
        goto halt_umount;
    }

    halt_umount:
    if (sys_faccessat(-1, "/disk", 0) == 0) {
        if (sys_umount2("/disk", 0) < 0) {
            sys_write(STDOUT_FILENO, hc_STR_COMMA_LEN("Failed to umount /disk\n"));
            cleanExit = false;
        }
    }
    if (sys_umount2("/mnt", 0) < 0) {
        sys_write(STDOUT_FILENO, hc_STR_COMMA_LEN("Failed to umount /mnt\n"));
        cleanExit = false;
    }
    halt:
    sys_sync();
    sys_reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, cleanExit ? LINUX_REBOOT_CMD_POWER_OFF : LINUX_REBOOT_CMD_HALT, NULL);
    return 0;
}
