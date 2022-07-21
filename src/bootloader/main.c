#include "hc/hc.h"
#include "hc/efi.h"
#include "hc/util.c"
#include "hc/libc/small.c"
#include "linuxBin.c"

#define linuxCmdLine_GUID_START 14
static uint16_t linuxCmdLine[] = u"root=PARTUUID=\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0 rootwait mitigations=off video=efifb:width:800,height:480";
static const uint8_t hexTable[16] = "0123456789abcdef";

int64_t main(void *imageHandle, struct efi_systemTable *systemTable) {
    // Find partition GUID.
    struct efi_loadedImageProtocol *loadedImageProtocol;
    int64_t status = systemTable->bootServices->handleProtocol(imageHandle, &(struct efi_guid) efi_guid_LOADED_IMAGE_PROTOCOL, (void **)&loadedImageProtocol);
    if (status < 0) return 1;

    struct efi_devicePathProtocol *devicePathProtocol;
    status = systemTable->bootServices->handleProtocol(loadedImageProtocol->deviceHandle, &(struct efi_guid) efi_guid_DEVICE_PATH_PROTOCOL, (void **)&devicePathProtocol);
    if (status < 0) return 1;

    while (devicePathProtocol->type != 0x7F) {
        if (devicePathProtocol->type == 4 && devicePathProtocol->subType == 1) { // Hard drive partition.
            struct efi_devicePathProtocol_mediaDevicePath *mediaDevicePath = (void *)devicePathProtocol;
            if (mediaDevicePath->signatureType == 2) { // GUID signature.
                // Write GUID to Linux command line.
                const struct {
                    int8_t start;
                    int8_t end;
                    int8_t step;
                } guidPrint[5] = { { 9, 15, 1 }, { 7, 9, 1 }, { 8, 6, -1 }, { 6, 4, -1 }, { 4, 0, -1 } };

                uint16_t *current = &linuxCmdLine[linuxCmdLine_GUID_START];
                int32_t i = 5;
                for (;;) {
                    --i;
                    int32_t readIndex = guidPrint[i].start;
                    do {
                        readIndex += guidPrint[i].step;
                        uint8_t byte = mediaDevicePath->partitionSignature[readIndex];
                        *current++ = hexTable[byte >> 4];
                        *current++ = hexTable[byte & 0xF];
                    } while (readIndex != guidPrint[i].end);
                    if (i != 0) *current++ = '-';
                    else goto foundPartitionGuid;
                }
            }
        }
        uint16_t length;
        hc_MEMCPY(&length, devicePathProtocol->length, sizeof(length));
        devicePathProtocol = (void *)&((char *)devicePathProtocol)[length];
    }
    return 1; // Not found.
    foundPartitionGuid:;

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