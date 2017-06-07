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
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan
 */

#pragma once

#include "composite.hpp"
#include "flat_button.hpp"
#include "multiline.hpp"
#include "image.hpp"
#include "widget2_rect.hpp"
#include "edit.hpp"
#include "password.hpp"
#include "utils/theme.hpp"
#include "group_box.hpp"
#include "utils/translation.hpp"
#include "gdi/graphic_api.hpp"


class FlatForm : public WidgetParent
{
public:
    CompositeArray composite_array;

    WidgetLabel      warning_msg;
    WidgetLabel      duration_label;
    WidgetEdit       duration_edit;
    WidgetLabel      duration_format;
    WidgetLabel      ticket_label;
    WidgetEdit       ticket_edit;
    WidgetLabel      comment_label;
    WidgetEdit       comment_edit;
    WidgetLabel      notes;
    WidgetFlatButton confirm;

    int flags;

    enum {
        NONE               = 0x00,
        COMMENT_DISPLAY    = 0x01,
        COMMENT_MANDATORY  = 0x02,
        TICKET_DISPLAY     = 0x04,
        TICKET_MANDATORY   = 0x08,
        DURATION_DISPLAY   = 0x10,
        DURATION_MANDATORY = 0x20,
    };

    const char * generic_warning;
    const char * format_warning;
    const char * toohigh_warning;
    const char * field_comment;
    const char * field_ticket;
    const char * field_duration;

    char warning_buffer[512];

    FlatForm(gdi::GraphicApi& drawable, int16_t left, int16_t top, int16_t width, int16_t height,
             Widget2 & parent, NotifyApi* notifier, int group_id,
             Font const & font, Theme const & theme, Translation::language_t lang,
             int flags = 0)
        : WidgetParent(drawable, parent, notifier, group_id)
        , warning_msg(drawable, *this, nullptr, "", group_id,
                      theme.global.error_color, theme.global.bgcolor, font)
        , duration_label(drawable, *this, nullptr, TR(trkeys::duration, lang),
                         group_id, theme.global.fgcolor, theme.global.bgcolor, font)
        , duration_edit(drawable, *this, this,
                        nullptr, group_id, theme.edit.fgcolor, theme.edit.bgcolor,
                        theme.edit.focus_color, font, -1, 1, 1)
        , duration_format(drawable, *this, nullptr, TR(trkeys::note_duration_format, lang),
                          group_id, theme.global.fgcolor, theme.global.bgcolor, font)
        , ticket_label(drawable, *this, nullptr, TR(trkeys::ticket, lang),
                       group_id, theme.global.fgcolor, theme.global.bgcolor, font)
        , ticket_edit(drawable, *this, this,
                      nullptr, group_id, theme.edit.fgcolor, theme.edit.bgcolor,
                      theme.edit.focus_color, font, -1, 1, 1)
        , comment_label(drawable, *this, nullptr, TR(trkeys::comment, lang),
                        group_id, theme.global.fgcolor, theme.global.bgcolor, font)
        , comment_edit(drawable, *this, this,
                       nullptr, group_id, theme.edit.fgcolor, theme.edit.bgcolor,
                       theme.edit.focus_color, font, -1, 1, 1)
        , notes(drawable, *this, nullptr, TR(trkeys::note_required, lang),
                group_id, theme.global.fgcolor, theme.global.bgcolor, font)
        , confirm(drawable, *this, this, TR(trkeys::confirm, lang), group_id,
                  theme.global.fgcolor, theme.global.bgcolor, theme.global.focus_color, 2, font,
                  6, 2)
        , flags(flags)
        , generic_warning(TR(trkeys::fmt_field_required, lang))
        , format_warning(TR(trkeys::fmt_invalid_format, lang))
        , toohigh_warning(TR(trkeys::fmt_toohigh_duration, lang))
        , field_comment(TR(trkeys::comment, lang))
        , field_ticket(TR(trkeys::ticket, lang))
        , field_duration(TR(trkeys::duration, lang))
        , warning_buffer()
    {
        this->set_bg_color(theme.global.bgcolor);

        this->impl = &composite_array;

        this->add_widget(&this->warning_msg);

        if (this->flags & DURATION_DISPLAY) {
            this->add_widget(&this->duration_label);
            this->add_widget(&this->duration_edit);
            this->add_widget(&this->duration_format);
        }
        if (this->flags & TICKET_DISPLAY) {
            this->add_widget(&this->ticket_label);
            this->add_widget(&this->ticket_edit);
        }
        if (this->flags & COMMENT_DISPLAY) {
            this->add_widget(&this->comment_label);
            this->add_widget(&this->comment_edit);
        }
        if (this->flags & DURATION_MANDATORY) {
            this->duration_label.set_text(TR(trkeys::duration_r, lang));
        }
        if (this->flags & TICKET_MANDATORY) {
            this->ticket_label.set_text(TR(trkeys::ticket_r, lang));
        }
        if (this->flags & COMMENT_MANDATORY) {
            this->comment_label.set_text(TR(trkeys::comment_r, lang));
        }

        if (this->flags & (COMMENT_MANDATORY | TICKET_MANDATORY | DURATION_MANDATORY)) {
            this->add_widget(&this->notes);
        }

        this->add_widget(&this->confirm);

        this->move_size_widget(left, top, width, height);
    }

