#define dhcp_IFINDEX 2

struct dhcpClient {
    struct ifreq ifreq; // ifr_name and ifr_addr.
    int32_t fd;
    int32_t timerFd;
    int32_t dnsIp; // 0 if none.
    int32_t __pad;
};

static struct dhcpClient dhcpClient = { 0 };

static void dhcpClient_init(void) {
    CHECK(dhcpClient.fd = sys_socket(AF_INET, SOCK_DGRAM, 0), RES > 0);

    // Populate interface name and MAC address.
    dhcpClient.ifreq.ifr_ifindex = dhcp_IFINDEX;
    CHECK(sys_ioctl(dhcpClient.fd, SIOCGIFNAME, &dhcpClient.ifreq), RES == 0);
    CHECK(sys_ioctl(dhcpClient.fd, SIOCGIFHWADDR, &dhcpClient.ifreq), RES == 0);

    // Bind the socket to the interface.
    CHECK(sys_setsockopt(dhcpClient.fd, SOL_SOCKET, SO_BINDTODEVICE, &dhcpClient.ifreq.ifr_name, IFNAMSIZ), RES == 0);

    // Allow sending/receiving multicast.
    int32_t broadcast = 1;
    CHECK(sys_setsockopt(dhcpClient.fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)), RES == 0);

    // Bind to any IP, port 68.
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = hc_BSWAP16(68),
        .sin_addr = { 0, 0, 0, 0 }
    };
    CHECK(sys_bind(dhcpClient.fd, &addr, sizeof(addr)), RES == 0);

    // Create timer fd, and set instant timeout.
    CHECK(dhcpClient.timerFd = sys_timerfd_create(CLOCK_MONOTONIC, 0), RES > 0);

    struct itimerspec timeout = { .it_value = { .tv_nsec = 1 } };
    CHECK(sys_timerfd_settime(dhcpClient.timerFd, 0, &timeout, NULL), RES == 0);
}

static void dhcpClient_onTimer(void) {
    uint64_t expirations;
    CHECK(sys_read(dhcpClient.timerFd, &expirations, sizeof(expirations)), RES == sizeof(expirations));
    debug_ASSERT(expirations, RES == 1);

    // We timed out. Restart DHCP process by sending a new discover.
    struct discover {
        struct dhcp_header hdr;
        struct dhcp_option messageTypeOpt;
        uint8_t messageType;
        struct dhcp_option paramRequestListOpt;
        uint8_t paramRequestList[3];
        struct dhcp_option leaseTimeOpt;
        uint8_t leaseTime[4];
        uint8_t endOpt;
        uint8_t __pad;
    };

    struct discover discoverMsg = {
        .hdr = {
            .opcode = dhcp_BOOTREQUEST,
            .hwAddrType = dhcp_ETHERNET,
            .hwAddrLen = 6,
            .flags = hc_BSWAP16(0x8000), // Request broadcast responses.
            .magicCookie = dhcp_MAGIC_COOKIE
        },
        .messageTypeOpt = {
            .code = dhcp_MESSAGE_TYPE,
            .length = sizeof(discoverMsg.messageType)
        },
        .messageType = dhcp_DISCOVER,
        .paramRequestListOpt = {
            .code = dhcp_PARAM_REQUEST_LIST,
            .length = sizeof(discoverMsg.paramRequestList)
        },
        .paramRequestList = { dhcp_SUBNET_MASK, dhcp_ROUTER, dhcp_DNS },
        .leaseTimeOpt = {
            .code = dhcp_LEASE_TIME,
            .length = sizeof(discoverMsg.leaseTime)
        },
        .leaseTime = { 0xff, 0xff, 0xff, 0xff }, // Infinity.
        .endOpt = dhcp_END
    };
    hc_MEMCPY(&discoverMsg.hdr.clientHwAddr, &dhcpClient.ifreq.ifr_addr[2], 6);

    struct sockaddr_in destAddr = {
        .sin_family = AF_INET,
        .sin_port = hc_BSWAP16(67),
        .sin_addr = { 255, 255, 255, 255 }
    };
    CHECK(sys_sendto(dhcpClient.fd, &discoverMsg, sizeof(discoverMsg), MSG_NOSIGNAL, &destAddr, sizeof(destAddr)), RES == sizeof(discoverMsg));

    // Set timeout to 10 seconds.
    struct itimerspec timeout = { .it_value = { .tv_sec = 10 } };
    CHECK(sys_timerfd_settime(dhcpClient.timerFd, 0, &timeout, NULL), RES == 0);
}

