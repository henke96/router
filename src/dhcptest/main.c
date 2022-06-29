#include "hc/hc.h"
#include "hc/util.c"
#include "hc/libc/small.c"
#include "hc/linux/linux.h"
#include "hc/linux/sys.c"
#include "hc/linux/debug.c"
#include "hc/linux/helpers/_start.c"

#define CHECK(EXPR, COND) do { typeof(EXPR) RES = (EXPR); if (!(COND)) debug_fail((int64_t)RES, #EXPR, __FILE_NAME__, __LINE__); } while (0)

int32_t main(hc_UNUSED int32_t argc, hc_UNUSED char **argv) {
    int32_t fd = sys_socket(AF_INET, SOCK_DGRAM, 0);
    CHECK(fd, RES > 0);

    CHECK(sys_setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, "eth0", 7), RES == 0);
    int32_t broadcast = 1;
    CHECK(sys_setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)), RES == 0);

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = hc_BSWAP16(68),
        .sin_addr = { 0, 0, 0, 0 }
    };
    CHECK(sys_bind(fd, &addr, sizeof(addr)), RES == 0);

    struct sockaddr_in destAddr = {
        .sin_family = AF_INET,
        .sin_port = hc_BSWAP16(67),
        .sin_addr = { 255, 255, 255, 255 }
    };
    uint8_t data = 137;
    CHECK(sys_sendto(fd, &data, sizeof(data), MSG_NOSIGNAL, &destAddr, sizeof(destAddr)), RES == sizeof(data));

    CHECK(sys_recvfrom(fd, &data, 1, 0, NULL, NULL), RES == 1);
    return 0;
}