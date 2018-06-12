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

#pragma once

#include "acl/auth_api.hpp"
#include "acl/mm_api.hpp"
#include "configs/config.hpp"
#include "core/session_reactor.hpp"
#include "front/front.hpp"
#include "gdi/protected_graphics.hpp"

#include "mod/internal/bouncer2_mod.hpp"
#include "mod/internal/client_execute.hpp"
#include "mod/internal/flat_dialog_mod.hpp"
#include "mod/internal/flat_login_mod.hpp"
#include "mod/internal/flat_wab_close_mod.hpp"
#include "mod/internal/flat_wait_mod.hpp"
#include "mod/internal/interactive_target_mod.hpp"
#include "mod/internal/rail_module_host_mod.hpp"
#include "mod/internal/replay_mod.hpp"
#include "mod/internal/selector_mod.hpp"
#include "mod/internal/test_card_mod.hpp"
#include "mod/internal/widget_test_mod.hpp"

#include "mod/mod_api.hpp"
#include "mod/null/null.hpp"
#include "mod/rdp/windowing_api.hpp"
#include "mod/xup/xup.hpp"

#include "transport/socket_transport.hpp"

#include "utils/netutils.hpp"
#include "utils/sugar/scope_exit.hpp"
#include "utils/translation.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


inline Rect get_widget_rect(uint16_t width, uint16_t height,
        GCC::UserData::CSMonitor const & monitors) {
    Rect widget_rect(0, 0, width - 1, height - 1);
    if (monitors.monitorCount) {
        Rect rect                 = monitors.get_rect();
        Rect primary_monitor_rect = monitors.get_primary_monitor_rect();

        widget_rect.x  = abs(rect.x);
        widget_rect.y  = abs(rect.y);
        widget_rect.cx = primary_monitor_rect.cx;
        widget_rect.cy = primary_monitor_rect.cy;
    }

    return widget_rect;
}

#define STRMODULE_LOGIN            "login"
#define STRMODULE_SELECTOR         "selector"
#define STRMODULE_SELECTOR_LEGACY  "selector_legacy"
#define STRMODULE_CONFIRM          "confirm"
#define STRMODULE_CHALLENGE        "challenge"
#define STRMODULE_VALID            "valid"
#define STRMODULE_TRANSITORY       "transitory"
#define STRMODULE_CLOSE            "close"
#define STRMODULE_CLOSE_BACK       "close_back"
#define STRMODULE_CONNECTION       "connection"
#define STRMODULE_TARGET           "interactive_target"
#define STRMODULE_MESSAGE          "message"
#define STRMODULE_RDP              "RDP"
#define STRMODULE_VNC              "VNC"
#define STRMODULE_INTERNAL         "INTERNAL"
#define STRMODULE_WAITINFO         "waitinfo"

// TODO enum class
enum {
    MODULE_EXIT,
    MODULE_WAITING,
    MODULE_RUNNING,
    MODULE_REFRESH,
    MODULE_VNC,
    MODULE_RDP,
    MODULE_XUP,
    MODULE_INTERNAL,
    MODULE_INTERNAL_CLOSE,
    MODULE_INTERNAL_CLOSE_BACK,
    MODULE_INTERNAL_WIDGET_DIALOG,
    MODULE_INTERNAL_WIDGET_MESSAGE,
    MODULE_INTERNAL_WIDGET_LOGIN,
    MODULE_INTERNAL_CARD,
    MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE,
    MODULE_INTERNAL_DIALOG_VALID_MESSAGE,
    MODULE_INTERNAL_DIALOG_CHALLENGE,
    MODULE_INTERNAL_TARGET,
    MODULE_INTERNAL_BOUNCER2,
    MODULE_INTERNAL_TEST,
    MODULE_INTERNAL_WIDGET_SELECTOR,
    MODULE_INTERNAL_WIDGET_SELECTOR_LEGACY,
    MODULE_INTERNAL_WIDGETTEST,
    MODULE_INTERNAL_WAIT_INFO,
    MODULE_EXIT_INTERNAL_CLOSE,
    MODULE_TRANSITORY,
    MODULE_AUTH,
    MODULE_CLI,

    MODULE_UNKNOWN
};

inline const char * get_module_name(int module_id) {
    switch (module_id) {
        case MODULE_EXIT:                               return "MODULE_EXIT";
        case MODULE_WAITING:                            return "MODULE_WAITING";
        case MODULE_RUNNING:                            return "MODULE_RUNNING";
        case MODULE_REFRESH:                            return "MODULE_REFRESH";
        case MODULE_VNC:                                return "MODULE_VNC";
        case MODULE_RDP:                                return "MODULE_RDP";
        case MODULE_XUP:                                return "MODULE_XUP";
        case MODULE_INTERNAL:                           return "MODULE_INTERNAL";
        case MODULE_INTERNAL_CLOSE:                     return "MODULE_INTERNAL_CLOSE";
        case MODULE_INTERNAL_CLOSE_BACK:                return "MODULE_INTERNAL_CLOSE_BACK";
        case MODULE_INTERNAL_WIDGET_DIALOG:             return "MODULE_INTERNAL_WIDGET_DIALOG";
        case MODULE_INTERNAL_WIDGET_MESSAGE:            return "MODULE_INTERNAL_WIDGET_MESSAGE";
        case MODULE_INTERNAL_WIDGET_LOGIN:              return "MODULE_INTERNAL_WIDGET_LOGIN";
        case MODULE_INTERNAL_CARD:                      return "MODULE_INTERNAL_CARD";
        case MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE:    return "MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE";
        case MODULE_INTERNAL_DIALOG_VALID_MESSAGE:      return "MODULE_INTERNAL_DIALOG_VALID_MESSAGE";
        case MODULE_INTERNAL_DIALOG_CHALLENGE:          return "MODULE_INTERNAL_DIALOG_CHALLENGE";
        case MODULE_INTERNAL_TARGET:                    return "MODULE_INTERNAL_TARGET";
        case MODULE_INTERNAL_BOUNCER2:                  return "MODULE_INTERNAL_BOUNCER2";
        case MODULE_INTERNAL_TEST:                      return "MODULE_INTERNAL_TEST";
        case MODULE_INTERNAL_WIDGET_SELECTOR:           return "MODULE_INTERNAL_WIDGET_SELECTOR";
        case MODULE_INTERNAL_WIDGET_SELECTOR_LEGACY:    return "MODULE_INTERNAL_WIDGET_SELECTOR_LEGACY";
        case MODULE_INTERNAL_WIDGETTEST:                return "MODULE_INTERNAL_WIDGETTEST";
        case MODULE_INTERNAL_WAIT_INFO:                 return "MODULE_INTERNAL_WAIT_INFO";
        case MODULE_EXIT_INTERNAL_CLOSE:                return "MODULE_EXIT_INTERNAL_CLOSE";
        case MODULE_TRANSITORY:                         return "MODULE_TRANSITORY";
        case MODULE_AUTH:                               return "MODULE_AUTH";
        case MODULE_CLI:                                return "MODULE_CLI";
    }

    return "<unknown>";
}

