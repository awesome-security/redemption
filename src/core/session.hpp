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
   Copyright (C) Wallix 2010-2012
   Author(s): Christophe Grosjean, Javier Caverni, Raphael Zhou, Meng Tan
*/

#pragma once

#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/resource.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>

#include <array>

#include "utils/invalid_socket.hpp"

#include "core/server.hpp"
#include "utils/colors.hpp"
#include "utils/stream.hpp"
#include "front/front.hpp"
#include "system/ssl_calls.hpp"
#include "utils/rect.hpp"
#include "utils/netutils.hpp"

#include "configs/config.hpp"
#include "core/wait_obj.hpp"
#include "transport/transport.hpp"
#include "utils/bitmap.hpp"

#include "acl/authentifier.hpp"


enum {
    // before anything else : exchange of credentials
//    SESSION_STATE_RSA_KEY_HANDSHAKE,
    // initial state no module loaded, init not done
    SESSION_STATE_ENTRY,
    // no module loaded
    // init_done
    // login window destoyed if necessary
    // user clicked on OK to run module  or provided connection info on cmd line
    // but did not received credentials yet
    SESSION_STATE_WAITING_FOR_NEXT_MODULE,
    // a module is loaded and active but required some action
    // involving requesting remote context
    SESSION_STATE_WAITING_FOR_CONTEXT,
    // init_done, module loaded and running
    SESSION_STATE_RUNNING,
    // display dialog when connection is closed
    SESSION_STATE_CLOSE_CONNECTION,
    // disconnect session
    SESSION_STATE_STOP
};

class Session {
    Inifile  & ini;

    int internal_state;

    Front * front;

    UdevRandom gen;
    TimeSystem timeobj;

    class Client {

    public:
        // TODO Looks like acl and mod can be unified into a common class, where events can happen
        // TODO move auth_event to acl
        SocketTransport auth_trans;
        wait_obj        auth_event;
        SessionManager  acl;

        Client(int client_sck, Inifile & ini, ActivityChecker & activity_checker, time_t now)
        : auth_trans( "Authentifier"
                    , client_sck
                    , ini.get<cfg::globals::authfile>().c_str()
                    , 0
                    , ini.get<cfg::debug::auth>()
        )
        , acl( ini
             , activity_checker
             , this->auth_trans
             , now        // acl start time
        )
        {}

        bool is_set(fd_set & rfds) {
            return this->auth_event.is_set(this->auth_trans.sck, rfds);
        }
    };

    Client * client = nullptr;

          time_t   perf_last_info_collect_time;
    const pid_t    perf_pid;
          FILE   * perf_file;

