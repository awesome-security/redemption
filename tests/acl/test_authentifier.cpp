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
  Copyright (C) Wallix 2013
  Author(s): Christophe Grosjean, Meng Tan, Jennifer Inthavong
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestAuthentifierNew
#include "system/redemption_unit_tests.hpp"


#define LOGNULL
// #define LOGPRINT

//#include "acl/module_manager.hpp"
#include "transport/count_transport.hpp"
#include "transport/test_transport.hpp"
#include "acl/authentifier.hpp"

struct ActivityAlwaysTrue : ActivityChecker {
    bool check_and_reset_activity() override { return true; }
};
struct ActivityAlwaysFalse : ActivityChecker {
    bool check_and_reset_activity() override { return false; }
};


BOOST_AUTO_TEST_CASE(TestAuthentifierNoKeepalive)
{
    BackEvent_t signal       = BACK_EVENT_NONE;
    BackEvent_t front_signal = BACK_EVENT_NONE;

    Inifile ini;

    ini.set<cfg::globals::keepalive_grace_delay>(cfg::globals::keepalive_grace_delay::type{30});
    ini.set<cfg::globals::session_timeout>(cfg::globals::session_timeout::type{900});
    ini.set<cfg::debug::auth>(255);

    MMIni mm(ini);

    char outdata[] =
        // Time: 10011
           "\x00\x00\x01\xA4"
           "login\nASK\n"
           "ip_client\n!\n"
           "ip_target\n!\n"
           "target_device\nASK\n"
           "target_login\nASK\n"
           "session_log_redirection\n!False\n"
           "bpp\n!24\n"
           "height\n!600\n"
           "width\n!800\n"
           "selector_current_page\n!1\n"
           "selector_device_filter\n!\n"
           "selector_group_filter\n!\n"
           "selector_proto_filter\n!\n"
           "selector_lines_per_page\n!0\n"
           "target_password\nASK\n"
           "target_host\nASK\n"
           "proto_dest\nASK\n"
           "password\nASK\n"
           "reporting\n!\n"
           "auth_channel_target\n!\n"
           "accept_message\n!False\n"
           "display_message\n!False\n"
           "real_target_device\n!\n"
        // Time: 10043
           "\x00\x00\x00\x0E"
            "keepalive\nASK\n"
    ;

//    printf("len=%x\n",
//        (unsigned)strlen(
//        "keepalive\nASK\n"
//        "password\ntotopass\n"
//        "target_device\nwin\n"
//        "target_login\nuser\n"
//        "target_password\nwhoknows\n"
//        "proto_dest\nRDP\n"
//        "authenticated\nTrue\n"
//        ));

//    exit(0);

    char indata[] =
        "\x00\x00\x00\x8F"
        "login\n!toto\n"
        "password\n!totopass\n"
        "target_device\n!win\n"
        "target_login\n!user\n"
        "target_password\n!whoknows\n"
        "proto_dest\n!RDP\n"
        "module\n!RDP\n"
        "authenticated\n!True\n"

    ;

    TestTransport acl_trans(indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);
    ActivityAlwaysTrue activity_checker;
    SessionManager sesman(ini, activity_checker, acl_trans, 10010);
    signal = BACK_EVENT_NEXT;

    // Ask next_module, send inital data to ACL
    sesman.check(mm, 10011, signal, front_signal);
    // Receive answer, OK to connect
    sesman.receive();
    // instanciate new mod, start keepalive (proxy ASK keepalive and should receive result in less than keepalive_grace_delay)
    sesman.check(mm, 10012, signal, front_signal);
    sesman.check(mm, 10042, signal, front_signal);
    // Send keepalive=ASK
    sesman.check(mm, 10043, signal, front_signal);
    sesman.check(mm, 10072, signal, front_signal);
    // still connected
    BOOST_CHECK_EQUAL(mm.last_module, false);
    // If no keepalive is received after 30 seconds => disconnection
    sesman.check(mm, 10073, signal, front_signal);
    BOOST_CHECK_EQUAL(mm.last_module, true);
}