class MMIni : public MMApi
{
protected:
    Inifile& ini;
    SessionReactor& session_reactor;

public:
    explicit MMIni(SessionReactor& session_reactor, Inifile & ini_)
    : ini(ini_)
    , session_reactor(session_reactor)
    {}

    void remove_mod() override {}

    void new_mod(int target_module, time_t now, AuthApi &, ReportMessageApi &) override
    {
        LOG(LOG_INFO, "new mod %d at time: %d\n", target_module, static_cast<int>(now));
        switch (target_module) {
        case MODULE_VNC:
        case MODULE_XUP:
        case MODULE_RDP:
            this->connected = true;
            break;
        default:
            break;
        };
    }

    void invoke_close_box(const char * auth_error_message, BackEvent_t & signal,
                          time_t now, AuthApi & authentifier, ReportMessageApi & report_message) override {
        LOG(LOG_INFO, "----------> ACL invoke_close_box <--------");
        this->last_module = true;
        if (auth_error_message) {
            this->ini.set<cfg::context::auth_error_message>(auth_error_message);
        }
        if (this->mod) {
            try {
                this->mod->disconnect(now);
            }
            catch (Error const& e) {
                LOG(LOG_INFO, "MMIni::invoke_close_box exception = %u!\n", e.id);
            }
        }
        this->remove_mod();
        if (this->ini.get<cfg::globals::enable_close_box>()) {
            this->new_mod(MODULE_INTERNAL_CLOSE, now, authentifier, report_message);
            signal = BACK_EVENT_NONE;
        }
        else {
            signal = BACK_EVENT_STOP;
        }
    }

    int next_module() override {
        LOG(LOG_INFO, "----------> ACL next_module <--------");
        auto & module_cstr = this->ini.get<cfg::context::module>();

        if (module_cstr == STRMODULE_LOGIN) {
            LOG(LOG_INFO, "===========> MODULE_LOGIN");
            return MODULE_INTERNAL_WIDGET_LOGIN;
        }
        else if (module_cstr == STRMODULE_SELECTOR) {
            LOG(LOG_INFO, "===============> MODULE_SELECTOR");
            return MODULE_INTERNAL_WIDGET_SELECTOR;
        }
        else if (module_cstr == STRMODULE_SELECTOR_LEGACY) {
            LOG(LOG_INFO, "===============> MODULE_SELECTOR_LEGACY");
            return MODULE_INTERNAL_WIDGET_SELECTOR_LEGACY;
        }
        else if (module_cstr == STRMODULE_CONFIRM) {
            LOG(LOG_INFO, "===============> MODULE_DIALOG_CONFIRM");
            return MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE;
        }
        else if (module_cstr == STRMODULE_CHALLENGE) {
            LOG(LOG_INFO, "===========> MODULE_DIALOG_CHALLENGE");
            return MODULE_INTERNAL_DIALOG_CHALLENGE;
        }
        else if (module_cstr == STRMODULE_VALID) {
            LOG(LOG_INFO, "===========> MODULE_DIALOG_VALID");
            return MODULE_INTERNAL_DIALOG_VALID_MESSAGE;
        }
        else if (module_cstr == STRMODULE_WAITINFO) {
            LOG(LOG_INFO, "===========> MODULE_WAITINFO");
            return MODULE_INTERNAL_WAIT_INFO;
        }
        else if (module_cstr == STRMODULE_TARGET) {
            LOG(LOG_INFO, "===========> MODULE_INTERACTIVE_TARGET");
            return MODULE_INTERNAL_TARGET;
        }
        else if (module_cstr == STRMODULE_TRANSITORY) {
            LOG(LOG_INFO, "===============> WAIT WITH CURRENT MODULE");
            return MODULE_TRANSITORY;
        }
        else if (module_cstr == STRMODULE_CLOSE) {
            LOG(LOG_INFO, "===========> MODULE_INTERNAL_CLOSE (1)");
            return MODULE_INTERNAL_CLOSE;
        }
        else if (module_cstr == STRMODULE_CLOSE_BACK) {
            LOG(LOG_INFO, "===========> MODULE_INTERNAL_CLOSE_BACK");
            return MODULE_INTERNAL_CLOSE_BACK;
        }
        if (this->connected &&
            (module_cstr == STRMODULE_RDP ||
             module_cstr == STRMODULE_VNC)) {
            LOG(LOG_INFO, "===========> MODULE_CLOSE");
            if (this->ini.get<cfg::context::auth_error_message>().empty()) {
                this->ini.set<cfg::context::auth_error_message>(TR(trkeys::end_connection, language(this->ini)));
            }
            return MODULE_INTERNAL_CLOSE;
        }
        else if (module_cstr == STRMODULE_RDP) {
            LOG(LOG_INFO, "===========> MODULE_RDP");
            return MODULE_RDP;
        }
        else if (module_cstr == STRMODULE_VNC) {
            LOG(LOG_INFO, "===========> MODULE_VNC");
            return MODULE_VNC;
        }
        else if (module_cstr == STRMODULE_INTERNAL) {
            int res = MODULE_EXIT;
            auto & target = this->ini.get<cfg::context::target_host>();
            if (target == "bouncer2") {
                LOG(LOG_INFO, "==========> MODULE_INTERNAL bouncer2");
                res = MODULE_INTERNAL_BOUNCER2;
            }
            else if (target == "autotest") {
                LOG(LOG_INFO, "==========> MODULE_INTERNAL test");
                std::string user = this->ini.get<cfg::globals::target_user>();
                if (user.size() < 5 || !std::equal(user.end() - 5u, user.end(), ".mwrm")) {
                    user += ".mwrm";
                }
                this->ini.set<cfg::context::movie>(std::move(user));
                res = MODULE_INTERNAL_TEST;
            }
            else if (target == "widget_message") {
                LOG(LOG_INFO, "==========> MODULE_INTERNAL widget_message");
                res = MODULE_INTERNAL_WIDGET_MESSAGE;
            }
            else if (target == "widgettest") {
                LOG(LOG_INFO, "==========> MODULE_INTERNAL widgettest");
                res = MODULE_INTERNAL_WIDGETTEST;
            }
            else {
                LOG(LOG_INFO, "==========> MODULE_INTERNAL card");
                res = MODULE_INTERNAL_CARD;
            }
            return res;
        }
        LOG(LOG_INFO, "===========> UNKNOWN MODULE");
        return MODULE_INTERNAL_CLOSE;
    }