    FlatForm(gdi::GraphicApi& drawable,
             Widget2 & parent, NotifyApi* notifier, int group_id,
             Font const & font, Theme const & theme, Translation::language_t lang,
             int flags = 0)
        : WidgetParent(drawable, parent, notifier, group_id)
        , warning_msg(drawable, *this, nullptr, "", group_id,
                      theme.global.error_color, theme.global.bgcolor, font)
        , duration_label(drawable, *this, nullptr, TR(trkeys::duration, lang),
                         group_id, theme.global.fgcolor, theme.global.bgcolor, font)
        , duration_edit(drawable, *this, this,
                        nullptr, group_id, theme.edit.fgcolor, theme.edit.bgcolor,
                        theme.edit.focus_color, font, -1, 1, 1)
        , duration_format(drawable, *this, nullptr, TR(trkeys::note_duration_format, lang),
                          group_id, theme.global.fgcolor, theme.global.bgcolor, font)
        , ticket_label(drawable, *this, nullptr, TR(trkeys::ticket, lang),
                       group_id, theme.global.fgcolor, theme.global.bgcolor, font)
        , ticket_edit(drawable, *this, this,
                      nullptr, group_id, theme.edit.fgcolor, theme.edit.bgcolor,
                      theme.edit.focus_color, font, -1, 1, 1)
        , comment_label(drawable, *this, nullptr, TR(trkeys::comment, lang),
                        group_id, theme.global.fgcolor, theme.global.bgcolor, font)
        , comment_edit(drawable, *this, this,
                       nullptr, group_id, theme.edit.fgcolor, theme.edit.bgcolor,
                       theme.edit.focus_color, font, -1, 1, 1)
        , notes(drawable, *this, nullptr, TR(trkeys::note_required, lang),
                group_id, theme.global.fgcolor, theme.global.bgcolor, font)
        , confirm(drawable, *this, this, TR(trkeys::confirm, lang), group_id,
                  theme.global.fgcolor, theme.global.bgcolor, theme.global.focus_color, 2, font,
                  6, 2)
        , flags(flags)
        , generic_warning(TR(trkeys::fmt_field_required, lang))
        , format_warning(TR(trkeys::fmt_invalid_format, lang))
        , toohigh_warning(TR(trkeys::fmt_toohigh_duration, lang))
        , field_comment(TR(trkeys::comment, lang))
        , field_ticket(TR(trkeys::ticket, lang))
        , field_duration(TR(trkeys::duration, lang))
        , warning_buffer()
    {
        this->set_bg_color(theme.global.bgcolor);

        this->impl = &composite_array;

        this->add_widget(&this->warning_msg);

        if (this->flags & DURATION_DISPLAY) {
            this->add_widget(&this->duration_label);
            this->add_widget(&this->duration_edit);
            this->add_widget(&this->duration_format);
        }
        if (this->flags & TICKET_DISPLAY) {
            this->add_widget(&this->ticket_label);
            this->add_widget(&this->ticket_edit);
        }
        if (this->flags & COMMENT_DISPLAY) {
            this->add_widget(&this->comment_label);
            this->add_widget(&this->comment_edit);
        }
        if (this->flags & DURATION_MANDATORY) {
            this->duration_label.set_text(TR(trkeys::duration_r, lang));
        }
        if (this->flags & TICKET_MANDATORY) {
            this->ticket_label.set_text(TR(trkeys::ticket_r, lang));
        }
        if (this->flags & COMMENT_MANDATORY) {
            this->comment_label.set_text(TR(trkeys::comment_r, lang));
        }

        if (this->flags & (COMMENT_MANDATORY | TICKET_MANDATORY | DURATION_MANDATORY)) {
            this->add_widget(&this->notes);
        }

        this->add_widget(&this->confirm);
    }

