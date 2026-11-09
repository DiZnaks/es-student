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

typedef struct {
    char     board[32]; 
    char     serial[17];        // 16 hex + '\0'
    uint32_t manufacturer;
    uint32_t part;
    uint32_t revision;
    char     sdk_version[16];
} device_info_t;

void device_get_info(device_info_t *info);