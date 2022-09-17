#include "hc/hc.h"
#include "hc/math.c"
#include "hc/util.c"
#include "hc/libc/small.c"
#include "hc/linux/linux.h"
#include "hc/linux/sys.c"
#include "hc/linux/util.c"
#include "hc/linux/debug.c"
#include "hc/linux/helpers/_start.c"

#define IMAGE_INSTALL_SIZE 0x800000 // 8 MiB

#define buffer_DEVNAME_OFFSET 3084
static char buffer[4096] hc_ALIGNED(8);

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
    num = sys_write(fd, "2", 1);
    if (sys_close(fd) != 0) return -6;
    if (num != 1) return -7;

    if (sys_mount("", "/sys", "sysfs", 0, NULL) < 0) return -8;
    if (sys_mount("", "/dev", "devtmpfs", 0, NULL) < 0) return -9;
    return 0;
}

// Stores name of device at `&buffer[buffer_DEVNAME_OFFSET]`.
// If `majorDev` is negative, it just prints available devices.
static int32_t iterateDevices(int32_t majorDev, uint32_t minorDev) {
    int32_t devFd = sys_openat(-1, "/dev", O_RDONLY, 0);
    if (devFd < 0) return -1;

    if (majorDev < 0) sys_write(STDOUT_FILENO, "Block devices:", 14);

    for (;;) {
        int64_t read = sys_getdents64(devFd, &buffer[0], 3072); // Leave 1024 bytes at the end.
        if (read == 0) break;
        if (read < 0) return -2;

        for (
            struct linux_dirent64 *current = (void *)&buffer[0];
            current != (void *)&buffer[read];
            current = (void *)&((char *)current)[current->d_reclen]
        ) {
            struct statx statx;
            if (sys_statx(devFd, &current->d_name[0], 0, 0, &statx) < 0) return -3;
            if (statx.stx_rdev_major == 0) continue;
            if (majorDev > 0 && ((uint32_t)majorDev != statx.stx_rdev_major || minorDev != statx.stx_rdev_minor)) continue;

            int64_t nameLen = util_cstrLen(&current->d_name[0]);
            char *buffer2 = &buffer[3072];

            // Check if it's a block device.
            hc_MEMMOVE(&buffer2[17], &current->d_name[0], (uint64_t)nameLen + 1);
            hc_MEMCPY(&buffer2[0], "/sys/class/block/", 17);
            if (sys_faccessat(-1, &buffer2[0], 0) != 0) continue;

            hc_MEMCPY(&buffer2[10], "  /dev", 6); // The slash ends up at `buffer_DEVNAME_OFFSET` (3072 + 10 + 2).

            if (majorDev < 0) { // Only print.
                sys_write(STDOUT_FILENO, &buffer2[10], 6 + 1 + nameLen);
                continue;
            }

            // Read fat volume id and label to verify it's the correct one.
            int32_t fd = sys_openat(-1, &buffer2[12], O_RDONLY, 0);
            if (fd < 0) return -4;

            char fatBuffer[10] hc_ALIGNED(2);
            if (sys_pread64(fd, &fatBuffer[0], sizeof(fatBuffer), 0x27) != sizeof(fatBuffer)) return -5;
            if (sys_close(fd) != 0) return -6;

            if (*(uint16_t *)&fatBuffer[0] != majorDev) continue; // Check major device id.
            if (*(uint16_t *)&fatBuffer[2] != minorDev) continue; // Check minor device id.
            if (hc_MEMCMP(&fatBuffer[4], "ROUTER", 6) != 0) continue; // Check label.

            if (sys_close(devFd) != 0) return -7;
            return 0; // Found.
        }
    }
    if (sys_close(devFd) != 0) return -8;
    return 1; // Not found.
}

