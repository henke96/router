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
#include "config.c"

int32_t main(hc_UNUSED int32_t argc, hc_UNUSED char **argv) {
    config_init();
    config_configure();
    config_deinit();
    uint8_t c;
    sys_read(STDIN_FILENO, &c, 1); // Block until user inputs.
    return 0;
}