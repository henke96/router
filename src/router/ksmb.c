struct ksmb {
    int32_t netlinkFd;
    uint16_t familyId;
    int16_t __pad;
};

static struct ksmb ksmb;

static hc_COLD void ksmb_init(void) {
    genetlink_requestFamily(KSMBD_GENL_NAME);
    struct nlattr *familyId = genetlink_findAttr(CTRL_ATTR_FAMILY_ID);
    ksmb.familyId = *(uint16_t *)&familyId[1];

    ksmb.netlinkFd = sys_socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    CHECK(ksmb.netlinkFd, RES > 0);

    struct {
        struct nlmsghdr hdr;
        struct genlmsghdr genHdr;
        struct nlattr requestAttr;
        struct ksmbd_startup_request request;
    } startupRequest = {
        .hdr = {
            .nlmsg_len = sizeof(startupRequest),
            .nlmsg_type = ksmb.familyId,
            .nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK,
        },
        .genHdr = {
            .cmd = KSMBD_EVENT_STARTING_UP,
            .version = KSMBD_GENL_VERSION
        },
        .requestAttr = {
            .nla_type = KSMBD_EVENT_STARTING_UP,
            .nla_len = sizeof(startupRequest.requestAttr) + sizeof(startupRequest.request)
        },
        .request = {
            .flags = 0,
            .signing = KSMBD_CONFIG_OPT_AUTO,
            .min_prot = {0},
            .max_prot = {0},
            .netbios_name = "ROUTER",
            .work_group = "WORKGROUP",
            .server_string = "ROUTER",
            .tcp_port = 445,
            .ipc_timeout = 0,
            .deadtime = 0,
            .file_max = 8192,
            .smb2_max_write = 0,
            .smb2_max_read = 0,
            .smb2_max_trans = 0,
            .share_fake_fscaps = 64,
            .sub_auth = {0},
            .smb2_max_credits = 0,
            .ifc_list_sz = 0
        }
    };
    netlink_talk(ksmb.netlinkFd, &(struct iovec) { .iov_base = &startupRequest, .iov_len = sizeof(startupRequest) }, 1);
}

static void ksmb_onNetlinkFd(void) {
    netlink_receive(ksmb.netlinkFd);
    struct nlmsghdr *hdr = (void *)&buffer[0];
    struct genlmsghdr *genHdr = (void *)&hdr[1];
    struct nlattr *attr = (void *)&genHdr[1];
    CHECK(attr->nla_type, RES == genHdr->cmd);
    debug_printNum("Got: ", attr->nla_type, "\n");
    switch (attr->nla_type) {
        case KSMBD_EVENT_LOGIN_REQUEST: {
            struct ksmbd_login_request *request = (void *)&attr[1];

            struct {
                struct nlmsghdr hdr;
                struct genlmsghdr genHdr;
                struct nlattr responseAttr;
                struct ksmbd_login_response response;
            } loginResponse = {
                .hdr = {
                    .nlmsg_len = sizeof(loginResponse),
                    .nlmsg_type = ksmb.familyId,
                    .nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK,
                },
                .genHdr = {
                    .cmd = KSMBD_EVENT_LOGIN_RESPONSE,
                    .version = KSMBD_GENL_VERSION
                },
                .responseAttr = {
                    .nla_type = KSMBD_EVENT_LOGIN_RESPONSE,
                    .nla_len = sizeof(loginResponse.responseAttr) + sizeof(loginResponse.response)
                },
                .response = {
                    .handle = request->handle,
                    .gid = 0,
                    .uid = 0,
                    .account = {0},
                    .status = KSMBD_USER_FLAG_OK | KSMBD_USER_FLAG_GUEST_ACCOUNT,
                    .hash_sz = 16,
                    .hash = {0}
                }
            };
            hc_MEMCPY(&loginResponse.response.account[0], &request->account[0], KSMBD_REQ_MAX_ACCOUNT_NAME_SZ);
            netlink_talk(ksmb.netlinkFd, &(struct iovec) { .iov_base = &loginResponse, .iov_len = sizeof(loginResponse) }, 1);
            break;
        }
        case KSMBD_EVENT_TREE_CONNECT_REQUEST: {
            struct ksmbd_tree_connect_request *request = (void *)&attr[1];

            uint16_t status = KSMBD_TREE_CONN_STATUS_NO_SHARE;
            if (util_cstrCmp(&request->share[0], "config") == 0) status = KSMBD_TREE_CONN_STATUS_OK;

            struct {
                struct nlmsghdr hdr;
                struct genlmsghdr genHdr;
                struct nlattr responseAttr;
                struct ksmbd_tree_connect_response response;
            } treeResponse = {
                .hdr = {
                    .nlmsg_len = sizeof(treeResponse),
                    .nlmsg_type = ksmb.familyId,
                    .nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK,
                },
                .genHdr = {
                    .cmd = KSMBD_EVENT_TREE_CONNECT_RESPONSE,
                    .version = KSMBD_GENL_VERSION
                },
                .responseAttr = {
                    .nla_type = KSMBD_EVENT_TREE_CONNECT_RESPONSE,
                    .nla_len = sizeof(treeResponse.responseAttr) + sizeof(treeResponse.response)
                },
                .response = {
                    .handle = request->handle,
                    .status = status,
                    .connection_flags = KSMBD_TREE_CONN_FLAG_WRITABLE
                }
            };
            netlink_talk(ksmb.netlinkFd, &(struct iovec) { .iov_base = &treeResponse, .iov_len = sizeof(treeResponse) }, 1);
            break;
        }
        case KSMBD_EVENT_SHARE_CONFIG_REQUEST: {
            struct ksmbd_share_config_request *request = (void *)&attr[1];
            struct {
                struct nlmsghdr hdr;
                struct genlmsghdr genHdr;
                struct nlattr responseAttr;
                struct ksmbd_share_config_response response;
                char path[4];
            } shareResponse = {
                .hdr = {
                    .nlmsg_len = sizeof(shareResponse),
                    .nlmsg_type = ksmb.familyId,
                    .nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK,
                },
                .genHdr = {
                    .cmd = KSMBD_EVENT_SHARE_CONFIG_RESPONSE,
                    .version = KSMBD_GENL_VERSION
                },
                .responseAttr = {
                    .nla_type = KSMBD_EVENT_SHARE_CONFIG_RESPONSE,
                    .nla_len = sizeof(shareResponse.responseAttr) + sizeof(shareResponse.response) + sizeof(shareResponse.path)
                },
                .response = {
                    .handle = request->handle,
                    .flags = KSMBD_SHARE_FLAG_AVAILABLE | KSMBD_SHARE_FLAG_BROWSEABLE | KSMBD_SHARE_FLAG_WRITEABLE,
                    .create_mask = 0744,
                    .directory_mask = 0755,
                    .force_create_mode = 0,
                    .force_directory_mode = 0,
                    .force_uid = (uint16_t)-1,
                    .force_gid = (uint16_t)-1,
                    .veto_list_sz = 0,
                },
                .path = "/mnt"
            };
            netlink_talk(ksmb.netlinkFd, &(struct iovec) { .iov_base = &shareResponse, .iov_len = sizeof(shareResponse) }, 1);
            break;
        }
    }
}

static hc_COLD void ksmb_deinit(void) {
    debug_CHECK(sys_close(ksmb.netlinkFd), RES == 0);
}
