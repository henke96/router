#include "hc/hc.h"
#include "hc/util.c"
#include "hc/base64.c"
#include "hc/libc/small.c"
#include "hc/linux/linux.h"
#include "hc/linux/sys.c"
#include "hc/linux/debug.c"
#include "hc/linux/helpers/_start.c"

#define CHECK(EXPR, COND) do { typeof(EXPR) RES = (EXPR); if (!(COND)) debug_fail((int64_t)RES, #EXPR, __FILE_NAME__, __LINE__); } while (0)

// Shared buffer space for whole program.
static char buffer[66000] hc_ALIGNED(8); // Netlink wants 8192, see NLMSG_GOODSIZE in <linux/netlink.h>.
                                         // packetDumper wants to support jumbo frames, so use 66000 to be (very) safe.

static void epollAdd(int32_t epollFd, int32_t fd) {
    CHECK(sys_epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &(struct epoll_event) { .events = EPOLLIN, .data.fd = fd }), RES == 0);
}

#include "dhcp.h"
#include "netlink.c"
#include "acpi.c"
#include "config.c"
#include "dhcpClient.c"
#include "dhcpServer.c"
#include "iptables.c"
#include "packetDumper.c"

int32_t main(hc_UNUSED int32_t argc, hc_UNUSED char **argv) {
    acpi_init();
    config_init();
    config_configure();

    dhcpClient_init();
    iptables_configure();

    struct dhcpServer dhcpServer;
    uint8_t lanIp[4] hc_ALIGNED(4) = { 10, 123, 0, 1 };
    dhcpServer_init(&dhcpServer, 4, *(uint32_t *)&lanIp[0]);

    struct packetDumper wanDumper;
    packetDumper_init(&wanDumper, config_WAN_IF_INDEX);

    // Setup epoll.
    int32_t epollFd = sys_epoll_create1(0);
    CHECK(epollFd, RES > 0);
    epollAdd(epollFd, acpi.netlinkFd);
    epollAdd(epollFd, dhcpClient.fd);
    epollAdd(epollFd, dhcpClient.timerFd);
    epollAdd(epollFd, dhcpServer.fd);
    epollAdd(epollFd, wanDumper.listenFd);

    for (;;) {
        struct epoll_event event;
        CHECK(sys_epoll_pwait(epollFd, &event, 1, -1, NULL), RES == 1);
        if (event.data.fd == acpi.netlinkFd) break;

        if (event.data.fd == dhcpClient.fd) dhcpClient_onMessage();
        else if (event.data.fd == dhcpClient.timerFd) dhcpClient_onTimer();
        else if (event.data.fd == dhcpServer.fd) dhcpServer_onMessage(&dhcpServer);
        else if (event.data.fd == wanDumper.listenFd) packetDumper_onListenFd(&wanDumper, epollFd);
        else if (event.data.fd == wanDumper.packetFd) packetDumper_onPacketFd(&wanDumper);
        else if (event.data.fd == wanDumper.clientFd) packetDumper_onClientFd(&wanDumper);
        else hc_UNREACHABLE;
    }

    packetDumper_deinit(&wanDumper);
    dhcpServer_deinit(&dhcpServer);
    dhcpClient_deinit();
    config_deinit();
    acpi_deinit();
    return 0;
}