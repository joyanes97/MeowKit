/**
 * @file lv_app_ui.cpp
 * @brief LVGL-based App UI component library — implementation
 */
#include "lv_app_ui.h"
#include <cstring>
#include <algorithm>

namespace LvAppUI {

/* ═══════════════════════════════════════════════════════════════════════
 *  ListMenu
 * ═══════════════════════════════════════════════════════════════════════ */

ListMenu::~ListMenu() { destroy(); }

void ListMenu::create(Button_Device* btn, const char* title)
{
    destroy();   /* clean up any prior instance */
    _btn     = btn;
    _sel     = 0;
    _prevSel = -1;

    /* ── Screen ── */
    _scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(_scr, lv_color_hex(_theme.bg), 0);
    lv_obj_clear_flag(_scr, LV_OBJ_FLAG_SCROLLABLE);

    /* ── Title bar (top TITLE_H px) ── */
    lv_obj_t* titleBar = lv_obj_create(_scr);
    lv_obj_set_size(titleBar, SCREEN_W, TITLE_H);
    lv_obj_set_pos(titleBar, 0, 0);
    lv_obj_set_style_bg_color(titleBar, lv_color_hex(_theme.titleBg), 0);
    lv_obj_set_style_bg_opa(titleBar, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(titleBar, 0, 0);
    lv_obj_set_style_border_width(titleBar, 0, 0);
    lv_obj_set_style_pad_all(titleBar, 0, 0);
    lv_obj_clear_flag(titleBar, LV_OBJ_FLAG_SCROLLABLE);

    _titleLabel = lv_label_create(titleBar);
    lv_label_set_text(_titleLabel, title ? title : "");
    lv_obj_set_style_text_color(_titleLabel, lv_color_hex(_theme.titleText), 0);
    lv_obj_set_style_text_font(_titleLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(_titleLabel, LV_ALIGN_LEFT_MID, 8, 0);

    /* ── Scrollable list container (flex column) ── */
    int listH = SCREEN_H - TITLE_H - STATUS_H;
    _listContainer = lv_obj_create(_scr);
    lv_obj_set_size(_listContainer, SCREEN_W, listH);
    lv_obj_set_pos(_listContainer, 0, TITLE_H);
    lv_obj_set_style_bg_opa(_listContainer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(_listContainer, 0, 0);
    lv_obj_set_style_pad_all(_listContainer, 4, 0);
    lv_obj_set_style_pad_row(_listContainer, 2, 0);
    lv_obj_add_flag(_listContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(_listContainer, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(_listContainer, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_flex_flow(_listContainer, LV_FLEX_FLOW_COLUMN);

    /* ── Status bar (bottom STATUS_H px) ── */
    _statusLabel = lv_label_create(_scr);
    lv_label_set_text(_statusLabel, "");
    lv_obj_set_style_text_color(_statusLabel, lv_color_hex(_theme.statusText), 0);
    lv_obj_set_style_text_font(_statusLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(_statusLabel, LV_ALIGN_BOTTOM_MID, 0, -4);
}

void ListMenu::destroy()
{
    if (_scr) {
        lv_obj_del(_scr);
        _scr = nullptr;
    }
    _items.clear();
    _itemObjs.clear();
    _titleLabel    = nullptr;
    _statusLabel   = nullptr;
    _listContainer = nullptr;
    _sel     = 0;
    _prevSel = -1;
}

void ListMenu::show()
{
    if (_scr) lv_disp_load_scr(_scr);
}

void ListMenu::clear()
{
    for (auto* obj : _itemObjs) {
        if (obj) lv_obj_del(obj);
    }
    _items.clear();
    _itemObjs.clear();
    _sel     = 0;
    _prevSel = -1;
}

void ListMenu::addItem(const MenuItem& item)
{
    if (!_listContainer) return;
    _items.push_back(item);

    bool hasSub = !item.subText.empty();
    int itemH   = hasSub ? 42 : 32;

    /* Item container (row in flex column) */
    lv_obj_t* row = lv_obj_create(_listContainer);
    lv_obj_set_size(row, lv_pct(100), itemH);
    lv_obj_set_style_bg_color(row, lv_color_hex(_theme.itemBg), 0);
    lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(row, 6, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_left(row, 10, 0);
    lv_obj_set_style_pad_ver(row, 2, 0);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    /* Primary label */
    lv_obj_t* mainLbl = lv_label_create(row);
    lv_label_set_text(mainLbl, item.text.c_str());
    lv_obj_set_style_text_color(mainLbl, lv_color_hex(_theme.itemText), 0);
    lv_obj_set_style_text_font(mainLbl, &lv_font_montserrat_14, 0);

    if (!hasSub) {
        lv_obj_align(mainLbl, LV_ALIGN_LEFT_MID, 0, 0);
    } else {
        lv_obj_align(mainLbl, LV_ALIGN_TOP_LEFT, 0, 2);

        /* Secondary label */
        lv_obj_t* subLbl = lv_label_create(row);
        lv_label_set_text(subLbl, item.subText.c_str());
        lv_obj_set_style_text_color(subLbl, lv_color_hex(_theme.subText), 0);
        lv_obj_set_style_text_font(subLbl, &lv_font_montserrat_10, 0);
        lv_obj_align(subLbl, LV_ALIGN_BOTTOM_LEFT, 0, -2);
    }

    _itemObjs.push_back(row);

    /* Set initial highlight if this is the first item */
    if ((int)_items.size() == 1) {
        _updateSelection();
    }
}

void ListMenu::setStatus(const char* text)
{
    if (_statusLabel) lv_label_set_text(_statusLabel, text ? text : "");
}

void ListMenu::update()
{
    if (!_btn) { lv_timer_handler(); return; }

    _btn->update();

    int count = (int)_items.size();
    if (count == 0) { lv_timer_handler(); return; }

    bool changed = false;

    /* D-pad navigation */
    if (_btn->Up.pressed()   && _sel > 0)           { _sel--; changed = true; }
    if (_btn->Down.pressed() && _sel < count - 1)   { _sel++; changed = true; }

    /* A = confirm */
    if (_btn->A.pressed() && _onSelect && _sel >= 0 && _sel < count) {
        _onSelect(_sel, _items[_sel]);
    }

    /* B = back */
    if (_btn->B.pressed() && _onBack) {
        _onBack();
    }

    if (changed) {
        _updateSelection();
        _ensureVisible();
    }

    lv_timer_handler();
}

void ListMenu::_updateSelection()
{
    for (int i = 0; i < (int)_itemObjs.size(); i++) {
        lv_obj_t* row = _itemObjs[i];
        if (!row) continue;

        bool sel = (i == _sel);
        lv_obj_set_style_bg_color(row, lv_color_hex(sel ? _theme.selBg : _theme.itemBg), 0);

        /* Update child label colours */
        uint32_t cnt = lv_obj_get_child_cnt(row);
        if (cnt > 0) {
            /* First child = primary label */
            lv_obj_set_style_text_color(
                lv_obj_get_child(row, 0),
                lv_color_hex(sel ? _theme.selText : _theme.itemText), 0);
        }
        if (cnt > 1) {
            /* Second child = sub label */
            lv_obj_set_style_text_color(
                lv_obj_get_child(row, 1),
                lv_color_hex(sel ? 0xDEDEDE : _theme.subText), 0);
        }
    }
    _prevSel = _sel;
}

void ListMenu::_ensureVisible()
{
    if (_sel >= 0 && _sel < (int)_itemObjs.size() && _itemObjs[_sel]) {
        lv_obj_scroll_to_view(_itemObjs[_sel], LV_ANIM_ON);
    }
}

/* ═══════════════════════════════════════════════════════════════════════
 *  InfoScreen
 * ═══════════════════════════════════════════════════════════════════════ */

InfoScreen::~InfoScreen() { destroy(); }

void InfoScreen::create(const char* title)
{
    destroy();

    /* ── Screen ── */
    _scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(_scr, lv_color_hex(_theme.bg), 0);
    lv_obj_clear_flag(_scr, LV_OBJ_FLAG_SCROLLABLE);

    /* ── Title bar ── */
    lv_obj_t* titleBar = lv_obj_create(_scr);
    lv_obj_set_size(titleBar, 320, 28);
    lv_obj_set_pos(titleBar, 0, 0);
    lv_obj_set_style_bg_color(titleBar, lv_color_hex(_theme.titleBg), 0);
    lv_obj_set_style_bg_opa(titleBar, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(titleBar, 0, 0);
    lv_obj_set_style_border_width(titleBar, 0, 0);
    lv_obj_clear_flag(titleBar, LV_OBJ_FLAG_SCROLLABLE);

    _titleLabel = lv_label_create(titleBar);
    lv_label_set_text(_titleLabel, title ? title : "");
    lv_obj_set_style_text_color(_titleLabel, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_text_font(_titleLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(_titleLabel, LV_ALIGN_LEFT_MID, 8, 0);

    /* ── Content area (flex column) ── */
    _contentArea = lv_obj_create(_scr);
    lv_obj_set_size(_contentArea, 300, 170);
    lv_obj_set_pos(_contentArea, 10, 34);
    lv_obj_set_style_bg_opa(_contentArea, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(_contentArea, 0, 0);
    lv_obj_set_style_pad_all(_contentArea, 0, 0);
    lv_obj_set_style_pad_row(_contentArea, 4, 0);
    lv_obj_set_flex_flow(_contentArea, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(_contentArea, LV_OBJ_FLAG_SCROLLABLE);

    /* ── Status bar ── */
    _statusLabel = lv_label_create(_scr);
    lv_label_set_text(_statusLabel, "");
    lv_obj_set_style_text_color(_statusLabel, lv_color_hex(0xFFFF00), 0);
    lv_obj_set_style_text_font(_statusLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(_statusLabel, LV_ALIGN_BOTTOM_MID, 0, -6);
}

void InfoScreen::destroy()
{
    _rows.clear();
    if (_scr) {
        lv_obj_del(_scr);
        _scr = nullptr;
    }
    _titleLabel  = nullptr;
    _statusLabel = nullptr;
    _contentArea = nullptr;
}

void InfoScreen::show()
{
    if (_scr) lv_disp_load_scr(_scr);
}

int InfoScreen::addRow(const char* text, uint32_t color)
{
    if (!_contentArea) return -1;

    lv_obj_t* label = lv_label_create(_contentArea);
    lv_label_set_text(label, text ? text : "");
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);

    _rows.push_back(label);
    return (int)_rows.size() - 1;
}

void InfoScreen::updateRow(int idx, const char* text)
{
    if (idx >= 0 && idx < (int)_rows.size() && _rows[idx]) {
        lv_label_set_text(_rows[idx], text ? text : "");
    }
}

void InfoScreen::setStatus(const char* text)
{
    if (_statusLabel) lv_label_set_text(_statusLabel, text ? text : "");
}

void InfoScreen::update()
{
    lv_timer_handler();
}

} // namespace LvAppUI
