/**
 * @file app_01.h
 * @author Mingo
 * @brief App01 — Dino Game
 * @version 0.1
 * @date 2025-08-05
 * @copyright Copyright (c) 2025
 */
#pragma once
#include <mooncake.h>
#include "../../bsp/devices.h"
#include "include/Game.h"

using namespace mooncake;

namespace MOONCAKE::APPS
{
    class App01 : public AppAbility {
    public:
        App01(DEVICES* device);
        void onOpen() override;
        void onRunning() override;
        void onClose() override;
    private:
        DEVICES* _device = nullptr;
    };
}