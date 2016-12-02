/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1 of the License, or
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
 *   Copyright (C) Wallix 2010-2014
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan
 */

#pragma once

#include "widget.hpp"
#include "label.hpp"
#include "keyboard/keymap2.hpp"
#include "utils/sugar/cast.hpp"
#include "gdi/graphic_api.hpp"

class WidgetEdit : public Widget2 {
public:
    WidgetLabel label;
    size_t buffer_size;
    size_t num_chars;
    size_t edit_buffer_pos;
    size_t edit_pos;
    size_t cursor_px_pos;
    int w_text;
    int h_text;
    int cursor_color;
    int focus_color;
    bool drawall;
    bool draw_border_focus;
    Font const & font;

    WidgetEdit(gdi::GraphicApi & drawable, int16_t x, int16_t y, uint16_t cx,
               Widget2 & parent, NotifyApi* notifier, const char * text,
               int group_id, int fgcolor, int bgcolor, int focus_color, Font const & font,
               std::size_t edit_position = -1, int xtext = 0, int ytext = 0)
    : Widget2(drawable, Rect(x,y,cx,1), parent, notifier, group_id)
    , label(drawable, 0, 0, *this, nullptr, text, false, 0, fgcolor, bgcolor, font, xtext, ytext)
    , w_text(0)
    , h_text(0)
    , cursor_color(0x888888)
    // , cursor_color(fgcolor)
    , focus_color(focus_color)
    , drawall(false)
    , draw_border_focus(true)
    , font(font)
    {
        if (text) {
            this->buffer_size = strlen(text);
            this->num_chars = UTF8Len(byte_ptr_cast(this->label.buffer));
            this->edit_pos = std::min(this->num_chars, edit_position);
            this->edit_buffer_pos = UTF8GetPos(reinterpret_cast<uint8_t *>(this->label.buffer), this->edit_pos);
            this->cursor_px_pos = 0;
            char c = this->label.buffer[this->edit_buffer_pos];
            this->label.buffer[this->edit_buffer_pos] = 0;
            gdi::TextMetrics tm1(this->font, this->label.buffer);
            this->w_text = tm1.width;
            this->h_text = tm1.height;
            this->cursor_px_pos = this->w_text;
            this->label.buffer[this->edit_buffer_pos] = c;
            // TODO: tm.height unused ?
            gdi::TextMetrics tm2(this->font, &this->label.buffer[this->edit_buffer_pos]);
            this->w_text += tm2.width;
        } else {
            this->buffer_size = 0;
            this->num_chars = 0;
            this->edit_buffer_pos = 0;
            this->edit_pos = 0;
            this->cursor_px_pos = 0;
        }

        // TODO: tm.width unused ?
        gdi::TextMetrics tm(this->font, "Édp");
        this->h_text = tm.height;
        this->set_cy(this->h_text + this->label.y_text * 2);
        this->label.set_cx(this->cx());
        this->label.set_cy(this->cy());
        this->label.set_x(this->label.x() + 1);
        this->label.set_y(this->label.y() + 1);
        this->set_cx(this->cx() + 2);
        this->set_cy(this->cy() + 2);
        this->h_text -= 1;

        this->pointer_flag = Pointer::POINTER_EDIT;
    }

    ~WidgetEdit() override {}

    virtual void set_text(const char * text/*, int position = 0*/)
    {
        this->label.buffer[0] = 0;
        this->buffer_size = 0;
        this->num_chars = 0;
        this->w_text = 0;
        if (text && *text) {
            const size_t n = strlen(text);
            const size_t remain_n = WidgetLabel::buffer_size - 1;

            this->buffer_size = ((remain_n >= n) ? n : ::UTF8StringAdjustedNbBytes(::byte_ptr_cast(text), remain_n));

            memcpy(this->label.buffer, text, this->buffer_size);
            this->label.buffer[this->buffer_size] = 0;
            gdi::TextMetrics tm(this->font, this->label.buffer);
            this->w_text = tm.width;
            this->h_text = tm.height;
            if (this->label.auto_resize_) {
                this->set_cx(this->label.x_text * 2 + this->w_text);
                this->set_cy(this->label.y_text * 2 + this->h_text);
                if (this->buffer_size == 1) {
                    this->set_cx(this->cx() - 2);
                }
            }
            this->num_chars = UTF8Len(byte_ptr_cast(this->label.buffer));
        }
        this->edit_pos = this->num_chars;
        this->edit_buffer_pos = this->buffer_size;
        this->cursor_px_pos = this->w_text;
    }

