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
static char buffer[8192] hc_ALIGNED(8); // See NLMSG_GOODSIZE in <linux/netlink.h>

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
    dhcpServer_init(&dhcpServer, 3, *(uint32_t *)&lanIp[0]);

    // Setup epoll.
    int32_t epollFd = sys_epoll_create1(0);
    CHECK(epollFd, RES > 0);
    CHECK(sys_epoll_ctl(epollFd, EPOLL_CTL_ADD, acpi.netlinkFd, &(struct epoll_event) { .events = EPOLLIN, .data.fd = acpi.netlinkFd }), RES == 0);
    CHECK(sys_epoll_ctl(epollFd, EPOLL_CTL_ADD, dhcpClient.fd, &(struct epoll_event) { .events = EPOLLIN, .data.fd = dhcpClient.fd }), RES == 0);
    CHECK(sys_epoll_ctl(epollFd, EPOLL_CTL_ADD, dhcpClient.timerFd, &(struct epoll_event) { .events = EPOLLIN, .data.fd = dhcpClient.timerFd }), RES == 0);
    CHECK(sys_epoll_ctl(epollFd, EPOLL_CTL_ADD, dhcpServer.fd, &(struct epoll_event) { .events = EPOLLIN, .data.fd = dhcpServer.fd }), RES == 0);
    CHECK(sys_epoll_ctl(epollFd, EPOLL_CTL_ADD, STDIN_FILENO, &(struct epoll_event) { .events = EPOLLIN, .data.fd = STDIN_FILENO }), RES == 0);

    bool isDumping = false;
    struct packetDumper wanDumper;
    for (;;) {
        struct epoll_event event;
        CHECK(sys_epoll_pwait(epollFd, &event, 1, -1, NULL), RES == 1);
        if (event.data.fd == acpi.netlinkFd) break;

        if (event.data.fd == STDIN_FILENO) {
            debug_CHECK(sys_read(STDIN_FILENO, &buffer[0], sizeof(buffer)), RES > 0);
            if (isDumping) {
                packetDumper_deinit(&wanDumper);
                sys_write(STDOUT_FILENO, "stop dumping\n", 13);
            } else {
                uint8_t dumpDestIp[4] hc_ALIGNED(4) = { 10, 123, 0, 255 };
                packetDumper_init(&wanDumper, 2, *(uint32_t *)&dumpDestIp[0]);
                CHECK(sys_epoll_ctl(epollFd, EPOLL_CTL_ADD, wanDumper.packetFd, &(struct epoll_event) { .events = EPOLLIN, .data.fd = wanDumper.packetFd }), RES == 0);
                sys_write(STDOUT_FILENO, "start dumping\n", 14);
            }
            isDumping = !isDumping;
        } else if (event.data.fd == dhcpClient.fd) dhcpClient_onMessage();
        else if (event.data.fd == dhcpClient.timerFd) dhcpClient_onTimer();
        else if (event.data.fd == dhcpServer.fd) dhcpServer_onMessage(&dhcpServer);
        else if (event.data.fd == wanDumper.packetFd) packetDumper_onMessage(&wanDumper);
        else hc_UNREACHABLE;
    }

    if (isDumping) packetDumper_deinit(&wanDumper);
    dhcpServer_deinit(&dhcpServer);
    dhcpClient_deinit();
    config_deinit();
    acpi_deinit();
    return 0;
}