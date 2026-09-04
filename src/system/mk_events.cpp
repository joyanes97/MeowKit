/**
 * @file mk_events.cpp
 * @brief MeowKit system event bus — FreeRTOS queue implementation.
 */
#include "mk_events.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#define QUEUE_DEPTH 16

static QueueHandle_t s_queue = nullptr;

void mk_events_init(void)
{
    if (!s_queue) {
        s_queue = xQueueCreate(QUEUE_DEPTH, sizeof(uint8_t));
    }
}

void mk_event_push(mk_event_t evt)
{
    if (!s_queue) return;
    uint8_t v = (uint8_t)evt;
    /* Non-blocking: drop event if queue is full (prevents ISR blocking). */
    xQueueSend(s_queue, &v, 0);
}

bool mk_event_pop(mk_event_t* out)
{
    if (!s_queue || !out) return false;
    uint8_t v = 0;
    if (xQueueReceive(s_queue, &v, 0) == pdTRUE) {
        *out = (mk_event_t)v;
        return true;
    }
    return false;
}

void mk_events_flush(void)
{
    if (!s_queue) return;
    xQueueReset(s_queue);
}
