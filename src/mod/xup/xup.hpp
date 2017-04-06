/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   xup module main header file
*/

#pragma once

#include "mod/mod_api.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <errno.h>

/* include "ther h files */
#include "core/RDP/pointer.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "core/front_api.hpp"
#include "transport/transport.hpp"
#include "utils/stream.hpp"
#include "utils/bitmap.hpp"

struct xup_mod : public mod_api {

enum {
    XUPWM_PAINT        = 3,
    XUPWM_KEYDOWN      = 15,
    XUPWM_KEYUP        = 16,
    XUPWM_SYNCHRONIZE  = 17,
    XUPWM_MOUSEMOVE    = 100,
    XUPWM_LBUTTONUP    = 101,
    XUPWM_LBUTTONDOWN  = 102,
    XUPWM_RBUTTONUP    = 103,
    XUPWM_RBUTTONDOWN  = 104,
    XUPWM_BUTTON3UP    = 105,
    XUPWM_BUTTON3DOWN  = 106,
    XUPWM_BUTTON4UP    = 107,
    XUPWM_BUTTON4DOWN  = 108,
    XUPWM_BUTTON5UP    = 109,
    XUPWM_BUTTON5DOWN  = 110,
    XUPWM_BUTTON_OK    = 300,
    XUPWM_SCREENUPDATE = 0x4444,
    XUPWM_CHANNELDATA  = 0x5555
};

    /* mod data */
    FrontAPI & front;
    int width;
    int height;
    int bpp;
    Transport & t;
    int rop;
    int fgcolor;
    BGRPalette const & palette332 = BGRPalette::classic_332();

    RDPPen pen;

    xup_mod( Transport & t
           , FrontAPI & front
           , uint16_t /*front_width*/
           , uint16_t /*front_height*/
           , int context_width
           , int context_height
           , int context_bpp
           )
    : front(front)
    , width(context_width)
    , height(context_height)
    , bpp(context_bpp)
    , t(t)
    , rop(0xCC)
    {
        StaticOutStream<256> stream;
        stream.out_skip_bytes(4);
        stream.out_uint16_le(103);
        stream.out_uint32_le(200);
        /* x and y */
        int xy = 0;
        stream.out_uint32_le(xy);
        /* width and height */
        int cxcy = ((this->width & 0xffff) << 16) | this->height;
        stream.out_uint32_le(cxcy);
        stream.out_uint32_le(0);
        stream.out_uint32_le(0);
        stream.set_out_uint32_le(stream.get_offset(), 0);
        this->t.send(stream.get_data(), stream.get_offset());
    }

    ~xup_mod() override {}

    int get_fd() const override { return this->t.get_fd(); }

