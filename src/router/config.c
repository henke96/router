#define config_WG_IF_NAME "wg0"
#define config_WG_IF_INDEX 123
// Try to use a port that is unlikely to be blocked by firewalls.
#define config_WG_LISTEN_PORT 123
#define config_WG_PEER1_ADDRESS { 10, 123, 1, 1 }
// Generate with `echo <base64-public-key> | base64 --decode | xxd -i -c 256`
#define config_WG_PEER1_PUBLIC_KEY { 0xd0, 0xe1, 0x80, 0xab, 0x2e, 0xec, 0xa9, 0x4f, 0x2a, 0x0c, 0xf1, 0xaa, 0xcc, 0xa3, 0x02, 0x78, 0x9d, 0xed, 0x4e, 0x31, 0x29, 0x95, 0x81, 0x45, 0x3c, 0x86, 0x00, 0x9e, 0xf3, 0xd7, 0xe7, 0x23 }

struct config {
    int32_t rtnetlinkFd;
    int32_t genetlinkFd;
    uint16_t wgFamilyId;
    uint16_t __pad;
    uint8_t wgPublicKey[32]; // Populated by config_configure().
};

static struct config config;

static void config_init(void) {
    config.rtnetlinkFd = sys_socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    CHECK(config.rtnetlinkFd, RES > 0);
    config.genetlinkFd = sys_socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    CHECK(config.genetlinkFd, RES > 0);

    // Get wireguard family id.
    struct getFamilyRequest {
        struct nlmsghdr hdr;
        struct genlmsghdr genHdr;
        struct nlattr familyNameAttr;
        char familyName[sizeof(WG_GENL_NAME)];
        char familyNamePad[util_PAD_BYTES(sizeof(WG_GENL_NAME), 4)];
    };
    struct getFamilyRequest request = {
        .hdr = {
            .nlmsg_len = sizeof(request),
            .nlmsg_type = GENL_ID_CTRL,
            .nlmsg_flags = NLM_F_REQUEST,
        },
        .genHdr = {
            .cmd = CTRL_CMD_GETFAMILY,
            .version = 1
        },
        .familyNameAttr = {
            .nla_len = sizeof(request.familyNameAttr) + sizeof(request.familyName),
            .nla_type = CTRL_ATTR_FAMILY_NAME
        },
        .familyName = WG_GENL_NAME
    };
    CHECK(netlink_talk(config.genetlinkFd, &request, sizeof(request)), RES == 0);

    // Iterate over all attributes, assume CTRL_ATTR_FAMILY_ID is one of them.
    for (
        struct nlattr *attr = (void *)&buffer[sizeof(struct nlmsghdr) + sizeof(struct genlmsghdr)];;
        attr = (void *)&((char *)attr)[util_ALIGN_FORWARD(attr->nla_len, 4)]
    ) {
        if (attr->nla_type == CTRL_ATTR_FAMILY_ID) {
            config.wgFamilyId = *(uint16_t *)&attr[1];
            break;
        }
    }
}

static void config_addIpv4(uint8_t ifIndex, uint8_t *address, uint8_t prefixLen) {
    struct addrRequest {
        struct nlmsghdr hdr;
        struct ifaddrmsg addrMsg;
        struct nlattr addrAttr;
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
            .nla_len = sizeof(request.addrAttr) + sizeof(request.address),
            .nla_type = IFA_LOCAL
        }
    };
    hc_MEMCPY(&request.address, address, sizeof(request.address));
    CHECK(netlink_talk(config.rtnetlinkFd, &request, sizeof(request)), RES == 0);
}

static void config_bringUp(uint8_t ifIndex) {
    struct linkRequest {
        struct nlmsghdr hdr;
        struct ifinfomsg ifInfo;
    };
    struct linkRequest request = {
        .hdr = {
            .nlmsg_len = sizeof(request),
            .nlmsg_type = RTM_NEWLINK,
            .nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK,
        },
        .ifInfo = {
            .ifi_family = AF_UNSPEC,
            .ifi_type = 0,
            .ifi_index = ifIndex,
            .ifi_flags = IFF_UP,
            .ifi_change = 0xFFFFFFFF
        }
    };
    CHECK(netlink_talk(config.rtnetlinkFd, &request, sizeof(request)), RES == 0);
}

