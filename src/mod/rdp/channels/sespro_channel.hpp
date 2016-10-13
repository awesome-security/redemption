/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2015
    Author(s): Christophe Grosjean, Raphael Zhou
*/


#pragma once

#include "core/front_api.hpp"
#include "mod/rdp/channels/base_channel.hpp"
#include "utils/outbound_connection_monitor_rules.hpp"
#include "utils/process_monitor_rules.hpp"
#include "utils/stream.hpp"
#include "utils/translation.hpp"
#include "core/error.hpp"
#include "mod/mod_api.hpp"

#include <chrono>
#include <memory>
#include <sstream>

class SessionProbeVirtualChannel : public BaseVirtualChannel
{
private:
    bool session_probe_ending_in_progress  = false;
    bool session_probe_keep_alive_received = true;
    bool session_probe_ready               = false;

    bool session_probe_launch_timeout_timer_started = false;

    const std::chrono::duration<unsigned, std::milli> session_probe_effective_launch_timeout;

    const std::chrono::duration<unsigned, std::milli> param_session_probe_keepalive_timeout;
    const bool     param_session_probe_on_keepalive_timeout_disconnect_user;

    const SessionProbeOnLaunchFailure param_session_probe_on_launch_failure;

    const bool     param_session_probe_end_disconnected_session;

    std::string    param_target_informations;

    const uint16_t param_front_width;
    const uint16_t param_front_height;

    const std::chrono::duration<unsigned, std::milli> param_session_probe_disconnected_application_limit;
    const std::chrono::duration<unsigned, std::milli> param_session_probe_disconnected_session_limit;
    const std::chrono::duration<unsigned, std::milli> param_session_probe_idle_session_limit;

    std::string param_real_alternate_shell;
    std::string param_real_working_dir;

    Translation::language_t param_lang;

    const bool param_bogus_refresh_rect_ex;

    FrontAPI& front;

    mod_api& mod;

    FileSystemVirtualChannel& file_system_virtual_channel;

    wait_obj session_probe_event;

    OutboundConnectionMonitorRules outbound_connection_monitor_rules;
    ProcessMonitorRules            process_monitor_rules;

    bool disconnection_reconnection_required = false; // Cause => Authenticated user changed.

    SessionProbeLauncher* session_probe_stop_launch_sequence_notifier = nullptr;

    bool has_additional_launch_time = false;

    std::string server_message;

public:
    struct Params : public BaseVirtualChannel::Params {
        std::chrono::duration<unsigned, std::milli> session_probe_launch_timeout;
        std::chrono::duration<unsigned, std::milli> session_probe_launch_fallback_timeout;
        std::chrono::duration<unsigned, std::milli> session_probe_keepalive_timeout;
        bool     session_probe_on_keepalive_timeout_disconnect_user;

        SessionProbeOnLaunchFailure session_probe_on_launch_failure;

        bool session_probe_end_disconnected_session;

        const char* target_informations;

        uint16_t front_width;
        uint16_t front_height;

        std::chrono::duration<unsigned, std::milli> session_probe_disconnected_application_limit;
        std::chrono::duration<unsigned, std::milli> session_probe_disconnected_session_limit;
        std::chrono::duration<unsigned, std::milli> session_probe_idle_session_limit;

        const char* real_alternate_shell;
        const char* real_working_dir;

        const char* outbound_connection_monitoring_rules;

        const char* process_monitoring_rules;

        Translation::language_t lang;

        bool bogus_refresh_rect_ex;
    };

