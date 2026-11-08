#include <stdio.h>

#include "pico/stdlib.h"
#include "device.h"
#include "profiling.h"
#include "api.h"
#include "stdio-text-protocol.h"
#include "led-task.h"
#include "button-task.h"
#include "pi-task.h"


int main(void)
{
    stdio_init_all();
    led_task_init(); 
    profiling_init();
    button_task_init(); 
    pi_task_init(); 
    stdio_text_protocol_init();

    while (1) {
        profiling_iteration();

        const command_t *command = stdio_text_protocol_handle();
        if (command != NULL) {
            api_handle(command);
        }

        led_task_handle();
        button_task_handle();
        pi_task_handle(); 
    }
}