/**
 * @file app_15.h
 * @author Mingo
 * @brief App14 — (TODO: add description)
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
    class App14 : public AppAbility {
    public:
        App14(DEVICES* device);
        void onOpen() override;
        void onRunning() override;
        void onClose() override;
    private:
        DEVICES*  _device = nullptr;
        lv_obj_t* _scr    = nullptr;
    };
}
