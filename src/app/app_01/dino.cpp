/**
 * @file app_01.cpp
 * @author Mingo
 * @brief App01 — Dino Game implementation
 * @version 0.1
 * @date 2025-08-05
 * @copyright Copyright (c) 2025
 */
#include "dino.h"

static Game dino;

namespace MOONCAKE::APPS
{
    App01::App01(DEVICES* device)
        : _device(device)
    {
        setAppInfo().name = "Dino";
    }

    void App01::onOpen()
    {
        dino.begin(_device);
    }

    void App01::onRunning()
    {
        dino.loop();
    }

    void App01::onClose()
    {
        /* Clean up heap allocations (framebuffer, dino sprite).
           Cannot call destructor on static object — use placement-style
           destroy + reconstruct so begin() works again next time. */
        dino.~Game();
        new (&dino) Game();   // reconstruct in-place so static stays valid
    }
}
