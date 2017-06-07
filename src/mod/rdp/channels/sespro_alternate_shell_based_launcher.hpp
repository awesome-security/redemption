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
    Copyright (C) Wallix 2016
    Author(s): Christophe Grosjean, Raphael Zhou
*/


#pragma once

#include "mod/rdp/channels/sespro_channel.hpp"
#include "mod/rdp/channels/sespro_launcher.hpp"

class SessionProbeAlternateShellBasedLauncher : public SessionProbeLauncher {
private:
    SessionProbeVirtualChannel* channel = nullptr;

    bool drive_redirection_initialized = false;

    bool image_readed = false;

    bool stopped = false;

    const RDPVerbose verbose;

public:
    explicit SessionProbeAlternateShellBasedLauncher(RDPVerbose verbose)
    : verbose(verbose)
    {}

    wait_obj* get_event() override { return nullptr; }

    bool on_clipboard_initialize() override { return false; }

    bool on_clipboard_monitor_ready() override { return false; }

    bool on_drive_access() override {
        if (bool(this->verbose & RDPVerbose::sesprobe_launcher)) {
            LOG(LOG_INFO,
                "SessionProbeAlternateShellBasedLauncher :=> on_drive_access");
        }

        if (this->stopped) {
            return false;
        }

        if (this->channel) {
            this->channel->give_additional_launch_time();
        }

        return false;
    }

    bool on_drive_redirection_initialize() override {
        if (bool(this->verbose & RDPVerbose::sesprobe_launcher)) {
            LOG(LOG_INFO,
                "SessionProbeAlternateShellBasedLauncher :=> on_drive_redirection_initialize");
        }

        this->drive_redirection_initialized = true;

        return false;
    }

    bool on_event() override { return false; }

    bool on_image_read(uint64_t offset, uint32_t length) override {
        (void)offset;
        (void)length;

        if (bool(this->verbose & RDPVerbose::sesprobe_launcher)) {
            LOG(LOG_INFO,
                "SessionProbeAlternateShellBasedLauncher :=> on_image_read");
        }

        if (this->stopped) {
            return false;
        }

        this->image_readed = true;

        if (this->channel) {
            this->channel->give_additional_launch_time();
        }

        return false;
    }

    bool on_server_format_data_request() override { return false; }

    bool on_server_format_list_response() override { return false; }

    // Returns false to prevent message to be sent to server.
    bool process_client_cliprdr_message(InStream & chunk,uint32_t length, uint32_t flags) override {
        (void)chunk;
        (void)length;
        (void)flags;
        return true;
    }

    void set_clipboard_virtual_channel(BaseVirtualChannel*) override {
    }

    void set_session_probe_virtual_channel(BaseVirtualChannel* channel) override {
        this->channel = static_cast<SessionProbeVirtualChannel*>(channel);
    }

    void stop(bool bLaunchSuccessful) override {
        if (bool(this->verbose & RDPVerbose::sesprobe_launcher)) {
            LOG(LOG_INFO,
                "SessionProbeAlternateShellBasedLauncher :=> stop");
        }

        this->stopped = true;

        if (!bLaunchSuccessful) {
            if (!this->drive_redirection_initialized) {
                LOG(LOG_ERR,
                    "SessionProbeAlternateShellBasedLauncher :=> "
                        "File System Virtual Channel is unavailable. "
                        "Please allow the drive redirection in the Remote Desktop Services settings of the target.");
            }
            else if (!this->image_readed) {
                LOG(LOG_ERR,
                    "SessionProbeAlternateShellBasedLauncher :=> "
                        "Session Probe is not launched. "
                        "Maybe something blocks it on the target. "
                        "Is the target running under Microsoft Server products? "
                        "The Command Prompt should be published as the RemoteApp program and accept any command-line parameters. "
                        "Please also check the temporary directory to ensure there is enough free space.");
            }
            else {
                LOG(LOG_ERR,
                    "SessionProbeAlternateShellBasedLauncher :=> "
                        "Session Probe launch has failed for unknown reason.");
            }
        }
    }
};