    void check_module() override {
        if (this->ini.get<cfg::context::forcemodule>() && !this->is_connected()) {
            this->session_reactor.set_event_next(BACK_EVENT_NEXT);
            this->ini.set<cfg::context::forcemodule>(false);
            // Do not send back the value to sesman.
        }
    }
};


inline void add_time_before_closing(std::string & msg, uint32_t elapsed_time, Translator tr)
{
    const auto hours = elapsed_time / (60*60);
    const auto minutes = elapsed_time / 60 - hours * 60;
    const auto seconds = elapsed_time - hours * (60*60) - minutes * 60;
    if (hours) {
        msg += std::to_string(hours);
        msg += ' ';
        msg += tr(trkeys::hour);
        msg += (hours > 1) ? "s, " : ", ";
    }
    if (minutes || hours) {
        msg += std::to_string(minutes);
        msg += ' ';
        msg += tr(trkeys::minute);
        msg += (minutes > 1) ? "s, " : ", ";
    }
    msg += std::to_string(seconds);
    msg += ' ';
    msg += tr(trkeys::second);
    msg += (seconds > 1) ? "s " : " ";
    msg += tr(trkeys::before_closing);
}

class ModuleManager : public MMIni
{
    struct IniAccounts {
        char username[255]; // should use string
        char password[255]; // should use string

        IniAccounts()
        {
            this->username[0] = 0;
            this->password[0] = 0;
        }
    } accounts;

private:
    class ModOSD : public gdi::ProtectedGraphics, public mod_api
    {
        ModuleManager & mm;

        std::string osd_message;
        Rect clip;
        RDPColor color;
        RDPColor background_color;
        bool is_disable_by_input = false;
        bool bogus_refresh_rect_ex;

    public:
        explicit ModOSD(ModuleManager & mm)
        : gdi::ProtectedGraphics(mm.front, Rect{})
        , mm(mm)
        {}

        bool is_input_owner() const { return this->is_disable_by_input; }

        void disable_osd()
        {
            this->is_disable_by_input = false;
            auto const protected_rect = this->get_protected_rect();
            this->set_protected_rect(Rect{});

            if (this->bogus_refresh_rect_ex) {
                this->mm.mod->rdp_suppress_display_updates();
                this->mm.mod->rdp_allow_display_updates(0, 0,
                    this->mm.front.client_info.width, this->mm.front.client_info.height);
            }

            if (this->mm.winapi) {
                this->mm.winapi->destroy_auxiliary_window();
            }

            this->mm.mod->rdp_input_invalidate(protected_rect);
        }

        const char* get_message() const {
            return this->osd_message.c_str();
        }

        void set_message(std::string message, bool is_disable_by_input)
        {
            this->osd_message = std::move(message);
            this->is_disable_by_input = is_disable_by_input;
            this->bogus_refresh_rect_ex = (this->mm.ini.get<cfg::globals::bogus_refresh_rect>()
             && this->mm.ini.get<cfg::globals::allow_using_multiple_monitors>()
             && (this->mm.front.client_info.cs_monitor.monitorCount > 1));

            if (is_disable_by_input) {
                this->osd_message += "  ";
                this->osd_message += TR(trkeys::disable_osd, language(this->mm.ini));
            }

            gdi::TextMetrics tm(this->mm.ini.get<cfg::font>(), this->osd_message.c_str());
            int w = tm.width + padw * 2;
            int h = tm.height + padh * 2;
            this->color = color_encode(BGRColor(BLACK), this->mm.front.client_info.bpp);
            this->background_color = color_encode(BGRColor(LIGHT_YELLOW), this->mm.front.client_info.bpp);

            if (this->mm.front.client_info.remote_program &&
                (this->mm.winapi == static_cast<windowing_api*>(&this->mm.client_execute))) {

                Rect current_work_area_rect = this->mm.client_execute.get_current_work_area_rect();

                this->clip = Rect(
                    current_work_area_rect.x +
                        (current_work_area_rect.cx < w ? 0 : (current_work_area_rect.cx - w) / 2),
                    0, w, h);
            }
            else {
                this->clip = Rect(this->mm.front.client_info.width < w ? 0 : (this->mm.front.client_info.width - w) / 2, 0, w, h);
            }

            this->set_protected_rect(this->clip);

            if (this->mm.winapi) {
                this->mm.winapi->create_auxiliary_window(this->clip);
            }
        }

        static constexpr int padw = 16;
        static constexpr int padh = 16;

