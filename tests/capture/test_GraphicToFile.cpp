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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestGraphicToFile
#include "system/redemption_unit_tests.hpp"


#define LOGNULL
//#define LOGPRINT

#include "utils/png.hpp"
#include "utils/drawable.hpp"

#include <memory>

#include "transport/test_transport.hpp"
#include "transport/out_file_transport.hpp"
#include "transport/in_file_transport.hpp"
#include "transport/out_meta_sequence_transport.hpp"
#include "capture/FileToGraphic.hpp"
#include "capture/GraphicToFile.hpp"

#include "utils/dump_png24_from_rdp_drawable_adapter.hpp"

class DrawableToFile
{
protected:
    Transport & trans;
    unsigned zoom_factor;
    unsigned scaled_width;
    unsigned scaled_height;

    const Drawable & drawable;

private:
    std::unique_ptr<uint8_t[]> scaled_buffer;

public:
    DrawableToFile(Transport & trans, const Drawable & drawable, unsigned zoom)
    : trans(trans)
    , zoom_factor(std::min(zoom, 100u))
    , scaled_width(drawable.width())
    , scaled_height(drawable.height())
    , drawable(drawable)
    {
        const unsigned zoom_width = (this->drawable.width() * this->zoom_factor) / 100;
        const unsigned zoom_height = (this->drawable.height() * this->zoom_factor) / 100;
        this->scaled_width = (zoom_width + 3) & 0xFFC;
        this->scaled_height = zoom_height;
        if (this->zoom_factor != 100) {
            this->scaled_buffer.reset(new uint8_t[this->scaled_width * this->scaled_height * 3]);
        }
    }

    ~DrawableToFile() = default;

    /// \param  percent  0 to 100 or 100 if greater
    void zoom(unsigned percent) {
        percent = std::min(percent, 100u);
        const unsigned zoom_width = (this->drawable.width() * percent) / 100;
        const unsigned zoom_height = (this->drawable.height() * percent) / 100;
        this->zoom_factor = percent;
        this->scaled_width = (zoom_width + 3) & 0xFFC;
        this->scaled_height = zoom_height;
        if (this->zoom_factor != 100) {
            this->scaled_buffer.reset(new uint8_t[this->scaled_width * this->scaled_height * 3]);
        }
    }

    bool logical_frame_ended() const {
        return this->drawable.logical_frame_ended;
    }

    void flush() {
        if (this->zoom_factor == 100) {
            this->dump24();
        }
        else {
            this->scale_dump24();
        }
    }

private:
    void dump24() const {
        ::transport_dump_png24(
            this->trans, this->drawable.data(),
            this->drawable.width(), this->drawable.height(),
            this->drawable.rowsize(), true);
    }

    void scale_dump24() const {
        scale_data(
            this->scaled_buffer.get(), this->drawable.data(),
            this->scaled_width, this->drawable.width(),
            this->scaled_height, this->drawable.height(),
            this->drawable.rowsize());
        ::transport_dump_png24(
            this->trans, this->scaled_buffer.get(),
            this->scaled_width, this->scaled_height,
            this->scaled_width * 3, false);
    }

    static void scale_data(uint8_t *dest, const uint8_t *src,
                           unsigned int dest_width, unsigned int src_width,
                           unsigned int dest_height, unsigned int src_height,
                           unsigned int src_rowsize) {
        const uint32_t Bpp = 3;
        unsigned int y_pixels = dest_height;
        unsigned int y_int_part = src_height / dest_height * src_rowsize;
        unsigned int y_fract_part = src_height % dest_height;
        unsigned int yE = 0;
        unsigned int x_int_part = src_width / dest_width * Bpp;
        unsigned int x_fract_part = src_width % dest_width;

        while (y_pixels-- > 0) {
            unsigned int xE = 0;
            const uint8_t * x_src = src;
            unsigned int x_pixels = dest_width;
            while (x_pixels-- > 0) {
                dest[0] = x_src[2];
                dest[1] = x_src[1];
                dest[2] = x_src[0];

                dest += Bpp;
                x_src += x_int_part;
                xE += x_fract_part;
                if (xE >= dest_width) {
                    xE -= dest_width;
                    x_src += Bpp;
                }
            }
            src += y_int_part;
            yE += y_fract_part;
            if (yE >= dest_height) {
                yE -= dest_height;
                src += src_rowsize;
            }
        }
    }
};


const char expected_stripped_wrm[] =
/* 0000 */ "\xEE\x03\x1C\x00\x00\x00\x01\x00" // 03EE: META 0010: chunk_len=28 0001: 1 order
           "\x03\x00\x20\x03\x58\x02\x18\x00" // WRM version = 3, width = 800, height=600, bpp=24
           "\x58\x02\x00\x01\x2c\x01\x00\x04\x06\x01\x00\x10"
           //"\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // For WRM version >3

