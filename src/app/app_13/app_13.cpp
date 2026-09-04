/**
 * @file app_14.cpp
 * @brief App13 — stub placeholder (app_13 slot)
 */
#include "app_13.h"
#include "../../ui/ui.h"

namespace MOONCAKE::APPS
{
    App13::App13(DEVICES* device)
        : _device(device)
    {
        setAppInfo().name = "app_13";
    }

    void App13::onOpen()
    {
        _scr = lv_obj_create(NULL);
        lv_obj_set_style_bg_color(_scr, lv_color_hex(0x000000), 0);
        lv_obj_clear_flag(_scr, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_t* lbl = lv_label_create(_scr);
        lv_label_set_text(lbl, "app_13");
        lv_obj_set_style_text_font(lbl, &ui_font_name_14, 0);
        lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
        lv_obj_center(lbl);
        lv_disp_load_scr(_scr);
        lv_timer_handler();
    }

    void App13::onRunning()
    {
        lv_timer_handler();
    }

    void App13::onClose()
    {
        if (_scr && lv_obj_is_valid(_scr)) {
            lv_obj_del(_scr);
            _scr = nullptr;
        }
    }
}
