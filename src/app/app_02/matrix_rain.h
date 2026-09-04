/**
 * @file app_02.h
 * @author Mingo
 * @brief App02 — Digital Rain (Matrix) Animation
 * @version 0.1
 * @date 2025-08-05
 * @copyright Copyright (c) 2025
 */
#pragma once
#include <mooncake.h>
#include "../../bsp/devices.h"
#include "DigitalRainAnimation.hpp"

using namespace mooncake;

namespace MOONCAKE::APPS
{
    class App02 : public AppAbility {
    public:
        App02(DEVICES* device);
        void onOpen() override;
        void onRunning() override;
        void onClose() override;
    private:
        DEVICES* _device = nullptr;
        DigitalRainAnimation<LGFX_Class> matrix_effect;
    };
}
