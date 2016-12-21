/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean, Javier Caverni, Meng Tan, Raphael Zhou
*/


#pragma once

#include <ctime>
#include <vector>

#include "core/callback.hpp"
#include "core/font.hpp"
#include "core/wait_obj.hpp"
#include "core/RDP/caches/glyphcache.hpp"
#include "core/RDP/orders/RDPOrdersCommon.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "gdi/graphic_api.hpp"

class Inifile;

enum {
    BUTTON_STATE_UP   = 0,
    BUTTON_STATE_DOWN = 1
};

class EventHandler {
public:
    class CB {
    public:
        virtual ~CB() = default;

        virtual void operator()(time_t now, gdi::GraphicApi& drawable) = 0;
    };

private:
    wait_obj* event_;

    CB* cb_;

    int fd_;

public:
    EventHandler(wait_obj* event, CB* cb, int fd = INVALID_SOCKET)
    : event_(event)
    , cb_(cb)
    , fd_(fd) {
        REDASSERT(event_);
        REDASSERT(cb_);
    }

    void operator()(time_t now, gdi::GraphicApi& drawable) {
        if (this->cb_) {
            (*this->cb_)(now, drawable);
        }
    }

    wait_obj* get_event() const {
        return this->event_;
    }

    int get_fd() const {
        return this->fd_;
    }
};

class mod_api : public Callback
{
protected:
    wait_obj event;

public:
    mod_api()
    {
        this->event.set(0);
    }

    ~mod_api() override {}

    virtual wait_obj& get_event() { return this->event; }

    virtual int get_fd() const { return INVALID_SOCKET; }

    virtual void get_event_handlers(std::vector<EventHandler>&/* out_event_handlers*/) {}

    virtual void send_to_front_channel(const char * const mod_channel_name,
        uint8_t const * data, size_t length, size_t chunk_size, int flags) = 0;

    // draw_event is run when mod socket received some data (drawing order)
    // or auto-generated by modules, say to comply to some refresh.
    // draw event decodes incoming traffic from backend and eventually calls front to draw things
    // may raise an exception (say if connection to server is closed), but returns nothings
    virtual void draw_event(time_t now, gdi::GraphicApi & drawable) = 0;

    // used when context changed to avoid creating a new module
    // it usually perform some task identical to what constructor does
    // henceforth it should often be called by constructors
    virtual void refresh_context(Inifile &) {}

    virtual bool is_up_and_running() { return false; }

    virtual void disconnect(time_t now) { (void)now; }

    virtual void display_osd_message(std::string const &) {}

    virtual void move_size_widget(int16_t/* left*/, int16_t/* top*/, uint16_t/* width*/, uint16_t/* height*/) {}

    virtual bool disable_input_event_and_graphics_update(
            bool disable_input_event, bool disable_graphics_update) {
        (void)disable_input_event;
        (void)disable_graphics_update;
        return false;
    }

    virtual void send_input(int/* time*/, int/* message_type*/, int/* device_flags*/, int/* param1*/, int/* param2*/) {}
};
