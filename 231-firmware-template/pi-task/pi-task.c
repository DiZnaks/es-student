#include "pi-task.h"
#include "pico/stdlib.h"
#include "systime/systime.h"

// Цена члена ряда ≈ 585 тактов = 4,7 мкс при 125 МГц.
// 100 членов ≈ 470 мкс < 0,5 мс — укладываемся в Н1.
#define PI_TERMS_PER_CALL 100

static pi_state_t pi_state = PI_STATE_IDLE;
static uint32_t pi_k = 0;          // текущий член
static uint32_t pi_n = 0;          // всего членов
static double   pi_sum = 0.0;
static double   pi_sign = 1.0;
static double   pi_result = 0.0;
static uint64_t pi_start_us = 0;
static uint64_t pi_time_ms = 0;

void pi_task_init(void)
{
    pi_state = PI_STATE_IDLE;
}

bool pi_task_start(uint32_t terms)
{
    if (pi_state == PI_STATE_RUNNING) {
        return false; // уже идёт — отклоняем
    }
    if (terms == 0) {
        return false; // ноль не принимаем
    }

    pi_n = terms;
    pi_k = 0;
    pi_sum = 0.0;
    pi_sign = 1.0;
    pi_result = 0.0;
    pi_start_us = systime_us();
    pi_time_ms = 0;
    pi_state = PI_STATE_RUNNING;
    return true;
}

void pi_task_handle(void)
{
    if (pi_state != PI_STATE_RUNNING) {
        return;
    }

    // Считаем порцию. Защита от переполнения при сложении:
    uint32_t end = pi_k + PI_TERMS_PER_CALL;
    if (end < pi_k || end > pi_n) {   // переполнение или дошли до конца
        end = pi_n;
    }

    for (; pi_k < end; pi_k++) {
        pi_sum += pi_sign / (2.0 * pi_k + 1.0);
        pi_sign = -pi_sign;
    }

    if (pi_k >= pi_n) {
        pi_result = 4.0 * pi_sum;
        pi_time_ms = (systime_us() - pi_start_us) / 1000;
        pi_state = PI_STATE_DONE;
    }
}

bool pi_task_get_result(double *result, uint64_t *time_ms)
{
    if (pi_state != PI_STATE_DONE) {
        return false;
    }
    if (result)  *result  = pi_result;
    if (time_ms) *time_ms = pi_time_ms;
    return true;
}

pi_state_t pi_task_get_state(void) { return pi_state; }