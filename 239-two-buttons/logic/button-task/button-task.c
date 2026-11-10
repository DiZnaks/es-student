#include "button-task/button-task.h"
#include "platform/platform.h"
#include "systime/systime.h"

#include <stddef.h>

#define DEBOUNCE_US 20000
#define NUM_BUTTONS 2

typedef enum {
    BTN_RELEASED, BTN_PRESSING, BTN_PRESSED, BTN_RELEASING,
} button_state_t;

typedef struct {
    button_state_t state;
    uint64_t       state_start_us;
    uint32_t       press_count;
    button_task_callback_t callback;
} button_data_t;

static button_data_t buttons[NUM_BUTTONS];

void button_task_init(platform_button_t button, button_task_callback_t on_press)
{
    buttons[button].state = BTN_RELEASED;
    buttons[button].state_start_us = 0;
    buttons[button].press_count = 0;
    buttons[button].callback = on_press;
}

void button_task_handle(void)
{
    uint64_t now_us = systime_us();

    // Обходим все кнопки платы
    for (int i = 0; i < NUM_BUTTONS; i++) {
        bool pressed = platform_button_read((platform_button_t)i);
        button_data_t *b = &buttons[i];

        switch (b->state) {
        case BTN_RELEASED:
            if (pressed) {
                b->state_start_us = now_us;
                b->state = BTN_PRESSING;
            }
            break;

        case BTN_PRESSING:
            if (!pressed) {
                b->state = BTN_RELEASED;
            } else if (now_us - b->state_start_us >= DEBOUNCE_US) {
                b->state = BTN_PRESSED;
                b->press_count++;
                if (b->callback != NULL) {
                    b->callback();
                }
            }
            break;

        case BTN_PRESSED:
            if (!pressed) {
                b->state_start_us = now_us;
                b->state = BTN_RELEASING;
            }
            break;

        case BTN_RELEASING:
            if (pressed) {
                b->state = BTN_PRESSED;
            } else if (now_us - b->state_start_us >= DEBOUNCE_US) {
                b->state = BTN_RELEASED;
            }
            break;
        }
    }
}

bool button_task_is_pressed(platform_button_t button)
{
    return buttons[button].state == BTN_PRESSED || buttons[button].state == BTN_RELEASING;
}

uint32_t button_task_get_press_count(platform_button_t button)
{
    return buttons[button].press_count;
}