    ~FlatForm() override {
        this->clear();
    }

    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height) {
        this->set_xy(left, top);
        this->set_wh(width, height);

        Dimension dim;

        uint16_t labelmaxwidth = 0;

        if (this->flags & DURATION_DISPLAY) {
            dim = this->duration_label.get_optimal_dim();
            this->duration_label.set_wh(dim);

            labelmaxwidth = std::max(labelmaxwidth, this->duration_label.cx());
        }

        if (this->flags & TICKET_DISPLAY) {
            dim = this->ticket_label.get_optimal_dim();
            this->ticket_label.set_wh(dim);

            labelmaxwidth = std::max(labelmaxwidth, this->ticket_label.cx());
        }

        if (this->flags & COMMENT_DISPLAY) {
            dim = this->comment_label.get_optimal_dim();
            this->comment_label.set_wh(dim);

            labelmaxwidth = std::max(labelmaxwidth, this->comment_label.cx());
        }

        dim = this->warning_msg.get_optimal_dim();
        this->warning_msg.set_wh(width - labelmaxwidth - 20, dim.h);
        this->warning_msg.set_xy(left + labelmaxwidth + 20, top);

        int y = 20;

        if (this->flags & DURATION_DISPLAY) {
            this->duration_label.set_xy(left, top + y);

            dim = this->duration_format.get_optimal_dim();
            this->duration_format.set_wh(dim);

            dim = this->duration_edit.get_optimal_dim();
            this->duration_edit.set_wh((width - labelmaxwidth - 20) - this->duration_format.cx() - 20,
                                       dim.h);
            this->duration_edit.set_xy(left + labelmaxwidth + 20, top + y);

            this->duration_format.set_xy(this->duration_edit.right() + 10, top + y + 2);

            y += 30;
        }

        if (this->flags & TICKET_DISPLAY) {
            this->ticket_label.set_xy(left, top + y);

            dim = this->ticket_edit.get_optimal_dim();
            this->ticket_edit.set_wh(width - labelmaxwidth - 20, dim.h);
            this->ticket_edit.set_xy(left + labelmaxwidth + 20, top + y);

            y += 30;
        }

        if (this->flags & COMMENT_DISPLAY) {
            this->comment_label.set_xy(left, top + y);

            dim = this->comment_edit.get_optimal_dim();
            this->comment_edit.set_wh(width - labelmaxwidth - 20, dim.h);
            this->comment_edit.set_xy(left + labelmaxwidth + 20, top + y);

            y += 30;
        }

        if (this->flags & (COMMENT_MANDATORY | TICKET_MANDATORY | DURATION_MANDATORY)) {
            dim = this->notes.get_optimal_dim();
            this->notes.set_wh(width - labelmaxwidth - 20, dim.h);
            this->notes.set_xy(left + labelmaxwidth + 20, top + y);
        }

        dim = this->confirm.get_optimal_dim();
        this->confirm.set_wh(dim);
        this->confirm.set_xy(left + width - this->confirm.cx(), top + y + 10);
    }

    void notify(Widget2* widget, NotifyApi::notify_event_t event) override {
        if ((widget->group_id == this->confirm.group_id) &&
            (NOTIFY_COPY != event) &&
            (NOTIFY_CUT != event) &&
            (NOTIFY_PASTE != event)) {
            if (NOTIFY_SUBMIT == event) {
                this->check_confirmation();
            }
        }
        else {
            if ((NOTIFY_COPY == event) ||
                (NOTIFY_CUT == event) ||
                (NOTIFY_PASTE == event)) {
                if (this->notifier) {
                    this->notifier->notify(widget, event);
                }
            }
            else {
                WidgetParent::notify(widget, event);
            }
        }
    }

    void set_warning_buffer(const char * field, const char * format) {
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wformat-nonliteral")
        std::sprintf(this->warning_buffer, format, field);
        REDEMPTION_DIAGNOSTIC_POP
        this->warning_msg.set_text(this->warning_buffer);
    }

    unsigned long check_duration(const char * duration) {
        unsigned long res = 0;
        unsigned long hours = 0;
        unsigned long minutes = 0;
        char * end_p = nullptr;
        try {
            long d = strtoul(duration, &end_p, 10);
            if (*end_p == 'h') {
                res = (d > 0);
                hours = d;
                end_p++;
                d = strtoul(end_p, &end_p, 10);
                if (*end_p == 'm') {
                    res |= (d > 0);
                    minutes = d;
                    end_p++;
                }
                else if (d > 0) {
                    res = 0;
                }
            }
            else if (*end_p == 'm') {
                res = (d > 0);
                minutes = d;
                end_p++;
            }
        }
        catch (...) {
            res = 0;
        }
        if (res && *end_p != 0) {
            res = 0;
        }
        if (res > 0) {
            res = hours * 60 + minutes;
        }
        return res;
    }

    void check_confirmation() {
        if (((this->flags & DURATION_DISPLAY) == DURATION_DISPLAY) &&
            ((this->flags & DURATION_MANDATORY) == DURATION_MANDATORY) &&
            (this->duration_edit.num_chars == 0)) {
            this->set_warning_buffer(this->field_duration, this->generic_warning);
            this->set_widget_focus(&this->duration_edit, focus_reason_mousebutton1);
            this->rdp_input_invalidate(this->get_rect());
            return;
        }
        if (((this->flags & DURATION_DISPLAY) == DURATION_DISPLAY) &&
            (this->duration_edit.num_chars != 0)) {
            long res = this->check_duration(this->duration_edit.get_text());
            // res is duration in hours.
            if ((res <= 0) || (res >= 600000)) {
                if (res <= 0) {
                    this->duration_edit.set_text("");
                    this->set_warning_buffer(this->field_duration, this->format_warning);
                }
                else {
                    this->set_warning_buffer(this->field_duration, this->toohigh_warning);
                }
                this->set_widget_focus(&this->duration_edit, focus_reason_mousebutton1);
                this->rdp_input_invalidate(this->get_rect());
                return;
            }
        }
        if (((this->flags & TICKET_DISPLAY) == TICKET_DISPLAY) &&
            ((this->flags & TICKET_MANDATORY) == TICKET_MANDATORY) &&
            (this->ticket_edit.num_chars == 0)) {
            this->set_warning_buffer(this->field_ticket, this->generic_warning);
            this->set_widget_focus(&this->ticket_edit, focus_reason_mousebutton1);
            this->rdp_input_invalidate(this->get_rect());
            return;
        }
        if (((this->flags & COMMENT_DISPLAY) == COMMENT_DISPLAY) &&
            ((this->flags & COMMENT_MANDATORY) == COMMENT_MANDATORY) &&
            (this->comment_edit.num_chars == 0)) {
            this->set_warning_buffer(this->field_comment, this->generic_warning);
            this->set_widget_focus(&this->comment_edit, focus_reason_mousebutton1);
            this->rdp_input_invalidate(this->get_rect());
            return;
        }

        if (this->notifier) {
            this->notifier->notify(&this->confirm, NOTIFY_SUBMIT);
        }
    }

    void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap) override {
        if (keymap->nb_kevent_available() > 0){
            switch (keymap->top_kevent()){
            case Keymap2::KEVENT_ESC:
                keymap->get_kevent();
                this->send_notify(NOTIFY_CANCEL);
                break;
            default:
                WidgetParent::rdp_input_scancode(param1, param2, param3, param4, keymap);
                break;
            }
        }
    }
};
