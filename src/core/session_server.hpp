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
  Copyright (C) Wallix 2012
  Author(s): Christophe Grosjean
*/


#pragma once

#include "configs/config.hpp"
#include "core/server.hpp"
#include "core/session.hpp"
#include "utils/parse_ip_conntrack.hpp"

class SessionServer : public Server
{
    CryptoContext & cctx;

    // Used for enable transparent proxying on accepted socket (ini.get<cfg::globals::enable_transparent_mode>() = true).
    unsigned uid;
    unsigned gid;
    bool debug_config;

    std::string config_filename;

public:
    SessionServer(CryptoContext & cctx, unsigned uid, unsigned gid, std::string config_filename, bool debug_config = true)
        : cctx(cctx)
        , uid(uid)
        , gid(gid)
        , debug_config(debug_config)
        , config_filename(config_filename)
    {
    }

    Server_status start(int incoming_sck) override
    {
        union
        {
            struct sockaddr s;
            struct sockaddr_storage ss;
            struct sockaddr_in s4;
            struct sockaddr_in6 s6;
        } u;
        unsigned int sin_size = sizeof(u);
        memset(&u, 0, sin_size);

        int sck = accept(incoming_sck, &u.s, &sin_size);
        if (-1 == sck) {
            LOG(LOG_INFO, "Accept failed on socket %u (%s)", incoming_sck, strerror(errno));
            _exit(1);
        }

        char source_ip[256];
        strcpy(source_ip, inet_ntoa(u.s4.sin_addr));
        const int source_port = ntohs(u.s4.sin_port);
        /* start new process */
        const pid_t pid = fork();
        switch (pid) {
        case 0: /* child */
        // TODO: see exit status of child, we could use it to diagnose session behaviours
        // TODO: we could probably use some session launcher object here. Something like
        // an abstraction layer that would manage either forking of threading behavior
        // this would also likely have some effect on network ressources management
        // (that means the select() on ressources could be managed by that layer)
            {
                close(incoming_sck);

                Inifile ini;
                ini.set<cfg::font>(Font(SHARE_PATH "/" DEFAULT_FONT_NAME));
                ini.set<cfg::debug::config>(this->debug_config);
                { ConfigurationLoader cfg_loader(ini.configuration_holder(), this->config_filename.c_str()); }

                if (ini.get<cfg::debug::session>()){
                    LOG(LOG_INFO, "Setting new session socket to %d\n", sck);
                }

                union
                {
                    struct sockaddr s;
                    struct sockaddr_storage ss;
                    struct sockaddr_in s4;
                    struct sockaddr_in6 s6;
                } localAddress;
                socklen_t addressLength = sizeof(localAddress);


                if (-1 == getsockname(sck, &localAddress.s, &addressLength)){
                    LOG(LOG_INFO, "getsockname failed error=%s", strerror(errno));
                    _exit(1);
                }

                char target_ip[256];
                const int target_port = ntohs(localAddress.s4.sin_port);
//                strcpy(real_target_ip, inet_ntoa(localAddress.s4.sin_addr));
                strcpy(target_ip, inet_ntoa(localAddress.s4.sin_addr));

                if (0 != strcmp(source_ip, "127.0.0.1")){
                    // do not log early messages for localhost (to avoid tracing in watchdog)
                    LOG(LOG_INFO, "src=%s sport=%d dst=%s dport=%d", source_ip, source_port, target_ip, target_port);
                }

                char real_target_ip[256];
                if (ini.get<cfg::globals::enable_transparent_mode>() &&
                    (0 != strcmp(source_ip, "127.0.0.1"))) {
                    int fd = open("/proc/net/ip_conntrack", O_RDONLY);
                    // source and dest are inverted because we get the information we want from reply path rule
                    int res = parse_ip_conntrack(fd, target_ip, source_ip, target_port, source_port, real_target_ip, sizeof(real_target_ip), 1);
                    if (res){
                        LOG(LOG_WARNING, "Failed to get transparent proxy target from ip_conntrack: %d", fd);
                    }
                    close(fd);

                    if (setgid(this->gid) != 0){
                        LOG(LOG_ERR, "Changing process group to %u failed with error: %s\n", this->gid, strerror(errno));
                        _exit(1);
                    }
                    if (setuid(this->uid) != 0){
                        LOG(LOG_ERR, "Changing process user to %u failed with error: %s\n", this->uid, strerror(errno));
                        _exit(1);
                    }

                    LOG(LOG_INFO, "src=%s sport=%d dst=%s dport=%d", source_ip, source_port, real_target_ip, target_port);
                }
                else {
                    ::memset(real_target_ip, 0, sizeof(real_target_ip));
                }

                int nodelay = 1;
                if (0 == setsockopt(sck, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay))){
                    // Create session file
                    int child_pid = getpid();
                    char session_file[256];
                    sprintf(session_file, "%s/redemption/session_%d.pid", PID_PATH, child_pid);
                    int fd = open(session_file, O_WRONLY | O_CREAT, S_IRWXU);
                    if (fd == -1) {
                        LOG(LOG_ERR, "Writing process id to SESSION ID FILE failed. Maybe no rights ?:%d:%s\n", errno, strerror(errno));
                        _exit(1);
                    }
                    char text[256];
                    const size_t lg = snprintf(text, 255, "%d", child_pid);
                    if (write(fd, text, lg) == -1) {
                        LOG(LOG_ERR, "Couldn't write pid to %s: %s", PID_PATH "/redemption/session_<pid>.pid", strerror(errno));
                        _exit(1);
                    }
                    close(fd);

                    // Launch session
                    if (0 != strcmp(source_ip, "127.0.0.1")){
                        // do not log early messages for localhost (to avoid tracing in watchdog)
                        LOG(LOG_INFO,
                            "New session on %d (pid=%d) from %s to %s",
                            sck, child_pid, source_ip, (real_target_ip[0] ? real_target_ip : target_ip));
                    }
                    ini.set_acl<cfg::globals::host>(source_ip);
//                    ini.context_set_value(AUTHID_TARGET, real_target_ip);
                    ini.set_acl<cfg::globals::target>(target_ip);
                    if (ini.get<cfg::globals::enable_transparent_mode>()
                        &&  strncmp(target_ip, real_target_ip, strlen(real_target_ip))) {
                        ini.set_acl<cfg::context::real_target_device>(real_target_ip);
                    }
                    Session session(sck, ini, this->cctx);

                    // Suppress session file
                    unlink(session_file);

                    if (ini.get<cfg::debug::session>()){
                        LOG(LOG_INFO, "Session::end of Session(%u)", sck);
                    }

                    shutdown(sck, 2);
                    close(sck);
                }
                else {
                    LOG(LOG_ERR, "Failed to set socket TCP_NODELAY option on client socket");
                }
                _exit(0);
            }
            break;
        default: /* father */
            {
                close(sck);
            }
            break;
        case -1:
            // error forking
            LOG(LOG_ERR, "Error creating process for new session : %s\n", strerror(errno));
            break;
        }
        return START_FAILED;
    }
};