// initial screen content PNG image
/* 0000 */ "\x00\x10\xcc\x05\x00\x00\x01\x00"
/* 0000 */ "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a\x00\x00\x00\x0d\x49\x48\x44\x52" //.PNG........IHDR
/* 0010 */ "\x00\x00\x03\x20\x00\x00\x02\x58\x08\x02\x00\x00\x00\x15\x14\x15" //... ...X........
/* 0020 */ "\x27\x00\x00\x05\x8b\x49\x44\x41\x54\x78\x9c\xed\xc1\x01\x0d\x00" //'....IDATx......
/* 0030 */ "\x00\x00\xc2\xa0\xf7\x4f\x6d\x0e\x37\xa0\x00\x00\x00\x00\x00\x00" //.....Om.7.......
/* 0040 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0050 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0070 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0080 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0090 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0160 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0170 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0180 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0190 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0200 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0210 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0220 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0230 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0240 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0250 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0260 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0270 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0280 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0290 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 02a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 02b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 02c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 02d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 02e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 02f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0300 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0310 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0320 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0330 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0340 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0350 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0360 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0370 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0380 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0390 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 03a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 03b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 03c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 03d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 03e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 03f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0400 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0410 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0420 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0430 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0440 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0450 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0460 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0470 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0480 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0490 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 04a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 04b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 04c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 04d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 04e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 04f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0500 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0510 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0520 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0530 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0540 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0550 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0560 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0570 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0580 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0590 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 05a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x57\x03" //..............W.
/* 05b0 */ "\xfc\x93\x00\x01\x4b\x66\x2c\x0e\x00\x00\x00\x00\x49\x45\x4e\x44" //....Kf,.....IEND
/* 05c0 */ "\xae\x42\x60\x82"                                                 //.B`.

           "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
/* 0000 */ "\x00\xca\x9a\x3B\x00\x00\x00\x00" // 0x3B9ACA00 = 1000000000

           "\x00\x00\x10\x00\x00\x00\x01\x00" // 0000: ORDERS  001A:chunk_len=26 0002: 2 orders
/* 0000 */ "\x09\x0a\x2c\x20\x03\x58\x02\xff"         // Green Rect(0, 0, 800, 600)

/* 0000 */ "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
           "\x40\x0C\xAA\x3B\x00\x00\x00\x00" // 0x3BAA0C40 = 1001000000

/* 0000 */ "\x00\x00\x12\x00\x00\x00\x01\x00" // 0000: ORDERS  0012:chunk_len=18 0002: 1 orders
           "\x01\x6e\x32\x00\xbc\x02\x1e\x00\x00\xff"  // Blue  Rect(0, 50, 700, 80)

/* 0000 */ "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
           "\x00\xd3\xd7\x3b\x00\x00\x00\x00" // time = 1004000000

/* 0000 */ "\x00\x00\x0d\x00\x00\x00\x01\x00"
           "\x11\x32\x32\xff\xff"

/* 0000 */ "\xf0\x03\x10\x00\x00\x00\x01\x00"
           "\x80\x57\xf6\x3b\x00\x00\x00\x00"

/* 0000 */ "\x00\x00\x0d\x00\x00\x00\x01\x00"
           "\x11\x62\x32\x00\x00"
    ;

BOOST_AUTO_TEST_CASE(Test6SecondsStrippedScreenToWrm)
{
    // Timestamps are applied only when flushing
    struct timeval now;
    now.tv_usec = 0;
    now.tv_sec = 1000;

    Rect screen_rect(0, 0, 800, 600);
    StaticOutStream<65536> stream;
    CheckTransport trans(expected_stripped_wrm, sizeof(expected_stripped_wrm)-1, 511);

    BmpCache bmp_cache(BmpCache::Recorder, 24, 3, false,
                       BmpCache::CacheOption(600, 256, false),
                       BmpCache::CacheOption(300, 1024, false),
                       BmpCache::CacheOption(262, 4096, false));
    GlyphCache gly_cache;
    PointerCache ptr_cache;
    RDPDrawable drawable(screen_rect.cx, screen_rect.cy, 24);
    DumpPng24FromRDPDrawableAdapter dump_png24(drawable);
    GraphicToFile consumer(now, trans, screen_rect.cx, screen_rect.cy, 24, bmp_cache, gly_cache, ptr_cache, dump_png24, WrmCompressionAlgorithm::no_compression);

    consumer.draw(RDPOpaqueRect(screen_rect, GREEN), screen_rect);

    now.tv_sec++;
    consumer.timestamp(now);

    consumer.draw(RDPOpaqueRect(Rect(0, 50, 700, 30), BLUE), screen_rect);
    consumer.sync();

    now.tv_sec++;
    consumer.timestamp(now);

    now.tv_sec++;
    consumer.timestamp(now);

    now.tv_sec++;
    consumer.timestamp(now);

    consumer.draw(RDPOpaqueRect(Rect(0, 100, 700, 30), WHITE), screen_rect);
    now.tv_sec++;
    consumer.timestamp(now);

    now.tv_sec++;
    consumer.timestamp(now);

    RDPOpaqueRect cmd3(Rect(0, 150, 700, 30), RED);
    consumer.draw(cmd3, screen_rect);
    now.tv_sec++;
    consumer.timestamp(now);

    consumer.sync();
}