static int32_t handleInstallation(void) {
    // Check if /mnt/install exists.
    int32_t installFd = sys_openat(-1, "/mnt/install", O_RDONLY, 0);
    if (installFd < 0) {
        if (installFd == -ENOENT) return 0;
        return -1;
    }

    // Open source fd (boot device).
    int32_t sourceFd = sys_openat(-1, &buffer[buffer_DEVNAME_OFFSET], O_RDONLY, 0);
    if (sourceFd < 0) return -2;

    // Read destination device name and open destFd.
    int64_t numRead = sys_read(installFd, &buffer[buffer_DEVNAME_OFFSET], sizeof(buffer) - buffer_DEVNAME_OFFSET - 1);
    while (numRead <= 1) {
        if (iterateDevices(-1, 0) < 0) return -3;
        sys_write(STDOUT_FILENO, "\nInstall on: ", 13);
        numRead = sys_read(STDIN_FILENO, &buffer[buffer_DEVNAME_OFFSET], sizeof(buffer) - buffer_DEVNAME_OFFSET - 1);
        if (numRead < 0) return -4;
    }
    if (buffer[buffer_DEVNAME_OFFSET + numRead - 1] == '\n') --numRead; // Ignore trailing newline.
    buffer[buffer_DEVNAME_OFFSET + numRead] = '\0';
    if (sys_close(installFd) < 0) return -5;

    // Unmount boot filesystem.
    if (sys_umount2("/mnt", 0) < 0) return -6;

    // Open destination fd (install device), and calculate volume id.
    int32_t destFd = sys_openat(-1, &buffer[buffer_DEVNAME_OFFSET], O_WRONLY, 0);
    if (destFd < 0) return -7;
    struct statx statx;
    if (sys_statx(destFd, "", AT_EMPTY_PATH, 0, &statx) != 0) return -8;
    uint32_t volumeId = (statx.stx_rdev_minor << 16) | statx.stx_rdev_major;

    // Perform installation.
    for (uint64_t totalWritten = 0; totalWritten < IMAGE_INSTALL_SIZE;) {
        int64_t read = sys_read(sourceFd, &buffer[0], buffer_DEVNAME_OFFSET);
        if (read <= 0) return -9;

        // Rewrite volume id.
        if (math_RANGES_OVERLAP(0x27, 0x27 + 4, totalWritten, totalWritten + (uint64_t)read)) {
            uint64_t volumeIdOffset = 0x27 - totalWritten;
            for (uint64_t i = 0; i < 4; ++i) {
                if (volumeIdOffset + i < (uint64_t)read) hc_MEMCPY(&buffer[volumeIdOffset + i], (char *)&volumeId + i, 1);
            }
        }

        int64_t written = sys_write(destFd, &buffer[0], read);
        if (written != read) return -10;
        totalWritten += (uint64_t)written;
    }
    if (sys_close(sourceFd) != 0) return -11;
    if (sys_close(destFd) != 0) return -12;

    // Mount destination filesystem.
    if (sys_mount(&buffer[buffer_DEVNAME_OFFSET], "/mnt", "msdos", MS_NOATIME, NULL) != 0) return -13;

    // Remove installation file.
    if (sys_unlinkat(-1, "/mnt/install", 0) != 0) return -14;

    // Generate wireguard key file if it doesn't exist.
    if (sys_faccessat(-1, "/mnt/wgkey", 0) != 0) {
        uint8_t key[32];
        if (sys_getrandom(&key, sizeof(key), 0) != sizeof(key)) return -15;
        int32_t keyFd = sys_openat(-1, "/mnt/wgkey", O_WRONLY | O_CREAT | O_EXCL, S_IRUSR);
        if (keyFd < 0) return -16;
        if (sys_write(keyFd, &key[0], sizeof(key)) != sizeof(key)) return -17;
        if (sys_close(keyFd) != 0) return -18;
    }
    return 1; // Success!
}

int32_t main(int32_t argc, char **argv) {
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
    while ((status = iterateDevices(volumeId & 0xFFFF, (uint32_t)(volumeId >> 16))) == 1) {
        // Try max 10 times per second.
        if (sys_clock_nanosleep(CLOCK_MONOTONIC, 0, &(struct timespec) { .tv_nsec = 100000000 }, NULL) != 0) goto halt;
    }
    if (status < 0) {
        debug_printNum("Failed to find boot filesystem (", status, ")\n");
        goto halt;
    }
    if (sys_mount(&buffer[buffer_DEVNAME_OFFSET], "/mnt", "msdos", MS_NOATIME, NULL) != 0) goto halt;

    status = handleInstallation();
    if (status != 0) {
        if (status < 0) debug_printNum("Installation failed (", status, ")\n");
        else cleanExit = true; // status == 1
        goto halt_umount;
    }

    struct clone_args args = {
        .flags = CLONE_VM | CLONE_VFORK,
        .exit_signal = SIGCHLD
    };
    status = sys_clone3(&args);
    if (status == 0) {
        const char *newArgv[] = { "/router", NULL };
        sys_execveat(-1, newArgv[0], &newArgv[0], (const char **)util_getEnvp(argc, argv), 0);
        return 1; // Let _start exit the child.
    }
    if (status < 0) goto halt_umount;

    // Wait for children.
    for (;;) {
        struct rusage rusage;
        int32_t pid = sys_wait4(-1, &status, 0, &rusage);
        if (pid < 0) goto halt_umount;

        #define PID_END util_INT32_MAX_CHARS
        #define STATUS_END (PID_END + util_INT32_MAX_CHARS)
        #define MAXRSS_END (STATUS_END + util_INT64_MAX_CHARS)
        char *pidStr = util_intToStr(&buffer[PID_END], pid);
        char *statusStr = util_intToStr(&buffer[STATUS_END], status);
        char *maxRssStr = util_intToStr(&buffer[MAXRSS_END], rusage.ru_maxrss);
        sys_writev(STDOUT_FILENO, (struct iovec[7]) {
            { .iov_base = "Pid ", .iov_len = 4 },
            { .iov_base = pidStr, .iov_len = (int64_t)(&buffer[PID_END] - pidStr) },
            { .iov_base = " exited (status=", .iov_len = 16 },
            { .iov_base = statusStr, .iov_len = (int64_t)(&buffer[STATUS_END] - statusStr) },
            { .iov_base = ", maxRss=", .iov_len = 9 },
            { .iov_base = maxRssStr, .iov_len = (int64_t)(&buffer[MAXRSS_END] - maxRssStr) },
            { .iov_base = ")\n", .iov_len = 2 }
        }, 7);

        // Currently we only have the router process, so shutdown if it exits.
        if (status == 0) cleanExit = true;
        goto halt_umount;
    }

    halt_umount:
    if (sys_umount2("/mnt", 0) < 0) {
        sys_write(STDOUT_FILENO, "Failed to umount /mnt\n", 22);
        cleanExit = false;
    }
    halt:
    sys_sync();
    sys_reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, cleanExit ? LINUX_REBOOT_CMD_POWER_OFF : LINUX_REBOOT_CMD_HALT, NULL);
    return 0;
}