static int32_t init(char **includePaths) {
    while (*includePaths != NULL) {
        int32_t fd = openat(AT_FDCWD, *includePaths, O_RDONLY, 0);
        if (fd < 0) return -1;

        if (allocator_resize(&htmlPacker_alloc, htmlPacker_alloc.size + (int64_t)sizeof(fd)) < 0) return -2;
        *((int32_t *)&htmlPacker_alloc.mem[htmlPacker_alloc.size] - 1) = fd;
        ++includePaths;
    }
    htmlPacker_buffer = &htmlPacker_alloc.mem[htmlPacker_alloc.size];
    return 0;
}

static void deinit(void) {
    for (int32_t *includePathFd = htmlPacker_alloc.mem; (char *)includePathFd != htmlPacker_buffer; ++includePathFd) {
        debug_CHECK(close(*includePathFd), RES == 0);
    }
}

static int32_t replaceWithFile(int64_t replaceIndex, int64_t replaceSize, char *path, int32_t pathLen, bool asBase64) {
    // Add null terminator to path.
    char *pathZ = &htmlPacker_alloc.mem[htmlPacker_alloc.size];
    if (allocator_resize(&htmlPacker_alloc, htmlPacker_alloc.size + (int64_t)pathLen + 1) < 0) return -1;
    hc_MEMCPY(pathZ, path, (uint64_t)pathLen);
    pathZ[pathLen] = '\0';

    // Find and open path relative to include paths.
    int32_t pathFd;
    for (int32_t *includePathFd = htmlPacker_alloc.mem; (char *)includePathFd != htmlPacker_buffer; ++includePathFd) {
        pathFd = openat(*includePathFd, pathZ, O_RDONLY, 0);
        if (pathFd >= 0) goto foundPath;
    }
    return -2;
    foundPath:;

    // Get file size.
    int32_t status;
    struct stat contentStat;
    contentStat.st_size = 0; // Make static analysis happy.
    if (fstatat(pathFd, "", &contentStat, AT_EMPTY_PATH) < 0) {
        status = -3;
        goto cleanup_pathFd;
    }
    int64_t contentSize = contentStat.st_size;

    // Resize buffer.
    int64_t insertSize = contentSize;
    if (asBase64) insertSize = base64_ENCODE_SIZE(contentSize);

    // Allocate an extra byte to be able to verify EOF when reading.
    int64_t newBufferSize = 1 + htmlPacker_bufferSize + (insertSize - replaceSize);
    if (allocator_resize(&htmlPacker_alloc, &htmlPacker_buffer[newBufferSize] - (char *)htmlPacker_alloc.mem) < 0) {
        status = -4;
        goto cleanup_pathFd;
    }

    // Move existing content to make room.
    hc_MEMMOVE(
        &htmlPacker_buffer[replaceIndex + insertSize],
        &htmlPacker_buffer[replaceIndex + replaceSize],
        (uint64_t)(htmlPacker_bufferSize - (replaceSize + replaceIndex))
    );
    htmlPacker_bufferSize = newBufferSize;

    // Read content into buffer, and verify EOF by attempting to read an extra byte.
    char *content = &htmlPacker_buffer[replaceIndex + insertSize - contentSize];
    if (util_readAll(pathFd, content, contentSize + 1) != contentSize) {
        status = -5;
        goto cleanup_pathFd;
    }

    // Convert to base 64 if requested. Can be done in place as we put content at end of insert gap.
    if (asBase64) base64_encode(&htmlPacker_buffer[replaceIndex], &content[0], contentSize);

    status = 0;
    cleanup_pathFd:
    debug_CHECK(close(pathFd), RES == 0);
    return status;
}

static int32_t writeToFile(char *path, char *content, int64_t contentSize) {
    int32_t fd = openat(AT_FDCWD, path, O_WRONLY | O_CREAT | O_TRUNC, 0664);
    if (fd < 0) return -1;

    int32_t status = util_writeAll(fd, content, contentSize);
    debug_CHECK(close(fd), RES == 0);
    return status;
}