    static const time_t select_timeout_tv_sec = 3;

public:
    Session(int sck, Inifile & ini, CryptoContext & cctx)
            : ini(ini)
            , perf_last_info_collect_time(0)
            , perf_pid(getpid())
            , perf_file(nullptr) {
        try {
            TRANSLATIONCONF.set_ini(&ini);

            SocketTransport front_trans("RDP Client", sck, "", 0, this->ini.get<cfg::debug::front>());
            // Contruct auth_trans (SocketTransport) and auth_event (wait_obj)
            //  here instead of inside Sessionmanager

            this->internal_state = SESSION_STATE_ENTRY;

            const bool mem3blt_support = true;

            time_t now = time(nullptr);

            this->front = new Front( front_trans, this->gen
                                   , this->ini, cctx, this->ini.get<cfg::client::fast_path>(), mem3blt_support
                                   , now);

            ModuleManager mm(*this->front, this->ini, this->gen, this->timeobj);
            BackEvent_t signal = BACK_EVENT_NONE;

            // Under conditions (if this->ini.get<cfg::video::inactivity_pause>() == true)
            PauseRecord pause_record(this->ini.get<cfg::video::inactivity_timeout>(), *this->front, mm, ini);

            if (this->ini.get<cfg::debug::session>()) {
                LOG(LOG_INFO, "Session::session_main_loop() starting");
            }

            const time_t start_time = time(nullptr);
            if (this->ini.get<cfg::debug::performance>() & 0x8000) {
                this->write_performance_log(start_time);
            }

            const timeval time_mark = { this->select_timeout_tv_sec, 0 };

            bool run_session = true;

            constexpr std::array<unsigned, 4> timers{{ 30*60, 10*60, 5*60, 1*60, }};
            const unsigned OSD_STATE_INVALID = timers.size();
            const unsigned OSD_STATE_NOT_YET_COMPUTED = OSD_STATE_INVALID + 1;
            unsigned osd_state = OSD_STATE_NOT_YET_COMPUTED;
            const bool enable_osd = this->ini.get<cfg::globals::enable_osd>();

            while (run_session) {
                unsigned max = 0;
                fd_set rfds;
                fd_set wfds;

                io_fd_zero(rfds);
                io_fd_zero(wfds);
                timeval timeout = time_mark;

                if (mm.mod->is_up_and_running() || !this->front->up_and_running) {
                    this->front->get_event().wait_on_fd(front_trans.sck, rfds, max, timeout);
                    if (this->front->capture) {
                        this->front->capture->get_capture_event().wait_on_timeout(timeout);
                    }
                }
                if (this->client) {
                    this->client->auth_event.wait_on_fd(this->client->auth_trans.sck, rfds, max, timeout);
                }

                mm.mod_transport ? mm.mod->get_event().wait_on_fd(mm.mod_transport->sck, rfds, max, timeout)
                                 : mm.mod->get_event().wait_on_timeout(timeout);

                wait_obj * secondary_event = mm.mod->get_secondary_event();
                if (secondary_event) {
                    secondary_event->wait_on_timeout(timeout);
                }

                int        asynchronous_task_fd    = INVALID_SOCKET;
                wait_obj * asynchronous_task_event = mm.mod->get_asynchronous_task_event(asynchronous_task_fd);
                if (asynchronous_task_event) {
                    asynchronous_task_event->wait_on_fd(asynchronous_task_fd, rfds, max, timeout);
                }

                wait_obj * session_probe_launcher_event = mm.mod->get_session_probe_launcher_event();
                if (session_probe_launcher_event) {
                    session_probe_launcher_event->wait_on_timeout(timeout);
                }

                const bool has_pending_data = (front_trans.tls && SSL_pending(front_trans.tls->allocated_ssl));
                if (has_pending_data){
                    memset(&timeout, 0, sizeof(timeout));
                }

                int num = select(max + 1, &rfds, &wfds, nullptr, &timeout);

                if (num < 0) {
                    if (errno == EINTR) {
                        continue;
                    }
                    // Cope with EBADF, EINVAL, ENOMEM : none of these should ever happen
                    // EBADF: means fd has been closed (by me) or as already returned an error on another call
                    // EINVAL: invalid value in timeout (my fault again)
                    // ENOMEM: no enough memory in kernel (unlikely fort 3 sockets)

                    LOG(LOG_ERR, "Proxy data wait loop raised error %u : %s", errno, strerror(errno));
                    run_session = false;
                    continue;
                }

                now = time(nullptr);
                if (this->ini.get<cfg::debug::performance>() & 0x8000) {
                    this->write_performance_log(now);
                }

                if (this->front->get_event().is_set(front_trans.sck, rfds) || (front_trans.tls && SSL_pending(front_trans.tls->allocated_ssl))) {
                    try {
                        this->front->incoming(mm.get_callback(), now);
                    } catch (Error & e) {
                        if (
                            // Can be caused by client disconnect.
                            (e.id != ERR_X224_RECV_ID_IS_RD_TPDU) &&
                            // Can be caused by client disconnect.
                            (e.id != ERR_MCS_APPID_IS_MCS_DPUM) &&
                            (e.id != ERR_RDP_HANDSHAKE_TIMEOUT) &&
                            // Can be caused by wabwatchdog.
                            (e.id != ERR_TRANSPORT_NO_MORE_DATA)) {
                            LOG(LOG_ERR, "Proxy data processing raised error %u : %s", e.id, e.errmsg(false));
                        }
                        run_session = false;
                        continue;
                    } catch (...) {
                        LOG(LOG_ERR, "Proxy data processing raised unknown error");
                        run_session = false;
                        continue;
                    };
                }

                try {
                    if (this->front->up_and_running) {
                        if (this->ini.get<cfg::video::inactivity_pause>()
                            && mm.connected
                            && this->front->capture) {
                            pause_record.check(now, *this->front);
                        }
                        // new value incomming from acl
                        if (this->ini.check_from_acl()) {
                            this->front->update_config(this->ini);
                            mm.check_module();
                        }

                        asynchronous_task_fd    = -1;
                        asynchronous_task_event = mm.mod->get_asynchronous_task_event(asynchronous_task_fd);
                        const bool asynchronous_task_event_is_set = (asynchronous_task_event &&
                                                                     asynchronous_task_event->is_set(asynchronous_task_fd,
                                                                            rfds));
                        if (asynchronous_task_event_is_set) {
                            mm.mod->process_asynchronous_task();
                        }

                        session_probe_launcher_event = mm.mod->get_session_probe_launcher_event();
                        const bool session_probe_launcher_event_is_set = (session_probe_launcher_event &&
                                                                          session_probe_launcher_event->is_set(asynchronous_task_fd, rfds));
                        if (session_probe_launcher_event_is_set) {
                            mm.mod->process_session_probe_launcher();
                        }

                        // Process incoming module trafic
                                   secondary_event        = mm.mod->get_secondary_event();
                        const bool secondary_event_is_set = (secondary_event &&
                                                             secondary_event->is_set(INVALID_SOCKET, rfds));
                        if (mm.mod->get_event().is_set(mm.mod_transport?mm.mod_transport->sck:INVALID_SOCKET, rfds) ||
                            secondary_event_is_set) {
                            try
                            {
                                mm.mod->draw_event(now, mm.get_graphic_wrapper(*this->front));

                                if (mm.mod->get_event().signal != BACK_EVENT_NONE) {
                                    signal = mm.mod->get_event().signal;
                                    mm.mod->get_event().reset();
                                }
                            }
                            catch (Error const & e) {
                                if ((e.id == ERR_SESSION_PROBE_LAUNCH) &&
                                    (this->ini.get<cfg::mod_rdp::session_probe_on_launch_failure>() ==
                                     SessionProbeOnLaunchFailure::retry_without_session_probe)) {
                                    this->ini.get_ref<cfg::mod_rdp::enable_session_probe>() = false;

                                    signal = BACK_EVENT_RETRY_CURRENT;
                                    mm.mod->get_event().reset();
                                }
                                else if (e.id == ERR_SESSION_PROBE_DISCONNECTION_RECONNECTION) {
                                    signal = BACK_EVENT_RETRY_CURRENT;
                                    mm.mod->get_event().reset();
                                }
                                else if (e.id == ERR_RDP_SERVER_REDIR) {
                                    // SET new target in ini
                                    const char * host = char_ptr_cast(
                                        this->ini.get<cfg::mod_rdp::redir_info>().host);
                                    const char * password = char_ptr_cast(
                                        this->ini.get<cfg::mod_rdp::redir_info>().password);
                                    const char * username = char_ptr_cast(
                                        this->ini.get<cfg::mod_rdp::redir_info>().username);
                                    const char * change_user = "";
                                    if (this->ini.get<cfg::mod_rdp::redir_info>().dont_store_username &&
                                        (username[0] != 0)) {
                                        LOG(LOG_INFO, "SrvRedir: Change target username to '%s'", username);
                                        this->ini.set_acl<cfg::globals::target_user>(username);
                                        change_user = username;
                                    }
                                    if (password[0] != 0) {
                                        LOG(LOG_INFO, "SrvRedir: Change target password");
                                        this->ini.set_acl<cfg::context::target_password>(password);
                                    }
                                    LOG(LOG_INFO, "SrvRedir: Change target host to '%s'", host);
                                    this->ini.set_acl<cfg::context::target_host>(host);
                                    char message[768] = {};
                                    sprintf(message, "%s@%s", change_user, host);
                                    if (this->client) {
                                        this->client->acl.report("SERVER_REDIRECTION", message);
                                    }

                                    signal = BACK_EVENT_RETRY_CURRENT;
                                    mm.mod->get_event().reset();
                                }
                                else {
                                    throw;
                                }
                            }
                        }
                        if (this->front->capture && this->front->capture->get_capture_event().is_set(INVALID_SOCKET, rfds)) {
                            this->front->periodic_snapshot();
                        }
                        // Incoming data from ACL, or opening acl
                        if (!this->client) {
                            if (!mm.last_module) {
                                // acl never opened or closed by me (close box)
                                try {
                                    int client_sck = local_connect(
                                        this->ini.get<cfg::globals::authfile>().c_str(),
                                        30, 1000
                                    );

                                    if (client_sck == -1) {
                                        LOG(LOG_ERR,
                                            "Failed to connect to authentifier (%s)",
                                            this->ini.get<cfg::globals::authfile>().c_str());
                                        throw Error(ERR_SOCKET_CONNECT_FAILED);
                                    }

                                    this->client = new Client(client_sck, ini, *this->front, now);
                                    signal = BACK_EVENT_NEXT;
                                }
                                catch (...) {
                                    mm.invoke_close_box("No authentifier available",signal, now);
                                }
                            }
                        }
                        else {
                            if (this->client->is_set(rfds)) {
                                // acl received updated values
                                this->client->acl.receive();
                            }
                        }

                        if (enable_osd) {
                            const uint32_t enddate = this->ini.get<cfg::context::end_date_cnx>();
                            if (enddate && mm.is_up_and_running()) {
                                if (osd_state == OSD_STATE_NOT_YET_COMPUTED) {
                                    osd_state = (enddate <= static_cast<uint32_t>(now))
                                        ? OSD_STATE_INVALID
                                        : timers.rbegin()
                                            - std::lower_bound(
                                                timers.rbegin(),
                                                timers.rend(),
                                                enddate - start_time);
                                }
                                else if (osd_state < OSD_STATE_INVALID
                                     && enddate - now <= timers[osd_state]) {
                                    std::string mes;
                                    mes.reserve(128);
                                    const unsigned minutes = (enddate - now + 30) / 60;
                                    mes += std::to_string(minutes);
                                    mes += ' ';
                                    mes += TR("minute", language(this->ini));
                                    mes += (minutes > 1) ? "s " : " ";
                                    mes += TR("before_closing", language(this->ini));
                                    mm.osd_message(std::move(mes), true);
                                    ++osd_state;
                                }
                            }
                        }

                        if (this->client) {
                            run_session = this->client->acl.check(mm, now, signal);
                        }
                        else if (signal == BACK_EVENT_STOP) {
                            mm.mod->get_event().reset();
                            run_session = false;
                        }
                        if (mm.last_module) {
                            delete this->client;
                            this->client = nullptr;
                        }
                    }
                } catch (Error & e) {
                    LOG(LOG_INFO, "Session::Session exception = %d!\n", e.id);
                    time_t now = time(nullptr);
                    mm.invoke_close_box(e.errmsg(), signal, now);
                };
            }
            if (mm.mod) {
                mm.mod->disconnect(time(nullptr));
            }
            this->front->disconnect();
        }
        catch (const Error & e) {
            LOG(LOG_INFO, "Session::Session Init exception = %d!\n", e.id);
        }
        catch(...) {
            LOG(LOG_INFO, "Session::Session other exception in Init\n");
        }
        // silent message for localhost for watchdog
        if (!this->ini.is_asked<cfg::globals::host>()
        && (this->ini.get<cfg::globals::host>() != "127.0.0.1")) {
            LOG(LOG_INFO, "Session::Client Session Disconnected\n");
        }
        this->front->must_be_stop_capture();
    }

