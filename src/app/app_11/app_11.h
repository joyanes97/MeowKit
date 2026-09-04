/**
 * @file app_12.h
 * @author Mingo
 * @brief App11 — (TODO: add description)
 * @version 0.1
 * @date 2025-08-05
 * @copyright Copyright (c) 2025
 */
#pragma once
#include <mooncake.h>
#include "../../bsp/devices.h"
#include <lvgl.h>

using namespace mooncake;

namespace MOONCAKE::APPS
{
    class App11 : public AppAbility {
    public:
        App11(DEVICES* device);
        void onOpen() override;
        void onRunning() override;
        void onClose() override;
    private:
        DEVICES*  _device = nullptr;
        lv_obj_t* _scr    = nullptr;
    };
}
