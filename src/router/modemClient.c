#define modemClient_cmd_HEADER_SIZE 2
#define modemClient_cmd_CONSTANT 0
#define modemClient_cmd_START_SMS 1
#define modemClient_cmd_SEND_SMS 2

struct modemClient_cmd {
    uint8_t type;
    uint8_t dataSize;
    char data[];
};

struct modemClient_cmd_constant {
    uint8_t type;
    uint8_t dataSize;
    uint8_t cmdLength;
    char __pad[5];
    const char *cmd;
};

struct modemClient {
    const char *path;
    int32_t fd; // -1 if not connected.
    int32_t timerFd;
    int32_t bufferLength;
    int32_t commandQueueLength;
    char buffer[4096];
    char commandQueue[4096];

};

static hc_COLD void modemClient_init(struct modemClient *self, const char *path) {
    self->path = path;
    self->fd = -1;
    self->timerFd = sys_timerfd_create(CLOCK_MONOTONIC, 0);
    CHECK(self->timerFd, RES > 0);

    struct itimerspec timeout = { .it_value = { .tv_sec = 5 } };
    CHECK(sys_timerfd_settime(self->timerFd, 0, &timeout, NULL), RES == 0);

}

static void modemClient_disconnect(struct modemClient *self) {
    if (self->fd > 0) {
        sys_write(self->fd, "\x1b", 1); // Attempt to exit potential `> `-mode by sending escape.
        debug_CHECK(sys_close(self->fd), RES == 0);
        self->fd = -1;
        sys_write(STDOUT_FILENO, "AT disconnect\n", 14);
    }
}

static hc_COLD int32_t modemClient_processQueue(struct modemClient *self) {
    debug_ASSERT(self->commandQueueLength > 0);
    struct modemClient_cmd *cmd = (void *)&self->commandQueue[0];
    switch (cmd->type) {
        case modemClient_cmd_CONSTANT: {
            struct modemClient_cmd_constant *cmdConstant = (void *)&self->commandQueue[0];
            int64_t written = sys_write(self->fd, cmdConstant->cmd, cmdConstant->cmdLength);
            if (written != cmdConstant->cmdLength) return -1;
            break;
        }
        case modemClient_cmd_START_SMS: {
            struct iovec iov[] = {
                { .iov_base = "AT+CMGS=\"", .iov_len = 9 },
                { .iov_base = &cmd->data[0], .iov_len = cmd->dataSize },
                { .iov_base = "\"\r", .iov_len = 2 }
            };
            int64_t written = sys_writev(self->fd, &iov[0], hc_ARRAY_LEN(iov));
            if (written != 9 + cmd->dataSize + 2) return -1;
            break;
        }
        case modemClient_cmd_SEND_SMS: {
            struct iovec iov[] = {
                { .iov_base = &cmd->data[0], .iov_len = cmd->dataSize },
                { .iov_base = "\x1a", .iov_len = 1 }
            };
            int64_t written = sys_writev(self->fd, &iov[0], hc_ARRAY_LEN(iov));
            if (written != cmd->dataSize + 1) return -1;
            break;
        }
        default: hc_UNREACHABLE;
    }
    struct itimerspec timeout = { .it_value = { .tv_sec = 10 } };
    CHECK(sys_timerfd_settime(self->timerFd, 0, &timeout, NULL), RES == 0);
    return 0;
}

static hc_COLD int32_t modemClient_queueSmsPoll(struct modemClient *self) {
    struct modemClient_cmd_constant *cmd = (void *)&self->commandQueue[self->commandQueueLength];
    self->commandQueueLength += sizeof(*cmd);
    if (self->commandQueueLength >= (int32_t)sizeof(self->commandQueue)) return -1;
    cmd->type = modemClient_cmd_CONSTANT;
    cmd->dataSize = sizeof(*cmd) - modemClient_cmd_HEADER_SIZE;
    cmd->cmdLength = 23;
    cmd->cmd = "AT+CMGD=,3;+CMGL=\"ALL\"\r";
    if (self->commandQueueLength == sizeof(*cmd)) {
        if (modemClient_processQueue(self) < 0) return -1;
    }
    return 0;
}

