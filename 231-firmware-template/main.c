#include <stdio.h>

#include "pico/stdlib.h"
#include "led.h"
#include "device.h"
#include "profiling.h"
#include "api.h"
#include "stdio-text-protocol.h"

static const uint32_t BLINK_HALF_PERIOD_US = 500000;
static uint64_t last_toggle_us = 0;

static void blink(void)
{
    uint64_t now_us = time_us_64();
    if (now_us - last_toggle_us >= BLINK_HALF_PERIOD_US) {
        last_toggle_us += BLINK_HALF_PERIOD_US;      // от прошлой метки (т2.3.3)
        led_toggle();
    }
}

int main(void)
{
    stdio_init_all();
    led_init();
    profiling_init();
    stdio_text_protocol_init();

    while (1) {
        profiling_iteration();

        const command_t *command = stdio_text_protocol_handle();
        if (command != NULL) {
            api_handle(command);
        }

        blink();
    }
}