static void config_addWgPeer1Route(void) {
    struct addRouteRequest {
        struct nlmsghdr hdr;
        struct rtmsg rtmsg;
        struct nlattr destAttr;
        uint8_t dest[4];
        struct nlattr outIfAttr;
        uint32_t outIfIndex;
    };
    struct addRouteRequest request = {
        .hdr = {
            .nlmsg_len = sizeof(request),
            .nlmsg_type = RTM_NEWROUTE,
            .nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE
        },
        .rtmsg = {
            .rtm_family = AF_INET,
            .rtm_dst_len = 32,
            .rtm_src_len = 0,
            .rtm_tos = 0,
            .rtm_table = RT_TABLE_MAIN,
            .rtm_protocol = RTPROT_BOOT,
            .rtm_scope = RT_SCOPE_LINK,
            .rtm_type = RTN_UNICAST,
            .rtm_flags = 0
        },
        .destAttr = {
            .nla_len = sizeof(request.destAttr) + sizeof(request.dest),
            .nla_type = RTA_DST
        },
        .dest = config_WG_PEER1_ADDRESS,
        .outIfAttr = {
            .nla_len = sizeof(request.outIfAttr) + sizeof(request.outIfIndex),
            .nla_type = RTA_OIF
        },
        .outIfIndex = config_WG_IF_INDEX
    };
    CHECK(netlink_talk(config.rtnetlinkFd, &request, sizeof(request)), RES == 0);
}

static void config_addWireguardIf(void) {
    struct linkRequest {
        struct nlmsghdr hdr;
        struct ifinfomsg ifInfo;
        struct nlattr ifNameAttr;
        char ifName[sizeof(config_WG_IF_NAME)];
        char ifNamePad[util_PAD_BYTES(sizeof(config_WG_IF_NAME), 4)];
        struct nlattr linkInfoAttr;
        struct nlattr linkInfoKindAttr;
        char infoKind[sizeof(WG_GENL_NAME)];
        char infoKindPad[util_PAD_BYTES(sizeof(WG_GENL_NAME), 4)];
    };
    struct linkRequest request = {
        .hdr = {
            .nlmsg_len = sizeof(request),
            .nlmsg_type = RTM_NEWLINK,
            .nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE,
        },
        .ifInfo = {
            .ifi_family = AF_UNSPEC,
            .ifi_type = 0,
            .ifi_index = config_WG_IF_INDEX,
            .ifi_flags = IFF_UP,
            .ifi_change = 0xFFFFFFFF
        },
        .ifNameAttr = {
            .nla_len = sizeof(request.ifNameAttr) + sizeof(request.ifName),
            .nla_type = IFLA_IFNAME
        },
        .ifName = config_WG_IF_NAME,
        .linkInfoAttr = {
            .nla_len = sizeof(request.linkInfoAttr) + sizeof(request.linkInfoKindAttr) + sizeof(request.infoKind) + sizeof(request.infoKindPad),
            .nla_type = IFLA_LINKINFO | NLA_F_NESTED
        },
        .linkInfoKindAttr = {
            .nla_len = sizeof(request.linkInfoKindAttr) + sizeof(request.infoKind),
            .nla_type = IFLA_INFO_KIND
        },
        .infoKind = WG_GENL_NAME
    };
    CHECK(netlink_talk(config.rtnetlinkFd, &request, sizeof(request)), RES == 0);
}