    SessionProbeVirtualChannel(
        VirtualChannelDataSender* to_server_sender_,
        FrontAPI& front,
        mod_api& mod,
        FileSystemVirtualChannel& file_system_virtual_channel,
        const Params& params)
    : BaseVirtualChannel(nullptr,
                         to_server_sender_,
                         params)
    , session_probe_effective_launch_timeout(
            (params.session_probe_on_launch_failure ==
             SessionProbeOnLaunchFailure::disconnect_user) ?
            params.session_probe_launch_timeout :
            params.session_probe_launch_fallback_timeout
        )
    , param_session_probe_keepalive_timeout(
          params.session_probe_keepalive_timeout)
    , param_session_probe_on_keepalive_timeout_disconnect_user(
          params.session_probe_on_keepalive_timeout_disconnect_user)
    , param_session_probe_on_launch_failure(
          params.session_probe_on_launch_failure)
    , param_session_probe_end_disconnected_session(
          params.session_probe_end_disconnected_session)
    , param_target_informations(params.target_informations)
    , param_front_width(params.front_width)
    , param_front_height(params.front_height)
    , param_session_probe_disconnected_application_limit(
        params.session_probe_disconnected_application_limit)
    , param_session_probe_disconnected_session_limit(
        params.session_probe_disconnected_session_limit)
    , param_session_probe_idle_session_limit(
        params.session_probe_idle_session_limit)
    , param_real_alternate_shell(params.real_alternate_shell)
    , param_real_working_dir(params.real_working_dir)
    , param_lang(params.lang)
    , param_bogus_refresh_rect_ex(params.bogus_refresh_rect_ex)
    , front(front)
    , mod(mod)
    , file_system_virtual_channel(file_system_virtual_channel)
    , outbound_connection_monitor_rules(
          params.outbound_connection_monitoring_rules)
    , process_monitor_rules(params.process_monitoring_rules)
    {
        if (this->verbose & MODRDP_LOGLEVEL_SESPROBE) {
            LOG(LOG_INFO,
                "SessionProbeVirtualChannel::SessionProbeVirtualChannel: "
                    "timeout=%u fallback_timeout=%u effective_timeout=%u on_launch_failure=%d",
                params.session_probe_launch_timeout.count(),
                params.session_probe_launch_fallback_timeout.count(),
                this->session_probe_effective_launch_timeout.count(),
                static_cast<int>(this->param_session_probe_on_launch_failure));
        }

        this->session_probe_event.object_and_time = true;

        REDASSERT(this->authentifier);
    }

    void start_launch_timeout_timer()
    {
        if ((this->session_probe_effective_launch_timeout.count() > 0) &&
            !this->session_probe_ready) {
            if (this->verbose & MODRDP_LOGLEVEL_SESPROBE) {
                LOG(LOG_INFO, "SessionProbeVirtualChannel::start_launch_timeout_timer");
            }

            if (!this->session_probe_launch_timeout_timer_started) {
                this->session_probe_event.set(
                    std::chrono::duration_cast<std::chrono::microseconds>(
                        this->session_probe_effective_launch_timeout).count());

                this->session_probe_launch_timeout_timer_started = true;
            }
        }
    }

protected:
    const char* get_reporting_reason_exchanged_data_limit_reached() const
        override
    {
        return "";
    }

public:
    wait_obj* get_event()
    {
        if (this->session_probe_event.set_state) {
            if (this->has_additional_launch_time) {
                if (!this->session_probe_ready) {
                    this->session_probe_event.set(
                        std::chrono::duration_cast<std::chrono::microseconds>(
                            this->session_probe_effective_launch_timeout).count());
                }

                this->has_additional_launch_time = false;
            }
            return &this->session_probe_event;
        }

        return nullptr;
    }

    void give_additional_launch_time() {
        if (!this->session_probe_ready) {
            this->has_additional_launch_time = true;

            if (this->verbose & MODRDP_LOGLEVEL_SESPROBE) {
                LOG(LOG_INFO,
                    "SessionProbeVirtualChannel::give_additional_launch_time");
            }
        }
    }

    bool is_event_signaled() {
        return (this->session_probe_event.set_state &&
            this->session_probe_event.waked_up_by_time);
    }

    bool is_disconnection_reconnection_required() {
        return this->disconnection_reconnection_required;
    }