    enum {
        XUPWM_INVALIDATE = 200
    };

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 *) override {
        LOG(LOG_INFO, "input mouse");

        if (device_flags & MOUSE_FLAG_MOVE) { /* 0x0800 */
            this->x_input_event(XUPWM_MOUSEMOVE, x, y, 0, 0);
        }
        if (device_flags & MOUSE_FLAG_BUTTON1) { /* 0x1000 */
            this->x_input_event(
                XUPWM_LBUTTONUP + ((device_flags & MOUSE_FLAG_DOWN) >> 15),
                x, y, 0, 0);
        }
        if (device_flags & MOUSE_FLAG_BUTTON2) { /* 0x2000 */
            this->x_input_event(
                XUPWM_RBUTTONUP + ((device_flags & MOUSE_FLAG_DOWN) >> 15),
                x, y, 0, 0);
        }
        if (device_flags & MOUSE_FLAG_BUTTON3) { /* 0x4000 */
            this->x_input_event(
                XUPWM_BUTTON3UP + ((device_flags & MOUSE_FLAG_DOWN) >> 15),
                x, y, 0, 0);
        }
        if (device_flags == MOUSE_FLAG_BUTTON4 || /* 0x0280 */ device_flags == 0x0278) {
            this->x_input_event(XUPWM_BUTTON4DOWN, x, y, 0, 0);
            this->x_input_event(XUPWM_BUTTON4UP, x, y, 0, 0);
        }
        if (device_flags == MOUSE_FLAG_BUTTON5 || /* 0x0380 */ device_flags == 0x0388) {
            this->x_input_event(XUPWM_BUTTON5DOWN, x, y, 0, 0);
            this->x_input_event(XUPWM_BUTTON5UP, x, y, 0, 0);
        }
    }

    void rdp_input_scancode(long param1, long param2, long device_flags, long param4, Keymap2 * keymap) override {
        // TODO xup_mod::rdp_input_scancode: unimplemented
        (void)param1;
        (void)param2;
        (void)device_flags;
        (void)param4;
        (void)keymap;
        LOG(LOG_INFO, "scan code");
        /*
        if (ki != 0) {
            int msg = (device_flags & KBD_FLAG_UP)?XUPWM_KEYUP:XUPWM_KEYDOWN;
            this->x_input_event(msg, ki->chr, ki->sym, param1, device_flags);
        }
        */
    }

    void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override {
        // TODO xup_mod::rdp_input_scancode: unimplemented
        (void)time;
        (void)device_flags;
        (void)param1;
        (void)param2;
        LOG(LOG_INFO, "overloaded by subclasses");
        return;
    }

    void rdp_input_invalidate(Rect r) override {
        LOG(LOG_INFO, "rdp_input_invalidate");
        if (!r.isempty()) {
            this->x_input_event(XUPWM_INVALIDATE,
                ((r.x & 0xffff) << 16) | (r.y & 0xffff),
                ((r.cx & 0xffff) << 16) | (r.cy & 0xffff),
                0, 0);
        }
    }

    void refresh(Rect r) override {
        this->rdp_input_invalidate(r);
    }

    void x_input_event(const int msg, const long param1, const long param2, const long param3, const long param4)
    {
        StaticOutStream<256> stream;
        stream.out_skip_bytes(4); // skip yet unknown len
        stream.out_uint16_le(103);
        stream.out_uint32_le(msg);
        stream.out_uint32_le(param1);
        stream.out_uint32_le(param2);
        stream.out_uint32_le(param3);
        stream.out_uint32_le(param4);
        uint32_t len = stream.get_offset();
        stream.set_out_uint32_le(len, 0);
        this->t.send(stream.get_data(), len);
    }

    void draw_event(time_t now, gdi::GraphicApi & drawable) override {
        (void)now;
        try{
            uint8_t buf[32768];

            if (!this->t.atomic_read(buf, 8)){
                throw Error(ERR_TRANSPORT_NO_MORE_DATA);
            }

            InStream stream(buf);
            unsigned type = stream.in_uint16_le();
            unsigned num_orders = stream.in_uint16_le();
            unsigned len = stream.in_uint32_le();
            if (type == 1) {
                std::unique_ptr<uint8_t[]> dynbuf;
                {
                    auto pbuf = buf;
                    if (len > stream.get_capacity()) {
                        pbuf = new uint8_t[len];
                        dynbuf.reset(pbuf);
                    }
                    stream = InStream(pbuf, len);
                }

                if (!this->t.atomic_read(buf, len)){
                    throw Error(ERR_TRANSPORT_NO_MORE_DATA);
                }


                for (unsigned index = 0; index < num_orders; index++) {
                    type = stream.in_uint16_le();
                    switch (type) {
                    case 1:
                        this->front.begin_update();
                        break;
                    case 2:
                        this->front.end_update();
                        break;
                    case 3:
                    {
                        const Rect r(
                            stream.in_sint16_le(),
                            stream.in_sint16_le(),
                            stream.in_uint16_le(),
                            stream.in_uint16_le());
                         drawable.draw(RDPPatBlt(r, this->rop, BLACK, WHITE,
                            RDPBrush(r.x, r.y, 3, 0xaa,
                            reinterpret_cast<const uint8_t *>("\xaa\x55\xaa\x55\xaa\x55\xaa\x55"))
                            ), r, gdi::ColorCtx::from_bpp(this->bpp, this->palette332));
                    }
                    break;
                    case 4:
                    {
                        const Rect r(
                            stream.in_sint16_le(),
                            stream.in_sint16_le(),
                            stream.in_uint16_le(),
                            stream.in_uint16_le());
                        const int srcx = stream.in_sint16_le();
                        const int srcy = stream.in_sint16_le();
                        const RDPScrBlt scrblt(r, 0xCC, srcx, srcy);
                        drawable.draw(scrblt, r);
                    }
                    break;
                    case 5:
                    {
                        const Rect r(
                            stream.in_sint16_le(),
                            stream.in_sint16_le(),
                            stream.in_uint16_le(),
                            stream.in_uint16_le());
                        int len_bmpdata = stream.in_uint32_le();
                        const uint8_t * bmpdata = stream.in_uint8p(len_bmpdata);
                        int width = stream.in_uint16_le();
                        int height = stream.in_uint16_le();
                        int srcx = stream.in_sint16_le();
                        int srcy = stream.in_sint16_le();
                        Bitmap bmp(this->bpp, bpp, &this->palette332, width, height, bmpdata, sizeof(bmpdata));
                        drawable.draw(RDPMemBlt(0, r, 0xCC, srcx, srcy, 0), r, bmp);
                    }
                    break;
                    case 10: /* server_set_clip */
                    {
                        const Rect r(
                            stream.in_sint16_le(),
                            stream.in_sint16_le(),
                            stream.in_uint16_le(),
                            stream.in_uint16_le());
                          // TODO see clip management
//                        this->server_set_clip(r);
                    }
                    break;
                    case 11: /* server_reset_clip */
                          // TODO see clip management
//                        this->server_reset_clip();
                    break;
                    case 12: /* server_set_fgcolor */
                    {
                        this->fgcolor = stream.in_uint32_le();
                    }
                    break;
                    case 14:
                        this->rop = stream.in_uint16_le();
                    break;
                    case 17:
                    {
                        int style = stream.in_uint16_le();
                        int width = stream.in_uint16_le();
                        this->pen.style = style;
                        this->pen.width = width;
                    }
                    break;
                    case 18:
                    {
                        int x1 = stream.in_sint16_le();
                        int y1 = stream.in_sint16_le();
                        int x2 = stream.in_sint16_le();
                        int y2 = stream.in_sint16_le();
                        const RDPLineTo lineto(1, x1, y1, x2, y2, WHITE,
                                               this->rop,
                                               RDPPen(this->pen.style, this->pen.width, this->fgcolor));
                        drawable.draw(lineto, Rect(0,0,1,1), gdi::ColorCtx::from_bpp(this->bpp, this->palette332));
                    }
                    break;
                    case 19:
                    {
                        Pointer cursor;
                        cursor.x = stream.in_sint16_le();
                        cursor.y = stream.in_sint16_le();
                        stream.in_copy_bytes(cursor.data, 32 * (32 * 3));
                        stream.in_copy_bytes(cursor.mask, 32 * (32 / 8));
                        this->front.set_pointer(cursor);
                    }
                    break;
                    default:
                        throw 1;
                    }
                }
            }
        }
        catch(...){
            this->event.signal = BACK_EVENT_NEXT;
            this->front.must_be_stop_capture();
        }
    }

    void send_to_front_channel(const char * const mod_channel_name, uint8_t const * data, size_t length, size_t chunk_size, int flags) override {
        // TODO xup_mod::send_to_front_channel: unimplemented
        (void)mod_channel_name;
        (void)data;
        (void)length;
        (void)chunk_size;
        (void)flags;
    }
};
