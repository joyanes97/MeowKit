/**
 * @file lv_app_ui.h
 * @brief LVGL-based App UI component library
 *
 * Provides reusable LVGL screen components for Mooncake apps:
 *   - ListMenu  : Scrollable list with d-pad navigation + button polling
 *   - InfoScreen: Labeled rows for status / info display
 *
 * Each component creates its own LVGL screen. The app calls show() to
 * activate it and update() each frame (which calls lv_timer_handler).
 * When the app exits, it destroys the component ⇒ launcher's persistent
 * UI screen is restored via returnToUI().
 *
 * Usage (inside an AppAbility):
 * @code
 *   LvAppUI::ListMenu menu;
 *   menu.create(&_device->button, "Title");
 *   menu.addItem({"Item 1", "sub", 0});
 *   menu.setOnSelect([](int i, auto& m) { });
 *   menu.show();
 *   // in onRunning():  menu.update();
 *   // in onClose():    menu.destroy();
 * @endcode
 */
#pragma once

#include <lvgl.h>
#include <cstdint>
#include <vector>
#include <string>
#include <functional>
#include "../../bsp/button/Button.hpp"

namespace LvAppUI {

/* ─────────────────────── Colour Theme (24-bit) ─────────────────────── */
struct Theme {
    uint32_t bg         = 0x000000;   // screen background
    uint32_t titleBg    = 0x1A1A2E;   // title bar background
    uint32_t titleText  = 0x00FF00;   // title bar text
    uint32_t selBg      = 0xE60012;   // selected item background
    uint32_t selText    = 0xFFFFFF;   // selected item text
    uint32_t itemBg     = 0x16213E;   // normal item background
    uint32_t itemText   = 0xB0B0B0;   // normal item text
    uint32_t subText    = 0x808080;   // sub-line text
    uint32_t statusText = 0x808080;   // status bar text
};

/* ─────────────────────── Menu Item ─────────────────────── */
struct MenuItem {
    std::string text;        // primary label
    std::string subText;     // optional secondary line
    int         tag = 0;     // user-defined tag / index
};

/* ─────────────────────── Callback types ─────────────────────── */
using SelectCb = std::function<void(int index, const MenuItem& item)>;
using BackCb   = std::function<void()>;

/* ═══════════════════════════════════════════════════════════════════════
 *  ListMenu — Scrollable list on a dedicated LVGL screen
 * ═══════════════════════════════════════════════════════════════════════ */
class ListMenu {
public:
    ListMenu() = default;
    ~ListMenu();

    /** Create a new LVGL screen with title bar, flex-column list, status bar */
    void create(Button_Device* btn, const char* title);

    /** Delete the LVGL screen and free all objects */
    void destroy();

    /** Load this screen as the active display */
    void show();

    /** Remove all items (keeps screen alive) */
    void clear();

    /** Append one item to the list */
    void addItem(const MenuItem& item);

    /** Callbacks */
    void setOnSelect(SelectCb cb)   { _onSelect = cb; }
    void setOnBack(BackCb cb)       { _onBack = cb; }

    /** Visual theme — call BEFORE addItem() for best effect */
    void setTheme(const Theme& t)   { _theme = t; }

    /** Update bottom status text */
    void setStatus(const char* text);

    /** Poll buttons + lv_timer_handler(). Call every frame in onRunning(). */
    void update();

    lv_obj_t* screen() const        { return _scr; }
    int selected() const             { return _sel; }

private:
    lv_obj_t*       _scr           = nullptr;
    lv_obj_t*       _titleLabel    = nullptr;
    lv_obj_t*       _statusLabel   = nullptr;
    lv_obj_t*       _listContainer = nullptr;
    Button_Device*  _btn           = nullptr;

    std::vector<MenuItem>   _items;
    std::vector<lv_obj_t*>  _itemObjs;
    SelectCb  _onSelect;
    BackCb    _onBack;
    Theme     _theme;

    int _sel     = 0;
    int _prevSel = -1;

    static constexpr int TITLE_H  = 28;
    static constexpr int STATUS_H = 20;
    static constexpr int SCREEN_W = 320;
    static constexpr int SCREEN_H = 240;

    void _updateSelection();
    void _ensureVisible();
};

/* ═══════════════════════════════════════════════════════════════════════
 *  InfoScreen — Labeled rows on a dedicated LVGL screen
 * ═══════════════════════════════════════════════════════════════════════ */
class InfoScreen {
public:
    InfoScreen() = default;
    ~InfoScreen();

    /** Create a new LVGL screen with title bar and vertical content area */
    void create(const char* title);

    /** Delete the LVGL screen */
    void destroy();

    /** Load this screen as the active display */
    void show();

    /** Add a text row; returns row index for updateRow() */
    int addRow(const char* text, uint32_t color = 0xFFFFFF);

    /** Update an existing row's text */
    void updateRow(int idx, const char* text);

    /** Set bottom status text */
    void setStatus(const char* text);

    /** Visual theme — call BEFORE addRow() */
    void setTheme(const Theme& t)   { _theme = t; }

    /** Drive LVGL rendering (lv_timer_handler). Call every frame. */
    void update();

    lv_obj_t* screen() const        { return _scr; }

private:
    lv_obj_t* _scr          = nullptr;
    lv_obj_t* _titleLabel   = nullptr;
    lv_obj_t* _statusLabel  = nullptr;
    lv_obj_t* _contentArea  = nullptr;

    std::vector<lv_obj_t*> _rows;
    Theme _theme;
};

} // namespace LvAppUI
