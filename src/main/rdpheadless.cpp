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
   Copyright (C) Wallix 2010-2017
   Author(s): Clément Moroldo
*/

//#define LOGPRINT

#include "configs/config.hpp"

#include "main/rdpheadless.hpp"
#include "utils/timeval_ops.hpp"

#include "test_only/lcg_random.hpp"



// bjam debug rdpheadless && bin/gcc-4.9.2/debug/rdpheadless --user QA\\proxyuser --pwd $mdp --ip 10.10.46.88 --port 3389 --script /home/cmoroldo/Bureau/redemption/script_rdp_test.txt --show_all

// bjam debug rdpheadless && bin/gcc-4.9.2/debug/rdpheadless --user admin --pwd $mdp --ip 10.10.47.54 --port 3389 --script /home/cmoroldo/Bureau/redemption/script_rdp_test.txt --show_all


int wait_and_draw_event(int sck, mod_api &, FrontAPI &, timeval timeout);
int run_mod(mod_api &, TestClientCLI &, int sck, EventList &, bool, std::chrono::milliseconds, bool);
void print_help();

///////////////////////////////
// APPLICATION
int main(int argc, char** argv)
{
    //================================
    //         Default config
    //================================
    ClientInfo info;
    info.keylayout = 0x040C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 800;
    info.height = 600;
    info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER
                               | PERF_DISABLE_FULLWINDOWDRAG
                               | PERF_DISABLE_MENUANIMATIONS;
    info.cs_monitor.monitorCount = 1;
    //info.encryptionLevel = 1;
    int verbose(0);
    bool protocol_is_VNC = false;
    std::string userName;
    std::string ip;
    std::string userPwd;
    int port(3389);
    std::string localIP;
    int nbTry(3);
    int retryDelay(1000);
    bool quick_connection_test = true;
    bool time_set_connection_test = false;
    std::string script_file_path;
    uint32_t encryptionMethods(GCC::UserData::CSSecurity::_40BIT_ENCRYPTION_FLAG |           GCC::UserData::CSSecurity::_128BIT_ENCRYPTION_FLAG);
    std::chrono::milliseconds time_out_response(TestClientCLI::DEFAULT_MAX_TIMEOUT_MILISEC_RESPONSE);
    bool script_on(false);
    std::string out_path;
    //=========================================================



//     std::cout << "\n " <<  std::endl;
//     std::cout << "\n ====== RDPHEADLESS CLIENT ======\n";

    if (argc == 1) {
        std::cout << " Version 4.2.3" << "\n";
        print_help();
    }


    uint8_t input_connection_data_complete(0);
    for (int i = 1; i < argc; i++) {

        // TODO string_view
        std::string word(argv[i]);

        //================================
        //    MOD RDP INITIALS PARAM
        //================================
        if (       word == "--user") {
            if (i+1 < argc) {
                userName = argv[i+1];
                input_connection_data_complete |= TestClientCLI::NAME;
                i++;
            }
        } else if (word == "--pwd" || word == "--password") {
            if (i+1 < argc) {
                userPwd = argv[i+1];
                input_connection_data_complete |= TestClientCLI::PWD;
                i++;
            }
        } else if (word == "--ip") {
            if (i+1 < argc) {
                ip = argv[i+1];
                input_connection_data_complete |= TestClientCLI::IP;
                i++;
            }
        } else if (word == "--local_ip") {
            if (i+1 < argc) {
                localIP = argv[i+1];
                i++;
            }
        }
    }

    //===========================================
    //       Default mod_rdp_param config
    //===========================================
    const char * name_param(userName.c_str());
    const char * targetIP_param(ip.c_str());
    const char * pwd_param(userPwd.c_str());
    const char * local_IP_param(localIP.c_str());

    Inifile ini;
    ModRDPParams mod_rdp_params( name_param
                               , pwd_param
                               , targetIP_param
                               , local_IP_param
                               , 2
                               , ini.get<cfg::font>()
                               , ini.get<cfg::theme>()
                               , ini.get_ref<cfg::context::server_auto_reconnect_packet>()
                               , to_verbose_flags(0)
                               );
    mod_rdp_params.enable_tls = true;
    mod_rdp_params.enable_nla = true;
    mod_rdp_params.device_id = "device_id";
    std::string allow_channels = "*";
    mod_rdp_params.allow_channels                  = &allow_channels;
    //mod_rdp_params.allow_using_multiple_monitors   = true;
    mod_rdp_params.bogus_refresh_rect              = true;
    mod_rdp_params.verbose = RDPVerbose::none;                  //RDPVerbose::cliprdr;



    const char * options[] = {

        "--user", "--pwd",  "--password", "--local_ip", "--ip", // 1-5
        "--port",                                                   // 6
        "--keylayout",                                              // 7
        "--bpp",
        "--width",
        "--height",
        "--wallpaper",
        "--fullwindowdrag",
        "--menuanimations",
        "--persist",
        "--mon_count",
        "--v",
        "--version",
        "--h",
        "--help",
        "--script_help",
        "--encrpt_methds",
        "--show_user_params",
        "--show_rdp_params",
        "--show_draw",
        "--show_clpbrd",
        "--show_cursor",
        "--show_core",
        "--show_security",
        "--show_keyboard",
        "--show_files_sys",
        "--show_caps",
        "--show_all",
        "--show_in_pdu",
        "--show_out_pdu",
        "--show_pdu",
        "--show_channels",
        "--timeout",
        "--script",
        "--tls",
        "--nla",
        "--fastpath",
        "--mem3blt",
        "--new_pointer",
        "--serv_red",
        "--krb",
        "--glph_cache",
        "--sess_prb",
        "--sess_prb_lnch_mask",
        "--disable_cb_log_sys",
        "--disable_cb_log_wrm",
        "--disable_file_syslog",
        "--disable_file_wrm",
        "--sess_prb_cb_based_lnch",
        "--sess_prb_slttoal",
        "--sess_prob_oktdu",
        "--sess_prb_eds",
        "--sess_prb_custom_exe",
        "--transp_mode",
        "--ignore_auth_channel",
        "--use_client_as",
        "--disconn_oluc",
        "--cert_store",
        "--hide_name",
        "--persist_bmp_cache",
        "--cache_wait_list",
        "--persist_bmp_disk",
        "--serv_redir_supp",
        "--bogus_size",
        "--bogus_rectc",
        "--multi_mon",
        "--adj_perf_rec",
        "--outpath",
        "--vnc",
        "--VNC"
    };



    for (int i = 1; i < argc; i++) {

        int cmd(in(argv[i], options));

//         std::cout << "lol " << cmd <<  std::endl;

        switch (cmd) {
            case 0:
            default:
                std::cout << "Unknow key word: \'" << argv[i] << "\'" <<  std::endl;
                return 11;

            case 1:
            case 2:
            case 3:
            case 4:
            case 5: if (i+1 < argc) {
                        i++;
                    }
                break;

            case 6:                                         // --port
                if (i+1 < argc) {
                    port = std::stoi(argv[i+1]);
                    i++;
                }
                break;
            case 7:                                         // --keylayout
                if (i+1 < argc) {
                    info.keylayout = std::stoi(argv[i+1]);
                    i++;
                }
                break;
            case 8:                                         // --bpp
                if (i+1 < argc) {
                    info.bpp = std::stoi(argv[i+1]);
                    i++;
                }
                break;
            case 9:                                         // --width
                if (i+1 < argc) {
                    info.width = std::stoi(argv[i+1]);
                    i++;
                }
                break;
            case 10:                                        // --height
                if (i+1 < argc) {
                    info.height = std::stoi(argv[i+1]);
                    i++;
                }
                break;
            case 11:                                        // --wallpaper
                info.rdp5_performanceflags -=  PERF_DISABLE_WALLPAPER;
                break;
            case 12:                                        // --fullwindowdrag
                info.rdp5_performanceflags -=  PERF_DISABLE_FULLWINDOWDRAG;
                break;
            case 13:                                        // --menuanimations
                info.rdp5_performanceflags -=  PERF_DISABLE_MENUANIMATIONS;
                break;
            case 14:                                        // --persist
                quick_connection_test = false;
                time_set_connection_test = false;
                break;
            case 15:                                        // --mon_count
                if (i+1 < argc) {
                    info.cs_monitor.monitorCount = std::stoi(argv[i+1]);
                    i++;
                }
                break;
            case 16:                                        // --v
            case 17:                                        // --version
                std::cout << " Version 4.2.3" << "\n";
                break;
            case 18:                                        // --h
            case 19:                                        // --help
                print_help();
                break;
            case 20:                                        // --script_help
                // TODO show all script cmd
                std::cout << "script help not yet implemented." << "\n";
                break;
            case 21:                                        // --encrpt_methds
                if (i+1 < argc) {
                    encryptionMethods = std::stoi(argv[i+1]);
                    i++;
                }
                break;
            case 22:                                        // --show_user_params
                    verbose |= TestClientCLI::SHOW_USER_AND_TARGET_PARAMS;
                break;
            case 23:                                        // --show_rdp_params
                    verbose |= TestClientCLI::SHOW_MOD_RDP_PARAMS;
                break;
            case 24:                                        // --show_draw
                    verbose |= TestClientCLI::SHOW_DRAW_ORDERS_INFO;
                break;
            case 25:                                        // --show_clpbrd
                    verbose |= TestClientCLI::SHOW_CLPBRD_PDU_EXCHANGE;
                break;
            case 26:                                        // --show_cursor
                    verbose |= TestClientCLI::SHOW_CURSOR_STATE_CHANGE;
                break;
            case 27:                                        // --show_core
                    verbose |= TestClientCLI::SHOW_CORE_SERVER_INFO;
                break;
            case 28:                                        // --show_security
                    verbose |= TestClientCLI::SHOW_SECURITY_SERVER_INFO;
                break;
            case 29:                                        // --show_keyboard
                    verbose |= TestClientCLI::SHOW_KEYBOARD_EVENT;
                break;
            case 30:                                        // --show_files_sys
                    verbose |= TestClientCLI::SHOW_FILE_SYSTEM_EXCHANGE;
                break;
            case 31:                                        // --show_caps
                    verbose |= TestClientCLI::SHOW_CAPS;
                break;
            case 32:                                        // --show_all
                    verbose |=    TestClientCLI::SHOW_CURSOR_STATE_CHANGE
                                | TestClientCLI::SHOW_USER_AND_TARGET_PARAMS
                                | TestClientCLI::SHOW_MOD_RDP_PARAMS
                                | TestClientCLI::SHOW_DRAW_ORDERS_INFO
                                | TestClientCLI::SHOW_CLPBRD_PDU_EXCHANGE
                                | TestClientCLI::SHOW_CORE_SERVER_INFO
                                | TestClientCLI::SHOW_SECURITY_SERVER_INFO
                                | TestClientCLI::SHOW_KEYBOARD_EVENT
                                | TestClientCLI::SHOW_FILE_SYSTEM_EXCHANGE
                                | TestClientCLI::SHOW_CAPS
                                ;
                break;
            case 33:                                        // --show_in_pdu
                    verbose |=    TestClientCLI::SHOW_IN_PDU;
                break;
            case 34:                                        // --show_out_pdu
                    verbose |= TestClientCLI::SHOW_OUT_PDU;
                break;
            case 35:                                        // --show_pdu
                    verbose |=  TestClientCLI::SHOW_OUT_PDU
                              | TestClientCLI::SHOW_IN_PDU;
                break;
            case 36:                                        // --show_channels
                    verbose |=  TestClientCLI::SHOW_FILE_SYSTEM_EXCHANGE
                              | TestClientCLI::SHOW_CLPBRD_PDU_EXCHANGE;
                break;
            case 37:                                        // --timeout
                if (i+1 < argc) {
                    time_set_connection_test = true;
                    quick_connection_test = false;
                    long time = std::stol(argv[i+1]);
                    time_out_response = std::chrono::seconds(time);
                    i++;
                }
                break;
            case 38:                                        // --script
                if (i+1 < argc) {
                    quick_connection_test = false;
                    time_set_connection_test = false;
                    script_file_path = argv[i+1];
                    script_on = true;
                    i++;
                }
                break;
            case 39:                                        // --tls
                if (i+1 < argc) {
                    if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.enable_tls = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.enable_tls = false;
                        i++;
                    }
                }
                break;
            case 40:                                        // --nla
                if (i+1 < argc) {
                    if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.enable_nla = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.enable_nla = false;
                        i++;
                    }
                }
                break;
            case 41:                                        // --fastpath
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.enable_fastpath = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.enable_fastpath = false;
                        i++;
                    }
                }
                break;
            case 42:                                        // --mem3blt
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.enable_mem3blt = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.enable_mem3blt = false;
                        i++;
                    }
                }
                break;
            case 43:                                        // --new_pointer
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.enable_new_pointer = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.enable_new_pointer = false;
                        i++;
                    }
                }
                break;
            case 44:                                        // --serv_red
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.server_redirection_support = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.server_redirection_support = false;
                        i++;
                    }
                }
                break;
            case 45:                                        // --krb
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.enable_krb = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.enable_krb = false;
                        i++;
                    }
                }
                break;
            case 46:                                        // --glph_cache
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.enable_glyph_cache = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.enable_glyph_cache = false;
                        i++;
                    }
                }
                break;
            case 47:                                        // --sess_prb
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.enable_session_probe = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.enable_session_probe = false;
                        i++;
                    }
                }
                break;
            case 48:                                        // --sess_prb_lnch_mask
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.session_probe_enable_launch_mask = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.session_probe_enable_launch_mask = false;
                        i++;
                    }
                }
                break;
            case 49:                                        // --disable_cb_log_sys
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.disable_clipboard_log_syslog = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.disable_clipboard_log_syslog = false;
                        i++;
                    }
                }
                break;
            case 50:                                        // --disable_cb_log_wrm
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.disable_clipboard_log_wrm = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.disable_clipboard_log_wrm = false;
                        i++;
                    }
                }
                break;
            case 51:                                        // --disable_file_syslog
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.disable_file_system_log_syslog = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.disable_file_system_log_syslog = false;
                        i++;
                    }
                }
                break;
            case 52:                                        // --disable_file_wrm
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.disable_file_system_log_wrm = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.disable_file_system_log_wrm = false;
                        i++;
                    }
                }
                break;
            case 53:                                        // --sess_prb_cb_based_lnch
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.session_probe_use_clipboard_based_launcher = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.session_probe_use_clipboard_based_launcher = false;
                        i++;
                    }
                }
                break;
            case 54:                                        // --sess_prb_slttoal
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.session_probe_start_launch_timeout_timer_only_after_logon = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.session_probe_start_launch_timeout_timer_only_after_logon = false;
                        i++;
                    }
                }
                break;
            case 55:                                        // --sess_prob_oktdu
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.session_probe_on_keepalive_timeout_disconnect_user = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.session_probe_on_keepalive_timeout_disconnect_user = false;
                        i++;
                    }
                }
                break;
            case 56:                                        // --sess_prb_eds
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.session_probe_end_disconnected_session = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.session_probe_end_disconnected_session = false;
                        i++;
                    }
                }
                break;
            case 57:                                        // --sess_prb_custom_exe
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.session_probe_customize_executable_name = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.session_probe_customize_executable_name = false;
                        i++;
                    }
                }
                break;
            case 58:                                        // --transp_mode
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.enable_transparent_mode = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.enable_transparent_mode = false;
                        i++;
                    }
                }
                break;
            case 59:                                        // --ignore_auth_channel
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.ignore_auth_channel = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.ignore_auth_channel = false;
                        i++;
                    }
                }
                break;
            case 60:                                        // --use_client_as
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.use_client_provided_alternate_shell = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.use_client_provided_alternate_shell = false;
                        i++;
                    }
                }
                break;
            case 61:                                        // --disconn_oluc
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.disconnect_on_logon_user_change = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.disconnect_on_logon_user_change = false;
                        i++;
                    }
                }
                break;
            case 62:                                        // --cert_store
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.server_cert_store = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.server_cert_store = false;
                        i++;
                    }
                }
                break;
            case 63:                                        // --hide_name
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.hide_client_name = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.hide_client_name = false;
                        i++;
                    }
                }
                break;
            case 64:                                        // --persist_bmp_cache
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.enable_persistent_disk_bitmap_cache = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.enable_persistent_disk_bitmap_cache = false;
                        i++;
                    }
                }
                break;
            case 65:                                        // --cache_wait_list
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.enable_cache_waiting_list = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.enable_cache_waiting_list = false;
                        i++;
                    }
                }
                break;
            case 66:                                        // --serv_redir_supp
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.persist_bitmap_cache_on_disk = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.persist_bitmap_cache_on_disk = false;
                        i++;
                    }
                }
                break;
            case 67:                                        // --bogus_size
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.bogus_sc_net_size = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.bogus_sc_net_size = false;
                        i++;
                    }
                }
                break;
            case 68:                                        // --bogus_rectc
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.bogus_refresh_rect = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.bogus_refresh_rect = false;
                        i++;
                    }
                }
                break;
            case 69:                                        // --multi_mon
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.allow_using_multiple_monitors = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.allow_using_multiple_monitors = false;
                        i++;
                    }
                }
                break;
            case 71:                                        // --adj_perf_rec
                if (i+1 < argc) {
                     if (      0 == strcmp(argv[i+1], "on")) {
                        mod_rdp_params.adjust_performance_flags_for_recording = true;
                        i++;
                    } else if (0 == strcmp(argv[i+1], "off")) {
                        mod_rdp_params.adjust_performance_flags_for_recording = false;
                        i++;
                    }
                }
                break;
            case 72:
                if (i+1 < argc) {
                    out_path = argv[i+1];
                    i++;
                }
                break;
            case 73:                                        // --vnc
            case 74:                                        // --VNC
                protocol_is_VNC = true;
                port = 5900;
                break;
        }

    }
