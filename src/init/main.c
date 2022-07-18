#include "hc/hc.h"
#include "hc/util.c"
#include "hc/libc/small.c"
#include "hc/linux/linux.h"
#include "hc/linux/sys.c"
#include "hc/linux/util.c"
#include "hc/linux/debug.c"
#include "hc/linux/helpers/_start.c"

static char buffer[4096];

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

    if (sys_mount("", "/sys", "sysfs", 0, NULL) < 0) return -6;
    return 0;
}

// If `majorDev` is 0, all devices are printed, otherwise the
// path to the device is written to `buffer`.
static int32_t iterateDevices(uint32_t majorDev, uint32_t minorDev) {
    int32_t devFd = sys_openat(-1, "/dev", O_RDONLY, 0);
    if (devFd < 0) return -1;

    if (majorDev == 0) sys_write(STDOUT_FILENO, "Block devices:", 14);

    for (;;) {
        int64_t read = sys_getdents64(devFd, &buffer[0], 3072); // Leave 1024 bytes at the end.
        if (read == 0) break;
        if (read < 0) return -2;

        char *end = &buffer[read];
        for (
            struct linux_dirent64 *current = (void *)&buffer[0];
            current != (void *)end;
            current = (void *)&((char *)current)[current->d_reclen]
        ) {
            struct statx statx;
            if (sys_statx(devFd, &current->d_name[0], 0, 0, &statx) < 0) return -3;
            if (statx.stx_rdev_major == 0) continue;

            int64_t nameLen = util_cstrLen(&current->d_name[0]);

            // Create path to the device in sysfs.
            hc_MEMMOVE(&end[17], &current->d_name[0], (uint64_t)nameLen + 1);
            hc_MEMCPY(&end[0], "/sys/class/block/", 17);

            // Continue if it doesn't exist (means it isn't a block device).
            if (sys_faccessat(-1, &end[0], 0) != 0) continue;

            if (majorDev == 0) { // Just print.
                // Don't print partitions.
                hc_MEMCPY(&end[17 + nameLen], "/partition", 11);
                if (sys_faccessat(-1, &end[0], 0) == 0) continue;

                sys_writev(STDOUT_FILENO, (struct iovec[2]) {
                    { .iov_base = "  /dev/", .iov_len = 7 },
                    { .iov_base = &end[17], .iov_len = nameLen }
                }, 2);
            } else if (majorDev == statx.stx_rdev_major && minorDev == statx.stx_rdev_minor) {
                // Found! Follow the sysfs symlink for the partition, and disk device name will be parent folder name.
                int32_t num = sys_readlinkat(-1, &end[0], &buffer[0], 4096);
                if (num <= 0 || num >= 4096) return -4;

                // Find last and second last slash characters. Replace last slash with \0 for convenience.
                char *lastSlash = &buffer[num - 1];
                while (*lastSlash != '/') --lastSlash;
                *lastSlash = '\0';
                char *secondLastSlash = lastSlash - 1;
                while (*secondLastSlash != '/') --secondLastSlash;

                // Construct final null-terminated device path.
                hc_MEMMOVE(&buffer[4], secondLastSlash, (uint64_t)(lastSlash - secondLastSlash) + 1);
                hc_MEMCPY(&buffer[0], "/dev", 4);
                goto found;
            }
        }
    }
    if (majorDev > 0) return -5; // Not found.
    found:;
    if (sys_close(devFd) < 0) return -6;
    return 0;
}