static void dhcpClient_onMessage(void) {
    int64_t read = sys_read(dhcpClient.fd, &buffer[0], sizeof(buffer));
    debug_ASSERT(read, RES > 0);
    if (read < (int64_t)sizeof(struct dhcp_header)) return;
    void *end = &buffer[read];
    struct dhcp_header *header = (void *)&buffer[0];

    // Make sure the packet is for us.
    if (hc_MEMCMP(&header->clientHwAddr, &dhcpClient.ifreq.ifr_addr[2], 6) != 0) return;

    // Find DHCP options.
    struct dhcp_option *messageType = NULL;
    struct dhcp_option *subnetMask = NULL;
    struct dhcp_option *router = NULL;
    struct dhcp_option *dns = NULL;
    struct dhcp_option *leaseTime = NULL;
    for (struct dhcp_option *current = (void *)&header->options[0];;) {
        void *next = &((char *)current)[sizeof(*current) + current->length];
        if (next > end) break;
        switch (current->code) {
            case dhcp_MESSAGE_TYPE: {
                if (current->length == 1) messageType = current;
                break;
            }
            case dhcp_SUBNET_MASK: {
                if (current->length == 4) subnetMask = current;
                break;
            }
            case dhcp_ROUTER: {
                if (current->length >= 4) router = current;
                break;
            }
            case dhcp_DNS: {
                if (current->length >= 4) dns = current;
                break;
            }
            case dhcp_LEASE_TIME: {
                if (current->length == 4) leaseTime = current;
                break;
            }
        }
        current = next;
    }
    if (messageType == NULL) return;

    switch (messageType->data[0]) {
        case dhcp_OFFER: {
            if (leaseTime == NULL) return;

            // Respond to the offer with a request.
            struct request {
                struct dhcp_header hdr;
                struct dhcp_option messageTypeOpt;
                uint8_t messageType;
                struct dhcp_option paramRequestListOpt;
                uint8_t paramRequestList[3];
                struct dhcp_option leaseTimeOpt;
                uint8_t leaseTime[4];
                struct dhcp_option serverIdOpt;
                uint8_t serverId[4];
                struct dhcp_option requestedIpOpt;
                uint8_t requestedIp[4];
                uint8_t endOpt;
                uint8_t __pad[1];
            };

            struct request requestMsg = {
                .hdr = {
                    .opcode = dhcp_BOOTREQUEST,
                    .hwAddrType = dhcp_ETHERNET,
                    .hwAddrLen = 6,
                    .flags = hc_BSWAP16(0x8000), // Request broadcast responses.
                    .magicCookie = dhcp_MAGIC_COOKIE
                },
                .messageTypeOpt = {
                    .code = dhcp_MESSAGE_TYPE,
                    .length = sizeof(requestMsg.messageType)
                },
                .messageType = dhcp_REQUEST,
                .paramRequestListOpt = {
                    .code = dhcp_PARAM_REQUEST_LIST,
                    .length = sizeof(requestMsg.paramRequestList)
                },
                .paramRequestList = { dhcp_SUBNET_MASK, dhcp_ROUTER, dhcp_DNS },
                .leaseTimeOpt = {
                    .code = dhcp_LEASE_TIME,
                    .length = sizeof(requestMsg.leaseTime)
                },
                .serverIdOpt = {
                    .code = dhcp_SERVER_IDENTIFIER,
                    .length = sizeof(requestMsg.serverId)
                },
                .requestedIpOpt = {
                    .code = dhcp_REQUESTED_IP_ADDRESS,
                    .length = sizeof(requestMsg.requestedIp)
                },
                .endOpt = dhcp_END
            };
            hc_MEMCPY(&requestMsg.leaseTime, &leaseTime->data[0], sizeof(requestMsg.leaseTime));
            hc_MEMCPY(&requestMsg.serverId, &header->serverIp, sizeof(requestMsg.serverId));
            hc_MEMCPY(&requestMsg.hdr.clientHwAddr, &dhcpClient.ifreq.ifr_addr[2], 6);
            hc_MEMCPY(&requestMsg.requestedIp, &header->yourIp, sizeof(requestMsg.requestedIp));

            struct sockaddr_in destAddr = {
                .sin_family = AF_INET,
                .sin_port = hc_BSWAP16(67),
                .sin_addr = { 255, 255, 255, 255 }
            };
            CHECK(sys_sendto(dhcpClient.fd, &requestMsg, sizeof(requestMsg), MSG_NOSIGNAL, &destAddr, sizeof(destAddr)), RES == sizeof(requestMsg));

            // Set timeout to 10 seconds.
            struct itimerspec timeout = { .it_value = { .tv_sec = 10 } };
            CHECK(sys_timerfd_settime(dhcpClient.timerFd, 0, &timeout, NULL), RES == 0);
            break;
        }
        case dhcp_NAK: {
            // Something didn't succeed, retry in a few seconds.
            struct itimerspec timeout = { .it_value = { .tv_sec = 5 } };
            CHECK(sys_timerfd_settime(dhcpClient.timerFd, 0, &timeout, NULL), RES == 0);
            break;
        }
        case dhcp_ACK: {
            if (subnetMask == NULL || router == NULL || leaseTime == NULL) return;
            if (dns != NULL) {
                hc_MEMCPY(&dhcpClient.dnsIp, &dns->data[0], 4);
            } else dhcpClient.dnsIp = 0;

            uint32_t leaseTimeValue;
            hc_MEMCPY(&leaseTimeValue, &leaseTime->data[0], 4);
            leaseTimeValue = hc_BSWAP32(leaseTimeValue);

            // Set timeout to half of the lease time, rounded up.
            struct itimerspec timeout = { .it_value = { .tv_sec = (leaseTimeValue + 1) / 2 } };
            CHECK(sys_timerfd_settime(dhcpClient.timerFd, 0, &timeout, NULL), RES == 0);

            uint32_t netmask;
            hc_MEMCPY(&netmask, &subnetMask->data[0], 4);

            // Add the address.
            {
                struct addrRequest {
                    struct nlmsghdr hdr;
                    struct ifaddrmsg addrMsg;
                    struct nlattr addrAttr;
                    uint8_t address[4];
                    struct nlattr cacheInfoAttr;
                    struct ifa_cacheinfo cacheInfo;
                };
                struct addrRequest request = {
                    .hdr = {
                        .nlmsg_len = sizeof(request),
                        .nlmsg_type = RTM_NEWADDR,
                        .nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_REPLACE,
                    },
                    .addrMsg = {
                        .ifa_family = AF_INET,
                        .ifa_prefixlen = (uint8_t)hc_POPCOUNT32(netmask),
                        .ifa_index = dhcp_IFINDEX
                    },
                    .addrAttr = {
                        .nla_len = sizeof(request.addrAttr) + sizeof(request.address),
                        .nla_type = IFA_LOCAL
                    },
                    .cacheInfoAttr = {
                        .nla_len = sizeof(request.cacheInfoAttr) + sizeof(request.cacheInfo),
                        .nla_type = IFA_CACHEINFO
                    },
                    .cacheInfo = {
                        .ifa_prefered = leaseTimeValue,
                        .ifa_valid = leaseTimeValue
                    }
                };
                hc_MEMCPY(&request.address, header->yourIp, sizeof(request.address));
                CHECK(netlink_talk(config.rtnetlinkFd, &request, sizeof(request)), RES == 0);
            }

            // Set default route.
            {
                struct routeRequest {
                    struct nlmsghdr hdr;
                    struct rtmsg routeMsg;
                    struct nlattr gatewayAttr;
                    uint8_t gateway[4];
                };
                struct routeRequest request = {
                    .hdr = {
                        .nlmsg_len = sizeof(request),
                        .nlmsg_type = RTM_NEWROUTE,
                        .nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_REPLACE,
                    },
                    .routeMsg = {
                        .rtm_family = AF_INET,
                        .rtm_dst_len = 0,
                        .rtm_table = RT_TABLE_MAIN,
                        .rtm_scope = RT_SCOPE_UNIVERSE,
                        .rtm_type = RTN_UNICAST
                    },
                    .gatewayAttr = {
                        .nla_len = sizeof(request.gatewayAttr) + sizeof(request.gateway),
                        .nla_type = RTA_GATEWAY
                    }
                };
                hc_MEMCPY(&request.gateway, &router->data[0], sizeof(request.gateway));
                CHECK(netlink_talk(config.rtnetlinkFd, &request, sizeof(request)), RES == 0);
            }
        }
    }
}

static void dhcpClient_deinit(void) {
    debug_CHECK(sys_close(dhcpClient.timerFd), RES == 0);
    debug_CHECK(sys_close(dhcpClient.fd), RES == 0);
}