    virtual void insert_text(const char * text/*, int position = 0*/)
    {
        if (text && *text) {
            const size_t n = strlen(text);
            const size_t tmp_buffer_size = this->buffer_size;

            const size_t remain_n = WidgetLabel::buffer_size - 1 - this->buffer_size;
            const size_t max_n = ((remain_n >= n) ? n : ::UTF8StringAdjustedNbBytes(::byte_ptr_cast(text), remain_n));
            const size_t total_n = max_n + this->buffer_size;

            if (this->edit_pos == this->buffer_size || total_n == WidgetLabel::buffer_size - 1) {
                memcpy(this->label.buffer + this->buffer_size, text, max_n);
            }
            else {
                memmove(this->label.buffer + this->edit_buffer_pos + n, this->label.buffer + this->edit_buffer_pos,
                        std::min(WidgetLabel::buffer_size - 1 - (this->edit_buffer_pos + n),
                                 this->buffer_size - this->edit_buffer_pos));
                memcpy(this->label.buffer + this->edit_buffer_pos, text, max_n);
            }
            this->buffer_size = total_n;
            this->label.buffer[this->buffer_size] = 0;
            gdi::TextMetrics tm(this->font, this->label.buffer);
            this->w_text = tm.width;
            this->h_text = tm.height;
            if (this->label.auto_resize_) {
                this->set_cx(this->label.x_text * 2 + this->w_text);
                this->set_cy(this->label.y_text * 2 + this->h_text);
                if (this->buffer_size == 1) {
                    this->set_cx(this->cx() - 2);
                }
            }
            const size_t tmp_num_chars = this->num_chars;
            this->num_chars = UTF8Len(byte_ptr_cast(this->label.buffer));
            Rect rect = this->get_cursor_rect();
            rect.cx = this->w_text - this->cursor_px_pos;
            if (this->edit_pos == tmp_buffer_size || total_n == WidgetLabel::buffer_size - 1) {
                this->cursor_px_pos = this->w_text;
                this->edit_buffer_pos = this->buffer_size;
            }
            else {
                const size_t pos = this->edit_buffer_pos + max_n;
                const char c = this->label.buffer[pos];
                this->label.buffer[pos] = 0;
                // TODO: tm.height unused ?
                gdi::TextMetrics tm(this->font, this->label.buffer + this->edit_buffer_pos);
                this->label.buffer[pos] = c;
                this->cursor_px_pos += tm.width;
                this->edit_buffer_pos += max_n;
            }
            this->edit_pos += this->num_chars - tmp_num_chars;
            this->update_draw_cursor(rect);
        }
    }

    const char * get_text() const
    {
        return this->label.get_text();
    }

    void set_x(int16_t x) override {
        Widget2::set_x(x);
        this->label.set_x(x + 1);
    }

    void set_y(int16_t y) override {
        Widget2::set_y(y);
        this->label.set_y(y + 1);
    }

    void set_cx(uint16_t cx) override
    {
        Widget2::set_cx(cx);
        this->label.set_cx(cx - 2);
    }

    void set_cy(uint16_t cy) override {
        Widget2::set_cy(cy);
        this->label.set_cy(cy - 2);
    }

    void draw(const Rect& clip) override {
        this->label.draw(clip);
        if (this->has_focus) {
            this->draw_cursor(this->get_cursor_rect());
            if (this->draw_border_focus) {
                this->draw_border(clip, this->focus_color);
            }
        }
        else {
            this->draw_border(clip, this->label.bg_color);
        }
    }