//     std::cout <<  "\n" <<  std::endl;




    if (verbose != 0) {

        std::cout << "\n";
        std::cout << " ================================" << "\n";
        std::cout << " ========== Log Config ==========" << "\n";
        std::cout << " ================================" << "\n";
        std::cout << " SHOW_USER_AND_TARGET_PARAMS = " << bool(verbose & TestClientCLI::SHOW_USER_AND_TARGET_PARAMS) <<  "\n";
        std::cout << " SHOW_MOD_RDP_PARAMS         = " << bool(verbose & TestClientCLI::SHOW_MOD_RDP_PARAMS) <<  "\n";
        std::cout << " SHOW_DRAW_ORDERS            = " << bool(verbose & TestClientCLI::SHOW_DRAW_ORDERS_INFO) <<  "\n";
        std::cout << " SHOW_CLPBRD_PDU_EXCHANGE    = " << bool(verbose & TestClientCLI::SHOW_CLPBRD_PDU_EXCHANGE) <<  "\n";
        std::cout << " SHOW_CURSOR_STATE_CHANGE    = " << bool(verbose & TestClientCLI::SHOW_CURSOR_STATE_CHANGE) <<  "\n";
        std::cout << " SHOW_CORE_SERVER_INFO       = " << bool(verbose & TestClientCLI::SHOW_CORE_SERVER_INFO) <<  "\n";
        std::cout << " SHOW_SECURITY_SERVER_INFO   = " << bool(verbose & TestClientCLI::SHOW_SECURITY_SERVER_INFO) <<  "\n";
        std::cout << " SHOW_KEYBOARD_EVENT         = " << bool(verbose & TestClientCLI::SHOW_KEYBOARD_EVENT) <<  "\n";
        std::cout << " SHOW_FILE_SYSTEM_EXCHANGE   = " << bool(verbose & TestClientCLI::SHOW_FILE_SYSTEM_EXCHANGE) <<  "\n";
        std::cout << " SHOW_CAPS                   = " << bool(verbose & TestClientCLI::SHOW_CAPS) <<  "\n";
        std::cout << " SHOW_OUT_PDU                = " << bool(verbose & TestClientCLI::SHOW_OUT_PDU) <<  "\n";
        std::cout << " SHOW_IN_PDU                 = " << bool(verbose & TestClientCLI::SHOW_IN_PDU) <<  "\n";
        std::cout <<  std::endl;
    }


    if (verbose & TestClientCLI::SHOW_USER_AND_TARGET_PARAMS) {

        std::cout << " ================================" << "\n";
        std::cout << " == User And Target Parameters ==" << "\n";
        std::cout << " ================================" << "\n";

        std::cout << " user_name= \"" << userName << "\"" <<  "\n";
        std::cout << " user_password= \"" << userPwd << "\"" << "\n";
        std::cout << " ip= \"" << ip << "\"" << "\n";
        std::cout << " port=" << port << "\n";
        std::cout << " ip_local= \"" << localIP << "\"" << "\n";
        std::cout << " keylayout=0x" << std::hex << info.keylayout << std::dec << "\n";
        std::cout << " bpp=" << info.bpp << "\n";
        std::cout << " width=" << info.width << "\n";
        std::cout << " height=" << info.height << "\n";
        std::cout << " wallpaper_on=" << bool(info.rdp5_performanceflags & PERF_DISABLE_WALLPAPER) << "\n";
        std::cout << " full_window_drag_on=" << bool(info.rdp5_performanceflags & PERF_DISABLE_FULLWINDOWDRAG) << "\n";
        std::cout << " menu_animations_on=" << bool(info.rdp5_performanceflags & PERF_DISABLE_MENUANIMATIONS) << "\n";
        std::cout <<  std::endl;
    }




    if (verbose & TestClientCLI::SHOW_MOD_RDP_PARAMS) {
        std::cout << " ================================" << "\n";
        std::cout << " ======= ModRDP Parameters ======" << "\n";
        std::cout << " ================================" << "\n";

        std::cout << " enable_tls = " << mod_rdp_params.enable_tls << "\n";
        std::cout << " enable_nla = " << mod_rdp_params.enable_nla << "\n";
        std::cout << " enable_fastpath = " << mod_rdp_params.enable_fastpath << "\n";
        std::cout << " enable_mem3blt = " << mod_rdp_params.enable_mem3blt << "\n";
        std::cout << " enable_new_pointer = " << mod_rdp_params.enable_new_pointer << "\n";
        std::cout << " server_redirection_support = " << mod_rdp_params.server_redirection_support << "\n";
        std::cout << " enable_krb = " << mod_rdp_params.enable_krb << "\n";
        std::cout << " enable_glyph_cache = " << mod_rdp_params.enable_glyph_cache << "\n";
        std::cout << " enable_session_probe = " << mod_rdp_params.enable_session_probe << "\n";
        std::cout << " session_probe_enable_launch_mask = t" << mod_rdp_params.session_probe_enable_launch_mask << "\n";
        std::cout << " disable_clipboard_log_syslog = " << mod_rdp_params.disable_clipboard_log_syslog << "\n";
        std::cout << " disable_clipboard_log_wrm = " << mod_rdp_params.disable_clipboard_log_wrm << "\n";
        std::cout << " disable_file_system_log_syslog = " << mod_rdp_params.disable_file_system_log_syslog << "\n";
        std::cout << " disable_file_system_log_wrm = " << mod_rdp_params.disable_file_system_log_wrm << "\n";
        std::cout << " session_probe_use_clipboard_based_launcher = " << mod_rdp_params.session_probe_use_clipboard_based_launcher << "\n";
        std::cout << " session_probe_start_launch_timeout_timer_only_after_logon = " << mod_rdp_params.session_probe_start_launch_timeout_timer_only_after_logon << "\n";
        std::cout << " session_probe_on_keepalive_timeout_disconnect_user = " << mod_rdp_params.session_probe_on_keepalive_timeout_disconnect_user << "\n";
        std::cout << " session_probe_end_disconnected_session = " << mod_rdp_params.session_probe_end_disconnected_session << "\n";
        std::cout << " session_probe_customize_executable_name = " << mod_rdp_params.session_probe_customize_executable_name << "\n";
        std::cout << " enable_transparent_mode = " << mod_rdp_params.enable_transparent_mode << "\n";
        std::cout << " ignore_auth_channel = " << mod_rdp_params.ignore_auth_channel << "\n";
        std::cout << " use_client_provided_alternate_shell = " << mod_rdp_params.use_client_provided_alternate_shell << "\n";
        std::cout << " disconnect_on_logon_user_change = " << mod_rdp_params.disconnect_on_logon_user_change << "\n";
        std::cout << " server_cert_store = " << mod_rdp_params.server_cert_store << "\n";
        std::cout << " hide_client_name = " << mod_rdp_params.hide_client_name << "\n";
        std::cout << " enable_persistent_disk_bitmap_cache = t" << mod_rdp_params.enable_persistent_disk_bitmap_cache << "\n";
        std::cout << " enable_cache_waiting_list = " << mod_rdp_params.enable_cache_waiting_list << "\n";
        std::cout << " persist_bitmap_cache_on_disk = " << mod_rdp_params.persist_bitmap_cache_on_disk << "\n";
        std::cout << " bogus_sc_net_size = " << mod_rdp_params.bogus_sc_net_size << "\n";
        std::cout << " bogus_refresh_rect = " << mod_rdp_params.bogus_refresh_rect << "\n";
        std::cout << " allow_using_multiple_monitors = " << mod_rdp_params.allow_using_multiple_monitors << "\n";
        std::cout << " adjust_performance_flags_for_recording = " << mod_rdp_params.adjust_performance_flags_for_recording << "\n";
        std::cout << "\n" << std::endl;
    } //======================================================================

    NullAuthentifier authentifier;
    NullReportMessage report_message;
    TestClientCLI front(info, report_message, verbose);
    front.out_path = out_path;
    int main_return = 40;

    if (input_connection_data_complete & TestClientCLI::IP) {
        // std::cout << " ================================" << "\n";
        // std::cout << " ======= Connection steps =======" << "\n";
        // std::cout << " ================================" << "\n";

        // Exception handler (pretty message)
        static std::terminate_handler old_terminate_handler =
        std::set_terminate([]{
            auto eptr = std::current_exception();
            try {
                if (eptr) {
                    std::rethrow_exception(eptr);
                }
            } catch(const Error& e) {
                std::cerr << e.errmsg() << "\n";
            } catch(...) {
            }
            old_terminate_handler();
        });

        // Signal handler (SIGPIPE)
        {
            struct sigaction sa;
            sa.sa_flags = 0;
            sigaddset(&sa.sa_mask, SIGPIPE);
            sa.sa_handler = [](int sig){
                std::cout << "got SIGPIPE(" << sig << ") : ignoring\n";
            };
            REDEMPTION_DIAGNOSTIC_PUSH
            REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast")
            REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wzero-as-null-pointer-constant")
            sigaction(SIGPIPE, &sa, nullptr);
            REDEMPTION_DIAGNOSTIC_POP
        }


        int const sck = ip_connect(ip.c_str(), port, nbTry, retryDelay);
        if (sck <= 0) {
            std::cerr << "ip_connect: Cannot connect to [" << ip << "]." << std::endl;
            return 1;
        }

        unique_fd auto_close_sck{sck};

        std::string error_message;
        SocketTransport socket(
            userName.c_str()
          , sck
          , ip.c_str()
          , port
          , to_verbose_flags(verbose)
          , &error_message
        );

        std::cout << " Connected to [" << ip <<  "]." << std::endl;

        LCGRandom gen(0); // To always get the same client random, in tests
        TimeSystem timeSystem;

        // for VNC
        NullReportMessage reportMessage;
        Theme      theme;

        front.connection_time = tvtime();
        struct : NullReportMessage {
            void report(const char* reason, const char* /*message*/) override
            {
                // std::cout << "report_message: " << message << "  reason:" << reason << std::endl;
                if (!strcmp(reason, "CLOSE_SESSION_SUCCESSFUL")) {
                    this->is_closed = true;
                }
            }

            bool is_closed = false;
        } report_message;

        mod_api * mod;
        GCC::UserData::SCCore const original_sc_core;
        GCC::UserData::SCSecurity const original_sc_sec1;

        non_null_ptr<GCC::UserData::SCCore const> sc_core_ptr = &original_sc_core;
        non_null_ptr<GCC::UserData::SCSecurity const> sc_sec1_ptr = &original_sc_sec1;

        if (protocol_is_VNC) {

            mod = new mod_vnc( socket
                            , userName.c_str()
                            , userPwd.c_str()
                            , front
                            , info.width
                            , info.height
                            , ini.get<cfg::font>()
                            , ""
                            , ""
                            , theme
                            , info.keylayout
                            , 0
                            , true
                            , true
                            , "0,1,-239"
                            , false
                            , true
                            , mod_vnc::ClipboardEncodingType::UTF8
                            , VncBogusClipboardInfiniteLoop::delayed
                            , reportMessage
                            , false
                            , to_verbose_flags(verbose));

        } else {
            auto * rdp = new mod_rdp (
                        socket
                        , front
                        , info
                        , ini.get_ref<cfg::mod_rdp::redir_info>()
                        , gen
                        , timeSystem
                        , mod_rdp_params
                        , authentifier
                        , report_message
                        , ini
                        );
            mod = rdp;

            GCC::UserData::CSSecurity & cs_security = rdp->cs_security;
            cs_security.encryptionMethods = encryptionMethods;

            sc_core_ptr = &rdp->sc_core;
            sc_sec1_ptr = &rdp->sc_sec1;
        }

        front._to_server_sender._callback = mod;
        front._callback = mod;

        try {
            while (!mod->is_up_and_running()) {
                // std::cout << " Early negociations...\n";
                if (int err = wait_and_draw_event(sck, *mod, front, {3, 0})) {
                    return err;
                }
            }
        } catch (const Error & e) {
            std::cout << " Error: Failed during RDP early negociations step. " << e.errmsg() << "\n";
            if (error_message.size()) {
                std::cout << " Error tls: " << error_message << "\n";
            }
            return 2;
        }
        std::cout << " Early negociations completes.\n";


        if (verbose & TestClientCLI::SHOW_CORE_SERVER_INFO && !protocol_is_VNC) {
            std::cout << " ================================" << "\n";
            std::cout << " ======= Server Core Info =======" << "\n";
            std::cout << " ================================" << "\n";

            std::cout << " userDataType = " << sc_core_ptr->userDataType << "\n";
            std::cout << " length = " << sc_core_ptr->length << "\n";
            std::cout << " version = " << sc_core_ptr->version << "\n";
            std::cout << " clientRequestedProtocols = " << sc_core_ptr->clientRequestedProtocols << "\n";
            std::cout << " earlyCapabilityFlags = " << sc_core_ptr->earlyCapabilityFlags << "\n";
            std::cout << std::endl;
        }

        if (verbose & TestClientCLI::SHOW_SECURITY_SERVER_INFO && !protocol_is_VNC) {
            std::cout << " ================================" << "\n";
            std::cout << " ===== Server Security Info =====" << "\n";
            std::cout << " ================================" << "\n";

            std::cout << " userDataType = " << sc_sec1_ptr->userDataType << "\n";
            std::cout << " length = " << sc_sec1_ptr->length << "\n";
            std::cout << " encryptionMethod = " << GCC::UserData::SCSecurity::get_encryptionMethod_name(sc_sec1_ptr->encryptionMethod) << "\n";
            std::cout << " encryptionLevel = " << GCC::UserData::SCSecurity::get_encryptionLevel_name(sc_sec1_ptr->encryptionLevel) << "\n";
            std::cout << " serverRandomLen = " << sc_sec1_ptr->serverRandomLen << "\n";
            std::cout << " serverCertLen = " << sc_sec1_ptr->serverCertLen << "\n";
            std::cout << " dwVersion = " << sc_sec1_ptr->dwVersion << "\n";
            std::cout << " temporary = " << sc_sec1_ptr->temporary << "\n";

            auto print_hex_data = [&sc_sec1_ptr](array_view_const_u8 av){
                for (size_t i = 0; i < av.size(); i++) {
                    if ((i % 16) == 0 && i != 0) {
                        std::cout << "\n                ";
                    }
                    std::cout <<"0x";
                    if (av[i] < 0x10) {
                        std::cout << "0";
                    }
                    std::cout << std::hex << int(sc_sec1_ptr->serverRandom[i]) << std::dec << " ";
                }
                std::cout << "\n";
                std::cout << "\n";
            };

            std::cout << " serverRandom : "; print_hex_data(sc_sec1_ptr->serverRandom);
            std::cout << " pri_exp : "; print_hex_data(sc_sec1_ptr->pri_exp);
            std::cout << " pub_sig : "; print_hex_data(sc_sec1_ptr->pub_sig);

            std::cout << " proprietaryCertificate : " << "\n";
            std::cout << "     dwSigAlgId = " << sc_sec1_ptr->proprietaryCertificate.dwSigAlgId << "\n";
            std::cout << "     dwKeyAlgId = " << sc_sec1_ptr->proprietaryCertificate.dwKeyAlgId << "\n";
            std::cout << "     wPublicKeyBlobType = " << sc_sec1_ptr->proprietaryCertificate.wPublicKeyBlobType << "\n";
            std::cout << "     wPublicKeyBlobLen = " << sc_sec1_ptr->proprietaryCertificate.wPublicKeyBlobLen << "\n";
            std::cout << "\n";
            std::cout << "     RSAPK : " << "\n";
            std::cout << "        magic = " << sc_sec1_ptr->proprietaryCertificate.RSAPK.magic << "\n";
            std::cout << "\n" << std::endl;

        }


        //===========================================
        //             Scripted Events
        //===========================================
        EventList eventList;
        if (script_on) {
            std::ifstream ifichier(script_file_path);
            if(ifichier) {
                std::string ligne;
                std::string delimiter = " ";

                while(std::getline(ifichier, ligne)) {
                    auto pos(ligne.find(delimiter));
                    std::string tag  = ligne.substr(0, pos);
                    std::string info = ligne.substr(pos + delimiter.length(), ligne.length());

                    if (       tag == "wait") {
                        eventList.wait(std::stoi(info));

                    } else if (tag == "key_press") {
                        pos = info.find(delimiter);
                        uint32_t scanCode(std::stoi(info.substr(0, pos)));
                        uint32_t flag(std::stoi(info.substr(pos + delimiter.length(), info.length())));

                        eventList.setKey_press(&front, scanCode, flag);

                    } else if (tag == "key_release") {
                        pos = info.find(delimiter);
                        uint32_t scanCode(std::stoi(info.substr(0, pos)));
                        uint32_t flag(std::stoi(info.substr(pos + delimiter.length(), info.length())));

                        eventList.setKey_release(&front, scanCode, flag);

                    } else if (tag == "mouse_press") {
                        pos = info.find(delimiter);
                        uint8_t button(std::stoi(info.substr(0, pos)));
                        info = info.substr(pos + delimiter.length(), info.length());
                        pos = info.find(delimiter);
                        uint32_t x(std::stoi(info.substr(0, pos)));
                        uint32_t y(std::stoi(info.substr(pos + delimiter.length(), info.length())));

                        eventList.setMouse_button(&front, button, x, y, true);

                    } else if (tag == "mouse_release") {
                        pos = info.find(delimiter);
                        uint8_t button(std::stoi(info.substr(0, pos)));
                        info = info.substr(pos + delimiter.length(), info.length());
                        pos = info.find(delimiter);
                        uint32_t x(std::stoi(info.substr(0, pos)));
                        uint32_t y(std::stoi(info.substr(pos + delimiter.length(), info.length())));

                        eventList.setMouse_button(&front, button, x, y, false);

                    } else if (tag == "clpbrd_change") {
                        // TODO dynamique data and format injection
                        uint32_t formatIDs                 = RDPECLIP::CF_TEXT;
                        std::string formatListDataLongName("\0\0", 2);

                        // TODO { formatListDataLongName, 1 } -> array_view
                        // TODO { formatIDs, 1 } -> array_view
                        eventList.setClpbrd_change(&front, &formatIDs, &formatListDataLongName, 1);

                    } else if (tag == "click") {
                        pos = info.find(delimiter);
                        uint8_t button(std::stoi(info.substr(0, pos)));
                        info = info.substr(pos + delimiter.length(), info.length());
                        pos = info.find(delimiter);
                        uint32_t x(std::stoi(info.substr(0, pos)));
                        uint32_t y(std::stoi(info.substr(pos + delimiter.length(), info.length())));

                        eventList.setClick(&front, button, x, y);

                    } else if (tag == "double_click") {
                        pos = info.find(delimiter);
                        uint32_t x(std::stoi(info.substr(0, pos)));
                        uint32_t y(std::stoi(info.substr(pos + delimiter.length(), info.length())));

                        eventList.setDouble_click(&front, x, y);

                    } else if (tag == "key") {
                        pos = info.find(delimiter);
                        uint32_t scanCode(std::stoi(info.substr(0, pos)));
                        uint32_t flag(std::stoi(info.substr(pos + delimiter.length(), info.length())));

                        eventList.setKey(&front, scanCode, flag);

                    } else if (tag == "loop") {
                        pos = info.find(delimiter);
                        uint32_t jump_size(std::stoi(info.substr(0, pos)));
                        uint32_t count_steps(std::stoi(info.substr(pos + delimiter.length(), info.length())));

                        eventList.setLoop(jump_size, count_steps);
                    }
                }
            } else {
                std::cerr <<  "Can't find " << script_file_path << "\n";
            }
        }

        if ((input_connection_data_complete & TestClientCLI::LOG_COMPLETE) || quick_connection_test) {
            try {
                main_return = run_mod(*mod, front, sck, eventList, quick_connection_test, time_out_response, time_set_connection_test);
                // std::cout << "RDP Headless end." <<  std::endl;
            }
            catch (Error const & e)
            {
                if (report_message.is_closed) {
                    main_return = 0;
                }
                else {
                    std::cerr << e.errmsg() << std::endl;
                }
            }
        }

        front.disconnect();
        if (!report_message.is_closed) {
            mod->disconnect(tvtime().tv_sec);
        }
    }

    return main_return;
}