        void draw_osd_message()
        {
            this->mm.front.begin_update();
            this->draw_osd_message_impl(this->mm.front);
            this->mm.front.end_update();
        }

        bool try_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap)
        {
            (void)param1;
            (void)param2;
            (void)param3;
            (void)param4;
            if (this->is_disable_by_input
             && keymap->nb_kevent_available() > 0
//             && !(param3 & SlowPath::KBDFLAGS_DOWN)
             && keymap->top_kevent() == Keymap2::KEVENT_INSERT
            ) {
                keymap->get_kevent();
                this->disable_osd();
                return true;
            }
            return false;
        }

        bool try_input_mouse(int device_flags, int x, int y, Keymap2 *)
        {
            if (this->is_disable_by_input
             && this->get_protected_rect().contains_pt(x, y)
             && device_flags == (MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN)) {
                this->disable_osd();
                return true;
            }
            return false;
        }

        bool try_input_invalidate(const Rect r)
        {
            if (!this->get_protected_rect().isempty() && r.has_intersection(this->get_protected_rect())) {
                auto rects = gdi::subrect4(r, this->get_protected_rect());
                auto p = std::begin(rects);
                auto e = std::remove_if(p, std::end(rects), [](Rect const & rect) {
                    return rect.isempty();
                });
                if (p != e) {
                    this->mm.mod->rdp_input_invalidate2({p, e});
                    this->clip = r.intersect(this->get_protected_rect());
                }
                return true;
            }
            return false;
        }

        bool try_input_invalidate2(array_view<Rect const> vr)
        {
            // TODO PERF multi opaque rect
            bool ret = false;
            for (Rect const & r : vr) {
                if (!this->try_input_invalidate(r)) {
                    this->mm.mod->rdp_input_invalidate(r);
                }
                else {
                    ret = true;
                }
            }
            return ret;
        }

    private:
        void draw_event(time_t now, gdi::GraphicApi & drawable) override
        {
            drawable.begin_update();
            this->draw_osd_message_impl(drawable);
            this->mm.mod->draw_event(now, drawable);
            drawable.end_update();
        }

        void draw_osd_message_impl(gdi::GraphicApi & drawable)
        {
            if (this->clip.isempty()) {
                return ;
            }

            auto const color_ctx = gdi::ColorCtx::from_bpp(this->mm.front.client_info.bpp, this->mm.front.get_palette());

            drawable.draw(RDPOpaqueRect(this->clip, this->background_color), this->clip, color_ctx);

            RDPLineTo line_left(1, this->clip.x, this->clip.y, this->clip.x, this->clip.y + this->clip.cy - 1,
                encode_color24()(BLACK), 0x0D, RDPPen(0, 0, encode_color24()(BLACK)));
            drawable.draw(line_left, this->clip, color_ctx);
            RDPLineTo line_bottom(1, this->clip.x, this->clip.y + this->clip.cy - 1, this->clip.x + this->clip.cx - 1, this->clip.y + this->clip.cy - 1,
                encode_color24()(BLACK), 0x0D, RDPPen(0, 0, encode_color24()(BLACK)));
            drawable.draw(line_bottom, this->clip, color_ctx);

            RDPLineTo line_right(1, this->clip.x + this->clip.cx - 1, this->clip.y + this->clip.cy - 1, this->clip.x + this->clip.cx - 1, this->clip.y,
                encode_color24()(BLACK), 0x0D, RDPPen(0, 0, encode_color24()(BLACK)));
            drawable.draw(line_right, this->clip, color_ctx);
            RDPLineTo line_top(1, this->clip.x + this->clip.cx - 1, this->clip.y, this->clip.x, this->clip.y,
                encode_color24()(BLACK), 0x0D, RDPPen(0, 0, encode_color24()(BLACK)));
            drawable.draw(line_top, this->clip, color_ctx);

            gdi::server_draw_text(
                drawable, this->mm.ini.get<cfg::font>(),
                this->get_protected_rect().x + padw, padh,
                this->osd_message.c_str(),
                this->color, this->background_color, color_ctx, this->clip
            );

            this->clip = Rect();
        }

        void refresh_rects(array_view<Rect const> av) override
        {
            this->mm.mod->rdp_input_invalidate2(av);
        }

        void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap) override
        {
            if (!this->try_input_scancode(param1, param2, param3, param4, keymap)) {
                this->mm.mod->rdp_input_scancode(param1, param2, param3, param4, keymap);
            }
        }

        void rdp_input_unicode(uint16_t unicode, uint16_t flag) override {
            this->mm.mod->rdp_input_unicode(unicode, flag);
        }

        void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override
        {
            if (!this->try_input_mouse(device_flags, x, y, keymap)) {
                this->mm.mod->rdp_input_mouse(device_flags, x, y, keymap);
            }
        }

        void rdp_input_invalidate(Rect r) override
        {
            if (!this->try_input_invalidate(r)) {
                this->mm.mod->rdp_input_invalidate(r);
            }
        }

        void rdp_input_invalidate2(array_view<Rect const> vr) override
        {
            if (!this->try_input_invalidate2(vr)) {
                this->mm.mod->rdp_input_invalidate2(vr);
            }
        }

        void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override
        { this->mm.mod->rdp_input_synchronize(time, device_flags, param1, param2); }

        void rdp_input_up_and_running() override
        { this->mm.mod->rdp_input_up_and_running(); }

        void rdp_allow_display_updates(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom) override
        { this->mm.mod->rdp_allow_display_updates(left, top, right, bottom); }

        void rdp_suppress_display_updates() override
        { this->mm.mod->rdp_suppress_display_updates(); }

        void refresh(Rect r) override
        {
            this->mm.mod->refresh(r);
        }

        void send_to_mod_channel(
            CHANNELS::ChannelNameId front_channel_name, InStream & chunk,
            std::size_t length, uint32_t flags
        ) override
        { this->mm.mod->send_to_mod_channel(front_channel_name, chunk, length, flags); }

        void send_auth_channel_data(const char * data) override
        { this->mm.mod->send_auth_channel_data(data); }

        void send_to_front_channel(CHANNELS::ChannelNameId mod_channel_name,
            uint8_t const * data, size_t length, size_t chunk_size, int flags) override
        { this->mm.mod->send_to_front_channel(mod_channel_name, data, length, chunk_size, flags); }

        void refresh_context() override
        { this->mm.mod->refresh_context(); }

        bool is_up_and_running() override
        { return this->mm.mod->is_up_and_running(); }

        void disconnect(time_t now) override
        { this->mm.mod->disconnect(now); }

        void display_osd_message(std::string const & message) override
        { this->mm.mod->display_osd_message(message); }

        Dimension get_dim() const override
        {
            return this->mm.mod->get_dim();
        }
    };