BOOST_AUTO_TEST_CASE(TestAuthentifierKeepalive)
{

    BackEvent_t signal       = BACK_EVENT_NONE;
    BackEvent_t front_signal = BACK_EVENT_NONE;

    Inifile ini;

    ini.set<cfg::globals::keepalive_grace_delay>(cfg::globals::keepalive_grace_delay::type{30});
    ini.set<cfg::globals::session_timeout>(cfg::globals::session_timeout::type{900});
    ini.set<cfg::debug::auth>(255);

    MMIni mm(ini);

    char outdata[] =
        // Time 10011
           "\x00\x00\x01\xA4"
           "login\nASK\n"
           "ip_client\n!\n"
           "ip_target\n!\n"
           "target_device\nASK\n"
           "target_login\nASK\n"
           "session_log_redirection\n!False\n"
           "bpp\n!24\n"
           "height\n!600\n"
           "width\n!800\n"
           "selector_current_page\n!1\n"
           "selector_device_filter\n!\n"
           "selector_group_filter\n!\n"
           "selector_proto_filter\n!\n"
           "selector_lines_per_page\n!0\n"
           "target_password\nASK\n"
           "target_host\nASK\n"
           "proto_dest\nASK\n"
           "password\nASK\n"
           "reporting\n!\n"
           "auth_channel_target\n!\n"
           "accept_message\n!False\n"
           "display_message\n!False\n"
           "real_target_device\n!\n"

        // Time 10043
           "\x00\x00\x00\x0E"
            "keepalive\nASK\n"

           "\x00\x00\x00\x0E"
            "keepalive\nASK\n"
    ;

//    printf("len=%x\n",
//        (unsigned)strlen(
//        "keepalive\nASK\n"
//        "password\ntotopass\n"
//        "target_device\nwin\n"
//        "target_login\nuser\n"
//        "target_password\nwhoknows\n"
//        "proto_dest\nRDP\n"
//        "authenticated\nTrue\n"
//        ));

//    exit(0);

    char indata[] =
        "\x00\x00\x00\x8F"
        "login\n!toto\n"
        "password\n!totopass\n"
        "target_device\n!win\n"
        "target_login\n!user\n"
        "target_password\n!whoknows\n"
        "proto_dest\n!RDP\n"
        "module\n!RDP\n"
        "authenticated\n!True\n"

        // Time 10045
        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        // Time 10072 : bad message
        "\x00\x00\x00\x10"
        "koopalive\n!True\n"

    ;

    TestTransport acl_trans(indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);
    ActivityAlwaysTrue activity_checker;
    SessionManager sesman(ini, activity_checker, acl_trans, 10010);
    signal = BACK_EVENT_NEXT;

    CountTransport keepalivetrans;
    // Ask next_module, send inital data to ACL
    sesman.check(mm, 10011, signal, front_signal);
    // Receive answer, OK to connect
    sesman.receive();
    // instanciate new mod, start keepalive (proxy ASK keepalive and should receive result in less than keepalive_grace_delay)
    sesman.check(mm, 10012, signal, front_signal);
    sesman.check(mm, 10042, signal, front_signal);
    // Send keepalive=ASK
    sesman.check(mm, 10043, signal, front_signal);

    sesman.receive();
    //  keepalive=True
    sesman.check(mm, 10045, signal, front_signal);

    // koopalive=True => unknown var...
    sesman.receive();
    sesman.check(mm, 10072, signal, front_signal);
    sesman.check(mm, 10075, signal, front_signal);
    BOOST_CHECK_EQUAL(mm.last_module, false);  // still connected

    // Renew Keepalive time:
    // Send keepalive=ASK
    sesman.check(mm, 10076, signal, front_signal);
    sesman.check(mm, 10105, signal, front_signal);
    BOOST_CHECK_EQUAL(mm.last_module, false); // still connected

    // Keep alive not received, disconnection
    sesman.check(mm, 10106, signal, front_signal);
    BOOST_CHECK_EQUAL(mm.last_module, true);  // close box
}

