#include "../include/dino_sfx.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <cmath>

static Speaker_Class* s_spk   = nullptr;
static TaskHandle_t   s_task  = nullptr;
static QueueHandle_t  s_queue = nullptr;

/* MIDI note → Hz  (equal temperament, A4 = 440 Hz) */
static uint32_t _midi_hz(uint8_t note)
{
    return (uint32_t)(440.0f * powf(2.0f, ((float)note - 69.0f) / 12.0f) + 0.5f);
}

/*
 * Parse and play one ESPboyPlaytune SFX score.
 * Only channel 0 produces audio; other channels contribute delays only.
 * Runs inside the FreeRTOS task — tone() blocks the task, not the game.
 */
static void _play_score(const uint8_t* p)
{
    while (*p != 0xe0 && *p != 0xf0) {
        uint8_t cmd = *p++;
        uint8_t ch  = cmd & 0x0F;

        if ((cmd & 0xF0) == 0x90) {          /* Note ON */
            uint8_t  note     = *p++;
            uint32_t delay_ms = 0;
            if (*p < 0x80) { delay_ms  = (uint32_t)*p++ << 8;
                             delay_ms |= *p++; }
            if (ch == 0 && s_spk && delay_ms > 0)
                s_spk->tone(_midi_hz(note), delay_ms);
            else if (delay_ms > 0)
                vTaskDelay(pdMS_TO_TICKS(delay_ms));

        } else if ((cmd & 0xF0) == 0x80) {   /* Note OFF */
            uint32_t delay_ms = 0;
            if (*p < 0x80) { delay_ms  = (uint32_t)*p++ << 8;
                             delay_ms |= *p++; }
            if (delay_ms > 0) vTaskDelay(pdMS_TO_TICKS(delay_ms));
        }
        /* 0xex / 0xfx already handled by the while condition */
    }
}

static void _sfx_task(void*)
{
    const uint8_t* score = nullptr;
    while (true) {
        if (xQueueReceive(s_queue, &score, portMAX_DELAY) == pdTRUE && score)
            _play_score(score);
    }
}

/* ── Public API ─────────────────────────────────────────────── */

void dino_sfx_init(Speaker_Class* spk)
{
    s_spk = spk;
    if (!s_spk->isEnabled())
        s_spk->begin();
    s_spk->setVolume(SPK_VOLUME_MAX / 2);

    /* Queue depth 1 — xQueueOverwrite ensures latest request wins */
    s_queue = xQueueCreate(1, sizeof(const uint8_t*));
    xTaskCreatePinnedToCore(_sfx_task, "dino_sfx", 4096, nullptr, 3, &s_task, 1);
}

void dino_sfx_deinit()
{
    if (s_task)  { vTaskDelete(s_task);  s_task  = nullptr; }
    if (s_queue) { vQueueDelete(s_queue); s_queue = nullptr; }
    if (s_spk && s_spk->isEnabled()) s_spk->end();
    s_spk = nullptr;
}

void dino_sfx_play(const uint8_t* score)
{
    if (!s_queue || !score || !s_spk) return;
    s_spk->stop();                    /* interrupt current tone */
    xQueueOverwrite(s_queue, &score); /* queue the new score */
}

void dino_sfx_stop()
{
    if (s_spk) s_spk->stop();
}