const char expected_stripped_wrm2[] =
/* 0000 */ "\xEE\x03\x1C\x00\x00\x00\x01\x00" // 03EE: META 0010: chunk_len=28 0001: 1 order
           "\x03\x00\x20\x03\x58\x02\x18\x00" // WRM version = 3, width = 800, height=600, bpp=24
           "\x58\x02\x00\x01\x2c\x01\x00\x04\x06\x01\x00\x10"
           //"\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // For WRM version >3

// initial screen content PNG image
/* 0000 */ "\x00\x10\xcc\x05\x00\x00\x01\x00"
/* 0000 */ "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a\x00\x00\x00\x0d\x49\x48\x44\x52" //.PNG........IHDR
/* 0010 */ "\x00\x00\x03\x20\x00\x00\x02\x58\x08\x02\x00\x00\x00\x15\x14\x15" //... ...X........
/* 0020 */ "\x27\x00\x00\x05\x8b\x49\x44\x41\x54\x78\x9c\xed\xc1\x01\x0d\x00" //'....IDATx......
/* 0030 */ "\x00\x00\xc2\xa0\xf7\x4f\x6d\x0e\x37\xa0\x00\x00\x00\x00\x00\x00" //.....Om.7.......
/* 0040 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0050 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0070 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0080 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0090 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0160 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0170 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0180 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0190 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0200 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0210 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0220 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0230 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0240 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0250 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0260 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0270 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0280 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0290 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 02a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 02b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 02c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 02d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 02e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 02f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0300 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0310 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0320 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0330 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0340 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0350 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0360 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0370 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0380 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0390 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 03a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 03b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 03c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 03d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 03e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 03f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0400 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0410 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0420 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0430 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0440 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0450 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0460 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0470 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0480 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0490 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 04a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 04b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 04c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 04d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 04e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 04f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0500 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0510 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0520 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0530 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0540 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0550 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0560 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0570 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0580 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0590 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 05a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x57\x03" //..............W.
/* 05b0 */ "\xfc\x93\x00\x01\x4b\x66\x2c\x0e\x00\x00\x00\x00\x49\x45\x4e\x44" //....Kf,.....IEND
/* 05c0 */ "\xae\x42\x60\x82"                                                 //.B`.

           "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
/* 0000 */ "\x00\xca\x9a\x3B\x00\x00\x00\x00" // 0x3B9ACA00 = 1000000000

           "\x00\x00\x1A\x00\x00\x00\x02\x00" // 0000: ORDERS  001A:chunk_len=26 0002: 2 orders
/* 0000 */ "\x09\x0a\x2c\x20\x03\x58\x02\xff"         // Green Rect(0, 0, 800, 600)
           "\x01\x6e\x32\x00\xbc\x02\x1e\x00\x00\xff"  // Blue  Rect(0, 50, 700, 80)

           "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
/* 0000 */ "\x40\x0c\xaa\x3b\x00\x00\x00\x00" // time = 1001000000

           "\x00\x00\x12\x00\x00\x00\x02\x00"
/* 0000 */ "\x11\x32\x32\xff\xff"             // WHITE rect
           "\x11\x62\x32\x00\x00"             // RED rect

           "\xf0\x03\x10\x00\x00\x00\x01\x00"
/* 0000 */ "\xc0\x99\x05\x3c\x00\x00\x00\x00" // time 1007000000

           "\x00\x00\x13\x00\x00\x00\x01\x00"
/* 0000 */ "\x01\x1f\x05\x00\x05\x00\x0a\x00\x0a\x00\x00" // BLACK rect
   ;


