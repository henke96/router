struct genetlink {
    int32_t fd;
};

static struct genetlink genetlink;

static hc_COLD void genetlink_init(void) {
    genetlink.fd = sys_socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    CHECK(genetlink.fd, RES > 0);
}

static hc_ALWAYS_INLINE void genetlink_talk(struct iovec *request, int32_t iovLen) {
    netlink_talk(genetlink.fd, request, iovLen);
}

static hc_NONULL struct nlattr *genetlink_findAttr(uint16_t attrType) {
    struct nlmsghdr *msgHdr = (void *)&buffer[0];
    struct nlattr *start = (void *)&buffer[sizeof(*msgHdr) + sizeof(struct genlmsghdr)];
    void *end = &buffer[msgHdr->nlmsg_len];
    return netlink_findAttr(start, end, attrType);
}

static hc_COLD void genetlink_requestFamily(const char *name) {
    int64_t nameLen = util_cstrLen(name) + 1;
    debug_ASSERT(nameLen < 1000);
    uint16_t namePadLen = math_PAD_BYTES(nameLen, 4);

    struct getFamilyRequest {
        struct nlmsghdr hdr;
        struct genlmsghdr genHdr;
        struct nlattr familyNameAttr;
    };
    struct getFamilyRequest request = {
        .hdr = {
            .nlmsg_len = sizeof(request) + (uint16_t)nameLen + namePadLen,
            .nlmsg_type = GENL_ID_CTRL,
            .nlmsg_flags = NLM_F_REQUEST,
        },
        .genHdr = {
            .cmd = CTRL_CMD_GETFAMILY,
            .version = 1
        },
        .familyNameAttr = {
            .nla_len = sizeof(request.familyNameAttr) + (uint16_t)nameLen,
            .nla_type = CTRL_ATTR_FAMILY_NAME
        },
    };
    struct iovec iov[] = {
        { .iov_base = &request,     .iov_len = sizeof(request) },
        { .iov_base = (void *)name, .iov_len = nameLen },
        { .iov_base = (void *)name, .iov_len = namePadLen }
    };
    genetlink_talk(&iov[0], hc_ARRAY_LEN(iov));
}

static hc_COLD void genetlink_deinit(void) {
    debug_CHECK(sys_close(genetlink.fd), RES == 0);
}