void print_help() {
    std::cout << "\n";

    std::cout << " COMMAND LINE HELPER:" << "\n\n";
    std::cout << "  ========= TOOLS =========" << "\n";
    std::cout << "  -h or --help              Show help" << "\n";
    std::cout << "  -v or --version           Show version" <<  "\n";
    std::cout << "  --script_help             Show all script event commands" << "\n";
    std::cout << "  --show_user_params        Show user info parameters" << "\n";
    std::cout << "  --show_rdp_params         Show mod rdp parameters" << "\n";
    std::cout << "  --show_draw               Show draw orders info" << "\n";
    std::cout << "  --show_clpbrd             Show clipboard echange PDU info" << "\n";
    std::cout << "  --show_cursor             Show cursor change" << "\n";
    std::cout << "  --show_all                Show all log info, except PDU content" << "\n";
    std::cout << "  --show_core               Show core server info" << "\n";
    std::cout << "  --show_security           Show scurity server info" << "\n";
    std::cout << "  --show_keyboard           Show keyboard event" << "\n";
    std::cout << "  --show_files_sys          Show files sytem exchange info" << "\n";
    std::cout << "  --show_channels           Show all channels exchange info" << "\n";
    std::cout << "  --show_in_pdu             Show received PDU content from shown channels" << "\n";
    std::cout << "  --show_out_pdu            Show sent PDU content from shown channels" << "\n";
    std::cout << "  --show_pdu                Show both sent and received PDU content from shown channels" << "\n";
    std::cout << "  --show_caps               Show capabilities PDU exchange" <<  "\n";
    std::cout << "  --script [file_path]      Set a test PDU file script" << "\n";
    std::cout << "  --persist                 Set connection to persist" << "\n";
    std::cout << "  --timeout [time]          Set timeout response before to disconnect in milisecond" << "\n";
    std::cout << "\n";
    std::cout << "  ========= USER =========" << "\n";
    std::cout << "  --user [user_name]        Set session user name" << "\n";
    std::cout << "  --pwd [user_password]     Set sessoion user password" << "\n";
    std::cout << "  --ip [ip]                 Set target IP" << "\n";
    std::cout << "  --port [port]             Set target port" << "\n";
    std::cout << "  --local_ip [local_ip]     Set local IP" << "\n";
    std::cout << "  --mon_count [number]      Set the number of monitor" <<  "\n";
    std::cout << "  --wallpaper [on/off]      Active/unactive wallpapert" << "\n";
    std::cout << "  --fullwindowdrag [on/off] Active/unactive full window drag" << "\n";
    std::cout << "  --menuanimations [on/off] Active/unactive menu animations" << "\n";
    std::cout << "  --keylayout [keylaout_id] Set decimal keylouat window id" << "\n";
    std::cout << "  --bpp [bpp_value]         Set bit per pixel value" << "\n";
    std::cout << "  --width [width_value]     Set screen width" << "\n";
    std::cout << "  --height [height_value]   Set screen height" << "\n";
    std::cout << "  --encrpt_methds           Set encryption methods as any addition of 1, 2, 8 and 16" <<  "\n";
    std::cout << "\n";
    std::cout << "  ======== CONFIG ========" << "\n";
    std::cout << "--tls [on/off]                     Active/unactive tls" <<  "\n";
    std::cout << "--nla [on/off]                     Active/unactive nla" <<  "\n";
    std::cout << "--fastpath [on/off]                Active/unactive fastpath" <<  "\n";
    std::cout << "--mem3blt [on/off]                 Active/unactive mem3blt" <<  "\n";
    std::cout << "--new_pointer [on/off]             Active/unactive new pointer" <<  "\n";
    std::cout << "--serv_red [on/off]                Active/unactive server redirection support" <<  "\n";
    std::cout << "--krb [on/off]                     Active/unactive krb" <<  "\n";
    std::cout << "--glph_cache [on/off]              Active/unactive glyph cache" <<  "\n";
    std::cout << "--sess_prb [on/off]                Active/unactive session probe" <<  "\n";
    std::cout << "--sess_prb_lnch_mask [on/off]      Active/unactive session probe launch mask" <<  "\n";
    std::cout << "--disable_cb_log_sys [on/off]      Active/unactive clipboard log syslog lock" <<  "\n";
    std::cout << "--disable_cb_log_wrm [on/off]      Active/unactive clipboard log wrm lock" <<  "\n";
    std::cout << "--disable_file_syslog [on/off]     Active/unactive file system log syslog lock" <<  "\n";
    std::cout << "--disable_file_wrm [on/off]        Active/unactive file system log wrm lock" <<  "\n";
    std::cout << "--sess_prb_cb_based_lnch [on/off]  Active/unactive session probe use clipboard based launcher" <<  "\n";
    std::cout << "--sess_prb_slttoal [on/off]        Active/unactive session probe start launch timeout timer only after logon" <<  "\n";
    std::cout << "--sess_prob_oktdu [on/off]         Active/unactive session probe on keepalive timeout disconnect user" <<  "\n";
    std::cout << "--sess_prb_eds [on/off]            Active/unactive session probe end disconnected session" <<  "\n";
    std::cout << "--sess_prb_custom_exe [on/off]     Active/unactive session probe customize executable name" <<  "\n";
    std::cout << "--transp_mode [on/off]             Active/unactive enable transparent mode" <<  "\n";
    std::cout << "--ignore_auth_channel [on/off]     Active/unactive ignore auth channel" <<  "\n";
    std::cout << "--use_client_as [on/off]           Active/unactive use client provided alternate shell" <<  "\n";
    std::cout << "--disconn_oluc [on/off]            Active/unactive disconnect on logon user change" <<  "\n";
    std::cout << "--cert_store [on/off]              Active/unactive enable server certifications store" <<  "\n";
    std::cout << "--hide_name [on/off]               Active/unactive hide client name" <<  "\n";
    std::cout << "--persist_bmp_cache [on/off]       Active/unactive enable persistent disk bitmap cache" <<  "\n";
    std::cout << "--cache_wait_list [on/off]         Active/unactive enable_cache_waiting_list" <<  "\n";
    std::cout << "--persist_bmp_disk  [on/off]       Active/unactive persist bitmap cache on disk" <<  "\n";
    std::cout << "--serv_redir_supp [on/off]         Active/unactive server redirection support" <<  "\n";
    std::cout << "--bogus_size [on/off]              Active/unactive bogus sc net size" <<  "\n";
    std::cout << "--bogus_rectc [on/off]             Active/unactive bogus refresh rect" <<  "\n";
    std::cout << "--multi_mon [on/off]               Active/unactive allow using multiple monitors" <<  "\n";
    std::cout << "--adj_perf_rec [on/off]            Active/unactive adjust performance flags for recording" <<  "\n";
    std::cout << "--outpath [outpath]                Set path where connection time will be written" <<  "\n";

    std::cout << std::endl;
}



