#include "button-task.h"
#include "led-task.h"
#include "pico/stdlib.h"

#ifndef BUTTON_PIN
#define BUTTON_PIN 15
#endif

// Время подавления дребезга: 20 мс в микросекундах
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

// Уровень на выводе: 0 = нажата (замыкает на землю), 1 = отпущена
static bool button_level_pressed(void)
{
    return gpio_get(BUTTON_PIN) == 0;
}

void button_task_init(void)
{
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);         // подтяжка к питанию

    button_state = BTN_RELEASED;
    state_start_us = 0;
    press_count = 0;
}

void button_task_handle(void)
{
    bool pressed = button_level_pressed();
    uint64_t now_us = time_us_64();

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
            led_task_next_state();                // одно нажатие — одно переключение
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
    // «нажата» и «отпускание?» — кнопка ещё нажата
    return button_state == BTN_PRESSED || button_state == BTN_RELEASING;
}

uint32_t button_task_get_press_count(void)
{
    return press_count;
}