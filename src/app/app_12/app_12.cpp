/**
 * @file app_13.cpp
 * @brief App12 — stub placeholder (app_12 slot)
 */
#include "app_12.h"
#include "../../ui/ui.h"

namespace MOONCAKE::APPS
{
    App12::App12(DEVICES* device)
        : _device(device)
    {
        setAppInfo().name = "app_12";
    }

    void App12::onOpen()
    {
        _scr = lv_obj_create(NULL);
        lv_obj_set_style_bg_color(_scr, lv_color_hex(0x000000), 0);
        lv_obj_clear_flag(_scr, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_t* lbl = lv_label_create(_scr);
        lv_label_set_text(lbl, "app_12");
        lv_obj_set_style_text_font(lbl, &ui_font_name_14, 0);
        lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
        lv_obj_center(lbl);
        lv_disp_load_scr(_scr);
        lv_timer_handler();
    }

    void App12::onRunning()
    {
        lv_timer_handler();
    }

    void App12::onClose()
    {
        if (_scr && lv_obj_is_valid(_scr)) {
            lv_obj_del(_scr);
            _scr = nullptr;
        }
    }
}