int run_mod(mod_api & mod, TestClientCLI & front, int sck, EventList & /*al*/, bool quick_connection_test, std::chrono::milliseconds time_out_response, bool time_set_connection_test) {
    const timeval time_stop = addusectimeval(time_out_response, tvtime());
    const timeval time_mark = { 0, 50000 };

    while (front.is_pipe_ok)
    {
        if (mod.logged_on == mod_api::CLIENT_LOGGED) {
            mod.logged_on = mod_api::CLIENT_UNLOGGED;

            std::cout << " RDP Session Log On." << std::endl;
            if (quick_connection_test) {
                return 0;
            }
            break;
        }

        if (time_set_connection_test) {
            if (time_stop > tvtime()) {
                //std::cerr <<  " Exit timeout (timeout = " << time_out_response.tv_sec << " sec " <<  time_out_response.tv_usec << " µsec)" << std::endl;
                return 8;
            }
        }

        if (int err = wait_and_draw_event(sck, mod, front, time_mark)) {
            return err;
        }

        // if (front.is_running()) {
        //     al.emit();
        // }
    }

    return 0;
}


int wait_and_draw_event(int sck, mod_api & mod, FrontAPI & front, timeval timeout)
{
    unsigned max = 0;
    fd_set   rfds;

    io_fd_zero(rfds);

    auto & event = mod.get_event();
    event.wait_on_fd(sck, rfds, max, timeout);

    int num = select(max + 1, &rfds, nullptr, nullptr, &timeout);
    // std::cout << "RDP CLIENT :: select num = " <<  num << "\n";

    if (num < 0) {
        if (errno == EINTR) {
            return 0;
            //continue;
        }

        std::cerr << "RDP CLIENT :: errno = " <<  errno << "\n";
        return 9;
    }

    if (event.is_set(sck, rfds)) {
        mod.draw_event(time(nullptr), front);
    }

    return 0;
}