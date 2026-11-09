#include "api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "device/device.h"
#include "profiling/profiling.h"
#include "led-task/led-task.h"
#include "button-task/button-task.h"
#include "pi-task/pi-task.h"

typedef void (*api_callback_t)(const command_t *command);

typedef struct
{
    const char *name;
    const char *help;
    api_callback_t callback;
} api_command_t;


// ---- parse_u32 из т2.3.8 ----
static bool parse_u32(const char *text, uint32_t *value)
{
    if (!isdigit((unsigned char)text[0])) { return false; }

    char *end;
    errno = 0;
    unsigned long number = strtoul(text, &end, 10);

    if (*end != '\0' || errno == ERANGE || number > UINT32_MAX) { return false; }

    *value = (uint32_t)number;
    return true;
}

static void cmd_uptime(const command_t *c)
{
    (void)c;
    printf("uptime: %llu ms\n", time_us_64() / 1000);
}

static void print_led_response(void)
{
    const char *names[] = { "off", "on", "blink" };
    printf("led: %s, period %lu ms\n",
           names[led_task_get_state()],
           (unsigned long)led_task_get_period_ms());
}

static void cmd_led_enable(const command_t *c)
{
    (void)c;
    led_task_set_state(LED_STATE_ON);
    print_led_response();
}

static void cmd_led_disable(const command_t *c)
{
    (void)c;
    led_task_set_state(LED_STATE_OFF);
    print_led_response();
}

static void cmd_led_blink(const command_t *c)
{
    (void)c;
    led_task_set_state(LED_STATE_BLINK);
    print_led_response();
}

static void cmd_led_period(const command_t *c)
{
    uint32_t p;
    if (c->argc != 1 || !parse_u32(c->argv[0], &p)) {
        printf("error: usage led_period <period_ms>\n");
        return;
    }
    if (!led_task_set_period_ms(p)) {
        printf("error: period_ms must be greater than 0\n");
        return;
    }
    print_led_response();
}

static void cmd_button(const command_t *c)
{
    (void)c;
    printf("button: %s, presses %lu\n",
           button_task_is_pressed() ? "pressed" : "released",
           (unsigned long)button_task_get_press_count());
}

static void cmd_pi_start(const command_t *c)
{
    uint32_t terms = 1000000;
    if (c->argc > 1) {
        printf("error: usage pi_start [<terms>]\n");
        return;
    }
    if (c->argc == 1 && !parse_u32(c->argv[0], &terms)) {
        printf("error: usage pi_start [<terms>]\n");
        return;
    }
    if (terms == 0) {
        printf("error: terms must be greater than 0\n");
        return;
    }

    if (!pi_task_start(terms)) {
        printf("error: pi is busy\n");
        return;
    }
    printf("pi: started, %lu terms\n", (unsigned long)terms);
}

static void cmd_pi(const command_t *c)
{
    (void)c;
    double result;
    uint64_t time_ms;

    if (pi_task_get_result(&result, &time_ms)) {
        printf("pi: %.9f in %llu ms\n", result, (unsigned long long)time_ms);
        return;
    }

    switch (pi_task_get_state()) {
    case PI_STATE_IDLE:
        printf("pi: not started\n");
        break;
    case PI_STATE_RUNNING:
        printf("pi: running\n");
        break;
    default:
        printf("pi: not started\n");
        break;
    }
}

static void cmd_info(const command_t *c)
{
    (void)c;
    device_info_t info;
    device_get_info(&info);
    printf("name: %s\n",    DEVICE_NAME);
    printf("version: %s\n", FIRMWARE_VERSION);
    printf("project: %s\n", DEVICE_PROJECT);
    printf("repo: %s\n",    DEVICE_REPO);
    printf("board: %s\n",   info.board);
    printf("serial: %s\n",  info.serial);
    printf("chip: manufacturer 0x%03x, part 0x%04x, revision %u\n",
           (unsigned)info.manufacturer, (unsigned)info.part, (unsigned)info.revision);
    printf("pico-sdk: %s\n", info.sdk_version);
}

static void cmd_profiling(const command_t *c)
{
    (void)c;
    profiling_result_t res;
    for (uint32_t i = 0; i < PROFILING_MAX_STOPWATCHES; i++) {
        if (!profiling_get(i, &res)) continue;
        if (res.count == 0) continue;
        float mean = (float)res.sum_us / res.count;
        printf("%-10s count %-8lu mean %.2f us  max %lu us\n",
               res.name,
               (unsigned long)res.count,
               mean,
               (unsigned long)res.max_us);
    }
}

static void cmd_profiling_reset(const command_t *c)
{
    (void)c;
    profiling_reset();
    printf("profiling reset\n");
}

// ---- таблица ----
static const api_command_t api_commands[] = {
    { "help",            "list of commands",                               NULL },
    { "info",            "device passport",                                cmd_info },
    { "uptime",          "time since reset",                               cmd_uptime },
    { "led_enable",      "turn led on",                                    cmd_led_enable },
    { "led_disable",     "turn led off",                                   cmd_led_disable },
    { "led_blink",       "make led blink",                                 cmd_led_blink },
    { "led_period",      "set blink period: led_period <period_ms>",       cmd_led_period },
    { "button",          "button state and press count",                   cmd_button },
    { "pi_start",        "start pi calc: pi_start [terms]",                cmd_pi_start },
    { "pi",              "last pi result or state",                        cmd_pi },
    { "profiling",       "stopwatch results",                              cmd_profiling }, 
    { "profiling_reset", "reset stopwatch results",                        cmd_profiling_reset },
};

static const uint32_t api_command_count =
    sizeof(api_commands) / sizeof(api_commands[0]);

void api_handle(const command_t *command)
{
    if (command->truncated) {
        printf("error: command longer than 63 characters or 4 arguments\n");
        return;
    }

    // help выводит всю таблицу
    if (strcmp(command->name, "help") == 0) {
        for (uint32_t i = 0; i < api_command_count; i++) {
            printf("%-16s %s\n", api_commands[i].name, api_commands[i].help);
        }
        return;
    }

    // поиск команды
    for (uint32_t i = 0; i < api_command_count; i++) {
        if (strcmp(command->name, api_commands[i].name) == 0) {
            if (api_commands[i].callback != NULL) {
                api_commands[i].callback(command);
            }
            return;
        }
    }

    printf("error: unknown command '%s', try help\n", command->name);
}