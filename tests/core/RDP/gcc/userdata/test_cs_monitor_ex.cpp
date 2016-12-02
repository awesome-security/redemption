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
   Copyright (C) Wallix 2016
   Author(s): Jennifer Inthavong

   T.124 Generic Conference Control (GCC) Unit Test
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestCS_MONITOR_EX
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
// #define LOGPRINT

#include "transport/test_transport.hpp"
#include "core/RDP/gcc/userdata/cs_monitor_ex.hpp"

BOOST_AUTO_TEST_CASE(Test_gcc_user_data_cs_monitor_ex)
{
    const char indata[] =
        "\x08\xc0"         // TS_UD_HEADER::type = CS_MONITOR_EX (0xc008)
        "\x24\x00"         // length = 36 bytes
        "\x00\x00\x00\x00" // TS_UD_CS_MONITOR_EX::flags. Unused. MUST be set to zero
        "\x14\x00\x00\x00" // TS_UD_CS_MONITOR_EX::monitorAttributeSize 20 bytes = size of TS_MONITOR_ATTRIBUTES
        "\x01\x00\x00\x00" // TS_UD_CS_MONITOR_EX::monitorCount

        				           // TS_UD_CS_MONITOR_EX::monitorAttributesArray
    		"\x96\x00\x00\x00" // | physicalWidth = 150 mm
    		"\x64\x00\x00\x00" // | physicalHeight = 100 mm
    		"\x5A\x00\x00\x00" // | orientation = ORIENTATION_PORTRAIT = 90
    		"\x78\x00\x00\x00" // | desktopScaleFactor // 120%
        "\x64\x00\x00\x00" // | deviceScaleFactor // 100%
    ;

    constexpr auto sz = sizeof(indata) - 1u;
    GeneratorTransport gt(indata, sz);
    uint8_t buf[sz];
    auto end = buf;
    gt.recv(&end, sz);
    GCC::UserData::CSMonitorEx cs_monitor_ex;
    InStream stream(buf);
    cs_monitor_ex.recv(stream);
    BOOST_CHECK_EQUAL(36, cs_monitor_ex.length);
    BOOST_CHECK_EQUAL(CS_MONITOR_EX, cs_monitor_ex.userDataType);
    BOOST_CHECK_EQUAL(20, cs_monitor_ex.monitorAttributeSize);
    BOOST_CHECK_EQUAL(1, cs_monitor_ex.monitorCount);
    
    BOOST_CHECK_EQUAL(150, cs_monitor_ex.monitorAttributesArray[0].physicalWidth);
    BOOST_CHECK_EQUAL(100, cs_monitor_ex.monitorAttributesArray[0].physicalHeight);
    BOOST_CHECK_EQUAL(90, cs_monitor_ex.monitorAttributesArray[0].orientation); // ORIENTATION_PORTRAIT = 90

    BOOST_CHECK_EQUAL(120, cs_monitor_ex.monitorAttributesArray[0].desktopScaleFactor);
	  BOOST_CHECK_EQUAL(100, cs_monitor_ex.monitorAttributesArray[0].deviceScaleFactor);

    cs_monitor_ex.log("Client Received");

}