BOOST_AUTO_TEST_CASE(Test6SecondsStrippedScreenToWrmReplay2)
{
    // Same as above, show timestamps are applied only when flushing
    struct timeval now;
    now.tv_usec = 0;
    now.tv_sec = 1000;

    Rect screen_rect(0, 0, 800, 600);
    StaticOutStream<65536> stream;
    CheckTransport trans(expected_stripped_wrm2, sizeof(expected_stripped_wrm2)-1, 511);
    BmpCache bmp_cache(BmpCache::Recorder, 24, 3, false,
                       BmpCache::CacheOption(600, 256, false),
                       BmpCache::CacheOption(300, 1024, false),
                       BmpCache::CacheOption(262, 4096, false));
    GlyphCache gly_cache;
    PointerCache ptr_cache;
    RDPDrawable drawable(screen_rect.cx, screen_rect.cy, 24);
    DumpPng24FromRDPDrawableAdapter dump_png24(drawable);
    GraphicToFile consumer(now, trans, screen_rect.cx, screen_rect.cy, 24, bmp_cache, gly_cache, ptr_cache, dump_png24, WrmCompressionAlgorithm::no_compression);

    consumer.draw(RDPOpaqueRect(screen_rect, GREEN), screen_rect);
    consumer.draw(RDPOpaqueRect(Rect(0, 50, 700, 30), BLUE), screen_rect);

    now.tv_sec++;
    consumer.timestamp(now);

    consumer.draw(RDPOpaqueRect(Rect(0, 100, 700, 30), WHITE), screen_rect);
    consumer.draw(RDPOpaqueRect(Rect(0, 150, 700, 30), RED), screen_rect);
    now.tv_sec+=6;
    consumer.timestamp(now);

    consumer.draw(RDPOpaqueRect(Rect(5, 5, 10, 10), BLACK), screen_rect);

    consumer.sync();
}

BOOST_AUTO_TEST_CASE(TestCaptureToWrmReplayToPng)
{
    // Same as above, show timestamps are applied only when flushing
    BOOST_CHECK_EQUAL(0, 0);
    timeval now;
    now.tv_usec = 0;
    now.tv_sec = 1000;

    Rect screen_rect(0, 0, 800, 600);
    StaticOutStream<65536> stream;

    const char * filename = "./testcap.wrm";
    size_t len = strlen(filename);
    char path[1024];
    memcpy(path, filename, len);
    path[len] = 0;
    int fd = ::creat(path, 0777);
    if (fd == -1){
        LOG(LOG_INFO, "open failed with error : %s on %s", strerror(errno), path);
        BOOST_CHECK(false);
        return;
    }

    OutFileTransport trans(fd);
    BOOST_CHECK_EQUAL(0, 0);
    BmpCache bmp_cache(BmpCache::Recorder, 24, 3, false,
                       BmpCache::CacheOption(600, 256, false),
                       BmpCache::CacheOption(300, 1024, false),
                       BmpCache::CacheOption(262, 4096, false));
    GlyphCache gly_cache;
    PointerCache ptr_cache;
    RDPDrawable drawable(screen_rect.cx, screen_rect.cy, 24);
    DumpPng24FromRDPDrawableAdapter dump_png24_api(drawable);
    GraphicToFile consumer(now, trans, screen_rect.cx, screen_rect.cy, 24, bmp_cache, gly_cache, ptr_cache, dump_png24_api, WrmCompressionAlgorithm::no_compression);
    BOOST_CHECK_EQUAL(0, 0);
    RDPOpaqueRect cmd0(screen_rect, GREEN);
    consumer.draw(cmd0, screen_rect);
    RDPOpaqueRect cmd1(Rect(0, 50, 700, 30), BLUE);
    consumer.draw(cmd1, screen_rect);
    now.tv_sec++;
    BOOST_CHECK_EQUAL(0, 0);
    consumer.timestamp(now);
    consumer.sync();
    BOOST_CHECK_EQUAL(0, 0);

    RDPOpaqueRect cmd2(Rect(0, 100, 700, 30), WHITE);
    consumer.draw(cmd2, screen_rect);
    RDPOpaqueRect cmd3(Rect(0, 150, 700, 30), RED);
    consumer.draw(cmd3, screen_rect);
    now.tv_sec+=6;
    consumer.timestamp(now);
    consumer.sync();
    BOOST_CHECK_EQUAL(0, 0);
    trans.disconnect(); // close file before reading filesize
    BOOST_CHECK_EQUAL(1588, filesize(filename));

    char in_path[1024];
    len = strlen(filename);
    memcpy(in_path, filename, len);
    in_path[len] = 0;

    fd = ::open(in_path, O_RDONLY);
    if (fd == -1){
        LOG(LOG_INFO, "open '%s' failed with error : %s", path, strerror(errno));
        BOOST_CHECK(false);
        return;
    }
    InFileTransport in_wrm_trans(fd);

    const int groupid = 0;
    OutFilenameSequenceTransport out_png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "testcap", ".png", groupid);

    timeval begin_capture;
    begin_capture.tv_sec = 0; begin_capture.tv_usec = 0;
    timeval end_capture;
    end_capture.tv_sec = 0; end_capture.tv_usec = 0;
    FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, to_verbose_flags(0));
    RDPDrawable drawable1(player.screen_rect.cx, player.screen_rect.cy, 24);
    DrawableToFile png_recorder(out_png_trans, drawable1.impl(), 100);
    player.add_consumer(&drawable1, nullptr, nullptr, nullptr, nullptr);

    png_recorder.flush();
    out_png_trans.next();

    // Green Rect
    BOOST_CHECK_EQUAL(true, player.next_order());
    player.interpret_order();
    png_recorder.flush();
    out_png_trans.next();

    // Blue Rect
    BOOST_CHECK_EQUAL(true, player.next_order());
    player.interpret_order();
    png_recorder.flush();
    out_png_trans.next();

    // Timestamp
    BOOST_CHECK_EQUAL(true, player.next_order());
    player.interpret_order();
    png_recorder.flush();
    out_png_trans.next();

    // White Rect
    BOOST_CHECK_EQUAL(true, player.next_order());
    player.interpret_order();
    png_recorder.flush();
    out_png_trans.next();

    // Red Rect
    BOOST_CHECK_EQUAL(true, player.next_order());
    player.interpret_order();
    png_recorder.flush();
    out_png_trans.next();

    BOOST_CHECK_EQUAL(false, player.next_order());
    in_wrm_trans.disconnect();

    // clear PNG files
    size_t sz[6] = {1476, 2786, 2800, 2800, 2814, 2823};
    for (int i = 0; i < 6 ; i++){
        const char * filename = out_png_trans.seqgen()->get(i);
        BOOST_CHECK_EQUAL(sz[i], ::filesize(filename));
        ::unlink(filename);
    }
   ::unlink("./testcap.wrm");
}


