/**
 * @file app_02.cpp
 * @author Mingo
 * @brief App02 — Digital Rain (Matrix) Animation
 * @version 0.1
 * @date 2025-08-05
 * @copyright Copyright (c) 2025
 */
#include "matrix_rain.h"

namespace MOONCAKE::APPS
{
    App02::App02(DEVICES* device)
        : _device(device)
    {
        setAppInfo().name = "Matrix Rain";
    }

    void App02::onOpen()
    {
        _device->Lcd.fillScreen(TFT_BLACK);
        matrix_effect.init(&_device->Lcd, true);
        matrix_effect.setup(
            10,  /* Line Min */
            30,  /* Line Max */
            5,   /* Speed Min */
            25,  /* Speed Max */
            30   /* Screen Update Interval */);
    }

    void App02::onRunning()
    {
        matrix_effect.loop();
    }

    void App02::onClose()
    {
        if (_device && _device->Lcd.width() > 0 && _device->Lcd.height() > 0) {
            _device->Lcd.fillScreen(TFT_BLACK);
        }
    }
}
