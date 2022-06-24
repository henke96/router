#include "hc/hc.h"
#include "hc/util.c"
#include "hc/windows/windows.h"

void noreturn main(void) {
    void *stdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (stdOutHandle == INVALID_HANDLE_VALUE) ExitProcess(1);
    WriteFile(stdOutHandle, "Hello!\n", 7, NULL, NULL);
    MessageBoxW(NULL, u"Hey!", u"Yo", MB_OK);
    ExitProcess(0);
}