const char expected_Red_on_Blue_wrm[] =
/* 0000 */ "\xEE\x03\x1C\x00\x00\x00\x01\x00" // 03EE: META 0010: chunk_len=28 0001: 1 order
           "\x03\x00\x64\x00\x64\x00\x18\x00" // WRM version 3, width = 20, height=10, bpp=24
           "\x02\x00\x00\x01\x02\x00\x00\x04\x02\x00\x00\x10"  // caches sizes
           //"\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // For WRM version >3

/* 0000 */ "\x00\x10\x75\x00\x00\x00\x01\x00"
/* 0000 */ "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a\x00\x00\x00\x0d\x49\x48\x44\x52" //.PNG........IHDR
/* 0010 */ "\x00\x00\x00\x64\x00\x00\x00\x64\x08\x02\x00\x00\x00\xff\x80\x02" //...d...d........
/* 0020 */ "\x03\x00\x00\x00\x34\x49\x44\x41\x54\x78\x9c\xed\xc1\x01\x0d\x00" //....4IDATx......
/* 0030 */ "\x00\x00\xc2\xa0\xf7\x4f\x6d\x0e\x37\xa0\x00\x00\x00\x00\x00\x00" //.....Om.7.......
/* 0040 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0050 */ "\x00\x00\x00\x00\x00\x00\x00\x7e\x0c\x75\x94\x00\x01\xa8\x50\xf2" //.......~.u....P.
/* 0060 */ "\x39\x00\x00\x00\x00\x49\x45\x4e\x44\xae\x42\x60\x82"             //9....IEND.B`.

/* 0000 */ "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
/* 0000 */ "\x00\xCA\x9A\x3B\x00\x00\x00\x00" // 0x000000003B9ACA00 = 1000000000

/* 0000 */ "\x00\x00\x2d\x00\x00\x00\x03\x00" // 0000: ORDERS  001A:chunk_len=26 0002: 2 orders
/* 0000 */ "\x19\x0a\x4c\x64\x64\xff"         // Blue rect  // order 0A=opaque rect
// -----------------------------------------------------
/* 0020 */ "\x03\x09\x00\x00\x04\x02"         // Secondary drawing order header. Order = 02: Compressed bitmap
           "\x01\x00\x14\x0a\x18\x07\x00\x00\x00" // 0x01=cacheId 0x00=pad 0x14=width(20) 0x0A=height(10) 0x18=24 bits
                                                  // 0x0007=bitmapLength 0x0000=cacheIndex
           "\xc0\x04\x00\x00\xff\x00\x94"         // compressed bitamp data (7 bytes)
// -----------------------------------------------------

           "\x59\x0d\x3d\x01\x00\x5a\x14\x0a\xcc" // order=0d : MEMBLT

           "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
           "\x40\x0C\xAA\x3B\x00\x00\x00\x00" // 0x000000003BAA0C40 = 1001000000

           "\x00\x00\x1e\x00\x00\x00\x01\x00" // 0000: ORDERS  001A:chunk_len=26 0002: 2 orders
// -----------------------------------------------------
/* 0000 */ "\x03\x09\x00\x00\x04\x02"
           "\x01\x00\x14\x0a\x18\x07\x00\x00\x00"
           "\xc0\x04\x00\x00\xff\x00\x94"
