/**
 * @file app_ui.cpp
 * @brief Common app-mode UI component library implementation
 */
#include "app_ui.h"
#include <algorithm>
#include <cstring>

namespace AppUI {

/* ══════════════════════════════════════════════════════════════
 *  ListMenu
 * ══════════════════════════════════════════════════════════════ */

void ListMenu::init(lgfx::LGFX_Device* lcd, Button_Device* btn, const char* title)
{
    _lcd   = lcd;
    _btn   = btn;
    _title = title ? title : "";
    _sel   = 0;
    _scrollTop = 0;
    _needRedraw = true;
}

void ListMenu::clear()
{
    _items.clear();
    _sel = 0;
    _scrollTop = 0;
    _needRedraw = true;
}

void ListMenu::addItem(const MenuItem& item)
{
    _items.push_back(item);
    _needRedraw = true;
}

void ListMenu::setItems(const std::vector<MenuItem>& items)
{
    _items = items;
    _sel = 0;
    _scrollTop = 0;
    _needRedraw = true;
}

void ListMenu::setStatus(const char* text)
{
    _status = text ? text : "";
    _drawStatus();
}

int ListMenu::_visibleCount() const
{
    if (!_lcd) return 1;
    int areaH = _lcd->height() - TITLE_H - STATUS_H;
    return std::max(1, areaH / ITEM_H);
}

/* ── Update (poll + draw) ── */
void ListMenu::update()
{
    if (!_lcd || !_btn) return;

    _btn->update();

    const int count = (int)_items.size();
    if (count == 0) {
        if (_needRedraw) _drawFull();
        return;
    }

    bool changed = false;

    /* Joystick navigation */
    if (_btn->Up.pressed()) {
        if (_sel > 0) { _sel--; changed = true; }
    }
    if (_btn->Down.pressed()) {
        if (_sel < count - 1) { _sel++; changed = true; }
    }

    /* Scroll to keep selection visible */
    int vis = _visibleCount();
    if (_sel < _scrollTop)          { _scrollTop = _sel; changed = true; }
    if (_sel >= _scrollTop + vis)   { _scrollTop = _sel - vis + 1; changed = true; }

    /* Button A — select */
    if (_btn->A.pressed()) {
        if (_onSelect && _sel >= 0 && _sel < count) {
            _onSelect(_sel, _items[_sel]);
        }
    }

    /* Button B — back */
    if (_btn->B.pressed()) {
        if (_onBack) _onBack();
    }

    /* Redraw */
    if (_needRedraw) {
        _drawFull();
    } else if (changed) {
        /* Incremental: erase old highlight, draw new */
        if (_prevSel >= 0 && _prevSel != _sel) {
            if (_prevSel >= _scrollTop && _prevSel < _scrollTop + vis)
                _drawItem(_prevSel, false);
        }
        if (_sel >= _scrollTop && _sel < _scrollTop + vis)
            _drawItem(_sel, true);
        _drawScrollBar();
    }

    _prevSel    = _sel;
    _prevScroll = _scrollTop;
}

/* ── Full redraw ── */
void ListMenu::_drawFull()
{
    _needRedraw = false;
    _lcd->fillScreen(_theme.bg);
    _drawTitle();

    int vis = _visibleCount();
    for (int v = 0; v < vis; v++) {
        int idx = _scrollTop + v;
        if (idx >= (int)_items.size()) break;
        _drawItem(idx, idx == _sel);
    }

    _drawScrollBar();
    _drawStatus();
    _prevSel    = _sel;
    _prevScroll = _scrollTop;
}

/* ── Draw single item ── */
void ListMenu::_drawItem(int idx, bool selected)
{
    int vis_pos = idx - _scrollTop;
    int y = TITLE_H + vis_pos * ITEM_H;
    int w = _lcd->width() - SCROLLBAR_W - 2;

    uint16_t bg   = selected ? _theme.selBg   : _theme.itemBg;
    uint16_t fg   = selected ? _theme.selText  : _theme.itemText;

    /* Item background with rounded corners */
    _lcd->fillRoundRect(PADDING, y + 1, w - PADDING, ITEM_H - 2, 4, bg);

    _lcd->setTextColor(fg);
    _lcd->setTextDatum(textdatum_t::middle_left);

    if (idx < (int)_items.size()) {
        const auto& item = _items[idx];
        if (item.subText.empty()) {
            /* Single line — vertically centred */
            _lcd->setFont(&fonts::FreeSans9pt7b);
            _lcd->drawString(item.text.c_str(), PADDING + 8, y + ITEM_H / 2);
        } else {
            /* Two lines */
            _lcd->setFont(&fonts::FreeSans9pt7b);
            _lcd->drawString(item.text.c_str(), PADDING + 8, y + ITEM_H / 2 - 6);
            _lcd->setFont(&fonts::Font0);
            _lcd->setTextColor(selected ? 0xDEDB : 0x7BEF);
            _lcd->drawString(item.subText.c_str(), PADDING + 8, y + ITEM_H / 2 + 8);
        }
    }
}

/* ── Draw scrollbar ── */
void ListMenu::_drawScrollBar()
{
    int count = (int)_items.size();
    if (count == 0) return;

    int areaH = _lcd->height() - TITLE_H - STATUS_H;
    int x = _lcd->width() - SCROLLBAR_W;

    /* Clear scrollbar area */
    _lcd->fillRect(x, TITLE_H, SCROLLBAR_W, areaH, _theme.bg);

    if (count <= _visibleCount()) return;  // no scrollbar needed

    int barH = std::max(8, areaH * _visibleCount() / count);
    int barY = TITLE_H + (_scrollTop * (areaH - barH)) / (count - _visibleCount());

    _lcd->fillRoundRect(x, barY, SCROLLBAR_W, barH, 2, _theme.scrollBar);
}

/* ── Draw title bar ── */
void ListMenu::_drawTitle()
{
    _lcd->fillRect(0, 0, _lcd->width(), TITLE_H, _theme.titleBg);
    _lcd->setFont(&fonts::FreeSansBold9pt7b);
    _lcd->setTextColor(_theme.titleText);
    _lcd->setTextDatum(textdatum_t::middle_left);
    _lcd->drawString(_title.c_str(), 8, TITLE_H / 2);
}

/* ── Draw status bar ── */
void ListMenu::_drawStatus()
{
    if (!_lcd) return;
    int y = _lcd->height() - STATUS_H;
    _lcd->fillRect(0, y, _lcd->width(), STATUS_H, _theme.bg);
    if (!_status.empty()) {
        _lcd->setFont(&fonts::Font0);
        _lcd->setTextColor(_theme.statusText);
        _lcd->setTextDatum(textdatum_t::middle_center);
        _lcd->drawString(_status.c_str(), _lcd->width() / 2, y + STATUS_H / 2);
    }
}

} // namespace AppUI