public:
    gdi::GraphicApi & get_graphic_wrapper()
    {
        gdi::GraphicApi& gd = this->mod_osd.get_protected_rect().isempty()
          ? static_cast<gdi::GraphicApi&>(this->front) : this->mod_osd;
        if (this->rail_module_host_mod_ptr) {
            return this->rail_module_host_mod_ptr->proxy_gd(gd);
        }
        return gd;
    }

    Callback & get_callback() noexcept
    {
        return *this->mod;
    }

private:
    struct sock_mod_barrier {};
    template<class Mod>
    class ModWithSocket final : private SocketTransport, public Mod
    {
        ModuleManager & mm;
        bool target_info_is_shown = false;

    public:
        template<class... Args>
        ModWithSocket(
            ModuleManager & mm, AuthApi & /*authentifier*/,
            const char * name, unique_fd sck, uint32_t verbose,
            std::string * error_message, sock_mod_barrier, Args && ... mod_args)
        : SocketTransport( name, std::move(sck)
                         , mm.ini.get<cfg::context::target_host>().c_str()
                         , mm.ini.get<cfg::context::target_port>()
                         , std::chrono::milliseconds(mm.ini.get<cfg::globals::mod_recv_timeout>())
                         , to_verbose_flags(verbose), error_message)
        , Mod(*this, std::forward<Args>(mod_args)...)
        , mm(mm)
        {
            this->mm.socket_transport = this;
        }

        ~ModWithSocket()
        {
            this->mm.socket_transport = nullptr;
        }

        void display_osd_message(std::string const & message) override {
            this->mm.osd_message(message, true);
        }

        void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap) override
        {
            //LOG(LOG_INFO, "mod_osd::rdp_input_scancode: keyCode=0x%X keyboardFlags=0x%04X this=<%p>", param1, param3, this);
            if (this->mm.mod_osd.try_input_scancode(param1, param2, param3, param4, keymap)) {
                this->target_info_is_shown = false;
                return ;
            }

            Mod::rdp_input_scancode(param1, param2, param3, param4, keymap);

            if (this->mm.ini.template get<cfg::globals::enable_osd_display_remote_target>() && (param1 == Keymap2::F12)) {
                bool const f12_released = (param3 & SlowPath::KBDFLAGS_RELEASE);
                if (this->target_info_is_shown && f12_released) {
                    // LOG(LOG_INFO, "Hide info");
                    this->mm.clear_osd_message();
                    this->target_info_is_shown = false;
                }
                else if (!this->target_info_is_shown && !f12_released) {
                    // LOG(LOG_INFO, "Show info");
                    std::string msg;
                    msg.reserve(64);
                    if (this->mm.ini.template get<cfg::client::show_target_user_in_f12_message>()) {
                        msg  = this->mm.ini.template get<cfg::globals::target_user>();
                        msg += "@";
                    }
                    msg += this->mm.ini.template get<cfg::globals::target_device>();
                    const uint32_t enddate = this->mm.ini.template get<cfg::context::end_date_cnx>();
                    if (enddate) {
                        const auto now = time(nullptr);
                        const auto elapsed_time = enddate - now;
                        // only if "reasonable" time
                        if (elapsed_time < 60*60*24*366L) {
                            msg += "  [";
                            add_time_before_closing(msg, elapsed_time, Translator(this->mm.ini));
                            msg += ']';
                        }
                    }
                    this->mm.osd_message(std::move(msg), false);
                    this->target_info_is_shown = true;
                }
            }
        }

        void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override
        {
            if (this->mm.mod_osd.try_input_mouse(device_flags, x, y, keymap)) {
                this->target_info_is_shown = false;
                return ;
            }

            Mod::rdp_input_mouse(device_flags, x, y, keymap);
        }

        void rdp_input_unicode(uint16_t unicode, uint16_t flag) override {
            Mod::rdp_input_unicode(unicode, flag);
        }

        void rdp_input_invalidate(const Rect r) override
        {
            if (this->mm.mod_osd.try_input_invalidate(r)) {
                return ;
            }

            Mod::rdp_input_invalidate(r);
        }

        void rdp_input_invalidate2(array_view<Rect const> vr) override
        {
            if (this->mm.mod_osd.try_input_invalidate2(vr)) {
                return ;
            }

            Mod::rdp_input_invalidate2(vr);
        }
    };

public:
    void clear_osd_message()
    {
        if (!this->mod_osd.get_protected_rect().isempty()) {
            this->mod_osd.disable_osd();
        }
    }

    void osd_message(std::string message, bool is_disable_by_input)
    {
        if (message.compare(this->mod_osd.get_message())) {
            this->clear_osd_message();
        }
        if (!message.empty()) {
            this->mod_osd.set_message(std::move(message), is_disable_by_input);
            this->mod_osd.draw_osd_message();
        }
    }

private:
    RailModuleHostMod* rail_module_host_mod_ptr = nullptr;
    Front & front;
    null_mod no_mod;
    ModOSD mod_osd;
    Random & gen;
    TimeObj & timeobj;

    ClientExecute client_execute;

    int old_target_module = MODULE_UNKNOWN;

public:
    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        new_mod = 0x1,
    };

private:
    rdp_api*       rdpapi = nullptr;
    windowing_api* winapi = nullptr;

    SocketTransport * socket_transport = nullptr;

