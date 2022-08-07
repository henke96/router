static void netlink_talk(int32_t fd, struct iovec *request, int32_t iovLen) {
    struct sockaddr_nl addr = { .nl_family = AF_NETLINK };
    struct msghdr msghdr = {
        .msg_name = &addr,
        .msg_namelen = sizeof(addr),
        .msg_iov = &request[0],
        .msg_iovlen = iovLen
    };
    int64_t sent = sys_sendmsg(fd, &msghdr, MSG_NOSIGNAL | MSG_DONTWAIT);

    int64_t expectedSent = 0;
    for (int32_t i = 0; i < iovLen; ++i) expectedSent += request[i].iov_len;
    CHECK(sent, RES == expectedSent);

    int64_t received = sys_recvfrom(fd, &buffer[0], sizeof(buffer), MSG_DONTWAIT, NULL, NULL);
    CHECK(received, RES > 0);

    struct nlmsghdr *hdr = (void *)&buffer[0];
    // TODO: Probably should do more error checking.
    if (hdr->nlmsg_type == NLMSG_ERROR) {
        int32_t error = *(int32_t *)&buffer[sizeof(*hdr)];
        CHECK(error, RES == 0);
    }
}
