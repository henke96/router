struct packetDumper {
    int32_t packetFd;
    int32_t destFd;
    struct sockaddr_in destAddr;
};

static void packetDumper_init(struct packetDumper *self, int32_t ifIndex, uint32_t destIp) {
    // Setup packet socket.
    self->packetFd = sys_socket(AF_PACKET, SOCK_RAW, hc_BSWAP16(ETH_P_ALL));
    CHECK(self->packetFd, RES > 0);

    struct sockaddr_ll addr = {
        .sll_family = AF_PACKET,
        .sll_ifindex = ifIndex
    };
    CHECK(sys_bind(self->packetFd, &addr, sizeof(addr)), RES == 0);

    self->destFd = sys_socket(AF_INET, SOCK_DGRAM, 0);
    CHECK(self->destFd, RES > 0);

    int32_t broadcast = 1;
    CHECK(sys_setsockopt(self->destFd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)), RES == 0);

    self->destAddr.sin_family = AF_INET;
    self->destAddr.sin_port = hc_BSWAP16(12345);
    hc_MEMCPY(&self->destAddr.sin_addr[0], &destIp, 4);
}

static void packetDumper_onMessage(struct packetDumper *self) {
    int64_t read = sys_recvfrom(self->packetFd, &buffer[0], sizeof(buffer), 0, NULL, NULL);
    CHECK(read, RES > 0);

    debug_CHECK(sys_sendto(self->destFd, &buffer[0], read, MSG_NOSIGNAL, &self->destAddr, sizeof(self->destAddr)), RES == read);
}

static void packetDumper_deinit(struct packetDumper *self) {
    debug_CHECK(sys_close(self->destFd), RES == 0);
    debug_CHECK(sys_close(self->packetFd), RES == 0);
}