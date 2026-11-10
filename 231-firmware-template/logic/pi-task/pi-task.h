#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    PI_STATE_IDLE,      // не запускался
    PI_STATE_RUNNING,   // считает
    PI_STATE_DONE,      // готово
} pi_state_t;

void pi_task_init(void);
void pi_task_handle(void);

// false, если расчёт уже идёт
bool pi_task_start(uint32_t terms);

// true, если готово; записывает результат и время в мс
bool pi_task_get_result(double *result, uint64_t *time_ms);

pi_state_t pi_task_get_state(void);