#include "hc/hc.h"
#include "hc/util.c"
#include "hc/libc/small.c"
#include "hc/linux/linux.h"
#include "hc/linux/sys.c"
#include "hc/linux/debug.c"
#include "hc/linux/helpers/_start.c"

#include "rtnetlink.c"
#include "config.c"

int32_t main(hc_UNUSED int32_t argc, hc_UNUSED char **argv) {
    int32_t status = config_init();
    if (status < 0) {
        debug_printNum("Failed to initialise config (", status, ")\n");
        return 1;
    }
    status = config_configure();
    config_deinit();
    if (status < 0) {
        debug_printNum("configure failed (", status, ")\n");
        return 1;
    }

    sys_read(STDIN_FILENO, &status, 1); // Block until user inputs.
    return 0;
}