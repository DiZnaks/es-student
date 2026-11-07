#pragma once

#include <stdint.h>

#define DEVICE_NAME "es-cmd-usb"
#define FIRMWARE_VERSION "1.1.0"
#define F_VERSION 0x00010000

#define DEVICE_PROJECT "231-firmware-template"
#define DEVICE_REPO "https://github.com/DiZnaks/es-student"

#ifndef DEVICE_BOARD
#define DEVICE_BOARD "unknown"
#endif

// struct info_t
// {
//     uint32_t version;
//     char name[13];
//     uint8_t revision;
// };

void device_info(void);