public:
    ModuleManager(SessionReactor& session_reactor, Front & front, Inifile & ini, Random & gen, TimeObj & timeobj)
        : MMIni(session_reactor, ini)
        , front(front)
        , no_mod()
        , mod_osd(*this)
        , gen(gen)
        , timeobj(timeobj)
        , client_execute(session_reactor, front, this->front.client_info.window_list_caps,
                         ini.get<cfg::debug::mod_internal>() & 1)
        , verbose(static_cast<Verbose>(ini.get<cfg::debug::auth>()))
    {
        this->mod = &this->no_mod;
    }

    bool has_pending_data() const
    {
        return this->socket_transport && this->socket_transport->has_pending_data();
    }

    void remove_mod() override {
        if (this->mod != &this->no_mod) {
            this->clear_osd_message();

            delete this->mod;
            this->mod = &this->no_mod;
            this->rdpapi = nullptr;
            this->winapi = nullptr;
            this->rail_module_host_mod_ptr = nullptr;
        }
    }

    ~ModuleManager() override {
        this->remove_mod();
    }

private:
    void set_mod(not_null_ptr<mod_api> mod, rdp_api* rdpapi = nullptr, windowing_api* winapi = nullptr)
    {
        while (this->front.keymap.nb_char_available())
            this->front.keymap.get_char();
        while (this->front.keymap.nb_kevent_available())
            this->front.keymap.get_kevent();

        this->clear_osd_message();

        this->mod = mod.get();
        this->rail_module_host_mod_ptr = nullptr;

        this->rdpapi = rdpapi;
        this->winapi = winapi;
    }