static hc_COLD void modemClient_onTimerFd(struct modemClient *self, int32_t epollFd) {
    if (self->fd < 0) { // Not connected, try to connect.
        self->fd = sys_openat(-1, &self->path[0], O_RDWR | O_NOCTTY, 0);
        debug_ASSERT(self->fd > 0 || self->fd == -ENOENT);
        if (self->fd < 0) goto out_fail;

        // Disable weird tty features, except ignoring CR. See termios(3).
        struct termios termios = {0};
        if (sys_ioctl(self->fd, TCGETS, &termios) != 0) goto out_fail;
        termios.c_iflag = IGNCR;
        termios.c_oflag &= ~(OPOST | OLCUC | ONLCR | OCRNL | ONLRET);
        termios.c_cflag &= ~(CSIZE | PARENB);
        termios.c_cflag |= CS8;
        termios.c_lflag &= ~(ISIG | ICANON | ECHO | IEXTEN);
        if (sys_ioctl(self->fd, TCSETS, &termios) != 0) goto out_fail;

        // Send escape, enable echo, set sms to text mode, extended sms info.
        struct modemClient_cmd_constant *cmd = (void *)&self->commandQueue[0];
        cmd->type = modemClient_cmd_CONSTANT;
        cmd->dataSize = sizeof(*cmd) - modemClient_cmd_HEADER_SIZE;
        cmd->cmdLength = 21;
        cmd->cmd = "ATE0;+CMGF=1;+CSDH=1\r";
        self->commandQueueLength = sizeof(*cmd);
        self->bufferLength = 0;

        epollAdd(epollFd, self->fd);
    } else if (self->commandQueueLength > 0) goto out_fail; // Some command timed out.

    debug_CHECK(modemClient_queueSmsPoll(self), RES == 0); // Should impossibly fail here.
    if (modemClient_processQueue(self) == 0) return;

    out_fail:
    modemClient_disconnect(self);
    struct itimerspec timeout = { .it_value = { .tv_sec = 10 } };
    CHECK(sys_timerfd_settime(self->timerFd, 0, &timeout, NULL), RES == 0);
}

