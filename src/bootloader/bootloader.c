#include "hc/hc.h"
#include "hc/efi.h"
#include "hc/util.c"
#include "hc/libc/small.c"
#include "linuxBin.c"

static uint16_t linuxCmdLine[] = L"         mitigations=off video=efifb:width:800,height:480";

int64_t _start(void *imageHandle, struct efi_systemTable *systemTable) {
    // Read fat volume id.
    struct efi_loadedImageProtocol *loadedImageProtocol;
    int64_t status = systemTable->bootServices->handleProtocol(imageHandle, &(struct efi_guid) efi_guid_LOADED_IMAGE_PROTOCOL, (void **)&loadedImageProtocol);
    if (status < 0) return 1;

    struct efi_blockIOProtocol *blockIOProtocol;
    status = systemTable->bootServices->handleProtocol(loadedImageProtocol->deviceHandle, &(struct efi_guid) efi_guid_BLOCK_IO_PROTOCOL, (void **)&blockIOProtocol);
    if (status < 0) return 1;

    // Allocate buffer to read first disk block.
    uint8_t *fatBuffer;
    status = systemTable->bootServices->allocatePool(efi_LOADER_DATA, blockIOProtocol->media->blockSize, (void **)&fatBuffer);
    if (status < 0) return 1;

    status = blockIOProtocol->readBlocks(blockIOProtocol, blockIOProtocol->media->mediaId, 0, blockIOProtocol->media->blockSize, fatBuffer);
    if (status < 0) return 1;

    uint8_t volumeId[4];
    hc_MEMCPY(&volumeId[0], &fatBuffer[0x27], sizeof(volumeId));

    uint16_t *current = &linuxCmdLine[0];
    int32_t i = 4;
    do {
        --i;
        static const uint8_t hexTable[16] = "0123456789abcdef";
        *current++ = hexTable[volumeId[i] >> 4];
        *current++ = hexTable[volumeId[i] & 0xF];
    } while (i);

    // Load Linux image.
    void *linuxImage;
    status = systemTable->bootServices->loadImage(0, imageHandle, NULL, &linuxBin[0], linuxBin_size, &linuxImage);
    if (status < 0) return 1;

    // Set command line.
    status = systemTable->bootServices->handleProtocol(linuxImage, &(struct efi_guid) efi_guid_LOADED_IMAGE_PROTOCOL, (void **)&loadedImageProtocol);
    if (status < 0) return 1;
    loadedImageProtocol->loadOptions = &linuxCmdLine[0];
    loadedImageProtocol->loadOptionsSize = sizeof(linuxCmdLine);

    systemTable->bootServices->startImage(linuxImage, NULL, NULL);
    return 1;
}
