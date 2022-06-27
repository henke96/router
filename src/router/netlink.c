static uint8_t netlink_buffer[8192]; // See NLMSG_GOODSIZE in <linux/netlink.h>

static int32_t netlink_talk(int32_t fd, void *request, int64_t requestLen) {
    struct sockaddr_nl addr = { .nl_family = AF_NETLINK };
    int64_t sent = sys_sendto(fd, request, requestLen, MSG_NOSIGNAL | MSG_DONTWAIT, &addr, sizeof(addr));
    if (sent != requestLen) return -1;

    int64_t received = sys_recvfrom(fd, &netlink_buffer[0], sizeof(netlink_buffer), MSG_DONTWAIT, NULL, NULL);
    if (received <= 0) return -2;

    struct nlmsghdr *hdr = (void *)&netlink_buffer[0];
    //if (hdr->nlmsg_len != received) return -3; TODO

    if (hdr->nlmsg_type == NLMSG_ERROR) {
        int32_t error = *(int32_t *)&netlink_buffer[sizeof(*hdr)];
        if (error != 0) {
            debug_printNum("netlink error: ", error, "\n");
            return -4;
        }
    }
    return 0;
}