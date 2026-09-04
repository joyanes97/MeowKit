/**
 * @file app_ui.h
 * @brief Common app-mode UI component library (LovyanGFX direct drawing)
 *        Provides reusable list menus and button input utilities for all apps.
 */
#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <functional>
#include <LovyanGFX.hpp>
#include "../../bsp/button/Button.hpp"

namespace AppUI {

/* ─────────────────────── Colour Theme ─────────────────────── */
struct Theme {
    uint16_t bg          = 0x0000;   // background
    uint16_t itemBg      = 0x18E3;   // normal item bg (dark grey)
    uint16_t itemText    = 0xC618;   // normal item text (light grey)
    uint16_t selBg       = 0x04BF;   // selected item bg (blue)
    uint16_t selText     = 0xFFFF;   // selected item text (white)
    uint16_t titleBg     = 0x2945;   // title bar bg (dark teal)
    uint16_t titleText   = 0x07E0;   // title bar text (green)
    uint16_t statusText  = 0x7BEF;   // status bar text (grey)
    uint16_t scrollBar   = 0x4A69;   // scrollbar colour
};

/* ─────────────────────── Menu Item ─────────────────────── */
struct MenuItem {
    std::string text;
    std::string subText;     // optional secondary line
    int         tag  = 0;    // user-defined tag / index
};

/* ─────────────────────── Callback Types ─────────────────────── */
using SelectCallback = std::function<void(int index, const MenuItem& item)>;
using BackCallback   = std::function<void()>;

// ─────────────────────── ListMenu ───────────────────────
// Scrollable list menu drawn with LovyanGFX.
// Call init(), addItem(), setOnSelect(), then update() each frame.
class ListMenu {
public:
    ListMenu() = default;

    /** Initialise with LCD, button device, and title string */
    void init(lgfx::LGFX_Device* lcd, Button_Device* btn, const char* title);

    /** Clear all items */
    void clear();

    /** Add a menu item */
    void addItem(const MenuItem& item);

    /** Batch set items */
    void setItems(const std::vector<MenuItem>& items);

    /** Set the selection changed / confirmed callback */
    void setOnSelect(SelectCallback cb)  { _onSelect = cb; }

    /** Set the back-button callback */
    void setOnBack(BackCallback cb)      { _onBack = cb; }

    /** Set visual theme */
    void setTheme(const Theme& t)        { _theme = t; _needRedraw = true; }

    /** Set status bar text (bottom line) */
    void setStatus(const char* text);

    /** Get currently selected index */
    int  selected() const                { return _sel; }

    /** Force full redraw */
    void invalidate()                    { _needRedraw = true; }

    /**
     * @brief Update: poll buttons, scroll, redraw if changed.
     *        Call this every frame in onRunning().
     */
    void update();

private:
    lgfx::LGFX_Device* _lcd  = nullptr;
    Button_Device*     _btn  = nullptr;

    std::string            _title;
    std::string            _status;
    std::vector<MenuItem>  _items;
    SelectCallback         _onSelect;
    BackCallback           _onBack;
    Theme                  _theme;

    int  _sel         = 0;    // selected index
    int  _scrollTop   = 0;    // first visible index
    int  _prevSel     = -1;
    int  _prevScroll  = -1;
    bool _needRedraw  = true;

    /* Layout constants */
    static constexpr int TITLE_H     = 24;
    static constexpr int STATUS_H    = 18;
    static constexpr int ITEM_H      = 32;
    static constexpr int SCROLLBAR_W = 4;
    static constexpr int PADDING     = 4;

    int _visibleCount() const;
    void _drawFull();
    void _drawItem(int idx, bool selected);
    void _drawScrollBar();
    void _drawTitle();
    void _drawStatus();
};

} // namespace AppUI
