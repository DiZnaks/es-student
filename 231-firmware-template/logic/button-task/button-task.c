#include "button-task/button-task.h"
#include "platform/platform.h"
#include "systime/systime.h"

#include <stddef.h> // для NULL
#include <stdbool.h>

#define DEBOUNCE_US 20000

typedef enum {
    BTN_RELEASED,
    BTN_PRESSING,
    BTN_PRESSED,
    BTN_RELEASING,
} button_state_t;

static button_state_t button_state = BTN_RELEASED;
static uint64_t       state_start_us = 0;
static uint32_t       press_count = 0;
static button_task_callback_t press_callback = NULL;

void button_task_init(button_task_callback_t on_press)
{
    // НИКАКИХ gpio_init! Железо настроено в platform_init()
    
    button_state = BTN_RELEASED;
    state_start_us = 0;
    press_count = 0;
    
    // Сохраняем переданный указатель на функцию
    press_callback = on_press;
}

void button_task_handle(void)
{
    bool pressed = platform_button_read();
    uint64_t now_us = systime_us();

    switch (button_state) {
    case BTN_RELEASED:
        if (pressed) {
            state_start_us = now_us;
            button_state = BTN_PRESSING;
        }
        break;

    case BTN_PRESSING:
        if (!pressed) {
            button_state = BTN_RELEASED;          // дребезг или помеха
        } else if (now_us - state_start_us >= DEBOUNCE_US) {
            button_state = BTN_PRESSED;           // подтверждено
            press_count++;
            // Вызываем переданную функцию (если она есть)
            if (press_callback != NULL) {
                press_callback();
            }
        }
        break;

    case BTN_PRESSED:
        if (!pressed) {
            state_start_us = now_us;
            button_state = BTN_RELEASING;
        }
        break;

    case BTN_RELEASING:
        if (pressed) {
            button_state = BTN_PRESSED;           // дребезг отпускания
        } else if (now_us - state_start_us >= DEBOUNCE_US) {
            button_state = BTN_RELEASED;
        }
        break;
    }
}

bool button_task_is_pressed(void)
{
    return button_state == BTN_PRESSED || button_state == BTN_RELEASING;
}

uint32_t button_task_get_press_count(void)
{
    return press_count;
}