public:
    void new_mod(int target_module, time_t now, AuthApi & authentifier, ReportMessageApi & report_message) override
    {
        LOG(LOG_INFO, "----------> ACL new_mod <--------");
        LOG(LOG_INFO, "target_module=%s(%d)", get_module_name(target_module), target_module);

        this->client_execute.enable_remote_program(this->front.client_info.remote_program);

        this->connected = false;

        if (this->old_target_module != target_module) {
            this->front.must_be_stop_capture();

            auto is_remote_mod = [](int mod_type){
                return
                    (mod_type == MODULE_XUP)
                 || (mod_type == MODULE_RDP)
                 || (mod_type == MODULE_VNC);
            };

            if (is_remote_mod(this->old_target_module)) {
                authentifier.delete_remote_mod();
            }

            if (is_remote_mod(target_module)) {
                authentifier.new_remote_mod();
            }
        }
        this->old_target_module = target_module;

        switch (target_module)
        {
        case MODULE_INTERNAL_BOUNCER2:
            LOG(LOG_INFO, "ModuleManager::Creation of internal module 'bouncer2_mod'");
            this->set_mod(new Bouncer2Mod(
                this->session_reactor,
                this->front,
                this->front.client_info.width,
                this->front.client_info.height,
                this->ini.get<cfg::font>()
            ));
            if (bool(this->verbose & Verbose::new_mod)) {
                LOG(LOG_INFO, "ModuleManager::internal module 'bouncer2_mod' ready");
            }
            break;
        case MODULE_INTERNAL_TEST:
            LOG(LOG_INFO, "ModuleManager::Creation of internal module 'test'");
            this->set_mod(new ReplayMod(
                this->session_reactor,
                this->front,
                this->ini.get<cfg::video::replay_path>().c_str(),
                this->ini.get<cfg::context::movie>().c_str(),
                this->front.client_info.width,
                this->front.client_info.height,
                this->ini.get_ref<cfg::context::auth_error_message>(),
                !this->ini.get<cfg::mod_replay::on_end_of_data>(),
                this->ini.get<cfg::mod_replay::replay_on_loop>(),
                to_verbose_flags(this->ini.get<cfg::debug::capture>())
            ));
            if (bool(this->verbose & Verbose::new_mod)) {
                LOG(LOG_INFO, "ModuleManager::internal module 'test' ready");
            }
            break;
        case MODULE_INTERNAL_WIDGETTEST:
            {
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'widgettest'");
                this->set_mod(new WidgetTestMod(
                    this->session_reactor,
                    this->front,
                    this->front.client_info.width,
                    this->front.client_info.height,
                    this->ini.get<cfg::font>()
                ));
                LOG(LOG_INFO, "ModuleManager::internal module 'widgettest' ready");
            }
            break;
        case MODULE_INTERNAL_CARD:
            LOG(LOG_INFO, "ModuleManager::Creation of internal module 'test_card'");
            this->set_mod(new TestCardMod(
                this->session_reactor,
                this->front,
                this->front.client_info.width,
                this->front.client_info.height,
                this->ini.get<cfg::font>(),
                false
            ));
            LOG(LOG_INFO, "ModuleManager::internal module 'test_card' ready");
            break;
        case MODULE_INTERNAL_WIDGET_SELECTOR:
        case MODULE_INTERNAL_WIDGET_SELECTOR_LEGACY:
            LOG(LOG_INFO, "ModuleManager::Creation of internal module 'selector'");
            if (report_message.get_inactivity_timeout() != this->ini.get<cfg::globals::session_timeout>().count()) {
                report_message.update_inactivity_timeout();
            }
            {

            this->set_mod(new SelectorMod(
                this->ini,
                this->session_reactor,
                this->front,
                this->front.client_info.width,
                this->front.client_info.height,
                this->client_execute.adjust_rect(get_widget_rect(
                    this->front.client_info.width,
                    this->front.client_info.height,
                    this->front.client_info.cs_monitor
                )),
                this->client_execute
            ));
            //if (bool(this->verbose & Verbose::new_mod)) {
                LOG(LOG_INFO, "ModuleManager::internal module 'selector' ready");
            //}
            }
            break;
        case MODULE_INTERNAL_CLOSE:
            {
                LOG(LOG_INFO, "ModuleManager::Creation of new mod 'INTERNAL::Close'");
                if (this->ini.get<cfg::context::auth_error_message>().empty()) {
                    this->ini.set<cfg::context::auth_error_message>(TR(trkeys::connection_ended, language(this->ini)));
                }
                this->set_mod(new FlatWabCloseMod(
                    this->ini,
                    this->session_reactor,
                    this->front,
                    this->front.client_info.width,
                    this->front.client_info.height,
                    this->client_execute.adjust_rect(get_widget_rect(
                        this->front.client_info.width,
                        this->front.client_info.height,
                        this->front.client_info.cs_monitor
                    )),
                    now,
                    this->client_execute,
//                     true,
                    true
                ));
            }
            LOG(LOG_INFO, "ModuleManager::internal module Close ready");
            break;
        case MODULE_INTERNAL_CLOSE_BACK:
            {
                if (this->ini.get<cfg::context::auth_error_message>().empty()) {
                    this->ini.set<cfg::context::auth_error_message>(TR(trkeys::connection_ended, language(this->ini)));
                }
                LOG(LOG_INFO, "ModuleManager::Creation of new mod 'INTERNAL::CloseBack'");
                this->set_mod(new FlatWabCloseMod(
                    this->ini,
                    this->session_reactor,
                    this->front,
                    this->front.client_info.width,
                    this->front.client_info.height,
                    this->client_execute.adjust_rect(get_widget_rect(
                        this->front.client_info.width,
                        this->front.client_info.height,
                        this->front.client_info.cs_monitor
                    )),
                    now,
                    this->client_execute,
                    true,
                    true
                ));
            }
            LOG(LOG_INFO, "ModuleManager::internal module Close Back ready");
            break;
        case MODULE_INTERNAL_TARGET:
            {
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Interactive Target'");
                this->set_mod(new InteractiveTargetMod(
                    this->ini,
                    this->session_reactor,
                    this->front,
                    this->front.client_info.width,
                    this->front.client_info.height,
                    this->client_execute.adjust_rect(get_widget_rect(
                        this->front.client_info.width,
                        this->front.client_info.height,
                        this->front.client_info.cs_monitor
                    )),
                    this->client_execute
                ));
                LOG(LOG_INFO, "ModuleManager::internal module 'Interactive Target' ready");
            }
            break;
        case MODULE_INTERNAL_DIALOG_VALID_MESSAGE:
        case MODULE_INTERNAL_WIDGET_DIALOG:
            {
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Dialog Accept Message'");
                const char * message = this->ini.get<cfg::context::message>().c_str();
                const char * button = TR(trkeys::refused, language(this->ini));
                const char * caption = "Information";
                this->set_mod(new FlatDialogMod(
                    this->ini,
                    this->session_reactor,
                    this->front,
                    this->front.client_info.width,
                    this->front.client_info.height,
                    this->client_execute.adjust_rect(get_widget_rect(
                        this->front.client_info.width,
                        this->front.client_info.height,
                        this->front.client_info.cs_monitor
                    )),
                    caption,
                    message,
                    button,
                    now,
                    this->client_execute
                ));
                LOG(LOG_INFO, "ModuleManager::internal module 'Dialog Accept Message' ready");
            }
            break;
        case MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE:
        case MODULE_INTERNAL_WIDGET_MESSAGE:
            {
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Dialog Display Message'");
                const char * message = this->ini.get<cfg::context::message>().c_str();
                const char * button = nullptr;
                const char * caption = "Information";
                this->set_mod(new FlatDialogMod(
                    this->ini,
                    this->session_reactor,
                    this->front,
                    this->front.client_info.width,
                    this->front.client_info.height,
                    this->client_execute.adjust_rect(get_widget_rect(
                        this->front.client_info.width,
                        this->front.client_info.height,
                        this->front.client_info.cs_monitor
                    )),
                    caption,
                    message,
                    button,
                    now,
                    this->client_execute
                ));
                LOG(LOG_INFO, "ModuleManager::internal module 'Dialog Display Message' ready");
            }
            break;
        case MODULE_INTERNAL_DIALOG_CHALLENGE:
            {
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Dialog Challenge'");
                const char * message = this->ini.get<cfg::context::message>().c_str();
                const char * button = nullptr;
                const char * caption = "Challenge";
                ChallengeOpt challenge = CHALLENGE_HIDE;
                if (this->ini.get<cfg::context::authentication_challenge>()) {
                    challenge = CHALLENGE_ECHO;
                }
                this->ini.ask<cfg::context::authentication_challenge>();
                this->ini.ask<cfg::context::password>();
                this->set_mod(new FlatDialogMod(
                    this->ini,
                    this->session_reactor,
                    this->front,
                    this->front.client_info.width,
                    this->front.client_info.height,
                    this->client_execute.adjust_rect(get_widget_rect(
                        this->front.client_info.width,
                        this->front.client_info.height,
                        this->front.client_info.cs_monitor
                    )),
                    caption,
                    message,
                    button,
                    now,
                    this->client_execute,
                    challenge
                ));
                LOG(LOG_INFO, "ModuleManager::internal module 'Dialog Challenge' ready");
            }

            break;
        case MODULE_INTERNAL_WAIT_INFO:
            {
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Wait Info Message'");
                const char * message = this->ini.get<cfg::context::message>().c_str();
                const char * caption = TR(trkeys::information, language(this->ini));
                bool showform = this->ini.get<cfg::context::showform>();
                uint flag = this->ini.get<cfg::context::formflag>();
                this->set_mod(new FlatWaitMod(
                    this->ini,
                    this->session_reactor,
                    this->front,
                    this->front.client_info.width,
                    this->front.client_info.height,
                    this->client_execute.adjust_rect(get_widget_rect(
                        this->front.client_info.width,
                        this->front.client_info.height,
                        this->front.client_info.cs_monitor
                    )),
                    caption,
                    message,
                    now,
                    this->client_execute,
                    showform,
                    flag
                ));
                LOG(LOG_INFO, "ModuleManager::internal module 'Wait Info Message' ready");
            }
            break;
        case MODULE_INTERNAL_WIDGET_LOGIN:
            LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Login'");
            if (this->ini.is_asked<cfg::globals::target_user>()
                ||  this->ini.is_asked<cfg::globals::target_device>()){
                if (this->ini.is_asked<cfg::globals::auth_user>()){
                    accounts.username[0] = 0;
                }
                else {
                    strncpy(accounts.username,
                            this->ini.get<cfg::globals::auth_user>().c_str(),
                            sizeof(accounts.username));
                    accounts.username[sizeof(accounts.username) - 1] = 0;
                }
            }
            else if (this->ini.is_asked<cfg::globals::auth_user>()) {
                accounts.username[0] = 0;
            }
            else {
                // TODO check this! Assembling parts to get user login with target is not obvious method used below il likely to show @: if target fields are empty
                char buffer[255];
                snprintf( buffer, sizeof(buffer), "%s@%s:%s%s%s"
                        , this->ini.get<cfg::globals::target_user>().c_str()
                        , this->ini.get<cfg::globals::target_device>().c_str()
                        , this->ini.get<cfg::context::target_protocol>().c_str()
                        , (!this->ini.get<cfg::context::target_protocol>().empty() ? ":" : "")
                        , this->ini.get<cfg::globals::auth_user>().c_str()
                        );
                strcpy(accounts.username, buffer);
            }

            this->set_mod(new FlatLoginMod(
                this->ini,
                this->session_reactor,
                accounts.username,
                accounts.password,
                this->front,
                this->front.client_info.width,
                this->front.client_info.height,
                this->client_execute.adjust_rect(get_widget_rect(
                    this->front.client_info.width,
                    this->front.client_info.height,
                    this->front.client_info.cs_monitor
                )),
                now,
                this->client_execute
            ));
            LOG(LOG_INFO, "ModuleManager::internal module Login ready");
            break;

        case MODULE_XUP:
            {
                const char * name = "XUP Target";
                if (bool(this->verbose & Verbose::new_mod)) {
                    LOG(LOG_INFO, "ModuleManager::Creation of new mod 'XUP'\n");
                }

                unique_fd client_sck = this->connect_to_target_host(
                    report_message, trkeys::authentification_x_fail);

                this->set_mod(new ModWithSocket<xup_mod>(
                    *this,
                    authentifier,
                    name,
                    std::move(client_sck),
                    this->ini.get<cfg::debug::mod_xup>(),
                    nullptr,
                    sock_mod_barrier(),
                    this->session_reactor,
                    this->front,
                    this->front.client_info.width,
                    this->front.client_info.height,
                    this->ini.get<cfg::context::opt_width>(),
                    this->ini.get<cfg::context::opt_height>(),
                    this->ini.get<cfg::context::opt_bpp>()
                    // TODO: shouldn't alls mods have access to sesman authentifier ?
                ));

                this->ini.get_ref<cfg::context::auth_error_message>().clear();
                LOG(LOG_INFO, "ModuleManager::Creation of new mod 'XUP' suceeded\n");
                this->connected = true;
            }
            break;

        case MODULE_RDP:
            this->create_mod_rdp(
                authentifier, report_message, this->ini,
                this->front, this->front.client_info,
                this->client_execute, this->front.keymap.key_flags);
            break;

        case MODULE_VNC:
            this->create_mod_vnc(
                authentifier, report_message, this->ini,
                this->front, this->front.client_info,
                this->client_execute, this->front.keymap.key_flags);
            break;

        default:
            {
                LOG(LOG_INFO, "ModuleManager::Unknown backend exception");
                throw Error(ERR_SESSION_UNKNOWN_BACKEND);
            }
        }
    }

    rdp_api* get_rdp_api() const override {
        return this->rdpapi;
    }

