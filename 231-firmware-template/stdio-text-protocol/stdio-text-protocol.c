#include "stdio-text-protocol.h"

#include <stdio.h>
#include "pico/stdlib.h"

static char line[STDIO_TEXT_PROTOCOL_LINE_SIZE];
static uint32_t line_len;
static bool line_truncated;
static command_t command;

void stdio_text_protocol_init(void)
{
    line_len = 0;
    line_truncated = false;
    command.name = NULL;
    command.argc = 0;
    command.truncated = false;
}

// Деление строки на слова НА МЕСТЕ: пробелы -> '\0', указатели в line
static void split_line(void)
{
    line[line_len] = '\0';
    command.name = NULL;
    command.argc = 0;
    command.truncated = line_truncated;

    bool in_word = false;
    for (uint32_t i = 0; i <= line_len; i++) {
        if (line[i] == ' ' || line[i] == '\0') {
            line[i] = '\0';
            in_word = false;
        } else if (!in_word) {
            in_word = true;
            if (command.name == NULL) {
                command.name = &line[i];
            } else if (command.argc < STDIO_TEXT_PROTOCOL_MAX_ARGS) {
                command.argv[command.argc++] = &line[i];
            } else {
                command.truncated = true;
            }
        }
    }
}

const command_t *stdio_text_protocol_handle(void)
{
    int c = getchar_timeout_us(0);
    if (c == PICO_ERROR_TIMEOUT) {
        return NULL;
    }

    if (c == '\r' || c == '\n') {
        putchar('\r');
        putchar('\n');
        if (line_len == 0) {
            return NULL;
        }
        split_line();
        line_len = 0;
        line_truncated = false;
        return (command.name != NULL) ? &command : NULL;
    }

    if (c == 0x08 || c == 0x7F) {
        if (line_len > 0) {
            line_len--;
            line_truncated = false;
            printf("\b \b");
        }
        return NULL;
    }

    if (c >= 0x20 && c <= 0x7e) {
        if (line_len < STDIO_TEXT_PROTOCOL_LINE_SIZE - 1) {
            line[line_len++] = (char)c;
            putchar(c);
        } else {
            line_truncated = true;
        }
    }
    return NULL;
}