// -----------------------------------------------------
           ;

BOOST_AUTO_TEST_CASE(TestSaveCache)
{
    // Timestamps are applied only when flushing
    struct timeval now;
    now.tv_usec = 0;
    now.tv_sec = 1000;

    Rect scr(0, 0, 100, 100);
    CheckTransport trans(expected_Red_on_Blue_wrm, sizeof(expected_Red_on_Blue_wrm)-1, 511);
    trans.disable_remaining_error();
    BmpCache bmp_cache(BmpCache::Recorder, 24, 3, false,
                       BmpCache::CacheOption(2, 256, false),
                       BmpCache::CacheOption(2, 1024, false),
                       BmpCache::CacheOption(2, 4096, false));
    GlyphCache gly_cache;
    PointerCache ptr_cache;
    RDPDrawable drawable(scr.cx, scr.cy, 24);
    DumpPng24FromRDPDrawableAdapter dump_png(drawable);
    GraphicToFile consumer(now, trans, scr.cx, scr.cy, 24, bmp_cache, gly_cache, ptr_cache, dump_png, WrmCompressionAlgorithm::no_compression);
    consumer.timestamp(now);

    consumer.draw(RDPOpaqueRect(scr, BLUE), scr);

    uint8_t comp20x10RED[] = {
        0xc0, 0x04, 0x00, 0x00, 0xFF, // MIX 20 (0, 0, FF)
        0x00, 0x94                    // FILL 9 * 20
    };

    Bitmap bloc20x10(24, 24, nullptr, 20, 10, comp20x10RED, sizeof(comp20x10RED), true );
    consumer.draw(
        RDPMemBlt(0, Rect(0, scr.cy - 10, bloc20x10.cx(), bloc20x10.cy()), 0xCC, 0, 0, 0),
        scr,
        bloc20x10);
    consumer.sync();

    now.tv_sec++;
    consumer.timestamp(now);

    consumer.save_bmp_caches();

    consumer.sync();
}

BOOST_AUTO_TEST_CASE(TestReloadSaveCache)
{
    GeneratorTransport in_wrm_trans(expected_Red_on_Blue_wrm, sizeof(expected_Red_on_Blue_wrm)-1);
    timeval begin_capture;
    begin_capture.tv_sec = 0; begin_capture.tv_usec = 0;
    timeval end_capture;
    end_capture.tv_sec = 0; end_capture.tv_usec = 0;
    FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, to_verbose_flags(0));

    const int groupid = 0;
    OutFilenameSequenceTransport out_png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "TestReloadSaveCache", ".png", groupid);
    RDPDrawable drawable(player.screen_rect.cx, player.screen_rect.cy, 24);
    DrawableToFile png_recorder(out_png_trans, drawable.impl(), 100);

    player.add_consumer(&drawable, nullptr, nullptr, nullptr, nullptr);
    while (player.next_order()){
        player.interpret_order();
    }
    png_recorder.flush();

    const char * filename = out_png_trans.seqgen()->get(0);
    BOOST_CHECK_EQUAL(298, ::filesize(filename));
    ::unlink(filename);
}

const char expected_reset_rect_wrm[] =
/* 0000 */ "\xEE\x03\x1C\x00\x00\x00\x01\x00" // 03EE: META 0010: chunk_len=28 0001: 1 order
           "\x03\x00\x64\x00\x64\x00\x18\x00" // WRM version 3, width = 20, height=10, bpp=24
           "\x02\x00\x00\x01\x02\x00\x00\x04\x02\x00\x00\x10"  // caches sizes
           //"\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // For WRM version >3

/* 0000 */ "\x00\x10\x75\x00\x00\x00\x01\x00"
/* 0000 */ "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a\x00\x00\x00\x0d\x49\x48\x44\x52" //.PNG........IHDR
/* 0010 */ "\x00\x00\x00\x64\x00\x00\x00\x64\x08\x02\x00\x00\x00\xff\x80\x02" //...d...d........
/* 0020 */ "\x03\x00\x00\x00\x34\x49\x44\x41\x54\x78\x9c\xed\xc1\x01\x0d\x00" //....4IDATx......
/* 0030 */ "\x00\x00\xc2\xa0\xf7\x4f\x6d\x0e\x37\xa0\x00\x00\x00\x00\x00\x00" //.....Om.7.......
/* 0040 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0050 */ "\x00\x00\x00\x00\x00\x00\x00\x7e\x0c\x75\x94\x00\x01\xa8\x50\xf2" //.......~.u....P.
/* 0060 */ "\x39\x00\x00\x00\x00\x49\x45\x4e\x44\xae\x42\x60\x82"             //9....IEND.B`.

/* 0000 */ "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
/* 0000 */ "\x00\xCA\x9A\x3B\x00\x00\x00\x00" // 0x000000003B9ACA00 = 1000000000

