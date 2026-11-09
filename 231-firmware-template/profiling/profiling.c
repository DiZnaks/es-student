#include "profiling.h"
#include "pico/stdlib.h"

typedef struct {
    const char *name;
    uint32_t count;
    uint32_t sum_us;
    uint32_t max_us;
    uint32_t start_us;
    bool active;
} stopwatch_t;

static stopwatch_t stopwatches[PROFILING_MAX_STOPWATCHES];

void profiling_stopwatch_init(uint32_t id, const char *name)
{
    if (id >= PROFILING_MAX_STOPWATCHES) return;
    stopwatches[id].name = name;
    stopwatches[id].count = 0;
    stopwatches[id].sum_us = 0;
    stopwatches[id].max_us = 0;
    stopwatches[id].active = false;
}

void profiling_start(uint32_t id)
{
    if (id >= PROFILING_MAX_STOPWATCHES) return;
    stopwatches[id].start_us = time_us_32();
    stopwatches[id].active = true;
}

void profiling_stop(uint32_t id)
{
    if (id >= PROFILING_MAX_STOPWATCHES) return;
    if (!stopwatches[id].active) return;
    uint32_t elapsed = time_us_32() - stopwatches[id].start_us;
    stopwatches[id].active = false;
    stopwatches[id].count++;
    stopwatches[id].sum_us += elapsed;
    if (elapsed > stopwatches[id].max_us) {
        stopwatches[id].max_us = elapsed;
    }
}

bool profiling_get(uint32_t id, profiling_result_t *result)
{
    if (id >= PROFILING_MAX_STOPWATCHES || !stopwatches[id].name) return false;
    result->name = stopwatches[id].name;
    result->count = stopwatches[id].count;
    result->sum_us = stopwatches[id].sum_us;
    result->max_us = stopwatches[id].max_us;
    return true;
}

void profiling_reset(void)
{
    for (uint32_t i = 0; i < PROFILING_MAX_STOPWATCHES; i++) {
        stopwatches[i].count = 0;
        stopwatches[i].sum_us = 0;
        stopwatches[i].max_us = 0;
        stopwatches[i].active = false;
    }
}