struct config {
    struct rtnetlink rtnetlink;
};

static struct config config;

static int32_t config_init(void) {
    return rtnetlink_init(&config.rtnetlink);
}

static int32_t config_addIpv4(uint8_t ifIndex, uint8_t *address, uint8_t prefixLen) {
    struct addrRequest {
        struct nlmsghdr hdr;
        struct ifaddrmsg addrMsg;
        struct rtattr addrAttr;
        uint8_t address[4];
    };
    struct addrRequest request = {
        .hdr = {
            .nlmsg_len = sizeof(request),
            .nlmsg_type = RTM_NEWADDR,
            .nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE,
        },
        .addrMsg = {
            .ifa_family = AF_INET,
            .ifa_prefixlen = prefixLen,
            .ifa_index = ifIndex
        },
        .addrAttr = {
            .rta_len = sizeof(request.addrAttr) + sizeof(request.address),
            .rta_type = IFA_LOCAL
        }
    };
    hc_MEMCPY(&request.address, address, sizeof(request.address));
    return rtnetlink_talk(&config.rtnetlink, &request, sizeof(request));
}

static int32_t config_bringUp(uint8_t ifIndex) {
    struct linkRequest {
        struct nlmsghdr hdr;
        struct ifinfomsg infoMsg;
    };
    struct linkRequest request = {
        .hdr = {
            .nlmsg_len = sizeof(request),
            .nlmsg_type = RTM_NEWLINK,
            .nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK,
        },
        .infoMsg = {
            .ifi_family = AF_UNSPEC,
            .ifi_type = 0,
            .ifi_index = ifIndex,
            .ifi_flags = IFF_UP,
            .ifi_change = 0xFFFFFFFF
        }
    };
    return rtnetlink_talk(&config.rtnetlink, &request, sizeof(request));
}

static int32_t config_configure(void) {
    // eth0
    uint8_t if2Address[] = { 10, 8, 16, 1 };
    int32_t status = config_addIpv4(2, &if2Address[0], 24);
    if (status < 0) {
        debug_printNum("Failed to add if2 address (", status, ")\n");
        return -1;
    }
    status = config_bringUp(2);
    if (status < 0) {
        debug_printNum("Failed to bring up if2 (", status, ")\n");
        return -1;
    }

    // eth1
    uint8_t if3Address[] = { 10, 8, 49, 1 };
    status = config_addIpv4(3, &if3Address[0], 24);
    if (status < 0) {
        debug_printNum("Failed to add if3 address (", status, ")\n");
        return -1;
    }
    status = config_bringUp(3);
    if (status < 0) {
        debug_printNum("Failed to bring up if3 (", status, ")\n");
        return -1;
    }

    // Enable routing.
    int32_t fd = sys_openat(-1, "/proc/sys/net/ipv4/ip_forward", O_WRONLY, 0);
    if (fd < 0) return -2;
    int64_t written = sys_write(fd, "1", 1);
    sys_close(fd);
    if (written != 1) return -3;
    return 0;
}

static void config_deinit(void) {
    rtnetlink_deinit(&config.rtnetlink);
}