#include "device/device.h"
#include <stdio.h>
#include <string.h>
#include "pico/unique_id.h"
#include "pico/version.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/sysinfo.h"

void device_get_info(device_info_t *info)
{
    if (!info) return;

    strncpy(info->board, DEVICE_BOARD, sizeof(info->board) - 1);
    info->board[sizeof(info->board) - 1] = '\0';

    pico_get_unique_board_id_string(info->serial, sizeof(info->serial));

    volatile uint32_t *chip_id = (uint32_t *)(SYSINFO_BASE + SYSINFO_CHIP_ID_OFFSET);
    uint32_t id = *chip_id;
    info->manufacturer = (id & SYSINFO_CHIP_ID_MANUFACTURER_BITS) >> SYSINFO_CHIP_ID_MANUFACTURER_LSB;
    info->part         = (id & SYSINFO_CHIP_ID_PART_BITS)         >> SYSINFO_CHIP_ID_PART_LSB;
    info->revision     = (id & SYSINFO_CHIP_ID_REVISION_BITS)     >> SYSINFO_CHIP_ID_REVISION_LSB;

    strncpy(info->sdk_version, PICO_SDK_VERSION_STRING, sizeof(info->sdk_version) - 1);
    info->sdk_version[sizeof(info->sdk_version) - 1] = '\0';
}