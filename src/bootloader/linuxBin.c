extern uint8_t linuxBin[];
extern uint32_t linuxBin_size;

asm(
    ".section .rdata\n"
    ".global linuxBin\n"
    ".align 8\n"
    "linuxBin:\n"
    ".incbin \"bzImage\"\n"
    "linuxBin_end:\n"
    ".global linuxBin_size\n"
    ".align 4\n"
    "linuxBin_size:\n"
    ".int linuxBin_end - linuxBin\n"
);