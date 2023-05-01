#include "hc/hc.h"
#include "hc/util.c"
#include "hc/linux/linux.h"
#include "hc/linux/sys.c"
#include "hc/linux/debug.c"
#include "hc/linux/android/liblog.so.h"
#include "hc/linux/android/libdl.so.h"
#include "hc/linux/helpers/_start.c"

int32_t start(hc_UNUSED int32_t argc, hc_UNUSED char **argv) {
    __android_log_write(android_LOG_INFO, "android", "Hello!\n");
    return 0;
}
