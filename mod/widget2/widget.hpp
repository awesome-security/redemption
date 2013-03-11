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
 */

#if !defined(REDEMPTION_MOD_WIDGET2_WIDGET_HPP_)
#define REDEMPTION_MOD_WIDGET2_WIDGET_HPP_

#include <vector>
#include "notify_api.hpp"
#include "mod_api.hpp"
#include "notify_api.hpp"
#include <rect.hpp>


#include <iostream>

class Keymap2;

enum EventType {
    FOCUS_BEGIN,
    FOCUS_END,
    KEYDOWN,
    KEYUP,
    CLIC_BUTTON1_UP,
    CLIC_BUTTON1_DOWN,
    CLIC_BUTTON2_UP,
    CLIC_BUTTON2_DOWN,
    CLIC_BUTTON3_UP,
    CLIC_BUTTON3_DOWN,
    WM_DRAW,
    TEXT_CHANGED,
    WIDGET_SUBMIT
};

enum NotifyEventType {
    NOTIFY_FOCUS_BEGIN,
    NOTIFY_FOCUS_END,
    NOTIFY_TEXT_CHANGED = TEXT_CHANGED,
    NOTIFY_SUBMIT,
    NOTIFY_CANCEL,
};

class Widget
{
public:
    //type & TYPE_WND -> WidgetComposite
    enum WidgetType {
        TYPE_WND     = 1,
        TYPE_SCREEN  = 3,
        TYPE_BITMAP  = 3 << 1,
        TYPE_BUTTON  = 4 << 1,
        TYPE_IMAGE   = 5 << 1,
        TYPE_EDIT    = 6 << 1,
        TYPE_LABEL   = 7 << 1,
    };

    enum OptionTab {
        IGNORE_TAB = 0,
        NORMAL_TAB = 1,
        DELEGATE_CONTROL_TAB = 2,
        REWIND_TAB = 4,
        REWIND_BACKTAB = 8
    };

public:
    Widget * parent;
    ModApi * drawable;
    NotifyApi * notifier;
    Rect rect;
    int type;
    int id;
    int bg_color;
    int fg_color;
    int tab_flag;
    bool has_focus;

public:
    Widget(ModApi * drawable, const Rect& rect, Widget * parent, int type, NotifyApi * notifier, int id = 0)
    : parent(parent)
    , drawable(drawable)
    , notifier(notifier)
    , rect(rect)
    , type(type)
    , id(id)
    , bg_color(BLACK)
    , fg_color(WHITE)
    , tab_flag(NORMAL_TAB)
    , has_focus(false)
    {
        if (this->parent)
            this->parent->attach_widget(this);
    }

    virtual ~Widget()
    {
        if (this->parent)
            this->parent->detach_widget(this);
    }

protected:
    virtual void attach_widget(Widget *)
    {}

    virtual void detach_widget(Widget *)
    {}

public:
    struct screen_position {
        Rect clip;
        int16_t x;
        int16_t y;

        explicit screen_position(const Rect & rect)
        : clip(rect)
        , x(rect.x)
        , y(rect.y)
        {}
    };

    screen_position position_in_screen()
    {
        screen_position ret(this->rect);
        for (Widget * p = this->parent; p; p = p->parent){
            ret.clip = ret.clip.intersect(p->rect.cx, p->rect.cy);
            ret.clip.x += p->rect.x;
            ret.clip.y += p->rect.y;
            ret.x += p->rect.x;
            ret.y += p->rect.y;
        }
        return ret;
    }

    virtual void draw(const Rect& rect, int16_t x, int16_t y, const Rect & clip)
    {
        this->drawable->draw(
            RDPOpaqueRect(
                rect.offset(x,y),
                this->bg_color
            ), clip
        );
    }

    virtual void draw(const Rect& rect, int16_t x, int16_t y, int16_t xclip, int16_t yclip)
    {
        this->draw(rect, x, y, Rect(xclip, yclip, rect.cx, rect.cy));
    }

    void refresh(const Rect & rect)
    {
        if (!rect.isempty() && this->drawable){
            Widget::screen_position sp = this->position_in_screen();
            Rect clip = sp.clip.intersect(Rect(sp.x + rect.x, sp.y + rect.y, rect.cx, rect.cy));
            if (clip.isempty()) {
                return ;
            }
            int dx = clip.x - sp.x;
            int dy = clip.y - sp.y;
            Rect new_rect = Rect(dx,
                                 dy,
                                 clip.cx - (rect.x - dx),
                                 clip.cy - (rect.y - dy)
                                );
            if (!new_rect.isempty()){
                this->drawable->begin_update();
                this->draw(new_rect, sp.x, sp.y, clip.x, clip.y);
                this->drawable->end_update();
            }
        }
    }

    virtual void send_event(EventType event, int param, int param2, Keymap2 * keymap)
    {
        if (event == WM_DRAW){
            this->refresh(Rect(0,0,this->rect.cx, this->rect.cy));
        }
    }

    void notify_self(NotifyApi::notify_event_t event)
    {
        if (this->notifier)
            this->notifier->notify(this, event);
    }

    void notify_parent(EventType event)
    {
        if (this->parent)
            this->parent->notify(this->id, event);
    }

    virtual void notify(int id, EventType event)
    {
        (void)id;
        this->notify_self(event);
        this->notify_parent(event);
    }

    virtual Widget * widget_at_pos(int x, int y)
    {
        if (this->rect.contains_pt(x, y))
            return this;
        return 0;
    }

    int dx() const
    {
        return this->rect.x;
    }

    int dy() const
    {
        return this->rect.y;
    }

    uint16_t cx() const
    {
        return this->rect.cx;
    }

    uint16_t cy() const
    {
        return this->rect.cy;
    }

    virtual Widget * widget_focused()
    {
        return 0; ///TODO
    }
};

#endif