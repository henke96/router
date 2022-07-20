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
static char buffer[8192]; // See NLMSG_GOODSIZE in <linux/netlink.h>

#include "dhcp.h"
#include "netlink.c"
#include "acpi.c"
#include "config.c"
#include "dhcpClient.c"
#include "iptables.c"

int32_t main(hc_UNUSED int32_t argc, hc_UNUSED char **argv) {
    acpi_init();
    config_init();
    config_configure();

    dhcpClient_init();
    iptables_configure();

    // Setup epoll.
    int32_t epollFd = sys_epoll_create1(0);
    CHECK(epollFd, RES > 0);
    CHECK(sys_epoll_ctl(epollFd, EPOLL_CTL_ADD, acpi.netlinkFd, &(struct epoll_event) { .events = EPOLLIN, .data.fd = acpi.netlinkFd }), RES == 0);
    CHECK(sys_epoll_ctl(epollFd, EPOLL_CTL_ADD, dhcpClient.fd, &(struct epoll_event) { .events = EPOLLIN, .data.fd = dhcpClient.fd }), RES == 0);
    CHECK(sys_epoll_ctl(epollFd, EPOLL_CTL_ADD, dhcpClient.timerFd, &(struct epoll_event) { .events = EPOLLIN, .data.fd = dhcpClient.timerFd }), RES == 0);

    for (;;) {
        struct epoll_event event;
        CHECK(sys_epoll_pwait(epollFd, &event, 1, -1, NULL), RES == 1);
        if (event.data.fd == acpi.netlinkFd) break;

        if (event.data.fd == dhcpClient.fd) dhcpClient_onMessage();
        else if (event.data.fd == dhcpClient.timerFd) dhcpClient_onTimer();
        else hc_UNREACHABLE;
    }

    dhcpClient_deinit();
    config_deinit();
    acpi_deinit();
    return 0;
}