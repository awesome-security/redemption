/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 *              Meng Tan
 */

#pragma once

#include "widget.hpp"
#include "multiline.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "gdi/graphic_api.hpp"

class WidgetTooltip : public Widget2
{
    uint w_border;
    uint h_border;
    WidgetMultiLine desc;
    int border_color;
public:
    WidgetTooltip(gdi::GraphicApi & drawable, int16_t x, int16_t y, Widget2 & parent,
                  NotifyApi* notifier, const char * text,
                  int fgcolor, int bgcolor, int border_color, Font const & font)
        : Widget2(drawable, Rect(x, y, 100, 100), parent, notifier, 0)
        , w_border(10)
        , h_border(10)
        , desc(drawable, w_border, h_border, *this, this, text, true, 0, fgcolor, bgcolor, font, 0, 0)
        , border_color(border_color)
    {
        this->tab_flag = IGNORE_TAB;
        this->focus_flag = IGNORE_FOCUS;
        this->set_cx(this->desc.cx() + 2 * w_border);
        this->set_cy(this->desc.cy() + 2 * h_border);
    }

    ~WidgetTooltip() override {
    }

    void set_text(const char * text)
    {
        this->desc.set_text(text);
        this->set_cx(this->desc.cx() + 2 * w_border);
        this->set_cy(this->desc.cy() + 2 * h_border);
    }

    void draw(const Rect& clip) override {
        this->drawable.draw(RDPOpaqueRect(this->get_rect(), desc.bg_color), clip);
        this->desc.draw(clip);
        this->draw_border(clip);
    }

    int get_tooltip_cx() {
        return this->cx();
    }
    int get_tooltip_cy() {
        return this->cy();
    }

    void set_tooltip_xy(int x, int y) {
        this->set_x(x);
        this->set_y(y);
        this->desc.set_x(x + w_border);
        this->desc.set_y(y + h_border);
    }

    void draw_border(const Rect& clip)
    {
        //top
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->x(), this->y(), this->cx() - 1, 1
        )), this->border_color), this->get_rect());
        //left
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->x(), this->y() + 1, 1, this->cy() - 2
        )), this->border_color), this->get_rect());
        //right
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->x() + this->cx() - 1, this->y(), 1, this->cy()
        )), this->border_color), this->get_rect());
        //bottom
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->x(), this->y() + this->cy() - 1, this->cx() - 1, 1
        )), this->border_color), this->get_rect());
    }
};