    void process_event()
    {
        if (!this->session_probe_event.set_state ||
            !this->session_probe_event.waked_up_by_time) {
            return;
        }

        this->session_probe_event.reset();
        this->session_probe_event.waked_up_by_time = false;

        if (this->session_probe_effective_launch_timeout.count() &&
            !this->session_probe_ready &&
            !this->has_additional_launch_time) {
            LOG(((this->param_session_probe_on_launch_failure ==
                  SessionProbeOnLaunchFailure::disconnect_user) ?
                 LOG_ERR : LOG_WARNING),
                "SessionProbeVirtualChannel::process_event: "
                    "Session Probe is not ready yet!");

            if (this->session_probe_stop_launch_sequence_notifier) {
                this->session_probe_stop_launch_sequence_notifier->stop(false);
                this->session_probe_stop_launch_sequence_notifier = nullptr;
            }

            const bool disable_input_event     = false;
            const bool disable_graphics_update = false;
            const bool need_full_screen_update =
                 this->front.disable_input_event_and_graphics_update(
                     disable_input_event, disable_graphics_update);

            if (this->param_session_probe_on_launch_failure ==
                SessionProbeOnLaunchFailure::ignore_and_continue) {
                if (need_full_screen_update) {
                    if (this->verbose & MODRDP_LOGLEVEL_SESPROBE) {
                        LOG(LOG_INFO,
                            "SessionProbeVirtualChannel::process_event: "
                                "Force full screen update. Rect=(0, 0, %u, %u)",
                            this->param_front_width, this->param_front_height);
                    }
                    this->mod.rdp_input_invalidate(Rect(0, 0,
                        this->param_front_width, this->param_front_height));
                }
            }
            else {
                throw Error(ERR_SESSION_PROBE_LAUNCH);
            }
        }

        if (this->session_probe_ready &&
            this->param_session_probe_keepalive_timeout.count()) {
            if (!this->session_probe_keep_alive_received) {
                const bool disable_input_event     = false;
                const bool disable_graphics_update = false;
                this->front.disable_input_event_and_graphics_update(
                    disable_input_event, disable_graphics_update);

                LOG(LOG_ERR,
                    "SessionProbeVirtualChannel::process_event: "
                        "No keep alive received from Session Probe!");

                if (!this->disconnection_reconnection_required) {
                    if (this->session_probe_ending_in_progress) {
                        throw Error(ERR_SESSION_PROBE_ENDING_IN_PROGRESS);
                    }

                    if (this->param_session_probe_on_keepalive_timeout_disconnect_user) {
                        this->authentifier->report("SESSION_PROBE_KEEPALIVE_MISSED", "");
                    }
                    else {
                        this->front.session_probe_started(false);
                    }
                }
            }
            else {
                this->session_probe_keep_alive_received = false;

                {
                    StaticOutStream<1024> out_s;

                    const size_t message_length_offset = out_s.get_offset();
                    out_s.out_skip_bytes(sizeof(uint16_t));

                    {
                        const char string[] = "Request=Keep-Alive";
                        out_s.out_copy_bytes(string, sizeof(string) - 1u);
                    }

                    out_s.out_clear_bytes(1);   // Null-terminator.

                    out_s.set_out_uint16_le(
                        out_s.get_offset() - message_length_offset -
                            sizeof(uint16_t),
                        message_length_offset);

                    this->send_message_to_server(out_s.get_offset(),
                        CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                        out_s.get_data(), out_s.get_offset());
                }

                if (this->verbose & MODRDP_LOGLEVEL_SESPROBE_REPETITIVE) {
                    LOG(LOG_INFO,
                        "SessionProbeVirtualChannel::process_event: "
                            "Session Probe keep alive requested");
                }

                this->session_probe_event.set(
                    std::chrono::duration_cast<std::chrono::microseconds>(
                        this->param_session_probe_keepalive_timeout ).count());
            }
        }
    }

    void process_server_message(uint32_t total_length,
        uint32_t flags, const uint8_t* chunk_data,
        uint32_t chunk_data_length,
        std::unique_ptr<AsynchronousTask>& out_asynchronous_task) override
    {
        (void)out_asynchronous_task;

        if (this->verbose & MODRDP_LOGLEVEL_SESPROBE) {
            LOG(LOG_INFO,
                "SessionProbeVirtualChannel::process_server_message: "
                    "total_length=%u flags=0x%08X chunk_data_length=%u",
                total_length, flags, chunk_data_length);
        }

        if (this->verbose & MODRDP_LOGLEVEL_SESPROBE_DUMP) {
            const bool send              = false;
            const bool from_or_to_client = false;
            ::msgdump_c(send, from_or_to_client, total_length, flags,
                chunk_data, chunk_data_length);
        }

        InStream chunk(chunk_data, chunk_data_length);

        uint16_t message_length = chunk.in_uint16_le();
        this->server_message.reserve(message_length);

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST)
            this->server_message.clear();

        this->server_message.append(char_ptr_cast(chunk.get_current()),
            chunk.in_remain());

        if (!(flags & CHANNELS::CHANNEL_FLAG_LAST))
            return;

        while (this->server_message.back() == '\0') {
            this->server_message.pop_back();
        }
        if (this->verbose & MODRDP_LOGLEVEL_SESPROBE) {
            LOG(LOG_INFO,
                "SessionProbeVirtualChannel::process_server_message: \"%s\"",
                this->server_message.c_str());
        }

        const char request_outbound_connection_monitoring_rule[] =
            "Request=Get outbound connection monitoring rule\x01";

        const char request_process_monitoring_rule[] =
            "Request=Get process monitoring rule\x01";

        const char request_hello[] = "Request=Hello";

        const char version[] = "Version=";

