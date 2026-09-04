/**
 * @file app_12.cpp
 * @brief App11 — stub placeholder (app_11 slot)
 */
#include "app_11.h"
#include "../../ui/ui.h"

namespace MOONCAKE::APPS
{
    App11::App11(DEVICES* device)
        : _device(device)
    {
        setAppInfo().name = "app_11";
    }

    void App11::onOpen()
    {
        _scr = lv_obj_create(NULL);
        lv_obj_set_style_bg_color(_scr, lv_color_hex(0x000000), 0);
        lv_obj_clear_flag(_scr, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_t* lbl = lv_label_create(_scr);
        lv_label_set_text(lbl, "app_11");
        lv_obj_set_style_text_font(lbl, &ui_font_name_14, 0);
        lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
        lv_obj_center(lbl);
        lv_disp_load_scr(_scr);
        lv_timer_handler();
    }

    void App11::onRunning()
    {
        lv_timer_handler();
    }

    void App11::onClose()
    {
        if (_scr && lv_obj_is_valid(_scr)) {
            lv_obj_del(_scr);
            _scr = nullptr;
        }
    }
}
