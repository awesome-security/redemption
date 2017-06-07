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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to writing RDP orders to file and rereading them

*/

#define RED_TEST_MODULE TestRdpClientTestCard
#include "system/redemption_unit_tests.hpp"


#define LOGNULL
//#define LOGPRINT

#include "core/error.hpp"
#include "core/font.hpp"
#include "mod/internal/test_card_mod.hpp"

#include "test_only/front/fake_front.hpp"

RED_AUTO_TEST_CASE(TestShowTestCard)
{
    RED_CHECK(true);
    ClientInfo info;
    info.keylayout = 0x04C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 800;
    info.height = 600;

    FakeFront front(info, 0);

    Font font;

    RED_CHECK(true);
    TestCardMod mod(front, info.width, info.height, font);
    RED_CHECK_NO_THROW(mod.draw_event(time(nullptr), front));
}
