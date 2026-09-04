/**
 * @file app_09.h
 * @author Mingo
 * @brief App10 — (TODO: add description)
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
    class App10 : public AppAbility {
    public:
        App10(DEVICES* device);
        void onOpen() override;
        void onRunning() override;
        void onClose() override;
    private:
        DEVICES*  _device = nullptr;
        lv_obj_t* _scr    = nullptr;
    };
}
