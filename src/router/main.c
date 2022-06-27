#include "hc/hc.h"
#include "hc/util.c"
#include "hc/base64.c"
#include "hc/libc/small.c"
#include "hc/linux/linux.h"
#include "hc/linux/sys.c"
#include "hc/linux/debug.c"
#include "hc/linux/helpers/_start.c"

#define CHECK(EXPR, COND) do { typeof(EXPR) RES = (EXPR); if (!(COND)) debug_fail((int64_t)RES, #EXPR, __FILE_NAME__, __LINE__); } while (0)

#include "netlink.c"
#include "acpi.c"
#include "config.c"

int32_t main(hc_UNUSED int32_t argc, hc_UNUSED char **argv) {
    acpi_init();
    config_init();
    config_configure();

    uint8_t c;
    CHECK(sys_read(acpi.netlinkFd, &c, 1), RES == 1); // Wait for any acpi event..

    config_deinit();
    acpi_deinit();
    return 0;
}