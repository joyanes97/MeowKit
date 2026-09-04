/**
 * @file app_07.h
 * @brief App15 — stub placeholder (app_15 slot)
 */
#pragma once
#include <mooncake.h>
#include "../../bsp/devices.h"
#include <lvgl.h>

using namespace mooncake;

namespace MOONCAKE::APPS
{
    class App15 : public AppAbility {
    public:
        App15(DEVICES* device);
        void onOpen() override;
        void onRunning() override;
        void onClose() override;
    private:
        DEVICES*  _device = nullptr;
        lv_obj_t* _scr    = nullptr;
    };
}
