#include "led-task/led-task.h"
#include "systime/systime.h"
#include "platform/platform.h"


static led_state_t led_state = LED_STATE_BLINK;
static uint32_t    period_ms = 1000;   
static bool        led_is_on = false;
static uint64_t    last_toggle_us = 0;


void led_task_init(void)
{
    period_ms = 1000;
    led_task_set_state(LED_STATE_BLINK);
}

void led_task_set_state(led_state_t state)
{
    led_state = state;

    switch (state) {
    case LED_STATE_OFF:
        led_is_on = false;
        platform_led_set(0);
        break;
    case LED_STATE_ON:
        led_is_on = true;
        platform_led_set(1);
        break;
    case LED_STATE_BLINK:
        last_toggle_us = 0;
        break;
    }
}

void led_task_handle(void)
{
    switch (led_state) {
    case LED_STATE_OFF:
    case LED_STATE_ON:
        break;
    case LED_STATE_BLINK:
        if (systime_period_elapsed(&last_toggle_us, (uint64_t)period_ms * 1000 / 2)) {
            led_is_on = !led_is_on;
            platform_led_set(led_is_on);
        }
        break;
    }
}

led_state_t led_task_get_state(void)
{
    return led_state;
}

bool led_task_set_period_ms(uint32_t p)
{
    if (p == 0) {
        return false; // Нулевой период не принимаем
    }
    period_ms = p;
    return true;
}

uint32_t led_task_get_period_ms(void)
{
    return period_ms;
}

void led_task_next_state(void)
{
    switch (led_state) {
    case LED_STATE_OFF:
        led_task_set_state(LED_STATE_ON);
        break;
    case LED_STATE_ON:
        led_task_set_state(LED_STATE_BLINK);
        break;
    case LED_STATE_BLINK:
        led_task_set_state(LED_STATE_OFF);
        break;
    }
}

void led_task_next_period(void)
{
    if (period_ms == 250) period_ms = 500;
    else if (period_ms == 500) period_ms = 1000;
    else period_ms = 250;
}