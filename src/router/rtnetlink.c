struct rtnetlink {
    int32_t fd;
    uint8_t receiveBuffer[8192]; // See NLMSG_GOODSIZE in <linux/netlink.h>
};

static int32_t rtnetlink_init(struct rtnetlink *self) {
    self->fd = sys_socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (self->fd < 0) return -1;
    return 0;
}

static int32_t rtnetlink_talk(struct rtnetlink *self, void *request, int64_t requestLen) {
    struct sockaddr_nl addr = { .nl_family = AF_NETLINK };
    int64_t sent = sys_sendto(self->fd, request, requestLen, MSG_NOSIGNAL | MSG_DONTWAIT, &addr, sizeof(addr));
    if (sent != requestLen) return -1;

    int64_t received = sys_recvfrom(self->fd, &self->receiveBuffer[0], sizeof(self->receiveBuffer), MSG_DONTWAIT, NULL, NULL);
    if (received <= 0) return -2;

    struct nlmsghdr *hdr = (void *)&self->receiveBuffer[0];
    if (hdr->nlmsg_len != received) return -3;

    if (hdr->nlmsg_type == NLMSG_ERROR) {
        int32_t error = *(int32_t *)&self->receiveBuffer[sizeof(*hdr)];
        if (error != 0) {
            debug_printNum("rtnetlink error: ", error, "\n");
            return -4;
        }
    }
    return 0;
}

static void rtnetlink_deinit(struct rtnetlink *self) {
    debug_CHECK(sys_close(self->fd), == 0);
}