        if (!this->server_message.compare(request_hello)) {
            if (this->verbose & MODRDP_LOGLEVEL_SESPROBE) {
                LOG(LOG_INFO,
                    "SessionProbeVirtualChannel::process_server_message: "
                        "Session Probe is ready.");
            }

            if (this->session_probe_stop_launch_sequence_notifier) {
                this->session_probe_stop_launch_sequence_notifier->stop(true);
                this->session_probe_stop_launch_sequence_notifier = nullptr;
            }

            this->session_probe_ready = true;

            this->front.session_probe_started(true);

            const bool disable_input_event     = false;
            const bool disable_graphics_update = false;
            if (this->front.disable_input_event_and_graphics_update(
                    disable_input_event, disable_graphics_update)) {
                if (this->verbose & MODRDP_LOGLEVEL_SESPROBE) {
                    LOG(LOG_INFO,
                        "SessionProbeVirtualChannel::process_server_message: "
                            "Force full screen update. Rect=(0, 0, %u, %u)",
                        this->param_front_width, this->param_front_height);
                }
                if (this->param_bogus_refresh_rect_ex) {
                    this->mod.rdp_suppress_display_updates();
                    this->mod.rdp_allow_display_updates(0, 0,
                        this->param_front_width, this->param_front_height);
                }
                this->mod.rdp_input_invalidate(Rect(0, 0,
                    this->param_front_width, this->param_front_height));
            }

            this->file_system_virtual_channel.disable_session_probe_drive();

            this->session_probe_event.reset();

            if (this->param_session_probe_keepalive_timeout.count() > 0) {
                {
                    StaticOutStream<1024> out_s;

                    const size_t message_length_offset = out_s.get_offset();
                    out_s.out_skip_bytes(sizeof(uint16_t));

                    {
                        const char cstr[] = "Request=Keep-Alive";
                        out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                    }

                    out_s.out_clear_bytes(1);   // Null-terminator.

                    out_s.set_out_uint16_le(
                        out_s.get_offset() - message_length_offset -
                            sizeof(uint16_t),
                        message_length_offset);

                    this->send_message_to_server(out_s.get_offset(),
                        CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                        out_s.get_data(), out_s.get_offset());
                }

                if (this->verbose & MODRDP_LOGLEVEL_SESPROBE_REPETITIVE) {
                    LOG(LOG_INFO,
                        "SessionProbeVirtualChannel::process_event: "
                            "Session Probe keep alive requested");
                }

                this->session_probe_event.set(
                    std::chrono::duration_cast<std::chrono::microseconds>(
                        this->param_session_probe_keepalive_timeout).count());
            }

            {
                StaticOutStream<1024> out_s;

                const size_t message_length_offset = out_s.get_offset();
                out_s.out_skip_bytes(sizeof(uint16_t));

                {
                    const char cstr[] = "Version=" "1" "\x01" "1";
                    out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                }

                out_s.out_clear_bytes(1);   // Null-terminator.

                out_s.set_out_uint16_le(
                    out_s.get_offset() - message_length_offset -
                        sizeof(uint16_t),
                    message_length_offset);

                this->send_message_to_server(out_s.get_offset(),
                    CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                    out_s.get_data(), out_s.get_offset());
            }

            {
                StaticOutStream<1024> out_s;

                const size_t message_length_offset = out_s.get_offset();
                out_s.out_skip_bytes(sizeof(uint16_t));

                {
                    const char cstr[] = "AutomaticallyEndDisconnectedSession=";
                    out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                }

                if (this->param_session_probe_end_disconnected_session) {
                    const char cstr[] = "Yes";
                    out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                }
                else {
                    const char cstr[] = "No";
                    out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                }

                out_s.out_clear_bytes(1);   // Null-terminator.

                out_s.set_out_uint16_le(
                    out_s.get_offset() - message_length_offset -
                        sizeof(uint16_t),
                    message_length_offset);

                this->send_message_to_server(out_s.get_offset(),
                    CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                    out_s.get_data(), out_s.get_offset());
            }

            unsigned int const disconnect_session_limit =
                (this->param_real_alternate_shell.empty() ?
                 // Normal RDP session
                 this->param_session_probe_disconnected_session_limit.count() :
                 // Application session
                 this->param_session_probe_disconnected_application_limit.count());

            if (disconnect_session_limit)
            {
                StaticOutStream<1024> out_s;

                const size_t message_length_offset = out_s.get_offset();
                out_s.out_skip_bytes(sizeof(uint16_t));

                {
                    const char cstr[] = "DisconnectedSessionLimit=";
                    out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                }

                {
                    char cstr[128];
                    snprintf(cstr, sizeof(cstr), "%u",
                        disconnect_session_limit);
                    out_s.out_copy_bytes(cstr, strlen(cstr));
                }

                out_s.out_clear_bytes(1);   // Null-terminator.

                out_s.set_out_uint16_le(
                    out_s.get_offset() - message_length_offset -
                        sizeof(uint16_t),
                    message_length_offset);

                this->send_message_to_server(out_s.get_offset(),
                    CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                    out_s.get_data(), out_s.get_offset());
            }

            if (this->param_session_probe_idle_session_limit.count())
            {
                StaticOutStream<1024> out_s;

                const size_t message_length_offset = out_s.get_offset();
                out_s.out_skip_bytes(sizeof(uint16_t));

                {
                    const char cstr[] = "IdleSessionLimit=";
                    out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                }

                {
                    char cstr[128];
                    snprintf(cstr, sizeof(cstr), "%u",
                        this->param_session_probe_idle_session_limit.count());
                    out_s.out_copy_bytes(cstr, strlen(cstr));
                }

                out_s.out_clear_bytes(1);   // Null-terminator.

                out_s.set_out_uint16_le(
                    out_s.get_offset() - message_length_offset -
                        sizeof(uint16_t),
                    message_length_offset);

                this->send_message_to_server(out_s.get_offset(),
                    CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                    out_s.get_data(), out_s.get_offset());
            }
        }
        else if (!this->server_message.compare(
                     "Request=Get target informations")) {
            StaticOutStream<1024> out_s;

            const size_t message_length_offset = out_s.get_offset();
            out_s.out_skip_bytes(sizeof(uint16_t));

            {
                const char cstr[] = "TargetInformations=";
                out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
            }

            out_s.out_copy_bytes(this->param_target_informations.data(),
                this->param_target_informations.size());

            out_s.out_clear_bytes(1);   // Null-terminator.

            out_s.set_out_uint16_le(
                out_s.get_offset() - message_length_offset -
                    sizeof(uint16_t),
                message_length_offset);

            this->send_message_to_server(out_s.get_offset(),
                CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                out_s.get_data(), out_s.get_offset());
        }
        else if (!this->server_message.compare(
                    "Request=Get startup application")) {
            StaticOutStream<8192> out_s;

            const size_t message_length_offset = out_s.get_offset();
            out_s.out_skip_bytes(sizeof(uint16_t));

            {
                const char cstr[] = "StartupApplication=";
                out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
            }

            if (this->param_real_alternate_shell.empty()) {
                const char cstr[] = "[Windows Explorer]";
                out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
            }
            else {
                if (!this->param_real_working_dir.empty()) {
                    out_s.out_copy_bytes(
                        this->param_real_working_dir.data(),
                        this->param_real_working_dir.size());
                }
                out_s.out_uint8('\x01');

                out_s.out_copy_bytes(
                    this->param_real_alternate_shell.data(),
                    this->param_real_alternate_shell.size());
            }

            out_s.out_clear_bytes(1);   // Null-terminator.

            out_s.set_out_uint16_le(
                out_s.get_offset() - message_length_offset -
                    sizeof(uint16_t),
                message_length_offset);

            this->send_message_to_server(out_s.get_offset(),
                CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                out_s.get_data(), out_s.get_offset());
        }
        else if (!this->server_message.compare(
                     "Request=Disconnection-Reconnection")) {
            if (this->verbose & MODRDP_LOGLEVEL_SESPROBE) {
                LOG(LOG_INFO,
                    "SessionProbeVirtualChannel::process_server_message: "
                        "Disconnection-Reconnection required.");
            }

            this->disconnection_reconnection_required = true;

            {
                StaticOutStream<512> out_s;

                const size_t message_length_offset = out_s.get_offset();
                out_s.out_skip_bytes(sizeof(uint16_t));

                {
                    const char cstr[] = "Confirm=Disconnection-Reconnection";
                    out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                }

                out_s.out_clear_bytes(1);   // Null-terminator.

                out_s.set_out_uint16_le(
                    out_s.get_offset() - message_length_offset -
                        sizeof(uint16_t),
                    message_length_offset);

                this->send_message_to_server(out_s.get_offset(),
                    CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                    out_s.get_data(), out_s.get_offset());
            }
        }
        else if (!this->server_message.compare(
                     0,
                     sizeof(version) - 1,
                     version)) {
            const char * subitems          =
                (this->server_message.c_str() + sizeof(version) - 1);
            const char * subitem_separator =
                ::strchr(subitems, '\x01');

            if (subitem_separator && (subitem_separator != subitems)) {
                if (this->verbose & MODRDP_LOGLEVEL_SESPROBE) {
                    LOG(LOG_INFO,
                        "SessionProbeVirtualChannel::process_server_message: "
                            "OtherVersion=%lu.%lu",
                        ::strtoul(subitems, nullptr, 10),
                        ::strtoul(subitem_separator + 1, nullptr, 10));
                }
            }
        }
        else if (!this->server_message.compare(
                     0,
                     sizeof(request_outbound_connection_monitoring_rule) - 1,
                     request_outbound_connection_monitoring_rule)) {
            const char * remaining_data =
                (this->server_message.c_str() +
                 sizeof(request_outbound_connection_monitoring_rule) - 1);

            const unsigned int rule_index =
                ::strtoul(remaining_data, nullptr, 10);

            // OutboundConnectionMonitoringRule=RuleIndex\x01ErrorCode[\x01RuleType\x01HostAddrOrSubnet\x01Port]
            // RuleType  : 0 - notify, 1 - deny, 2 - allow.
            // ErrorCode : 0 on success. -1 if an error occurred.

            {
                StaticOutStream<8192> out_s;

                const size_t message_length_offset = out_s.get_offset();
                out_s.out_skip_bytes(sizeof(uint16_t));

                {
                    const char cstr[] = "OutboundConnectionMonitoringRule=";
                    out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                }

                unsigned int type = 0;
                std::string  host_address_or_subnet;
                std::string  port_range;
                std::string  description;

                const bool result =
                    this->outbound_connection_monitor_rules.get(
                        rule_index, type, host_address_or_subnet, port_range,
                        description);

                {
                    const int error_code = (result ? 0 : -1);
                    char cstr[128];
                    snprintf(cstr, sizeof(cstr), "%u" "\x01" "%d",
                        rule_index, error_code);
                    out_s.out_copy_bytes(cstr, strlen(cstr));
                }

                if (result) {
                    char cstr[1024];
                    snprintf(cstr, sizeof(cstr), "\x01" "%u" "\x01" "%s" "\x01" "%s",
                        type, host_address_or_subnet.c_str(), port_range.c_str());
                    out_s.out_copy_bytes(cstr, strlen(cstr));
                }

                out_s.out_clear_bytes(1);   // Null-terminator.

                out_s.set_out_uint16_le(
                    out_s.get_offset() - message_length_offset -
                        sizeof(uint16_t),
                    message_length_offset);

                this->send_message_to_server(out_s.get_offset(),
                    CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                    out_s.get_data(), out_s.get_offset());
            }
        }
        else if (!this->server_message.compare(
                     0,
                     sizeof(request_process_monitoring_rule) - 1,
                     request_process_monitoring_rule)) {
            const char * remaining_data =
                (this->server_message.c_str() +
                 sizeof(request_process_monitoring_rule) - 1);

            const unsigned int rule_index =
                ::strtoul(remaining_data, nullptr, 10);

            // ProcessMonitoringRule=RuleIndex\x01ErrorCode[\x01RuleType\x01Pattern]
            // RuleType  : 0 - notify, 1 - deny.
            // ErrorCode : 0 on success. -1 if an error occurred.

            {
                StaticOutStream<8192> out_s;

                const size_t message_length_offset = out_s.get_offset();
                out_s.out_skip_bytes(sizeof(uint16_t));

                {
                    const char cstr[] = "ProcessMonitoringRule=";
                    out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                }

                unsigned int type = 0;
                std::string  pattern;
                std::string  description;

                const bool result =
                    this->process_monitor_rules.get(
                        rule_index, type, pattern, description);

                {
                    const int error_code = (result ? 0 : -1);
                    char cstr[128];
                    snprintf(cstr, sizeof(cstr), "%u" "\x01" "%d",
                        rule_index, error_code);
                    out_s.out_copy_bytes(cstr, strlen(cstr));
                }

                if (result) {
                    char cstr[1024];
                    snprintf(cstr, sizeof(cstr), "\x01" "%u" "\x01" "%s",
                        type, pattern.c_str());
                    out_s.out_copy_bytes(cstr, strlen(cstr));
                }

                out_s.out_clear_bytes(1);   // Null-terminator.

                out_s.set_out_uint16_le(
                    out_s.get_offset() - message_length_offset -
                        sizeof(uint16_t),
                    message_length_offset);

                this->send_message_to_server(out_s.get_offset(),
                    CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                    out_s.get_data(), out_s.get_offset());
            }
        }
        else if (!this->server_message.compare("KeepAlive=OK")) {
            if (this->verbose & MODRDP_LOGLEVEL_SESPROBE_REPETITIVE) {
                LOG(LOG_INFO,
                    "SessionProbeVirtualChannel::process_server_message: "
                        "Recevied Keep-Alive from Session Probe.");
            }
            this->session_probe_keep_alive_received = true;
        }
        else if (!this->server_message.compare("SESSION_ENDING_IN_PROGRESS")) {
            this->authentifier->log4(
                (this->verbose & MODRDP_LOGLEVEL_SESPROBE),
                "SESSION_ENDING_IN_PROGRESS");

            this->session_probe_ending_in_progress = true;
        }
        else {
            const char * message   = this->server_message.c_str();
            this->front.session_update({message, this->server_message.size()});

            const char * separator = ::strchr(message, '=');

            bool message_format_invalid = false;

            if (separator) {
                // TODO string_view
                std::string order(message, separator - message);
                // TODO vector<string_view>
                std::vector<std::string> parameters;

                /** TODO
                 * for (r : get_split(separator, this->server_message.c_str() + this->server_message.size(), '\ x01')) {
                 *     parameters.push_back({r.begin(), r.end()});
                 * }
                 */
                {
                    std::istringstream ss(std::string(separator + 1));
                    std::string        parameter;

                    while (std::getline(ss, parameter, '\x01')) {
                        parameters.push_back(std::move(parameter));
                    }
                }

                if (!order.compare("PASSWORD_TEXT_BOX_GET_FOCUS")) {
                    std::string info("status='" + parameters[0] + "'");
                    this->authentifier->log4(
                        (this->verbose & MODRDP_LOGLEVEL_SESPROBE),
                        order.c_str(), info.c_str());

                    if (parameters.size() == 1) {
                        this->front.set_focus_on_password_textbox(
                            !parameters[0].compare("yes"));
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!order.compare("UAC_PROMPT_BECOME_VISIBLE")) {
                    std::string info("status='" + parameters[0] + "'");
                    this->authentifier->log4
                        ((this->verbose & MODRDP_LOGLEVEL_SESPROBE),
                        order.c_str(), info.c_str());

                    if (parameters.size() == 1) {
                        this->front.set_consent_ui_visible(
                            !parameters[0].compare("yes"));
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!order.compare("INPUT_LANGUAGE")) {
                    if (parameters.size() == 2) {
                        std::string info(
                            "identifier='" + parameters[0] +
                            "' display_name='" + parameters[1] + "'");
                        this->authentifier->log4(
                            (this->verbose & MODRDP_LOGLEVEL_SESPROBE),
                            order.c_str(), info.c_str());

                        this->front.set_keylayout(
                            ::strtol(parameters[0].c_str(), nullptr, 16));
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!order.compare("NEW_PROCESS") ||
                         !order.compare("COMPLETED_PROCESS")) {
                    if (parameters.size() == 1) {
                        std::string info("command_line='" + parameters[0] + "'");
                        this->authentifier->log4(
                            (this->verbose & MODRDP_LOGLEVEL_SESPROBE),
                            order.c_str(), info.c_str());
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!order.compare("OUTBOUND_CONNECTION_BLOCKED") ||
                         !order.compare("OUTBOUND_CONNECTION_DETECTED")) {
                    bool deny = (!order.compare("OUTBOUND_CONNECTION_BLOCKED"));

                    if (parameters.size() == 2) {
                        std::string info(
                            "rule='" + parameters[0] +
                            "' application_name='" + parameters[1]);
                        this->authentifier->log4(
                            (this->verbose & MODRDP_LOGLEVEL_SESPROBE),
                            order.c_str(), info.c_str());

                        if (deny) {
                            char message[4096];

#ifdef __GNUG__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-nonliteral"
# endif
                            snprintf(message, sizeof(message),
                                TR("process_interrupted_security_policies",
                                   this->param_lang),
                                parameters[1].c_str());
#ifdef __GNUG__
    #pragma GCC diagnostic pop
# endif

                            std::string string_message = message;
                            this->mod.display_osd_message(string_message);
                        }
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!order.compare("OUTBOUND_CONNECTION_BLOCKED_2") ||
                         !order.compare("OUTBOUND_CONNECTION_DETECTED_2")) {
                    bool deny = (!order.compare("OUTBOUND_CONNECTION_BLOCKED_2"));

                    if ((!deny && (parameters.size() == 5)) ||
                        (deny && (parameters.size() == 6))) {
                        unsigned int type = 0;
                        std::string  host_address_or_subnet;
                        std::string  port_range;
                        std::string  description;

                        const bool result =
                            this->outbound_connection_monitor_rules.get(
                                ::strtoul(parameters[0].c_str(), nullptr, 10),
                                type, host_address_or_subnet, port_range,
                                description);

                        if (result) {
                            std::string info(
                                "rule='" + description +
                                "' app_name='" + parameters[1] +
                                "' app_cmd_line='" + parameters[2] +
                                "' dst_addr='" + parameters[3] +
                                "' dst_port='" + parameters[4] + "'");
                            this->authentifier->log4(
                                (this->verbose & MODRDP_LOGLEVEL_SESPROBE),
                                order.c_str(), info.c_str());

                            if (deny) {
                                if (::strtoul(parameters[5].c_str(), nullptr, 10)) {
                                    LOG(LOG_ERR,
                                        "Session Probe failed to block outbound connection!");
                                    this->authentifier->report(
                                        "SESSION_PROBE_OUTBOUND_CONNECTION_BLOCKING_FAILED", "");
                                }
                                else {
                                    char message[4096];

#ifdef __GNUG__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-nonliteral"
# endif
                                    snprintf(message, sizeof(message),
                                        TR("process_interrupted_security_policies",
                                           this->param_lang),
                                        parameters[1].c_str());
#ifdef __GNUG__
    #pragma GCC diagnostic pop
# endif

                                    std::string string_message = message;
                                    this->mod.display_osd_message(string_message);
                                }
                            }
                        }
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!order.compare("PROCESS_BLOCKED") ||
                         !order.compare("PROCESS_DETECTED")) {
                    bool deny = (!order.compare("PROCESS_BLOCKED"));

                    if ((!deny && (parameters.size() == 3)) ||
                        (deny && (parameters.size() == 4))) {
                        unsigned int type = 0;
                        std::string  pattern;
                        std::string  description;
                        const bool result =
                            this->process_monitor_rules.get(
                                ::strtoul(parameters[0].c_str(), nullptr, 10),
                                type, pattern, description);

                        if (result) {
                            std::string info(
                                "rule='" + description +
                                "' app_name='" + parameters[1] +
                                "' app_cmd_line='" + parameters[2] + "'");
                            this->authentifier->log4(
                                (this->verbose & MODRDP_LOGLEVEL_SESPROBE),
                                order.c_str(), info.c_str());

                            if (deny) {
                                if (::strtoul(parameters[3].c_str(), nullptr, 10)) {
                                    LOG(LOG_ERR,
                                        "Session Probe failed to block process!");
                                    this->authentifier->report(
                                        "SESSION_PROBE_PROCESS_BLOCKING_FAILED", "");
                                }
                                else {
                                    char message[4096];

#ifdef __GNUG__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-nonliteral"
# endif
                                    snprintf(message, sizeof(message),
                                        TR("process_interrupted_security_policies",
                                           this->param_lang),
                                        parameters[1].c_str());
#ifdef __GNUG__
    #pragma GCC diagnostic pop
# endif

                                    std::string string_message = message;
                                    this->mod.display_osd_message(string_message);
                                }
                            }
                        }
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!order.compare("FOREGROUND_WINDOW_CHANGED")) {
                    if (parameters.size() == 3) {
                        std::string info(
                            "source='Probe' window='" + parameters[0] + "'");
                        this->authentifier->log4(
                            (this->verbose & MODRDP_LOGLEVEL_SESPROBE),
                            "TITLE_BAR", info.c_str());
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!order.compare("BUTTON_CLICKED")) {
                    if (parameters.size() == 2) {
                        std::string info(
                            "windows='" + parameters[0] +
                            "' button='" + parameters[1] + "'");
                        this->authentifier->log4(
                            (this->verbose & MODRDP_LOGLEVEL_SESPROBE),
                            order.c_str(), info.c_str());
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!order.compare("EDIT_CHANGED")) {
                    if (parameters.size() == 2) {
                        std::string info(
                            "windows='" + parameters[0] +
                            "' edit='" + parameters[1] + "'");
                        this->authentifier->log4(
                            (this->verbose & MODRDP_LOGLEVEL_SESPROBE),
                            order.c_str(), info.c_str());
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else {
                    LOG(LOG_WARNING,
                        "SessionProbeVirtualChannel::process_server_message: "
                            "Unexpected order. Message=\"%s\"",
                        message);
                }
            }
            else {
                message_format_invalid = true;
            }

            if (message_format_invalid) {
                LOG(LOG_WARNING,
                    "SessionProbeVirtualChannel::process_server_message: "
                        "Invalid message format. Message=\"%s\"",
                    message);
            }
        }
    }   // process_server_message

    void set_session_probe_launcher(SessionProbeLauncher* launcher) {
        this->session_probe_stop_launch_sequence_notifier = launcher;
    }
};  // class SessionProbeVirtualChannel