static void config_setWgDevice(void) {
    struct setDeviceRequest {
        struct nlmsghdr hdr;
        struct genlmsghdr genHdr;
        struct nlattr ifIndexAttr;
        uint32_t ifIndex;
        struct nlattr privateKeyAttr;
        uint8_t privateKey[32];
        struct nlattr listenPortAttr;
        uint32_t listenPort; // uint16_t
        struct nlattr peersAttr;
        // Peer1
        struct nlattr peer1Attr;
        struct nlattr peer1PublicKeyAttr;
        uint8_t peer1PublicKey[32];
        struct nlattr peer1AllowedIpsAttr;
        struct nlattr peer1AllowedIpAttr;
        struct nlattr peer1AllowedIpFamilyAttr;
        uint32_t peer1AllowedIpFamily; // uint16_t
        struct nlattr peer1AllowedIpAddressAttr;
        uint8_t peer1AllowedIpAddress[4];
        struct nlattr peer1AllowedIpNetmaskAttr;
        uint32_t peer1AllowedIpNetmask; // uint8_t
        uint32_t peer1End[]; // For use with offsetof()
    };
    struct setDeviceRequest request = {
        .hdr = {
            .nlmsg_len = sizeof(request),
            .nlmsg_type = config.wgFamilyId,
            .nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK,
        },
        .genHdr = {
            .cmd = WG_CMD_SET_DEVICE,
            .version = WG_GENL_VERSION
        },
        .ifIndexAttr = {
            .nla_len = sizeof(request.ifIndexAttr) + sizeof(request.ifIndex),
            .nla_type = WGDEVICE_A_IFINDEX
        },
        .ifIndex = config_WG_IF_INDEX,
        .privateKeyAttr = {
            .nla_len = sizeof(request.privateKeyAttr) + sizeof(request.privateKey),
            .nla_type = WGDEVICE_A_PRIVATE_KEY
        },
        .listenPortAttr = {
            .nla_len = sizeof(request.listenPortAttr) + sizeof(uint16_t),
            .nla_type = WGDEVICE_A_LISTEN_PORT
        },
        .listenPort = config_WG_LISTEN_PORT,
        .peersAttr = {
            .nla_len = offsetof(struct setDeviceRequest, peer1End) - offsetof(struct setDeviceRequest, peersAttr),
            .nla_type = WGDEVICE_A_PEERS | NLA_F_NESTED
        },
        .peer1Attr = {
            .nla_len = offsetof(struct setDeviceRequest, peer1End) - offsetof(struct setDeviceRequest, peer1Attr),
            .nla_type = NLA_F_NESTED
        },
        .peer1PublicKeyAttr = {
            .nla_len = sizeof(request.peer1PublicKeyAttr) + sizeof(request.peer1PublicKey),
            .nla_type = WGPEER_A_PUBLIC_KEY
        },
        .peer1PublicKey = config_WG_PEER1_PUBLIC_KEY,
        .peer1AllowedIpsAttr = {
            .nla_len = offsetof(struct setDeviceRequest, peer1End) - offsetof(struct setDeviceRequest, peer1AllowedIpsAttr),
            .nla_type = WGPEER_A_ALLOWEDIPS | NLA_F_NESTED
        },
        .peer1AllowedIpAttr = {
            .nla_len = offsetof(struct setDeviceRequest, peer1End) - offsetof(struct setDeviceRequest, peer1AllowedIpAttr),
            .nla_type = NLA_F_NESTED
        },
        .peer1AllowedIpFamilyAttr = {
            .nla_len = sizeof(request.peer1AllowedIpFamilyAttr) + sizeof(uint16_t),
            .nla_type = WGALLOWEDIP_A_FAMILY
        },
        .peer1AllowedIpFamily = AF_INET,
        .peer1AllowedIpAddressAttr = {
            .nla_len = sizeof(request.peer1AllowedIpAddressAttr) + sizeof(request.peer1AllowedIpAddress),
            .nla_type = WGALLOWEDIP_A_IPADDR
        },
        .peer1AllowedIpAddress = config_WG_PEER1_ADDRESS,
        .peer1AllowedIpNetmaskAttr = {
            .nla_len = sizeof(request.peer1AllowedIpNetmaskAttr) + sizeof(uint8_t),
            .nla_type = WGALLOWEDIP_A_CIDR_MASK
        },
        .peer1AllowedIpNetmask = 32
    };
    // Read private key from /wgprivate, or generate it if file doesn't exist.
    int32_t fd = sys_openat(-1, "/wgprivate", O_RDONLY, 0);
    CHECK(fd, RES > 0 || RES == -ENOENT);
    if (fd > 0) {
        CHECK(sys_read(fd, &request.privateKey, sizeof(request.privateKey)), RES == sizeof(request.privateKey));
    } else {
        // Generate key and write to file.
        CHECK(sys_getrandom(&request.privateKey, sizeof(request.privateKey), 0), RES == sizeof(request.privateKey));
        fd = sys_openat(-1, "/wgprivate", O_WRONLY | O_CREAT | O_EXCL, S_IRUSR);
        CHECK(fd, RES > 0);
        CHECK(sys_write(fd, &request.privateKey, sizeof(request.privateKey)), RES == sizeof(request.privateKey));
    }
    debug_CHECK(sys_close(fd), RES == 0);
    CHECK(netlink_talk(config.genetlinkFd, &request, sizeof(request)), RES == 0);
}

