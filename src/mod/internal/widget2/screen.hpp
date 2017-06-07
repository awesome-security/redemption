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

#include "core/font.hpp"
#include "composite.hpp"
#include "tooltip.hpp"
#include "utils/theme.hpp"

#include <typeinfo>
#include "gdi/graphic_api.hpp"

class WidgetScreen : public WidgetParent
{
public:
    Theme theme;
    WidgetTooltip * tooltip;
    Widget2 * current_over;

    CompositeArray composite_array;

    Pointer normal_pointer;
    Pointer edit_pointer;

    Font const & font;

    bool allow_mouse_pointer_change_ = true;

    WidgetScreen(gdi::GraphicApi & drawable, Font const & font,
                 NotifyApi * notifier, Theme const & theme)
        : WidgetParent(drawable, *this, notifier)
        , theme(theme)
        , tooltip(nullptr)
        , current_over(nullptr)
        , normal_pointer(Pointer::POINTER_NORMAL)
        , edit_pointer(Pointer::POINTER_EDIT)
        , font(font)
    {
        this->impl = &composite_array;

        this->tab_flag = IGNORE_TAB;

        this->set_xy(0, 0);
    }

    ~WidgetScreen() override {
        if (this->tooltip) {
            delete this->tooltip;
            this->tooltip = nullptr;
        }
    }

    void show_tooltip(Widget2 * widget, const char * text, int x, int y,
                      Rect const preferred_display_rect, int = 10) override {
        if (text == nullptr) {
            if (this->tooltip) {
                this->remove_widget(this->tooltip);
                this->rdp_input_invalidate(this->tooltip->get_rect());
                delete this->tooltip;
                this->tooltip = nullptr;
            }
        }
        else if (this->tooltip == nullptr) {
            Rect display_rect = this->get_rect();
            if (!preferred_display_rect.isempty()) {
                display_rect = this->get_rect().intersect(preferred_display_rect);
            }

            this->tooltip = new WidgetTooltip(this->drawable,
                                              *this, widget,
                                              text,
                                              this->theme.tooltip.fgcolor,
                                              this->theme.tooltip.bgcolor,
                                              this->theme.tooltip.border_color,
                                              this->font);
            Dimension dim = this->tooltip->get_optimal_dim();
            this->tooltip->set_wh(dim);

            int w = this->tooltip->cx();
            int h = this->tooltip->cy();
            int sw = display_rect.x + display_rect.cx;
            int posx = ((x + w) > sw)?(sw - w):x;
            int posy = (y > h)?(y - h):0;
            this->tooltip->set_xy(posx, posy);

            this->add_widget(this->tooltip);
            this->rdp_input_invalidate(this->tooltip->get_rect());
        }
    }

    bool next_focus() override {
        if (this->current_focus) {
            if (this->current_focus->next_focus()) {
                return true;
            }

            Widget2 * future_focus_w = this->get_next_focus(this->current_focus, false);
            if (!future_focus_w) {
                future_focus_w = this->get_next_focus(nullptr, false);
            }
            REDASSERT(this->current_focus);
            this->set_widget_focus(future_focus_w, focus_reason_tabkey);

            return true;
        }

        return false;
    }
    bool previous_focus() override {
        if (this->current_focus) {
            if (this->current_focus->previous_focus()) {
                return true;
            }

            Widget2 * future_focus_w = this->get_previous_focus(this->current_focus, false);
            if (!future_focus_w) {
                future_focus_w = this->get_previous_focus(nullptr, false);
            }
            REDASSERT(this->current_focus);
            this->set_widget_focus(future_focus_w, focus_reason_backtabkey);

            return true;
        }

        return false;
    }

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override {
        Widget2 * w = this->last_widget_at_pos(x, y);
        if (this->current_over != w) {
            const Pointer* pointer = &this->normal_pointer;

            if (nullptr != w) {
                if (Pointer::POINTER_EDIT == w->pointer_flag) {
                    pointer = &this->edit_pointer;
                }
                else if (Pointer::POINTER_CUSTOM == w->pointer_flag) {
                    const Pointer* temp_pointer = w->get_pointer();
                    if (temp_pointer) {
                        pointer = temp_pointer;
                    }
                }
            }

            if (this->allow_mouse_pointer_change_) {
                this->drawable.set_pointer(*pointer);
            }

            this->current_over = w;
        }
        if (this->tooltip) {
            if (device_flags & MOUSE_FLAG_MOVE) {
                if (w != this->tooltip->notifier) {
                    this->hide_tooltip();
                }
            }
            if (device_flags & (MOUSE_FLAG_BUTTON1)) {
                this->hide_tooltip();
            }
        }
        WidgetParent::rdp_input_mouse(device_flags, x, y, keymap);
    }

    void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap) override {
        if (this->tooltip) {
            this->hide_tooltip();
        }
        WidgetParent::rdp_input_scancode(param1, param2, param3, param4, keymap);
    }

    void allow_mouse_pointer_change(bool allow) {
        this->allow_mouse_pointer_change_ = allow;
    }

    void redo_mouse_pointer_change() {
        const Pointer* pointer = &this->normal_pointer;

        if (nullptr != this->current_over) {
            if (Pointer::POINTER_EDIT == this->current_over->pointer_flag) {
                pointer = &this->edit_pointer;
            }
            else if (Pointer::POINTER_CUSTOM == this->current_over->pointer_flag) {
                const Pointer* temp_pointer = this->current_over->get_pointer();
                if (temp_pointer) {
                    pointer = temp_pointer;
                }
            }
        }

        this->drawable.set_pointer(*pointer);
    }
};