/* 0000 */ "\x00\x00\x1e\x00\x00\x00\x03\x00" // 0000: ORDERS  001A:chunk_len=26 0002: 2 orders
           "\x19\x0a\x1c\x64\x64\xff\x11"     // Red Rect
           "\x5f\x05\x05\xf6\xf6\x00\xff"     // Blue Rect
           "\x11\x5f\x05\x05\xf6\xf6\xff\x00" // Red Rect

           // save orders cache
/* 0000 */ "\x02\x10"
/* 0000 */ "\x0F\x02"                         //.data length
/* 0000 */ "\x00\x00\x01\x00"

/* 0000 */ "\x0a\x00\x00\x00\x00\x01\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0010 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0030 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0a\x00\x0a\x00\x50" //...............P
/* 0040 */ "\x00\x50\x00\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //.P..............
/* 0050 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0070 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0080 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0090 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00" //................
/* 00a0 */ "\x01\x00\x00\x00\x00\x00\x01\x00\x01\x00\x00\x00\x00\x00\x00\x00" //................
/* 00b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0160 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0170 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0180 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0190 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01c0 */ "\x00\x00\x00\x00\x00\x00"                                         //......

/* 0000 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // MultiDstBlt

/* 0000 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // MultiOpaqueRect

/* 0000 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // MultiOpaqueRect
/* 0000 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

/* 0000 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // MultiScrBlt

           "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
           "\x40\x0C\xAA\x3B\x00\x00\x00\x00" // 0x000000003BAA0C40 = 1001000000

           "\x00\x00\x10\x00\x00\x00\x01\x00"
           "\x11\x3f\x0a\x0a\xec\xec\x00\xff" // Green Rect
           ;

BOOST_AUTO_TEST_CASE(TestSaveOrderStates)
{
    // Timestamps are applied only when flushing
    struct timeval now;
    now.tv_usec = 0;
    now.tv_sec = 1000;

    Rect scr(0, 0, 100, 100);
    CheckTransport trans(expected_reset_rect_wrm, sizeof(expected_reset_rect_wrm)-1, 511);
    BmpCache bmp_cache(BmpCache::Recorder, 24, 3, false,
                       BmpCache::CacheOption(2, 256, false),
                       BmpCache::CacheOption(2, 1024, false),
                       BmpCache::CacheOption(2, 4096, false));
    GlyphCache gly_cache;
    PointerCache ptr_cache;
    RDPDrawable drawable(scr.cx, scr.cy, 24);
    DumpPng24FromRDPDrawableAdapter dump_png(drawable);
    GraphicToFile consumer(now, trans, scr.cx, scr.cy, 24, bmp_cache, gly_cache, ptr_cache, dump_png, WrmCompressionAlgorithm::no_compression);
    consumer.timestamp(now);

    consumer.draw(RDPOpaqueRect(scr, RED), scr);
    consumer.draw(RDPOpaqueRect(scr.shrink(5), BLUE), scr);
    consumer.draw(RDPOpaqueRect(scr.shrink(10), RED), scr);

    consumer.sync();

    consumer.send_save_state_chunk();

    now.tv_sec++;
    consumer.timestamp(now);
    consumer.draw(RDPOpaqueRect(scr.shrink(20), GREEN), scr);
    consumer.sync();
}

BOOST_AUTO_TEST_CASE(TestReloadOrderStates)
{
    GeneratorTransport in_wrm_trans(expected_reset_rect_wrm, sizeof(expected_reset_rect_wrm)-1);
    timeval begin_capture;
    begin_capture.tv_sec = 0; begin_capture.tv_usec = 0;
    timeval end_capture;
    end_capture.tv_sec = 0; end_capture.tv_usec = 0;
    FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, to_verbose_flags(0));

    const int groupid = 0;
    OutFilenameSequenceTransport out_png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "TestReloadOrderStates", ".png", groupid);
    RDPDrawable drawable(player.screen_rect.cx, player.screen_rect.cy, 24);
    DrawableToFile png_recorder(out_png_trans, drawable.impl(), 100);

    player.add_consumer(&drawable, nullptr, nullptr, nullptr, nullptr);
    while (player.next_order()){
        player.interpret_order();
    }
    png_recorder.flush();
    const char * filename = out_png_trans.seqgen()->get(0);
    BOOST_CHECK_EQUAL(341, ::filesize(filename));
    ::unlink(filename);
}

const char expected_continuation_wrm[] =
/* 0000 */ "\xEE\x03\x1C\x00\x00\x00\x01\x00" // 03EE: META 0010: chunk_len=16 0001: 1 order
           "\x01\x00\x64\x00\x64\x00\x18\x00" // WRM version 1, width = 20, height=10, bpp=24
           "\x02\x00\x00\x01\x02\x00\x00\x04\x02\x00\x00\x10"  // caches sizes

           "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
           "\x40\x0C\xAA\x3B\x00\x00\x00\x00" // 0x000000003BAA0C40 = 1001000000

           // save images