    void draw_border(const Rect& clip, int color)
    {
        //top
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->x(), this->y(), this->cx() - 1, 1
        )), color), this->get_rect());
        //left
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->x(), this->y() + 1, 1, this->cy() - 2
        )), color), this->get_rect());
        //right
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->x() + this->cx() - 1, this->y(), 1, this->cy()
        )), color), this->get_rect());
        //bottom
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->x(), this->y() + this->cy() - 1, this->cx(), 1
        )), color), this->get_rect());
    }

    virtual Rect get_cursor_rect() const
    {
        return Rect(this->label.x_text + this->cursor_px_pos + this->label.x() + 1,
                    this->label.y_text + this->label.y(),
                    1,
                    this->h_text);
    }

    void draw_current_cursor() {
        if (this->has_focus) {
            this->draw_cursor(this->get_cursor_rect());
        }
    }
    void draw_cursor(const Rect& clip)
    {
        if (!clip.isempty()) {
            this->drawable.draw(RDPOpaqueRect(clip, this->cursor_color), this->get_rect());
        }
    }

    void increment_edit_pos()
    {
        this->edit_pos++;
        size_t n = UTF8GetPos(reinterpret_cast<uint8_t *>(this->label.buffer + this->edit_buffer_pos), 1);
        char c = this->label.buffer[this->edit_buffer_pos + n];
        this->label.buffer[this->edit_buffer_pos + n] = 0;
        gdi::TextMetrics tm(this->font, this->label.buffer + this->edit_buffer_pos);
        this->h_text = tm.height;
        this->cursor_px_pos += tm.width;
        this->label.buffer[this->edit_buffer_pos + n] = c;
        this->edit_buffer_pos += n;

        if (this->label.shift_text(this->cursor_px_pos)) {
            this->drawall = true;
        }
    }

    size_t utf8len_current_char()
    {
        size_t len = 1;
        while ((this->label.buffer[this->edit_buffer_pos + len] & 0xC0) == 0x80){
            ++len;
        }
        return len;
    }

    void decrement_edit_pos()
    {
        size_t len = 1;
        while (/*this->edit_buffer_pos - len >= 0 &&
               (*/(this->label.buffer[this->edit_buffer_pos - len] & 0xC0) == 0x80/*)*/){
            ++len;
        }

        this->edit_pos--;
        char c = this->label.buffer[this->edit_buffer_pos];
        this->label.buffer[this->edit_buffer_pos] = 0;
        gdi::TextMetrics tm(this->font, this->label.buffer + this->edit_buffer_pos - len);
        this->h_text = tm.height;
        this->cursor_px_pos -= tm.width;
        this->label.buffer[this->edit_buffer_pos] = c;
        this->edit_buffer_pos -= len;

        if (this->label.shift_text(this->cursor_px_pos)) {
            this->drawall = true;
        }
    }

    virtual void update_draw_cursor(Rect old_cursor)
    {
        this->drawable.begin_update();
        if (this->drawall) {
            this->drawall = false;
            this->draw(this->get_rect());
        }
        else {
            this->label.draw(old_cursor);
            this->draw_cursor(this->get_cursor_rect());
        }
        this->drawable.end_update();
    }

    void move_to_last_character()
    {
        Rect old_cursor_rect = this->get_cursor_rect();
        this->edit_pos = this->num_chars;
        this->edit_buffer_pos = this->buffer_size;
        this->cursor_px_pos = this->w_text;

        if (this->label.shift_text(this->cursor_px_pos)) {
            this->drawall = true;
        }

        this->update_draw_cursor(old_cursor_rect);
    }

    void move_to_first_character()
    {
        Rect old_cursor_rect = this->get_cursor_rect();
        this->edit_pos = 0;
        this->edit_buffer_pos = 0;
        this->cursor_px_pos = 0;

        if (this->label.shift_text(this->cursor_px_pos)) {
            this->drawall = true;
        }

        this->update_draw_cursor(old_cursor_rect);
    }

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap) override {
        if (device_flags == (MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN)) {
            if (x <= this->x() + this->label.x_text) {
                if (this->edit_pos) {
                    this->move_to_first_character();
                }
            }
            else if (x >= this->w_text + this->x() + this->label.x_text) {
                if (this->edit_pos < this->num_chars) {
                    this->move_to_last_character();
                }
            }
            else {
                Rect old_cursor_rect = this->get_cursor_rect();
                int xx = this->x() + this->label.x_text;
                size_t e = this->edit_pos;
                this->edit_pos = 0;
                this->edit_buffer_pos = 0;
                size_t len = this->utf8len_current_char();
                while (this->edit_buffer_pos < this->buffer_size) {
                    char c = this->label.buffer[this->edit_buffer_pos + len];
                    this->label.buffer[this->edit_buffer_pos + len] = 0;
                    gdi::TextMetrics tm(this->font, this->label.buffer + this->edit_buffer_pos);
                    // TODO: tm.height unused ?
                    this->label.buffer[this->edit_buffer_pos + len] = c;
                    xx += tm.width;
                    if (xx >= x) {
                        xx -= tm.width;
                        break;
                    }
                    len = this->utf8len_current_char();
                    this->edit_buffer_pos += len;
                    ++this->edit_pos;
                }
                this->cursor_px_pos = xx - (this->x() + this->label.x_text);
                if (e != this->edit_pos) {
                    this->update_draw_cursor(old_cursor_rect);
                }
            }
        } else {
            Widget2::rdp_input_mouse(device_flags, x, y, keymap);
        }
    }

    void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap) override {
        while (keymap->nb_kevent_available() > 0){
            uint32_t nb_kevent = keymap->nb_kevent_available();
            switch (keymap->top_kevent()){
                case Keymap2::KEVENT_LEFT_ARROW:
                case Keymap2::KEVENT_UP_ARROW:
                    keymap->get_kevent();
                    if (this->edit_pos > 0) {
                        Rect old_cursor_rect = this->get_cursor_rect();
                        this->decrement_edit_pos();
                        this->update_draw_cursor(old_cursor_rect);
                    }

                    if (keymap->is_ctrl_pressed()) {
                        while ( (this->label.buffer[(this->edit_buffer_pos)-1] != ' ')
                                 || (this->label.buffer[(this->edit_buffer_pos)] ==' ') ){
                            if (this->edit_pos > 0) {
                                Rect old_cursor_rect = this->get_cursor_rect();
                                this->decrement_edit_pos();
                                this->update_draw_cursor(old_cursor_rect);
                            }
                            else
                                break;
                        }
                        break;
                    }
                    break;

                case Keymap2::KEVENT_RIGHT_ARROW:
                case Keymap2::KEVENT_DOWN_ARROW:
                    keymap->get_kevent();
                    if (this->edit_pos < this->num_chars) {
                        Rect old_cursor_rect = this->get_cursor_rect();
                        this->increment_edit_pos();
                        this->update_draw_cursor(old_cursor_rect);
                    }

                    if (keymap->is_ctrl_pressed()) {
                        while ( (this->label.buffer[(this->edit_buffer_pos)-1] == ' ')
                                || (this->label.buffer[(this->edit_buffer_pos)] != ' ') ){
                            if (this->edit_pos < this->num_chars) {
                                Rect old_cursor_rect = this->get_cursor_rect();
                                this->increment_edit_pos();
                                this->update_draw_cursor(old_cursor_rect);
                            }
                            else
                                break;
                        }
                        break;
                    }
                    break;

                case Keymap2::KEVENT_BACKSPACE:
                    keymap->get_kevent();
                    if (this->edit_pos > 0) {
                        this->num_chars--;
                        size_t pxtmp = this->cursor_px_pos;
                        size_t ebpos = this->edit_buffer_pos;
                        this->decrement_edit_pos();
                        UTF8RemoveOneAtPos(reinterpret_cast<uint8_t *>(this->label.buffer + this->edit_buffer_pos), 0);
                        this->buffer_size += this->edit_buffer_pos - ebpos;
                        this->drawable.begin_update();
                        this->draw(Rect(this->x() + this->cursor_px_pos + this->label.x_text,
                                        this->y() + this->label.y_text + 1,
                                        this->w_text - this->cursor_px_pos + 3,
                                        this->h_text
                                        ));
                        this->drawable.end_update();
                        this->w_text -= pxtmp - this->cursor_px_pos;
                    }

                    if (keymap->is_ctrl_pressed()) {
                        while (this->label.buffer[(this->edit_buffer_pos)-1] != ' '){
                            if (this->edit_pos > 0) {
                                this->num_chars--;
                                size_t pxtmp = this->cursor_px_pos;
                                size_t ebpos = this->edit_buffer_pos;
                                this->decrement_edit_pos();
                                UTF8RemoveOneAtPos(reinterpret_cast<uint8_t *>(this->label.buffer + this->edit_buffer_pos), 0);
                                this->buffer_size += this->edit_buffer_pos - ebpos;
                                this->drawable.begin_update();
                                this->draw(Rect(this->x() + this->cursor_px_pos + this->label.x_text,
                                        this->y() + this->label.y_text + 1,
                                        this->w_text - this->cursor_px_pos + 3,
                                        this->h_text
                                ));
                                this->drawable.end_update();
                                this->w_text -= pxtmp - this->cursor_px_pos;
                            }
                            else
                                break;
                        }
                        break;
                    }


                    break;
                case Keymap2::KEVENT_DELETE:
                    keymap->get_kevent();
                    if (this->edit_pos < this->num_chars) {
                        size_t len = this->utf8len_current_char();
                        char c = this->label.buffer[this->edit_buffer_pos + len];
                        this->label.buffer[this->edit_buffer_pos + len] = 0;
                        gdi::TextMetrics tm(this->font, this->label.buffer + this->edit_buffer_pos);
                        this->h_text = tm.height;
                        this->label.buffer[this->edit_buffer_pos + len] = c;
                        UTF8RemoveOneAtPos(reinterpret_cast<uint8_t *>(this->label.buffer + this->edit_buffer_pos), 0);
                        this->buffer_size -= len;
                        this->num_chars--;
                        this->drawable.begin_update();
                        this->draw(Rect(this->x() + this->cursor_px_pos + this->label.x_text,
                                        this->y() + this->label.y_text + 1,
                                        this->w_text - this->cursor_px_pos + 3,
                                        this->h_text
                                        ));
                        this->draw_cursor(this->get_cursor_rect());
                        this->drawable.end_update();
                        this->w_text -= tm.width;
                    }


                    if (keymap->is_ctrl_pressed()) {
                        while ((this->label.buffer[(this->edit_buffer_pos)] != ' ')
                            || (this->label.buffer[(this->edit_buffer_pos)-1] == ' ')){
                            if (this->edit_pos < this->num_chars) {
                                size_t len = this->utf8len_current_char();
                                char c = this->label.buffer[this->edit_buffer_pos + len];
                                this->label.buffer[this->edit_buffer_pos + len] = 0;
                                gdi::TextMetrics tm(this->font, this->label.buffer + this->edit_buffer_pos);
                                this->h_text = tm.height;
                                this->label.buffer[this->edit_buffer_pos + len] = c;
                                UTF8RemoveOneAtPos(reinterpret_cast<uint8_t *>(this->label.buffer + this->edit_buffer_pos), 0);
                                this->buffer_size -= len;
                                this->num_chars--;
                                this->drawable.begin_update();
                                this->draw(Rect(this->x() + this->cursor_px_pos + this->label.x_text,
                                                this->y() + this->label.y_text + 1,
                                                this->w_text - this->cursor_px_pos + 3,
                                                this->h_text
                                                ));
                                this->draw_cursor(this->get_cursor_rect());
                                this->drawable.end_update();
                                this->w_text -= tm.width;
                            }
                            else
                                break;
                        }
                        break;
                    }

                    break;
                case Keymap2::KEVENT_END:
                    keymap->get_kevent();
                    if (this->edit_pos < this->num_chars) {
                        this->move_to_last_character();
                    }
                    break;
                case Keymap2::KEVENT_HOME:
                    keymap->get_kevent();
                    if (this->edit_pos) {
                        this->move_to_first_character();
                    }
                    break;
                case Keymap2::KEVENT_KEY:
                    if (this->num_chars < WidgetLabel::buffer_size - 5) {
                        uint32_t c = keymap->get_char();
                        UTF8InsertOneAtPos(reinterpret_cast<uint8_t *>(this->label.buffer + this->edit_buffer_pos), 0, c, WidgetLabel::buffer_size - 1 - this->edit_buffer_pos);
                        size_t tmp = this->edit_buffer_pos;
                        size_t pxtmp = this->cursor_px_pos;
                        this->increment_edit_pos();
                        this->buffer_size += this->edit_buffer_pos - tmp;
                        this->num_chars++;
                        this->send_notify(NOTIFY_TEXT_CHANGED);
                        this->w_text += this->cursor_px_pos - pxtmp;
                        this->update_draw_cursor(Rect(
                            this->x() + pxtmp + this->label.x_text + 1,
                            this->y() + this->label.y_text + 1,
                            this->w_text - pxtmp + 1,
                            this->h_text
                            ));
                    }
                    else {
                        // No need to get_event if get_char has been called already
                        keymap->get_kevent();
                    }
                    break;
                case Keymap2::KEVENT_ENTER:
                    keymap->get_kevent();
                    this->send_notify(NOTIFY_SUBMIT);
                    break;
                case Keymap2::KEVENT_PASTE:
                    keymap->get_kevent();
                    this->send_notify(NOTIFY_PASTE);
                    break;
                case Keymap2::KEVENT_COPY:
                    keymap->get_kevent();
                    this->send_notify(NOTIFY_COPY);
                    break;
                case Keymap2::KEVENT_CUT:
                    keymap->get_kevent();
                    this->send_notify(NOTIFY_CUT);
                    {
                        this->drawable.begin_update();
                        this->label.draw(this->label.get_rect());
                        this->draw_cursor(this->get_cursor_rect());
                        this->drawable.end_update();
                    }
                    break;
                default:
                    Widget2::rdp_input_scancode(param1, param2, param3, param4, keymap);
                    break;
            }
            if (nb_kevent == keymap->nb_kevent_available()) {
                // this is to prevent infinite loop if the kevent is not consummed
                keymap->get_kevent();
            }
        }
    }
};