static void config_printWgPublicKey(void) {
    struct getDeviceRequest {
        struct nlmsghdr hdr;
        struct genlmsghdr genHdr;
        struct nlattr ifIndexAttr;
        uint32_t ifIndex;
    };
    struct getDeviceRequest request = {
        .hdr = {
            .nlmsg_len = sizeof(request),
            .nlmsg_type = config.wgFamilyId,
            .nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP,
            .nlmsg_seq = 1
        },
        .genHdr = {
            .cmd = WG_CMD_GET_DEVICE,
            .version = WG_GENL_VERSION
        },
        .ifIndexAttr = {
            .nla_len = sizeof(request.ifIndexAttr) + sizeof(request.ifIndex),
            .nla_type = WGDEVICE_A_IFINDEX
        },
        .ifIndex = config_WG_IF_INDEX
    };
    CHECK(netlink_talk(config.genetlinkFd, &request, sizeof(request)), RES == 0);

    // Iterate over all attributes, assume WGDEVICE_A_PUBLIC_KEY is one of them.
    for (
        struct nlattr *attr = (void *)&buffer[sizeof(struct nlmsghdr) + sizeof(struct genlmsghdr)];;
        attr = (void *)&((char *)attr)[util_ALIGN_FORWARD(attr->nla_len, 4)]
    ) {
        if (attr->nla_type == WGDEVICE_A_PUBLIC_KEY) {
            hc_MEMCPY(config.wgPublicKey, (uint8_t *)&attr[1], 32);
            break;
        }
    }

    uint8_t base64PublicKey[base64_ENCODE_SIZE(32)];
    base64_encode(&base64PublicKey[0], config.wgPublicKey, 32);
    int64_t written = sys_writev(STDOUT_FILENO, (struct iovec[3]) {
        { .iov_base = "Wireguard PK: ", .iov_len = 14 },
        { .iov_base = &base64PublicKey[0], .iov_len = sizeof(base64PublicKey) },
        { .iov_base = "\n", .iov_len = 1 }
    }, 3);
    CHECK(written, RES == 14 + sizeof(base64PublicKey) + 1);
}

static void config_configure(void) {
    // eth0
    config_bringUp(2);

    // eth1
    uint8_t if3Address[] = { 10, 123, 0, 1 };
    config_addIpv4(3, &if3Address[0], 24);
    config_bringUp(3);

    // wg0
    config_addWireguardIf();
    config_setWgDevice();
    config_printWgPublicKey();
    config_addWgPeer1Route();

    // Enable routing.
    int32_t fd = sys_openat(-1, "/proc/sys/net/ipv4/ip_forward", O_WRONLY, 0);
    CHECK(fd, RES > 0);
    CHECK(sys_write(fd, "1", 1), RES == 1);
    debug_CHECK(sys_close(fd), RES == 0);
}

static void config_deinit(void) {
    debug_CHECK(sys_close(config.rtnetlinkFd), RES == 0);
    debug_CHECK(sys_close(config.genetlinkFd), RES == 0);
}