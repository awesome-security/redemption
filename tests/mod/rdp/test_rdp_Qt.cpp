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
   Author(s): Christophe Grosjean, Javier Caverni, Clément Moroldo
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to writing RDP orders to file and rereading them
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestRdp
#include <boost/test/auto_unit_test.hpp>
#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGNULL
//#define LOGPRINT
#include "config.hpp"
#include "socket_transport.hpp"
#include "test_transport.hpp"
#include "client_info.hpp"
#include "rdp/rdp.hpp"

#include "../src/front/front_Qt.hpp"



BOOST_AUTO_TEST_CASE(TestRDPQtDrawablePNGLike)
{
    int argc(0);
    char chartab[] = "myprog";
    char *argv[] {chartab};
    QApplication app(argc, argv);

    ClientInfo info;
    info.keylayout = 0x04C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 800;
    info.height = 600;
    info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
    snprintf(info.hostname,sizeof(info.hostname),"test");
    int verbose = 511;

    //const char * name = "RDP W2008 Target";
    const char * name = "QA\\administrateur";

    int client_sck = ip_connect("10.10.46.88", 3389, 3, 1000, verbose);
    
    Front_Qt front(info, verbose, client_sck);
    
    std::string error_message;
    SocketTransport t( name
                     , client_sck
                     , "10.10.46.88"
                     , 3389
                     , verbose
                     , &error_message
                     );

    //#include "fixtures/dump_w2008.hpp"
    //TestTransport t(name, indata, sizeof(indata), outdata, sizeof(outdata), verbose);

    if (verbose > 2){
        LOG(LOG_INFO, "--------- CREATION OF MOD ------------------------");
    }
    
    

    Inifile ini;

    ModRDPParams mod_rdp_params( "qa\\administrateur"
                               , "S3cur3!1nux"
                               , "10.10.46.88"
                               , "10.10.43.46"
                               , 2
                               , 0
                               );
    mod_rdp_params.device_id                       = "device_id";
    mod_rdp_params.enable_tls                      = false;
    mod_rdp_params.enable_nla                      = false;
    //mod_rdp_params.enable_krb                      = false;
    //mod_rdp_params.enable_clipboard                = true;
    mod_rdp_params.enable_fastpath                 = false;
    mod_rdp_params.enable_mem3blt                  = false;
    mod_rdp_params.enable_bitmap_update            = true;
    mod_rdp_params.enable_new_pointer              = false;
    //mod_rdp_params.rdp_compression                 = 0;
    //mod_rdp_params.error_message                   = nullptr;
    //mod_rdp_params.disconnect_on_logon_user_change = false;
    //mod_rdp_params.open_session_timeout            = 0;
    //mod_rdp_params.certificate_change_action       = 0;
    //mod_rdp_params.extra_orders                    = "";
    mod_rdp_params.server_redirection_support        = true;

    // To always get the same client random, in tests
    LCGRandom gen(0);
    
    mod_rdp mod_(t, front, info, ini.get_ref<cfg::mod_rdp::redir_info>(), gen, mod_rdp_params);
    mod_api * mod = &mod_;
    front.setCallback(mod);
    if (verbose > 2){
        LOG(LOG_INFO, "========= CREATION OF MOD DONE ====================\n\n");
    }
    BOOST_CHECK(t.get_status());
    BOOST_CHECK_EQUAL(mod->get_front_width(), 800);
    BOOST_CHECK_EQUAL(mod->get_front_height(), 600);

    //QTimerParent qTimerParent(&front, 100, 0, mod);
    
    
    app.exec();
    
/*
    uint32_t count = 0;
    BackEvent_t res = BACK_EVENT_NONE;

    while (res == BACK_EVENT_NONE){
        //QCoreApplication::processEvents();
        
        LOG(LOG_INFO, "===================> count = %u", count);
        if (count++ >= 38) break;
        front.gd.reInitView();
        mod->draw_event(time(nullptr));
        front.gd.flush();
    }*/
    
    
    
}