    Session(Session const &) = delete;

    ~Session() {
        if (this->ini.get<cfg::debug::performance>() & 0x8000) {
            this->write_performance_log(this->perf_last_info_collect_time + 3);
        }
        if (this->perf_file) {
            ::fclose(this->perf_file);
        }
        delete this->front;
        delete this->client;
        // Suppress Session file from disk (original name with PID or renamed with session_id)
        if (!this->ini.get<cfg::context::session_id>().empty()) {
            char new_session_file[256];
            snprintf( new_session_file, sizeof(new_session_file), "%s/session_%s.pid"
                    , PID_PATH , this->ini.get<cfg::context::session_id>().c_str());
            unlink(new_session_file);
        }
        else {
            int child_pid = getpid();
            char old_session_file[256];
            sprintf(old_session_file, "%s/session_%d.pid", PID_PATH, child_pid);
            unlink(old_session_file);
        }
    }

private:
    void write_performance_log(time_t now) {
        if (!this->perf_last_info_collect_time) {
            REDASSERT(!this->perf_file);

            this->perf_last_info_collect_time = now - this->select_timeout_tv_sec;

            struct tm tm_;

            localtime_r(&this->perf_last_info_collect_time, &tm_);

            char filename[2048];
            snprintf(filename, sizeof(filename), "%s/rdpproxy,%04d%02d%02d-%02d%02d%02d,%d.perf",
                this->ini.get<cfg::video::record_tmp_path>().c_str(),
                tm_.tm_year + 1900, tm_.tm_mon, tm_.tm_mday, tm_.tm_hour, tm_.tm_min, tm_.tm_sec, this->perf_pid
                );

            this->perf_file = ::fopen(filename, "w");

            ::fprintf(this->perf_file,
                "time_t;"
                "ru_utime.tv_sec;ru_utime.tv_usec;ru_stime.tv_sec;ru_stime.tv_usec;"
                "ru_maxrss;ru_ixrss;ru_idrss;ru_isrss;ru_minflt;ru_majflt;ru_nswap;"
                "ru_inblock;ru_oublock;ru_msgsnd;ru_msgrcv;ru_nsignals;ru_nvcsw;ru_nivcsw\n");

        }
        else if (this->perf_last_info_collect_time + this->select_timeout_tv_sec > now) {
            return;
        }

        struct rusage resource_usage;

        getrusage(RUSAGE_SELF, &resource_usage);

        do {
            this->perf_last_info_collect_time += this->select_timeout_tv_sec;

            struct tm result;

            localtime_r(&this->perf_last_info_collect_time, &result);

            ::fprintf(
                  this->perf_file
                , "%lu;"
                  "%lu;%lu;%lu;%lu;%ld;%ld;%ld;%ld;%ld;%ld;%ld;%ld;%ld;%ld;%ld;%ld;%ld;%ld\n"
                , static_cast<unsigned long>(now)
                , static_cast<unsigned long>(resource_usage.ru_utime.tv_sec)  /* user CPU time used */
                , static_cast<unsigned long>(resource_usage.ru_utime.tv_usec)
                , static_cast<unsigned long>(resource_usage.ru_stime.tv_sec)  /* system CPU time used */
                , static_cast<unsigned long>(resource_usage.ru_stime.tv_usec)
                , resource_usage.ru_maxrss                                    /* maximum resident set size */
                , resource_usage.ru_ixrss                                     /* integral shared memory size */
                , resource_usage.ru_idrss                                     /* integral unshared data size */
                , resource_usage.ru_isrss                                     /* integral unshared stack size */
                , resource_usage.ru_minflt                                    /* page reclaims (soft page faults) */
                , resource_usage.ru_majflt                                    /* page faults (hard page faults)   */
                , resource_usage.ru_nswap                                     /* swaps */
                , resource_usage.ru_inblock                                   /* block input operations */
                , resource_usage.ru_oublock                                   /* block output operations */
                , resource_usage.ru_msgsnd                                    /* IPC messages sent */
                , resource_usage.ru_msgrcv                                    /* IPC messages received */
                , resource_usage.ru_nsignals                                  /* signals received */
                , resource_usage.ru_nvcsw                                     /* voluntary context switches */
                , resource_usage.ru_nivcsw                                    /* involuntary context switches */
            );
            ::fflush(this->perf_file);
        }
        while (this->perf_last_info_collect_time + this->select_timeout_tv_sec <= now);
    }
};
