static void netlink_talk(int32_t fd, struct iovec *request, int32_t iovLen) {
    memset(&buffer[0], 0xbb, sizeof(buffer));
    struct sockaddr_nl addr = { .nl_family = AF_NETLINK };
    struct msghdr msghdr = {
        .msg_name = &addr,
        .msg_namelen = sizeof(addr),
        .msg_iov = &request[0],
        .msg_iovlen = iovLen
    };
    int64_t sent = sys_sendmsg(fd, &msghdr, MSG_NOSIGNAL | MSG_DONTWAIT);

    struct nlmsghdr *reqHdr = (void *)request[0].iov_base;
    debug_ASSERT(request[0].iov_len >= (int64_t)sizeof(*reqHdr));
    CHECK(sent, RES == reqHdr->nlmsg_len);

#ifndef debug_NDEBUG
    // Check that the header length matches the total iov length.
    int64_t totalLen = 0;
    for (int32_t i = 0; i < iovLen; ++i) totalLen += request[i].iov_len;
    CHECK(reqHdr->nlmsg_len, RES == totalLen);
#endif

    int64_t received = sys_recvfrom(fd, &buffer[0], sizeof(buffer), MSG_DONTWAIT, NULL, NULL);
    CHECK(received, RES > (int64_t)sizeof(struct nlmsghdr));

    struct nlmsghdr *respHdr = (void *)&buffer[0];
    if (respHdr->nlmsg_type == NLMSG_ERROR) {
        int32_t error = *(int32_t *)&buffer[sizeof(*respHdr)];
        CHECK(error, RES == 0);
    }
}

static hc_NONULL struct nlattr *netlink_findAttr(struct nlattr *start, void *end, uint16_t attrType) {
    for (;;) {
        void *next = (void *)start + math_ALIGN_FORWARD(start->nla_len, 4);
        CHECK(next, RES <= end);

        if (start->nla_type == attrType) return start;
        start = next;
    }
}