static void modemClient_onFd(struct modemClient *self) {
    int32_t lineStart = 0;

    int64_t read = sys_read(self->fd, &self->buffer[self->bufferLength], (int32_t)sizeof(self->buffer) - self->bufferLength);
    if (read <= 0) goto out_fail;
    self->bufferLength += read;

    int32_t lineLength;
    for (;; lineStart += lineLength + 1) {
        char *bufferLineStart = &self->buffer[lineStart];

        // Find line length.
        for (lineLength = 0; lineLength < self->bufferLength - lineStart; ++lineLength) {
            if (bufferLineStart[lineLength] == '\n') goto foundLine;
        }
        if (lineLength != 2 || bufferLineStart[0] != '>' || bufferLineStart[1] != ' ') goto out; // No line found.
        foundLine:

        // OK or `> `.
        if (
            lineLength == 2 &&
            (
                (bufferLineStart[0] == 'O' && bufferLineStart[1] == 'K') ||
                (bufferLineStart[0] == '>' && bufferLineStart[1] == ' ')
            )
        ) {
            struct modemClient_cmd *cmd = (void *)&self->commandQueue[0];
            if (self->commandQueueLength <= 0) goto out_fail;

            if (bufferLineStart[0] == '>') {
                if (cmd->type != modemClient_cmd_START_SMS) goto out_fail;
                --lineLength; // Compensate for missing newline..
            } else if (cmd->type == modemClient_cmd_START_SMS) goto out_fail;

            int32_t commandLength = modemClient_cmd_HEADER_SIZE + cmd->dataSize;
            self->commandQueueLength -= commandLength;
            hc_MEMMOVE(&self->commandQueue[0], &self->commandQueue[commandLength], (uint64_t)self->commandQueueLength);
            if (self->commandQueueLength > 0) {
                if (modemClient_processQueue(self) < 0) goto out_fail;
            }
            continue;
        }

        // Error.
        if (lineLength == 5 && hc_MEMCMP(&bufferLineStart[0], "ERROR", 5) == 0) {
            sys_write(STDOUT_FILENO, "AT error!\n", 10);
            goto out_fail;
        }

        // Read SMS.
        if (lineLength >= 7 && hc_MEMCMP(&bufferLineStart[0], "+CMGL: ", 7) == 0) {
            // Find end of slot number.
            int32_t slotNumberEnd = 7;
            for (; slotNumberEnd < lineLength; ++slotNumberEnd) {
                if (bufferLineStart[slotNumberEnd] == ',') goto foundSlotNumberEnd;
            }
            goto out_fail; // Not found.
            foundSlotNumberEnd:;

            // Parse whether the message is read or unread.
            int32_t unreadCheckIndex = slotNumberEnd + 6;
            if (unreadCheckIndex >= lineLength) goto out_fail;
            bool unread = bufferLineStart[unreadCheckIndex] == 'U'; // "REC UNREAD" or "REC READ"

            // Find start of sender number.
            int32_t senderNumberIndex = unreadCheckIndex + (unread ? 9 : 7);

            // Find end of sender number.
            int32_t senderNumberEnd = senderNumberIndex;
            for (; senderNumberEnd < lineLength; ++senderNumberEnd) {
                if (bufferLineStart[senderNumberEnd] == '"') goto foundSenderNumberEnd;
            }
            goto out_fail; // Not found.
            foundSenderNumberEnd:;

            // Find SMS length (by locating last comma).
            int32_t smsLengthIndex = lineLength;
            for (;;) { // We know there is atleast one comma in the line, so skip checks.
                if (bufferLineStart[smsLengthIndex - 1] == ',') goto foundSmsLength;
                --smsLengthIndex;
            }
            goto out_fail; // Not found.
            foundSmsLength:;

            uint64_t smsLength;
            int32_t parsed = util_strToUint(&bufferLineStart[smsLengthIndex], lineLength - smsLengthIndex, &smsLength);
            if (parsed <= 0) goto out_fail;

            // Add the SMS contents to the "line" length.
            lineLength += smsLength + 1;

            // Check if we have received the whole SMS.
            if (lineLength + 1 >= self->bufferLength - lineStart) goto out; // Nope.

            if (unread) {
                uint8_t senderNumberLength = (uint8_t)(senderNumberEnd - senderNumberIndex);
                struct modemClient_cmd *cmd = (void *)&self->commandQueue[self->commandQueueLength];
                int32_t cmdLength = modemClient_cmd_HEADER_SIZE + senderNumberLength;
                self->commandQueueLength += cmdLength;
                if (self->commandQueueLength >= (int32_t)sizeof(self->commandQueue)) goto out_fail;
                cmd->type = modemClient_cmd_START_SMS;
                cmd->dataSize = senderNumberLength;
                hc_MEMCPY(&cmd->data[0], &bufferLineStart[senderNumberIndex], cmd->dataSize);
                if (self->commandQueueLength == cmdLength) {
                    if (modemClient_processQueue(self) < 0) goto out_fail;
                }

                char *pos = util_intToStr(&buffer[18], dhcpClient.leasedIpNetmask);
                *--pos = '/';
                pos = util_intToStr(pos, ((uint8_t *)&dhcpClient.leasedIp)[3]);
                *--pos = '.';
                pos = util_intToStr(pos, ((uint8_t *)&dhcpClient.leasedIp)[2]);
                *--pos = '.';
                pos = util_intToStr(pos, ((uint8_t *)&dhcpClient.leasedIp)[1]);
                *--pos = '.';
                pos = util_intToStr(pos, ((uint8_t *)&dhcpClient.leasedIp)[0]);

                uint8_t sendSmsLength = (uint8_t)(&buffer[18] - pos);
                cmd = (void *)&self->commandQueue[self->commandQueueLength];
                self->commandQueueLength += modemClient_cmd_HEADER_SIZE + sendSmsLength;
                if (self->commandQueueLength >= (int32_t)sizeof(self->commandQueue)) goto out_fail;
                cmd->type = modemClient_cmd_SEND_SMS;
                cmd->dataSize = sendSmsLength;
                hc_MEMCPY(&cmd->data[0], pos, cmd->dataSize);
            }
            continue;
        }

        // SMS notification.
        if (lineLength >= 11 && hc_MEMCMP(&bufferLineStart[0], "+CMTI: \"SM\"", 11) == 0) {
            if (modemClient_queueSmsPoll(self) < 0) goto out_fail;
            continue;
        }
    }

    out_fail:
    modemClient_disconnect(self);
    out:
    // Move incomplete line to start of buffer.
    self->bufferLength -= lineStart;
    hc_MEMMOVE(&self->buffer[0], &self->buffer[lineStart], (uint64_t)self->bufferLength);
}

static hc_COLD void modemClient_deinit(struct modemClient *self) {
    modemClient_disconnect(self);
    debug_CHECK(sys_close(self->timerFd), RES == 0);
}
