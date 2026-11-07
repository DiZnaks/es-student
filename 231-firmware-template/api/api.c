#include "api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "device.h"
#include "profiling.h"

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
    if (!isdigit((unsigned char)text[0])) return false;

    char *end;
    errno = 0;
    unsigned long number = strtoul(text, &end, 10);

    if (*end != '\0' || errno == ERANGE || number > UINT32_MAX) return false;

    *value = (uint32_t)number;
    return true;
}


// ---- calc_pi из 221-command-time ----
static volatile double pi_result;
static const uint32_t CALC_PI_TERMS = 1000000;

static double calc_pi(uint32_t terms)
{
    double sum = 0.0, sign = 1.0;
    for (uint32_t k = 0; k < terms; k++) {
        sum += sign / (2.0 * k + 1.0);
        sign = -sign;
    }
    return 4.0 * sum;
}

static void cmd_calc_pi(const command_t *c)
{
    uint32_t terms = CALC_PI_TERMS;
    if (c->argc > 1) { printf("error: usage calc_pi [terms]\n"); return; }
    if (c->argc == 1 && !parse_u32(c->argv[0], &terms)) {
        printf("error: usage calc_pi [terms]\n");
        return;
    }
    if (terms == 0) { printf("error: terms must be greater than 0\n"); return; }

    uint64_t start_us = time_us_64();
    pi_result = calc_pi(terms);
    uint64_t spent_us = time_us_64() - start_us;

    printf("pi: %.9f\n", pi_result);
    printf("time: %llu ms\n", spent_us / 1000);
}

static void cmd_uptime(const command_t *c)
{
    (void)c;
    printf("uptime: %llu ms\n", time_us_64() / 1000);
}

static void cmd_info(const command_t *c)
{
    (void)c;
    device_info(); // твоя функция из device.c
}

static void cmd_main_time_exec(const command_t *c)
{
    (void)c;
    printf("iteration avg %.2f us, max %u us\n",
           profiling_avg_us(), (unsigned)profiling_max_us());
}

static void cmd_main_time_reset(const command_t *c)
{
    (void)c;
    profiling_reset_max();
    printf("max reset\n");
}


// ---- таблица ----
static const api_command_t api_commands[] = {
    { "help",            "list of commands",                               NULL },
    { "info",            "device passport",                                cmd_info },
    { "uptime",          "time since reset",                               cmd_uptime },
    { "calc_pi",         "calculate pi: calc_pi [terms]",                  cmd_calc_pi },
    { "main_time_exec",  "superloop iteration: average and maximum",       cmd_main_time_exec },
    { "main_time_reset", "reset maximum iteration",                        cmd_main_time_reset },
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