private:
    unique_fd connect_to_target_host(ReportMessageApi& report_message, trkeys::TrKey const& authentification_fail)
    {
        const char * ip = this->ini.get<cfg::context::target_host>().c_str();
        char ip_addr[256] {};
        in_addr s4_sin_addr;
        int status = resolve_ipv4_address(ip, s4_sin_addr);
        if (status){
            ArcsightLogInfo arc_info;
            arc_info.ApplicationProtocol = "xup";
            arc_info.name = "CONNECTION";
            arc_info.WallixBastionStatus = "FAIL";

            report_message.log6("type=\"CONNECTION_FAILED\"", arc_info);

            this->ini.set<cfg::context::auth_error_message>(TR(trkeys::target_fail, language(this->ini)));
            // TODO: actually this is DNS Failure or invalid address
            LOG(LOG_ERR, "Failed to connect to remote TCP host (1)");
            throw Error(ERR_SOCKET_CONNECT_FAILED);
        }

        snprintf(ip_addr, sizeof(ip_addr), "%s", inet_ntoa(s4_sin_addr));

        unique_fd client_sck = ip_connect(ip, this->ini.get<cfg::context::target_port>(), 3, 1000);

        if (!client_sck.is_open()){
            ArcsightLogInfo arc_info;
            arc_info.ApplicationProtocol = "xup";
            arc_info.name = "CONNECTION";
            arc_info.WallixBastionStatus = "FAIL";

            report_message.log6("type=\"CONNECTION_FAILED\"", arc_info);

            this->ini.set<cfg::context::auth_error_message>(TR(trkeys::target_fail, language(this->ini)));
            LOG(LOG_ERR, "Failed to connect to remote TCP host (2)");
            throw Error(ERR_SOCKET_CONNECT_FAILED);
        }

        this->ini.set<cfg::context::auth_error_message>(TR(authentification_fail, language(this->ini)));
        this->ini.set<cfg::context::ip_target>(ip_addr);

        return client_sck;
    }

    void create_mod_rdp(
        AuthApi& authentifier, ReportMessageApi& report_message,
        Inifile& ini, FrontAPI& front, ClientInfo const& client_info,
        ClientExecute& client_execute, Keymap2::KeyFlags key_flags);

    void create_mod_vnc(
        AuthApi& authentifier, ReportMessageApi& report_message,
        Inifile& ini, FrontAPI& front, ClientInfo const& client_info,
        ClientExecute& client_execute, Keymap2::KeyFlags key_flags);
};
