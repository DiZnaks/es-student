#include "platform/platform.h"
#include "pico/stdlib.h"

#define LED_PIN    25
#define BUTTON_1_PIN 15
#define BUTTON_2_PIN 14

void platform_init(void)
{
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);

    gpio_init(BUTTON_1_PIN);
    gpio_set_dir(BUTTON_1_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_1_PIN);

    gpio_init(BUTTON_2_PIN);
    gpio_set_dir(BUTTON_2_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_2_PIN);
}

void platform_led_set(bool on) { gpio_put(LED_PIN, on); }

bool platform_button_read(platform_button_t button)
{
    uint pin = (button == PLATFORM_BUTTON_1) ? BUTTON_1_PIN : BUTTON_2_PIN;
    return gpio_get(pin) == 0;
}