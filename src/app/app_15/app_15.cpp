/**
 * @file app_07.cpp
 * @brief App15 — stub placeholder (app_15 slot)
 */
#include "app_15.h"
#include "../../ui/ui.h"

namespace MOONCAKE::APPS
{
    App15::App15(DEVICES* device)
        : _device(device)
    {
        setAppInfo().name = "app_15";
    }

    void App15::onOpen()
    {
        _scr = lv_obj_create(NULL);
        lv_obj_set_style_bg_color(_scr, lv_color_hex(0x000000), 0);
        lv_obj_clear_flag(_scr, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_t* lbl = lv_label_create(_scr);
        lv_label_set_text(lbl, "app_15");
        lv_obj_set_style_text_font(lbl, &ui_font_name_14, 0);
        lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
        lv_obj_center(lbl);
        lv_disp_load_scr(_scr);
        lv_timer_handler();
    }

    void App15::onRunning()
    {
        lv_timer_handler();
    }

    void App15::onClose()
    {
        if (_scr && lv_obj_is_valid(_scr)) {
            lv_obj_del(_scr);
            _scr = nullptr;
        }
    }
}