/* 0000 */ "\x00\x10\x49\x01\x00\x00\x01\x00"

/* 0000 */ "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a\x00\x00\x00\x0d\x49\x48\x44\x52" //.PNG........IHDR
/* 0010 */ "\x00\x00\x00\x64\x00\x00\x00\x64\x08\x02\x00\x00\x00\xff\x80\x02" //...d...d........
/* 0020 */ "\x03\x00\x00\x01\x08\x49\x44\x41\x54\x78\x9c\xed\xdd\x31\x0e\x83" //.....IDATx...1..
/* 0030 */ "\x30\x10\x00\x41\x3b\xe2\xff\x5f\x86\x32\xc8\x05\xb0\x55\x14\x34" //0..A;.._.2...U.4
/* 0040 */ "\xd3\xb9\xb3\x56\xa7\x73\x07\x73\x1f\x3c\xf5\xf9\xf5\x05\xfe\x89" //...V.s.s.<......
/* 0050 */ "\x58\x81\x58\x81\x58\xc1\xb6\x9c\xe7\xb0\xf1\xbf\xf6\x31\xcf\x47" //X.X.X........1.G
/* 0060 */ "\x93\x15\x88\x15\x88\x15\x88\x15\xac\x0b\x7e\xb1\x6c\xb8\xd7\xbb" //..........~.l...
/* 0070 */ "\x7e\xdf\x4c\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56" //~.LV V V V V V V
/* 0080 */ "\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56" // V V V V V V V V
/* 0090 */ "\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56" // V V V V V V V V
/* 00a0 */ "\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56" // V V V V V V V V
/* 00b0 */ "\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56" // V V V V V V V V
/* 00c0 */ "\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56" // V V V V V V V V
/* 00d0 */ "\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56" // V V V V V V V V
/* 00e0 */ "\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56" // V V V V V V V V
/* 00f0 */ "\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56" // V V V V V V V V
/* 0100 */ "\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56" // V V V V V V V V
/* 0110 */ "\x70\xf3\x3d\x78\xff\xff\x38\x33\x59\x81\x58\x81\x58\x81\x58\xc1" //p.=x..83Y.X.X.X.
/* 0120 */ "\xb4\xc0\x9f\x33\x59\x81\x58\x81\x58\x81\x58\xc1\x01\x8e\xa9\x07" //...3Y.X.X.X.....
/* 0130 */ "\xcb\xdb\x96\x4d\x96\x00\x00\x00\x00\x49\x45\x4e\x44\xae\x42\x60" //...M.....IEND.B`
/* 0140 */ "\x82"

           // save orders cache
           "\x02\x10\xA0\x01\x00\x00\x01\x00"
/* 0000 */ "\x0a\x00\x00\x00\x00\x01\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0010 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0030 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

/* 0000 */ "\x0a\x00\x0a\x00\x50\x00\x50\x00\xff\x00\x00\x00\x00\x00\x00\x00" //....P.P.........
/* 0010 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0030 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0040 */ "\x01\x00\x01\x00\x00\x00\x00\x00\x01\x00\x01\x00\x00\x00\x00\x00" //................
/* 0050 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0070 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0080 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0090 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

           "\x00\x00\x10\x00\x00\x00\x01\x00"
           "\x11\x3f\x0a\x0a\xec\xec\x00\xff" // Green Rect
           ;

BOOST_AUTO_TEST_CASE(TestContinuationOrderStates)
{
    GeneratorTransport in_wrm_trans(expected_continuation_wrm, sizeof(expected_continuation_wrm)-1);
    timeval begin_capture;
    begin_capture.tv_sec = 0; begin_capture.tv_usec = 0;
    timeval end_capture;
    end_capture.tv_sec = 0; end_capture.tv_usec = 0;
    FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, to_verbose_flags(0));

    const int groupid = 0;
    OutFilenameSequenceTransport out_png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "TestContinuationOrderStates", ".png", groupid);
    const FilenameGenerator * seq = out_png_trans.seqgen();
    BOOST_CHECK(seq);
    RDPDrawable drawable(player.screen_rect.cx, player.screen_rect.cy, 24);
    DrawableToFile png_recorder(out_png_trans, drawable.impl(), 100);

    player.add_consumer(&drawable, nullptr, nullptr, nullptr, nullptr);
    while (player.next_order()){
        player.interpret_order();
    }
    png_recorder.flush();
    const char * filename = seq->get(0);
    BOOST_CHECK_EQUAL(341, ::filesize(filename));
    ::unlink(filename);
}