BOOST_AUTO_TEST_CASE(TestAuthentifierInactivity)
{

    BackEvent_t signal       = BACK_EVENT_NONE;
    BackEvent_t front_signal = BACK_EVENT_NONE;

    Inifile ini;
    ini.set<cfg::globals::keepalive_grace_delay>(cfg::globals::keepalive_grace_delay::type{30});
    ini.set<cfg::globals::session_timeout>(cfg::globals::session_timeout::type{240}); // = 8*30 = 240secs inactivity>
    ini.set<cfg::debug::auth>(255);
    MMIni mm(ini);

    char outdata[] =
        // Time 10011
        "\x00\x00\x01\xA4"
        "login\nASK\n"
        "ip_client\n!\n"
        "ip_target\n!\n"
        "target_device\nASK\n"
        "target_login\nASK\n"
        "session_log_redirection\n!False\n"
        "bpp\n!24\n"
        "height\n!600\n"
        "width\n!800\n"
        "selector_current_page\n!1\n"
        "selector_device_filter\n!\n"
        "selector_group_filter\n!\n"
        "selector_proto_filter\n!\n"
        "selector_lines_per_page\n!0\n"
        "target_password\nASK\n"
        "target_host\nASK\n"
        "proto_dest\nASK\n"
        "password\nASK\n"
        "reporting\n!\n"
        "auth_channel_target\n!\n"
        "accept_message\n!False\n"
        "display_message\n!False\n"
        "real_target_device\n!\n"
    ;

//    printf("len=%x\n",
//        (unsigned)strlen(
//        "keepalive\nASK\n"
//        "password\ntotopass\n"
//        "target_device\nwin\n"
//        "target_login\nuser\n"
//        "target_password\nwhoknows\n"
//        "proto_dest\nRDP\n"
//        "authenticated\nTrue\n"
//        ));

//    exit(0);

    char indata[] =
        "\x00\x00\x00\x83"
        "login\n!toto\n"
        "password\n!totopass\n"
        "target_device\n!win\n"
        "target_login\n!user\n"
        "target_password\n!whoknows\n"
        "proto_dest\n!RDP\n"
        "authenticated\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"
    ;

    TestTransport acl_trans(indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);
    CountTransport keepalivetrans;
    ActivityAlwaysFalse activity_checker;
    SessionManager sesman(ini, activity_checker, acl_trans, 10010);
    signal = BACK_EVENT_NEXT;


    // Ask next_module, send inital data to ACL
    sesman.check(mm, 10011, signal, front_signal);
    // Receive answer, OK to connect
    sesman.receive();
    // instanciate new mod, start keepalive (proxy ASK keepalive and should receive result in less than keepalive_grace_delay)
    sesman.check(mm, 10012, signal, front_signal);
    sesman.check(mm, 10042, signal, front_signal);
    // Send keepalive=ASK
    sesman.check(mm, 10043, signal, front_signal);

    sesman.receive();
    //  keepalive=True
    sesman.check(mm, 10045, signal, front_signal);

    // keepalive=True
    sesman.receive();
    sesman.check(mm, 10072, signal, front_signal);
    sesman.check(mm, 10075, signal, front_signal);
    BOOST_CHECK_EQUAL(mm.last_module, false);  // still connected

    // Renew Keepalive time:
    // Send keepalive=ASK
    sesman.check(mm, 10076, signal, front_signal);
    sesman.receive();
    sesman.check(mm, 10079, signal, front_signal);
    BOOST_CHECK_EQUAL(mm.last_module, false); // still connected


    // Send keepalive=ASK
    sesman.check(mm, 10106, signal, front_signal);
    sesman.check(mm, 10135, signal, front_signal);
    BOOST_CHECK_EQUAL(mm.last_module, false); // still connected

    sesman.receive();
    sesman.check(mm, 10136, signal, front_signal);
    sesman.check(mm, 10165, signal, front_signal);

    BOOST_CHECK_EQUAL(mm.last_module, false); // still connected


    sesman.check(mm, 10166, signal, front_signal);
    sesman.receive();
    sesman.check(mm, 10195, signal, front_signal);
    BOOST_CHECK_EQUAL(mm.last_module, false); // still connected

    sesman.receive();
    sesman.check(mm, 10196, signal, front_signal);
    sesman.check(mm, 10225, signal, front_signal);
    BOOST_CHECK_EQUAL(mm.last_module, false); // still connected

    sesman.receive();
    sesman.check(mm, 10227, signal, front_signal);
    sesman.check(mm, 10255, signal, front_signal);
    BOOST_CHECK_EQUAL(mm.last_module, true); // disconnected on inactivity
}