static int32_t handleInstallation(void) {
    // Check if /install exists.
    int32_t fd = sys_openat(-1, "/install", O_RDONLY, 0);
    if (fd < 0) {
        if (fd == -ENOENT) return 0;
        return -1;
    }

    // Read destination device name and open destFd.
    int64_t numRead = sys_read(fd, &buffer[0], sizeof(buffer) - 1);
    if (numRead == 0) {
        if (iterateDevices(0, 0) < 0) return -2;
        sys_write(STDOUT_FILENO, "\nInstall on: ", 13);
        numRead = sys_read(STDIN_FILENO, &buffer[0], sizeof(buffer));
    }
    if (numRead <= 0) return -3;
    if (buffer[numRead - 1] == '\n') --numRead; // Ignore trailing newline.
    buffer[numRead] = '\0';
    if (sys_close(fd) < 0) return -4;

    int32_t destFd = sys_openat(-1, &buffer[0], O_WRONLY, 0);
    if (destFd < 0) return -5;

    // Open a fd to /sys/class/block/<dest-device> for later. Also checks that it's a block device.
    hc_MEMMOVE(&buffer[17], &buffer[5], (uint64_t)util_cstrLen(&buffer[5]) + 1);
    hc_MEMCPY(&buffer[0], "/sys/class/block/", 17);
    int32_t destSysFd = sys_openat(-1, &buffer[0], O_RDONLY, 0);
    if (destSysFd < 0) return -6;

    // Find root drive and open sourceFd.
    struct statx statx;
    if (sys_statx(-1, "/", 0, 0, &statx) < 0) return -7;
    if (iterateDevices(statx.stx_dev_major, statx.stx_dev_minor) < 0) return -8;

    int32_t sourceFd = sys_openat(-1, &buffer[0], O_RDONLY, 0);
    if (sourceFd < 0) return -9;

    // Perform installation.
    for (;;) {
        int64_t read = sys_read(sourceFd, &buffer[0], sizeof(buffer));
        if (read < 0) return -10;
        if (read == 0) break;
        int64_t written = sys_write(destFd, &buffer[0], read);
        if (written != read) return -11;
    }
    if (sys_close(sourceFd) != 0) return -12;
    sys_sync(); // Make sure everything is written. Might not be needed.

    // Re-read partition table on dest drive.
    if (sys_ioctl(destFd, BLKRRPART, NULL) != 0) return -13;
    if (sys_close(destFd) != 0) return -14;

    // Find the new partition and mount it at /mnt.
    for (;;) {
        int64_t read = sys_getdents64(destSysFd, &buffer[0], 3072); // Leave 1024 bytes at the end.
        if (read == 0) break;
        if (read < 0) return -15;

        char *end = &buffer[read];
        hc_MEMCPY(&end[0], "/dev/", 5);
        for (
            struct linux_dirent64 *current = (void *)&buffer[0];
            current != (void *)end;
            current = (void *)&((char *)current)[current->d_reclen]
        ) {
            int64_t nameLen = util_cstrLen(&current->d_name[0]);

            // Find subdirectory with partition file. Don't need to parse it since we only have one partition.
            hc_MEMMOVE(&end[5], &current->d_name[0], (uint64_t)nameLen);
            hc_MEMCPY(&end[5 + nameLen], "/partition", 11);
            if (sys_faccessat(destSysFd, &end[5], 0) != 0) continue;
            if (sys_close(destSysFd) != 0) return -16;

            // Mount it!
            end[5 + nameLen] = '\0';
            if (sys_mount(&end[0], "/mnt", "msdos", MS_NOATIME, NULL) < 0) return -17;
            goto found;
        }
    }
    return -18; // Not found.

    found:;
    // Remove the /install file.
    if (sys_unlinkat(-1, "/mnt/install", 0) != 0) return -19;

    if (sys_umount2("/mnt", 0) < 0) return -20;
    return 1; // Success!
}

int32_t main(int32_t argc, char **argv) {
    bool cleanExit = false;
    int32_t status = initialise();
    if (status < 0) {
        debug_printNum("Failed to initialise (", status, ")\n");
        goto halt;
    }

    status = handleInstallation();
    if (status != 0) {
        if (status < 0) debug_printNum("Installation failed (", status, ")\n");
        else cleanExit = true; // status == 1
        goto halt;
    }

    // Remount root fs as writable.
    status = sys_mount(NULL, "/", NULL, MS_REMOUNT | MS_NOATIME, NULL);
    if (status < 0) {
        debug_printNum("Failed to remount / (", status, ")\n");
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
        goto halt;
    }

    halt:
    if (sys_umount2("/", 0) < 0) {
        sys_write(STDOUT_FILENO, "Failed to umount /\n", 20);
        cleanExit = false;
    }
    sys_sync();
    sys_reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, cleanExit ? LINUX_REBOOT_CMD_POWER_OFF : LINUX_REBOOT_CMD_HALT, NULL);
    return 0;
}