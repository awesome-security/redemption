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
             Raphael Zhou, Meng Tan, Jennifer Inthavong

  Session related with ACL
  find out the next module to run from context reading
*/


#pragma once

#include "utils/log.hpp"
#include "configs/config.hpp"
#include "acl/acl_serializer.hpp"

#include "utils/verbose_flags.hpp"


class Authentifier : public AuthApi, public ReportMessageApi
{
    struct LogParam
    {
        std::string info;
    };

    bool session_log_is_open = false;
    std::vector<LogParam> buffered_log_params;

    Inifile & ini;
    CryptoContext & cctx;

    AclSerializer * acl_serial;

public:
    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        state = 0x10,
    };

    Authentifier(Inifile & ini, CryptoContext & cctx, Verbose verbose)
        : ini(ini)
        , cctx(cctx)
        , acl_serial(nullptr)
        , verbose(verbose)
    {
        if (bool(this->verbose & Verbose::state)) {
            LOG(LOG_INFO, "auth::Authentifier");
        }
    }

    void set_acl_serial(AclSerializer * acl_serial)
    {
        this->acl_serial = acl_serial;
    }

    ~Authentifier() override {
        if (bool(this->verbose & Verbose::state)) {
            LOG(LOG_INFO, "auth::~Authentifier");
        }
    }

    void receive() {
        if (this->acl_serial){
            this->acl_serial->receive();
        }
    }

    void set_auth_channel_target(const char * target) override {
        if (this->acl_serial){
            this->acl_serial->ini.set_acl<cfg::context::auth_channel_target>(target);
        }
    }

    void set_auth_error_message(const char * error_message) override {
        if (this->acl_serial){
            this->acl_serial->ini.set<cfg::context::auth_error_message>(error_message);
        }
    }

    void report(const char * reason, const char * message) override {
        if (this->acl_serial){
            this->acl_serial->report(reason, message);
        }
    }

    void disconnect_target() override {
        if (this->acl_serial &&
            // Call disconnect_target >>> Show Close Box (with back to selector)
            this->acl_serial->ini.get<cfg::globals::enable_close_box>()) {
            this->acl_serial->ini.set_acl<cfg::context::module>(STRMODULE_CLOSE);
        }
    }

    void log5(const std::string & info) override
    {
        // TODO: should we delay logs sent to SIEM ?
        if (this->acl_serial && this->session_log_is_open) {
            this->acl_serial->log5(info);
        }
        else {
            this->buffered_log_params.push_back({info});
        }
    }

    void log6(const std::string & info, const ArcsightLogInfo & arc_info) override {
        if (this->acl_serial && this->session_log_is_open) {
            this->acl_serial->log6(info, arc_info);
        }
        else {
            this->buffered_log_params.push_back({info});
        }
    }

    void update_inactivity_timeout() override
    {
        if (this->acl_serial){
            this->acl_serial->update_inactivity_timeout();
        }
    }

    time_t get_inactivity_timeout() override
    {
        if (this->acl_serial){
            return this->acl_serial->get_inactivity_timeout();
        }
        else
            return 0;
    }

    void new_remote_mod() override
    {
        if (this->acl_serial){
            this->cctx.set_master_key(ini.get<cfg::crypto::key0>());
            this->cctx.set_hmac_key(ini.get<cfg::crypto::key1>());
            this->cctx.set_trace_type(ini.get<cfg::globals::trace_type>());

            this->acl_serial->start_session_log();
            this->session_log_is_open = true;

            for (LogParam const & log_param : this->buffered_log_params) {
                ArcsightLogInfo arc_info;
                arc_info.name = "NEW_REMOTE_MOD";

                this->acl_serial->log6(log_param.info, arc_info);
            }
            this->buffered_log_params.clear();
            this->buffered_log_params.shrink_to_fit();
        }
    }

    void delete_remote_mod() override
    {
        if (this->acl_serial){
            if (this->session_log_is_open) {
                this->acl_serial->close_session_log();
            }
        }
    }
};
