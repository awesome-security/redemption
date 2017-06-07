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
   Author(s): Christophe Grosjean

   Unit test to RDP Orders coder/decoder
   Using lib boost functions for testing
*/

#define RED_TEST_MODULE TestOrderScrBlt
#include "system/redemption_unit_tests.hpp"

//#define LOGPRINT
#define LOGNULL

#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"

#include "./test_orders.hpp"

RED_AUTO_TEST_CASE(TestScrBlt)
{
    using namespace RDP;

    RDPScrBlt scrblt_ref(Rect(0, 0, 10, 10), 0, 0, 0);

    {
        StaticOutStream<1000> out_stream;

        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPScrBlt state_scrblt(Rect(0, 0, 10, 10), 0, 0, 0);

        RDPOrderCommon newcommon(SCREENBLT, Rect(10, 10, 800, 600));
        RDPScrBlt(Rect(100, 150, 50, 60), 0xFF, 300, 400).emit(out_stream, newcommon, state_common, state_scrblt);

        uint8_t datas[16] = {CHANGE | STANDARD, SCREENBLT,
            0x7F,   // x, y, cx, cy, rop, srcx, srcy coordinates changed
            100, 0, // x = 100
            150, 0, // y = 150
            50,  0, // cx = 50
            60,  0, // cy = 60
            0xFF,   // rop = 0xFF
            0x2C, 1,  // srcx = 300
            0x90, 1,  // srcy = 400
        };
        check_datas(out_stream.get_offset(), out_stream.get_data(), 16, datas, "ScrBlt 1");

        InStream in_stream(out_stream.get_data(), out_stream.get_offset());


        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(SCREENBLT), common_cmd.order);

        RDPScrBlt cmd(Rect(0, 0, 10, 10), 0, 0, 0);
        cmd.receive(in_stream, header);

        check<RDPScrBlt>(common_cmd, cmd,
            RDPOrderCommon(SCREENBLT, Rect(0, 0, 800, 600)),
            RDPScrBlt(Rect(100, 150, 50, 60), 0xFF, 300, 400),
            "ScrBlt 1");
    }

    {
        StaticOutStream<1000> out_stream;

        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPScrBlt state_scrblt(Rect(0, 0, 10, 10), 0, 0, 0);

        RDPOrderCommon newcommon(SCREENBLT, Rect(10, 10, 800, 600));
        RDPScrBlt(Rect(300, 400, 50, 60), 0xFF, 100, 150).emit(out_stream, newcommon, state_common, state_scrblt);

        uint8_t datas[16] = {
            CHANGE | STANDARD,
            SCREENBLT,
            0x7F,   // x, y, cx, cy, rop, srcx, srcy coordinates changed
            0x2C, 1, // x = 300
            0x90, 1, // y = 400
            50,  0, // cx = 50
            60,  0, // cy = 60
            0xFF,   // rop = 0xFF
            100, 0,  // srcx = 100
            150, 0,  // srcy = 150
        };
        check_datas(out_stream.get_offset(), out_stream.get_data(), 16, datas, "ScrBlt 2");

        InStream in_stream(out_stream.get_data(), out_stream.get_offset());


        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(SCREENBLT), common_cmd.order);

        RDPScrBlt cmd(Rect(0, 0, 10, 10), 0, 0, 0);
        cmd.receive(in_stream, header);

        check<RDPScrBlt>(common_cmd, cmd,
            RDPOrderCommon(SCREENBLT, Rect(0, 0, 800, 600)),
            RDPScrBlt(Rect(300, 400, 50, 60), 0xFF, 100, 150),
            "ScrBlt 2");
    }

    {
        StaticOutStream<1000> out_stream;

        RDPOrderCommon state_common(0, Rect(0, 0, 800, 600));
        RDPScrBlt state_scrblt(Rect(310, 390, 10, 10), 0xFF, 110, 140);

        RDPOrderCommon newcommon(SCREENBLT, Rect(10, 10, 800, 600));
        RDPScrBlt(Rect(300, 400, 50, 60), 0xFF, 100, 150).emit(out_stream, newcommon, state_common, state_scrblt);

        uint8_t datas[9] = {
            static_cast<uint8_t>(CHANGE | STANDARD | DELTA),
            static_cast<uint8_t>(SCREENBLT),
            static_cast<uint8_t>(0x6F),   // x, y, cx, cy, srcx, srcy coordinates changed
            static_cast<uint8_t>(-10),    // x = -10 -> 300
            static_cast<uint8_t>(+10),    // y = 390 +10 = 400
            static_cast<uint8_t>(+40),    // cx = 10 + 40 -> 50
            static_cast<uint8_t>(+50),    // cy = 10 + 50 -> 60
            static_cast<uint8_t>(-10),     // srcx = 110 - 10 = 100
            static_cast<uint8_t>(+10),    // srcy = 140 +10 = 150
        };
        check_datas(out_stream.get_offset(), out_stream.get_data(), 9, datas, "ScrBlt 3");

        InStream in_stream(out_stream.get_data(), out_stream.get_offset());


        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(SCREENBLT), common_cmd.order);

        RDPScrBlt cmd(Rect(310, 390, 10, 10), 0xFF, 110, 140);
        cmd.receive(in_stream, header);

        check<RDPScrBlt>(common_cmd, cmd,
            RDPOrderCommon(SCREENBLT, Rect(0, 0, 800, 600)),
            RDPScrBlt(Rect(300, 400, 50, 60), 0xFF, 100, 150),
            "ScrBlt 3");
    }

    {
        StaticOutStream<1000> out_stream;

        RDPOrderCommon state_common(SCREENBLT, Rect(311, 0, 800, 600));
        RDPScrBlt state_scrblt(Rect(310, 390, 10, 10), 0xFF, 110, 140);

        RDPOrderCommon newcommon(SCREENBLT, Rect(311, 0, 800, 600));
        RDPScrBlt(Rect(300, 400, 50, 60), 0xFF, 100, 150).emit(out_stream, newcommon, state_common, state_scrblt);

        uint8_t datas[8] = {
            static_cast<uint8_t>(STANDARD | BOUNDS | DELTA | LASTBOUNDS),
            static_cast<uint8_t>(0x6F),   // x, y, cx, cy, srcx, srcy coordinates changed
            static_cast<uint8_t>(-10),    // x = -10 -> 300
            static_cast<uint8_t>(+10),    // y = 390 +10 = 400
            static_cast<uint8_t>(+40),    // cx = 10 + 40 -> 50
            static_cast<uint8_t>(+50),    // cy = 10 + 50 -> 60
            static_cast<uint8_t>(-10),     // srcx = 110 - 10 = 100
            static_cast<uint8_t>(+10),    // srcy = 140 +10 = 150
        };
        check_datas(out_stream.get_offset(), out_stream.get_data(), 8, datas, "ScrBlt 4");

        InStream in_stream(out_stream.get_data(), out_stream.get_offset());


        RDPOrderCommon common_cmd = state_common;
        uint8_t control = in_stream.in_uint8();
        RED_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(in_stream, control);

        RED_CHECK_EQUAL(static_cast<uint8_t>(SCREENBLT), common_cmd.order);

        RDPScrBlt cmd(Rect(310, 390, 10, 10), 0xFF, 110, 140);
        cmd.receive(in_stream, header);

        check<RDPScrBlt>(common_cmd, cmd,
            RDPOrderCommon(SCREENBLT, Rect(311, 0, 800, 600)),
            RDPScrBlt(Rect(300, 400, 50, 60), 0xFF, 100, 150),
            "ScrBlt 4");
    }
}
