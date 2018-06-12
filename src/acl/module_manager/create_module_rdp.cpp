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
  Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
             Raphael Zhou, Meng Tan

  Manage Modules Life cycle : creation, destruction and chaining
  find out the next module to run from context reading
*/

#include "configs/config.hpp"
#include "core/client_info.hpp"
#include "core/front_api.hpp"
#include "core/report_message_api.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "utils/authorization_channels.hpp"
#include "utils/sugar/scope_exit.hpp"
#include "mod/rdp/rdp_params.hpp"
#include "mod/rdp/rdp.hpp"
#include "keyboard/keymap2.hpp"

#include "acl/module_manager.hpp"


void ModuleManager::create_mod_rdp(
    AuthApi& authentifier, ReportMessageApi& report_message,
    Inifile& ini, FrontAPI& front, ClientInfo const& client_info_, ClientExecute& client_execute,
    Keymap2::KeyFlags key_flags)
{
    LOG(LOG_INFO, "ModuleManager::Creation of new mod 'RDP'");

    auto client_info = client_info_;

    switch (ini.get<cfg::context::mode_console>()) {
        case RdpModeConsole::force:
            client_info.console_session = true;
            LOG(LOG_INFO, "Session::mode console : force");
            break;
        case RdpModeConsole::forbid:
            client_info.console_session = false;
            LOG(LOG_INFO, "Session::mode console : forbid");
            break;
        case RdpModeConsole::allow:
            break;
    }
    //else {
    //    // default is "allow", do nothing special
    //}

    unique_fd client_sck = this->connect_to_target_host(
        report_message, trkeys::authentification_rdp_fail);

    // BEGIN READ PROXY_OPT
    if (ini.get<cfg::globals::enable_wab_integration>()) {
        AuthorizationChannels::update_authorized_channels(
            ini.get_ref<cfg::mod_rdp::allow_channels>(),
            ini.get_ref<cfg::mod_rdp::deny_channels>(),
            ini.get<cfg::context::proxy_opt>()
        );
    }
    // END READ PROXY_OPT

    ini.get_ref<cfg::context::close_box_extra_message>().clear();
    ModRDPParams mod_rdp_params(
        ini.get<cfg::globals::target_user>().c_str()
      , ini.get<cfg::context::target_password>().c_str()
      , ini.get<cfg::context::target_host>().c_str()
      , "0.0.0.0"   // client ip is silenced
      , key_flags
      , ini.get<cfg::font>()
      , ini.get<cfg::theme>()
      // TODO move to member
      , ini.get_ref<cfg::context::server_auto_reconnect_packet>()
      , ini.get_ref<cfg::context::close_box_extra_message>()
      , to_verbose_flags(ini.get<cfg::debug::mod_rdp>())
      //, RDPVerbose::basic_trace4 | RDPVerbose::basic_trace3 | RDPVerbose::basic_trace7 | RDPVerbose::basic_trace
    );

    SCOPE_EXIT(ini.set<cfg::context::perform_automatic_reconnection>(false));
    mod_rdp_params.perform_automatic_reconnection = ini.get<cfg::context::perform_automatic_reconnection>();

    mod_rdp_params.device_id                           = ini.get<cfg::globals::device_id>().c_str();

    mod_rdp_params.primary_user_id                     = ini.get<cfg::globals::primary_user_id>().c_str();
    mod_rdp_params.target_application                  = ini.get<cfg::globals::target_application>().c_str();

    //mod_rdp_params.enable_tls                          = true;
    if (!mod_rdp_params.target_password[0]) {
        mod_rdp_params.enable_nla                      = false;
    }
    else {
        mod_rdp_params.enable_nla                      = ini.get<cfg::mod_rdp::enable_nla>();
    }
    mod_rdp_params.enable_krb                          = ini.get<cfg::mod_rdp::enable_kerberos>();
    mod_rdp_params.enable_fastpath                     = ini.get<cfg::mod_rdp::fast_path>();
    //mod_rdp_params.enable_mem3blt                      = true;
    //mod_rdp_params.enable_new_pointer                  = true;
    mod_rdp_params.enable_glyph_cache                  = ini.get<cfg::globals::glyph_cache>();

    mod_rdp_params.enable_session_probe                = ini.get<cfg::mod_rdp::enable_session_probe>();
    mod_rdp_params.session_probe_enable_launch_mask    = ini.get<cfg::mod_rdp::session_probe_enable_launch_mask>();

    mod_rdp_params.session_probe_use_clipboard_based_launcher
                                                        = ini.get<cfg::mod_rdp::session_probe_use_clipboard_based_launcher>();
    mod_rdp_params.session_probe_launch_timeout        = ini.get<cfg::mod_rdp::session_probe_launch_timeout>();
    mod_rdp_params.session_probe_launch_fallback_timeout
                                                        = ini.get<cfg::mod_rdp::session_probe_launch_fallback_timeout>();
    mod_rdp_params.session_probe_start_launch_timeout_timer_only_after_logon
                                                        = ini.get<cfg::mod_rdp::session_probe_start_launch_timeout_timer_only_after_logon>();
    mod_rdp_params.session_probe_on_launch_failure     = ini.get<cfg::mod_rdp::session_probe_on_launch_failure>();
    mod_rdp_params.session_probe_keepalive_timeout     = ini.get<cfg::mod_rdp::session_probe_keepalive_timeout>();
    mod_rdp_params.session_probe_on_keepalive_timeout  =
                                                            ini.get<cfg::mod_rdp::session_probe_on_keepalive_timeout>();
    mod_rdp_params.session_probe_end_disconnected_session
                                                        = ini.get<cfg::mod_rdp::session_probe_end_disconnected_session>();
    mod_rdp_params.session_probe_customize_executable_name
                                                        = ini.get<cfg::mod_rdp::session_probe_customize_executable_name>();
    mod_rdp_params.session_probe_disconnected_application_limit =
                                                        ini.get<cfg::mod_rdp::session_probe_disconnected_application_limit>();
    mod_rdp_params.session_probe_disconnected_session_limit =
                                                        ini.get<cfg::mod_rdp::session_probe_disconnected_session_limit>();
    mod_rdp_params.session_probe_idle_session_limit    =
                                                        ini.get<cfg::mod_rdp::session_probe_idle_session_limit>();
    mod_rdp_params.session_probe_exe_or_file           = ini.get<cfg::mod_rdp::session_probe_exe_or_file>();
    mod_rdp_params.session_probe_arguments             = ini.get<cfg::mod_rdp::session_probe_arguments>();

    mod_rdp_params.session_probe_clipboard_based_launcher_clipboard_initialization_delay = ini.get<cfg::mod_rdp::session_probe_clipboard_based_launcher_clipboard_initialization_delay>();
    mod_rdp_params.session_probe_clipboard_based_launcher_start_delay                    = ini.get<cfg::mod_rdp::session_probe_clipboard_based_launcher_start_delay>();
    mod_rdp_params.session_probe_clipboard_based_launcher_long_delay                     = ini.get<cfg::mod_rdp::session_probe_clipboard_based_launcher_long_delay>();
    mod_rdp_params.session_probe_clipboard_based_launcher_short_delay                    = ini.get<cfg::mod_rdp::session_probe_clipboard_based_launcher_short_delay>();

    mod_rdp_params.disable_clipboard_log_syslog        = bool(ini.get<cfg::video::disable_clipboard_log>() & ClipboardLogFlags::syslog);
    mod_rdp_params.disable_clipboard_log_wrm           = bool(ini.get<cfg::video::disable_clipboard_log>() & ClipboardLogFlags::wrm);
    mod_rdp_params.disable_file_system_log_syslog      = bool(ini.get<cfg::video::disable_file_system_log>() & FileSystemLogFlags::syslog);
    mod_rdp_params.disable_file_system_log_wrm         = bool(ini.get<cfg::video::disable_file_system_log>() & FileSystemLogFlags::wrm);
    mod_rdp_params.session_probe_extra_system_processes               =
        ini.get<cfg::context::session_probe_extra_system_processes>().c_str();
    mod_rdp_params.session_probe_outbound_connection_monitoring_rules =
        ini.get<cfg::context::session_probe_outbound_connection_monitoring_rules>().c_str();
    mod_rdp_params.session_probe_process_monitoring_rules             =
        ini.get<cfg::context::session_probe_process_monitoring_rules>().c_str();

    mod_rdp_params.session_probe_enable_log            = ini.get<cfg::mod_rdp::session_probe_enable_log>();
    mod_rdp_params.session_probe_enable_log_rotation   = ini.get<cfg::mod_rdp::session_probe_enable_log_rotation>();

    mod_rdp_params.session_probe_allow_multiple_handshake
                                                        = ini.get<cfg::mod_rdp::session_probe_allow_multiple_handshake>();

    mod_rdp_params.session_probe_enable_crash_dump     = ini.get<cfg::mod_rdp::session_probe_enable_crash_dump>();

    mod_rdp_params.session_probe_handle_usage_limit    = ini.get<cfg::mod_rdp::session_probe_handle_usage_limit>();
    mod_rdp_params.session_probe_memory_usage_limit    = ini.get<cfg::mod_rdp::session_probe_memory_usage_limit>();

    mod_rdp_params.ignore_auth_channel                 = ini.get<cfg::mod_rdp::ignore_auth_channel>();
    mod_rdp_params.auth_channel                        = CHANNELS::ChannelNameId(ini.get<cfg::mod_rdp::auth_channel>());
    mod_rdp_params.checkout_channel                    = CHANNELS::ChannelNameId(ini.get<cfg::mod_rdp::checkout_channel>());
    mod_rdp_params.alternate_shell                     = ini.get<cfg::mod_rdp::alternate_shell>().c_str();
    mod_rdp_params.shell_arguments                     = ini.get<cfg::mod_rdp::shell_arguments>().c_str();
    mod_rdp_params.shell_working_dir                   = ini.get<cfg::mod_rdp::shell_working_directory>().c_str();
    mod_rdp_params.use_client_provided_alternate_shell = ini.get<cfg::mod_rdp::use_client_provided_alternate_shell>();
    mod_rdp_params.target_application_account          = ini.get<cfg::globals::target_application_account>().c_str();
    mod_rdp_params.target_application_password         = ini.get<cfg::globals::target_application_password>().c_str();
    mod_rdp_params.rdp_compression                     = ini.get<cfg::mod_rdp::rdp_compression>();
    mod_rdp_params.error_message                       = &ini.get_ref<cfg::context::auth_error_message>();
    mod_rdp_params.disconnect_on_logon_user_change     = ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>();
    mod_rdp_params.open_session_timeout                = ini.get<cfg::mod_rdp::open_session_timeout>();

    mod_rdp_params.server_cert_store                   = ini.get<cfg::mod_rdp::server_cert_store>();
    mod_rdp_params.server_cert_check                   = ini.get<cfg::mod_rdp::server_cert_check>();
    mod_rdp_params.server_access_allowed_message       = ini.get<cfg::mod_rdp::server_access_allowed_message>();
    mod_rdp_params.server_cert_create_message          = ini.get<cfg::mod_rdp::server_cert_create_message>();
    mod_rdp_params.server_cert_success_message         = ini.get<cfg::mod_rdp::server_cert_success_message>();
    mod_rdp_params.server_cert_failure_message         = ini.get<cfg::mod_rdp::server_cert_failure_message>();
    mod_rdp_params.server_cert_error_message           = ini.get<cfg::mod_rdp::server_cert_error_message>();

    mod_rdp_params.hide_client_name                    = ini.get<cfg::mod_rdp::hide_client_name>();

    mod_rdp_params.enable_persistent_disk_bitmap_cache = ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>();
    mod_rdp_params.enable_cache_waiting_list           = ini.get<cfg::mod_rdp::cache_waiting_list>();
    mod_rdp_params.persist_bitmap_cache_on_disk        = ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>();
    mod_rdp_params.password_printing_mode              = ini.get<cfg::debug::password>();
    mod_rdp_params.cache_verbose                       = to_verbose_flags(ini.get<cfg::debug::cache>());

    mod_rdp_params.extra_orders                        = ini.get<cfg::mod_rdp::extra_orders>().c_str();

    mod_rdp_params.allow_channels                      = &(ini.get<cfg::mod_rdp::allow_channels>());
    mod_rdp_params.deny_channels                       = &(ini.get<cfg::mod_rdp::deny_channels>());

    mod_rdp_params.bogus_sc_net_size                   = ini.get<cfg::mod_rdp::bogus_sc_net_size>();
    mod_rdp_params.bogus_linux_cursor                  = ini.get<cfg::mod_rdp::bogus_linux_cursor>();
    mod_rdp_params.bogus_refresh_rect                  = ini.get<cfg::globals::bogus_refresh_rect>();

    mod_rdp_params.proxy_managed_drives                = ini.get<cfg::mod_rdp::proxy_managed_drives>().c_str();
    mod_rdp_params.proxy_managed_drive_prefix          = app_path(AppPath::DriveRedirection);

    mod_rdp_params.lang                                = language(ini);

    mod_rdp_params.allow_using_multiple_monitors       = ini.get<cfg::globals::allow_using_multiple_monitors>();

    mod_rdp_params.adjust_performance_flags_for_recording
                                                        = (ini.get<cfg::globals::is_rec>() &&
                                                            ini.get<cfg::client::auto_adjust_performance_flags>() &&
                                                            ((ini.get<cfg::video::capture_flags>() &
                                                            (CaptureFlags::wrm | CaptureFlags::ocr)) !=
                                                            CaptureFlags::none));
    mod_rdp_params.client_execute                      = &client_execute;
    mod_rdp_params.client_execute_flags                = client_execute.Flags();
    mod_rdp_params.client_execute_exe_or_file          = client_execute.ExeOrFile();
    mod_rdp_params.client_execute_working_dir          = client_execute.WorkingDir();
    mod_rdp_params.client_execute_arguments            = client_execute.Arguments();

    mod_rdp_params.should_ignore_first_client_execute  = client_execute.should_ignore_first_client_execute();

    mod_rdp_params.remote_program                      = (client_info.remote_program &&
                                                            ini.get<cfg::mod_rdp::use_native_remoteapp_capability>() &&
                                                            ((mod_rdp_params.target_application &&
                                                            (*mod_rdp_params.target_application)) ||
                                                            (ini.get<cfg::mod_rdp::use_client_provided_remoteapp>() &&
                                                            mod_rdp_params.client_execute_exe_or_file &&
                                                            (*mod_rdp_params.client_execute_exe_or_file))));
    mod_rdp_params.remote_program_enhanced             = client_info.remote_program_enhanced;
    mod_rdp_params.use_client_provided_remoteapp       = ini.get<cfg::mod_rdp::use_client_provided_remoteapp>();

    mod_rdp_params.clean_up_32_bpp_cursor              = ini.get<cfg::mod_rdp::clean_up_32_bpp_cursor>();

    mod_rdp_params.large_pointer_support               = ini.get<cfg::globals::large_pointer_support>();

    mod_rdp_params.load_balance_info                   = ini.get<cfg::mod_rdp::load_balance_info>().c_str();

    mod_rdp_params.rail_disconnect_message_delay       = ini.get<cfg::context::rail_disconnect_message_delay>();

    mod_rdp_params.use_session_probe_to_launch_remote_program
                                                        = ini.get<cfg::context::use_session_probe_to_launch_remote_program>();

    mod_rdp_params.bogus_ios_rdpdr_virtual_channel     = ini.get<cfg::mod_rdp::bogus_ios_rdpdr_virtual_channel>();

    mod_rdp_params.enable_rdpdr_data_analysis          = ini.get<cfg::mod_rdp::enable_rdpdr_data_analysis>();

    mod_rdp_params.experimental_fix_input_event_sync   = ini.get<cfg::mod_rdp::experimental_fix_input_event_sync>();

    try {
        const char * const name = "RDP Target";

        Rect const adjusted_client_execute_rect =
            client_execute.adjust_rect(get_widget_rect(
                    client_info.width,
                    client_info.height,
                    client_info.cs_monitor
                ));

        const bool host_mod_in_widget =
            (client_info.remote_program &&
                !mod_rdp_params.remote_program);

        if (host_mod_in_widget) {
            client_info.width  = adjusted_client_execute_rect.cx / 4 * 4;
            client_info.height = adjusted_client_execute_rect.cy;

            ::memset(&client_info.cs_monitor, 0, sizeof(client_info.cs_monitor));
        }
        else {
            client_execute.reset(false);
        }

        ModWithSocket<mod_rdp>* new_mod = new ModWithSocket<mod_rdp>(
            *this,
            authentifier,
            name,
            std::move(client_sck),
            ini.get<cfg::debug::mod_rdp>(),
            &ini.get_ref<cfg::context::auth_error_message>(),
            sock_mod_barrier(),
            this->session_reactor,
            front,
            client_info,
            ini.get_ref<cfg::mod_rdp::redir_info>(),
            this->gen,
            this->timeobj,
            mod_rdp_params,
            authentifier,
            report_message,
            ini
        );
        std::unique_ptr<mod_api> managed_mod(new_mod);

        if (host_mod_in_widget) {
            LOG(LOG_INFO, "ModuleManager::Creation of internal module 'RailModuleHostMod'");

            std::string target_info
              = ini.get<cfg::context::target_str>()
              + ":"
              + ini.get<cfg::globals::primary_user_id>();

            client_execute.set_target_info(target_info.c_str());

            auto* host_mod = new RailModuleHostMod(
                ini,
                this->session_reactor,
                front,
                client_info.width,
                client_info.height,
                adjusted_client_execute_rect,
                std::move(managed_mod),
                client_execute,
                client_info.cs_monitor,
                !ini.get<cfg::globals::is_rec>()
            );

            this->set_mod(host_mod, nullptr, &client_execute);
            this->rail_module_host_mod_ptr = host_mod;
            LOG(LOG_INFO, "ModuleManager::internal module 'RailModuleHostMod' ready");
        }
        else {
            rdp_api*       rdpapi = new_mod;
            windowing_api* winapi = new_mod->get_windowing_api();
            this->set_mod(managed_mod.release(), rdpapi, winapi);
        }

        /* If provided by connection policy, session timeout update */
        report_message.update_inactivity_timeout();
    }
    catch (...) {
        ArcsightLogInfo arc_info;
        arc_info.ApplicationProtocol = "rdp";
        arc_info.name = "SESSION_CREATION";
        arc_info.WallixBastionStatus = "FAIL";

        report_message.log6("type=\"SESSION_CREATION_FAILED\"", arc_info);

        throw;
    }

    if (ini.get<cfg::globals::bogus_refresh_rect>() &&
        ini.get<cfg::globals::allow_using_multiple_monitors>() &&
        (client_info.cs_monitor.monitorCount > 1)) {
        this->mod->rdp_suppress_display_updates();
        this->mod->rdp_allow_display_updates(0, 0,
            client_info.width, client_info.height);
    }
    this->mod->rdp_input_invalidate(Rect(0, 0, client_info.width, client_info.height));
    LOG(LOG_INFO, "ModuleManager::Creation of new mod 'RDP' suceeded\n");
    ini.get_ref<cfg::context::auth_error_message>().clear();
    this->connected = true;
}
