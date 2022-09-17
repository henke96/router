struct packetDumper {
    int32_t listenFd;
    int32_t clientFd; // -1 if not dumping.
    int32_t packetFd; // -1 if not dumping.
    int32_t ifIndex;
};

static hc_COLD void packetDumper_init(struct packetDumper *self, int32_t ifIndex) {
    self->ifIndex = ifIndex;
    self->clientFd = -1;
    self->packetFd = -1;

    self->listenFd = sys_socket(AF_INET, SOCK_STREAM, 0);
    CHECK(self->listenFd, RES > 0);

    struct sockaddr_in listenAddr = {
        .sin_family = AF_INET,
        .sin_port = hc_BSWAP16((uint16_t)ifIndex), // Bind to the ifIndex as port..
        .sin_addr = { 10, 123, 0, 1 }
    };
    CHECK(sys_bind(self->listenFd, &listenAddr, sizeof(listenAddr)), RES == 0);
    CHECK(sys_listen(self->listenFd, 1), RES == 0);
}

static void packetDumper_stop(struct packetDumper *self) {
    if (self->packetFd >= 0) {
        debug_CHECK(sys_close(self->packetFd), RES == 0);
        debug_CHECK(sys_close(self->clientFd), RES == 0);
        self->packetFd = -1;
        self->clientFd = -1;
    }
}

static void packetDumper_onListenFd(struct packetDumper *self, int32_t epollFd) {
    int32_t newClientFd = sys_accept4(self->listenFd, NULL, NULL, 0);
    debug_ASSERT(newClientFd > 0);
    if (newClientFd < 0) return;

    packetDumper_stop(self); // Stop any ongoing dump.
    self->clientFd = newClientFd;

    // Setup packet socket.
    self->packetFd = sys_socket(AF_PACKET, SOCK_RAW, hc_BSWAP16(ETH_P_ALL));
    CHECK(self->packetFd, RES > 0);

    // My hardware doesn't seem to support hardware timestamps :-(
    int32_t enable = SOF_TIMESTAMPING_RX_SOFTWARE | SOF_TIMESTAMPING_SOFTWARE;
    CHECK(sys_setsockopt(self->packetFd, SOL_SOCKET, SO_TIMESTAMPING, &enable, sizeof(enable)), RES == 0);

    struct sockaddr_ll addr = {
        .sll_family = AF_PACKET,
        .sll_ifindex = self->ifIndex
    };
    CHECK(sys_bind(self->packetFd, &addr, sizeof(addr)), RES == 0);

    epollAdd(epollFd, self->clientFd);
    epollAdd(epollFd, self->packetFd);

    // Send pcap header.
    struct {
        uint32_t magic;
        uint16_t majorVersion;
        uint16_t minorVersion;
        uint32_t reserved1;
        uint32_t reserved2;
        uint32_t snapLength;
        uint32_t linkType;
    } pcapHeader = {
        .magic = 0xA1B23C4D,
        .majorVersion = 2,
        .minorVersion = 4,
        .snapLength = sizeof(buffer),
        .linkType = 1 // Ethernet.
    };
    if (sys_sendto(self->clientFd, &pcapHeader, sizeof(pcapHeader), MSG_NOSIGNAL, NULL, 0) != sizeof(pcapHeader)) {
        packetDumper_stop(self);
    }
}

static void packetDumper_onPacketFd(struct packetDumper *self) {
    struct cmsg {
        struct cmsghdr cmsghdr;
        struct timespec ts[3];
    } cmsg;

    struct msghdr recvMsghdr = {
        .msg_iov = &(struct iovec) { .iov_base = &buffer[0], .iov_len = sizeof(buffer) },
        .msg_iovlen = 1,
        .msg_control = &cmsg,
        .msg_controllen = sizeof(cmsg)
    };

    int64_t read = sys_recvmsg(self->packetFd, &recvMsghdr, 0);
    CHECK(read, RES > 0);
    debug_ASSERT(cmsg.cmsghdr.cmsg_type == SO_TIMESTAMPING);
    debug_ASSERT(cmsg.cmsghdr.cmsg_len == sizeof(cmsg));

    struct {
        uint32_t timestampSec;
        uint32_t timestampNs;
        uint32_t capturedLength;
        uint32_t originalLength;
    } pcapPacketRecord = {
        .timestampSec = (uint32_t)cmsg.ts[0].tv_sec,
        .timestampNs = (uint32_t)cmsg.ts[0].tv_nsec,
        .capturedLength = (uint32_t)read,
        .originalLength = (uint32_t)read
    };

    struct msghdr sendMsghdr = {
        .msg_iov = &(struct iovec[]) {
            { .iov_base = &pcapPacketRecord, .iov_len = sizeof(pcapPacketRecord) },
            { .iov_base = &buffer[0], .iov_len = read }
        }[0],
        .msg_iovlen = 2
    };
    int64_t written = sys_sendmsg(self->clientFd, &sendMsghdr, MSG_NOSIGNAL);
    if (written != (int64_t)sizeof(pcapPacketRecord) + read) packetDumper_stop(self);
}

static void packetDumper_onClientFd(struct packetDumper *self) {
    packetDumper_stop(self);
}

static hc_COLD void packetDumper_deinit(struct packetDumper *self) {
    packetDumper_stop(self);
    debug_CHECK(sys_close(self->listenFd), RES == 0);
}