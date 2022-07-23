struct acpi {
    int32_t netlinkFd;
    uint16_t familyId;
    uint16_t mcastGrpId;
};

static struct acpi acpi;

static void acpi_init(void) {
    acpi.netlinkFd = sys_socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    CHECK(acpi.netlinkFd, RES > 0);

    // Get acpi family id and multicast group id.
    struct getFamilyRequest {
        struct nlmsghdr hdr;
        struct genlmsghdr genHdr;
        struct nlattr familyNameAttr;
        char familyName[sizeof(ACPI_EVENT_FAMILY_NAME)];
        char familyNamePad[util_PAD_BYTES(sizeof(ACPI_EVENT_FAMILY_NAME), 4)];
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
        .familyName = ACPI_EVENT_FAMILY_NAME
    };
    netlink_talk(acpi.netlinkFd, &request, sizeof(request));

    for (
        struct nlattr *attr = (void *)&buffer[sizeof(struct nlmsghdr) + sizeof(struct genlmsghdr)];;
        attr = (void *)&((char *)attr)[util_ALIGN_FORWARD(attr->nla_len, 4)]
    ) {
        if (attr->nla_type == CTRL_ATTR_FAMILY_ID) {
            acpi.familyId = *(uint16_t *)&attr[1];
        } else if (attr->nla_type == CTRL_ATTR_MCAST_GROUPS) {
            // TODO: This is super ugly, assuming there is only one mcast group, and that the
            //       ID is the first attribute.. Also, we assume mcast groups come after family id.
            acpi.mcastGrpId = *(uint16_t *)&attr[3];
            break;
        }
    }

    // For whatever reason we can't bind after sending a request, so recreate socket. TODO: Why?
    debug_CHECK(sys_close(acpi.netlinkFd), RES == 0);
    acpi.netlinkFd = sys_socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    CHECK(acpi.netlinkFd, RES > 0);

    // Bind the mcast group.
    struct sockaddr_nl addr = {
        .nl_family = AF_NETLINK,
        .nl_groups = 1 << (acpi.mcastGrpId - 1)
    };
    CHECK(sys_bind(acpi.netlinkFd, &addr, sizeof(addr)), RES == 0);
}

static void acpi_deinit(void) {
    debug_CHECK(sys_close(acpi.netlinkFd), RES == 0);
}