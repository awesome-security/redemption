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
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan
 */
#pragma once

#include "widget.hpp"
#include "utils/bitmap_with_png.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "gdi/graphic_api.hpp"

class WidgetImage : public Widget2
{
    Bitmap bmp;

public:
    WidgetImage(gdi::GraphicApi & drawable, int x, int y, const char * filename, Widget2 & parent, NotifyApi* notifier, int group_id = 0)
    : Widget2(drawable, Rect(x,y,1,1), parent, notifier, group_id)
    , bmp(bitmap_from_file(filename))
    {
        this->tab_flag = IGNORE_TAB;
        this->focus_flag = IGNORE_FOCUS;

        this->set_cx(this->bmp.cx());
        this->set_cy(this->bmp.cy());
    }

    ~WidgetImage() override {}

    void draw(const Rect& clip) override {
        int16_t mx = std::max<int16_t>(clip.x, 0);
        int16_t my = std::max<int16_t>(clip.y, 0);
        this->drawable.draw(
            RDPMemBlt(
                0,
                Rect(mx, my, clip.cx, clip.cy),
                0xCC,
                mx - this->x(),
                my - this->y(),
                0
            ),
            this->get_rect(),
            this->bmp
        );
    }
};

