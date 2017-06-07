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

#include "core/RDP/bitmapupdate.hpp"
#include "core/RDP/gcc/userdata/cs_monitor.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "gdi/graphic_api.hpp"
#include "mod/internal/client_execute.hpp"
#include "mod/internal/widget2/composite.hpp"
#include "mod/internal/widget2/scroll.hpp"
#include "mod/mod_api.hpp"

#include <type_traits>

class WidgetModuleHost : public WidgetParent, public gdi::GraphicApi
{
private:
    class ModuleHolder : public mod_api
    {
    private:
        WidgetModuleHost& host;

        std::unique_ptr<mod_api> managed_mod;

    public:
        ModuleHolder(WidgetModuleHost& host,
                     std::unique_ptr<mod_api> managed_mod)
        : host(host)
        , managed_mod(std::move(managed_mod))
        {
            REDASSERT(this->managed_mod);
        }

        // Callback
        void send_to_mod_channel(const char* front_channel_name,
                                 InStream& chunk, size_t length,
                                 uint32_t flags) override
        {
            if (this->managed_mod)
            {
                return this->managed_mod->send_to_mod_channel(
                        front_channel_name,
                        chunk,
                        length,
                        flags
                    );
            }
        }

        // mod_api

        void draw_event(time_t now, gdi::GraphicApi& drawable) override
        {
            if (this->managed_mod)
            {
                this->host.drawable_ptr = &drawable;

                this->managed_mod->draw_event(now, this->host);

                this->host.drawable_ptr = nullptr;
            }
        }

        wait_obj& get_event() override
        {
            if (this->managed_mod)
            {
                return this->managed_mod->get_event();
            }

            return mod_api::get_event();
        }

        int get_fd() const override
        {
            if (this->managed_mod)
            {
                return this->managed_mod->get_fd();
            }

            return INVALID_SOCKET;
        }

        void get_event_handlers(std::vector<EventHandler>& out_event_handlers)
            override
        {
            if (this->managed_mod)
            {
                this->managed_mod->get_event_handlers(out_event_handlers);
            }
        }

        bool is_up_and_running() override
        {
            if (this->managed_mod)
            {
                return this->managed_mod->is_up_and_running();
            }

            return mod_api::is_up_and_running();
        }

        bool is_auto_reconnectable() override {
            if (this->managed_mod)
            {
                return this->managed_mod->is_auto_reconnectable();
            }

            return false;
        }

        void send_to_front_channel(const char* const mod_channel_name,
                                   const uint8_t* data, size_t length,
                                   size_t chunk_size, int flags) override
        {
            if (this->managed_mod)
            {
                this->managed_mod->send_to_front_channel(mod_channel_name,
                    data, length, chunk_size, flags);
            }
        }

        // RdpInput

        void rdp_input_invalidate(Rect r) override
        {
            if (this->managed_mod)
            {
                this->managed_mod->rdp_input_invalidate(r);
            }
        }

        void rdp_input_mouse(int device_flags, int x, int y,
                             Keymap2* keymap) override
        {
            if (this->managed_mod)
            {
                this->managed_mod->rdp_input_mouse(device_flags, x, y,
                    keymap);
            }
        }

        void rdp_input_scancode(long param1, long param2, long param3,
                                long param4, Keymap2* keymap) override
        {
            if (this->managed_mod)
            {
                this->managed_mod->rdp_input_scancode(param1, param2, param3,
                    param4, keymap);
            }
        }

        void rdp_input_synchronize(uint32_t time, uint16_t device_flags,
                                   int16_t param1, int16_t param2) override
        {
            if (this->managed_mod)
            {
                this->managed_mod->rdp_input_synchronize(time, device_flags,
                    param1, param2);
            }
        }

        void rdp_input_up_and_running() override
        {
            if (this->managed_mod)
            {
                this->managed_mod->rdp_input_up_and_running();
            }
        }

        void refresh(Rect r) override
        {
            if (this->managed_mod)
            {
                this->managed_mod->refresh(r);
            }
        }

        Dimension get_dim() const override
        {
            if (this->managed_mod)
            {
                return this->managed_mod->get_dim();
            }

            return mod_api::get_dim();
        }
    } module_holder;

    CompositeArray composite_array;

    gdi::GraphicApi* drawable_ptr = nullptr;

    gdi::GraphicApi& drawable_ref;

    WidgetScrollBar hscroll;
    WidgetScrollBar vscroll;

    unsigned int hscroll_height = 0;
    unsigned int vscroll_width  = 0;

    bool hscroll_added = false;
    bool vscroll_added = false;

    Rect mod_visible_rect;

    Rect vision_rect;

    GCC::UserData::CSMonitor monitors;

    GCC::UserData::CSMonitor monitor_one;

    Pointer current_pointer;

    int current_pointer_pos_x = 0;
    int current_pointer_pos_y = 0;

public:
    void draw(RDP::FrameMarker    const & cmd) override { this->draw_impl(cmd); }
    void draw(RDPDestBlt          const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
    void draw(RDPMultiDstBlt      const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
    void draw(RDPPatBlt           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPOpaqueRect       const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPMultiOpaqueRect  const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPScrBlt           const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
    void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
    void draw(RDPLineTo           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPPolygonSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPPolygonCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPPolyline         const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPEllipseSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPEllipseCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) override { this->draw_impl(cmd, bmp); }
    void draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) override { this->draw_impl(cmd, clip, bmp);}
    void draw(RDPMem3Blt          const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) override { this->draw_impl(cmd, clip, color_ctx, bmp); }
    void draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache) override { this->draw_impl(cmd, clip, color_ctx, gly_cache); }

    void draw(const RDP::RAIL::NewOrExistingWindow            & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::WindowIcon                     & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::CachedIcon                     & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::DeletedWindow                  & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::NewOrExistingNotificationIcons & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::DeletedNotificationIcons       & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::ActivelyMonitoredDesktop       & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::NonMonitoredDesktop            & cmd) override { this->draw_impl(cmd); }

    void draw(RDPColCache   const & cmd) override { this->draw_impl(cmd); }
    void draw(RDPBrushCache const & cmd) override { this->draw_impl(cmd); }

    virtual void set_pointer(Pointer const & pointer) override {
        gdi::GraphicApi& drawable_ = this->get_drawable();

        Rect rect = this->get_rect();
        rect.x  += (BORDER_WIDTH_HEIGHT - 1);
        rect.cx -= (BORDER_WIDTH_HEIGHT - 1) * 2;
        rect.cy -= (BORDER_WIDTH_HEIGHT - 1);

        if (rect.contains_pt(this->current_pointer_pos_x, this->current_pointer_pos_y)) {
            drawable_.set_pointer(pointer);
        }

        this->current_pointer = pointer;
    }

    WidgetModuleHost(gdi::GraphicApi& drawable, Widget2& parent,
                     NotifyApi* notifier,
                     std::unique_ptr<mod_api> managed_mod, Font const & font,
                     Theme const & /*theme*/,
                     const GCC::UserData::CSMonitor& cs_monitor,
                     uint16_t front_width, uint16_t front_height,
                     int group_id = 0)
    : WidgetParent(drawable, parent, notifier, group_id)
    , module_holder(*this, std::move(managed_mod))
    , drawable_ref(drawable)
    , hscroll(drawable, *this, this, true, BLACK,
        BGRColor(0x606060), BGRColor(0xF0F0F0), BGRColor(0xCDCDCD), font, true)
    , vscroll(drawable, *this, this, false, BLACK,
        BGRColor(0x606060), BGRColor(0xF0F0F0), BGRColor(0xCDCDCD), font, true)
    , monitors(cs_monitor)
    {
        this->pointer_flag = Pointer::POINTER_CUSTOM;

        this->impl = &composite_array;

        this->tab_flag   = NORMAL_TAB;
        this->focus_flag = NORMAL_FOCUS;

        Dimension dim = this->hscroll.get_optimal_dim();
        this->hscroll_height = dim.h;
        this->hscroll.set_wh(this->hscroll.cx(), this->hscroll_height);

        dim = this->vscroll.get_optimal_dim();
        this->vscroll_width = dim.w;
        this->vscroll.set_wh(this->vscroll_width, this->vscroll.cy());

        this->monitor_one.monitorCount              = 1;
        this->monitor_one.monitorDefArray[0].left   = 0;
        this->monitor_one.monitorDefArray[0].top    = 0;
        this->monitor_one.monitorDefArray[0].right  = front_width - 1;
        this->monitor_one.monitorDefArray[0].bottom = front_height - 1;
    }

    mod_api& get_managed_mod()
    {
        return this->module_holder;
    }

    const mod_api& get_managed_mod() const
    {
        return this->module_holder;
    }

private:
    gdi::GraphicApi& get_drawable()
    {
        if (this->drawable_ptr)
        {
            return *this->drawable_ptr;
        }

        return this->drawable_ref;
    }

    void update_rects() {
        const Dimension module_dim = this->module_holder.get_dim();

        this->vision_rect = this->get_rect();

        const bool old_hscroll_added = this->hscroll_added;
        const bool old_vscroll_added = this->vscroll_added;

        if ((this->cx() >= module_dim.w) && (this->cy() >= module_dim.h)) {
            this->hscroll_added = false;
            this->vscroll_added = false;

            this->mod_visible_rect = Rect(0, 0, module_dim.w, module_dim.h);
        }
        else {
            if (this->vision_rect.cx < module_dim.w) {
                this->vision_rect.cy -= this->hscroll_height;

                this->hscroll_added = true;
            }
            else {
                this->hscroll_added = false;
            }

            if (this->vision_rect.cy < module_dim.h) {
                this->vision_rect.cx -= this->vscroll_width;

                this->vscroll_added = true;
            }
            else {
                this->vscroll_added = false;
            }

            if ((this->vision_rect.cx < module_dim.w) && !this->hscroll_added) {
                this->vision_rect.cy -= this->hscroll_height;

                this->hscroll_added = true;
            }

            if ((this->vision_rect.cy < module_dim.h) && !this->vscroll_added) {
                this->vision_rect.cx -= this->vscroll_width;

                this->vscroll_added = true;
            }
        }

        if (old_hscroll_added != this->hscroll_added) {
            if (this->hscroll_added) {
                this->add_widget(&this->hscroll);
            }
            else {
                this->remove_widget(&this->hscroll);
            }

        }
        if (old_vscroll_added != this->vscroll_added) {
            if (this->vscroll_added) {
                this->add_widget(&this->vscroll);
            }
            else {
                this->remove_widget(&this->vscroll);
            }

        }

        this->mod_visible_rect.cx = std::min<uint16_t>(
            this->vision_rect.cx, module_dim.w);
        this->mod_visible_rect.cy = std::min<uint16_t>(
            this->vision_rect.cy, module_dim.h);

        if (this->hscroll_added) {
            const unsigned int new_max_value = module_dim.w - this->vision_rect.cx;

            this->hscroll.set_max_value(new_max_value);

            if (this->mod_visible_rect.x > static_cast<int>(new_max_value)) {
                this->mod_visible_rect.x = static_cast<int>(new_max_value);
            }

            this->hscroll.set_current_value(this->mod_visible_rect.x);

            this->hscroll.set_xy(this->vision_rect.x, this->vision_rect.y + this->vision_rect.cy);
            this->hscroll.set_wh(this->vision_rect.cx, this->hscroll_height);
        }
        if (this->vscroll_added) {
            const unsigned int new_max_value = module_dim.h - this->vision_rect.cy;

            this->vscroll.set_max_value(new_max_value);

            if (this->mod_visible_rect.y > static_cast<int>(new_max_value)) {
                this->mod_visible_rect.y = static_cast<int>(new_max_value);
            }

            this->vscroll.set_current_value(this->mod_visible_rect.y);

            this->vscroll.set_xy(this->vision_rect.x + this->vision_rect.cx, this->vision_rect.y);
            this->vscroll.set_wh(this->vscroll_width, this->vision_rect.cy);
        }
    }

private:
    void screen_copy(Rect old_rect, Rect new_rect) {
        gdi::GraphicApi& drawable_ = this->get_drawable();

        RDPScrBlt cmd(new_rect, 0xCC, old_rect.x, old_rect.y);

        drawable_.draw(cmd, new_rect);

        GCC::UserData::CSMonitor& cs_monitor = this->monitors;
        if (!cs_monitor.monitorCount) {
            cs_monitor = this->monitor_one;
        }

        SubRegion region;

        region.rects.push_back(old_rect);

        for (uint32_t i = 0; i < cs_monitor.monitorCount; ++i) {
            Rect intersect_rect = old_rect.intersect(
                    Rect(cs_monitor.monitorDefArray[i].left,
                         cs_monitor.monitorDefArray[i].top,
                         cs_monitor.monitorDefArray[i].right  - cs_monitor.monitorDefArray[i].left + 1,
                         cs_monitor.monitorDefArray[i].bottom - cs_monitor.monitorDefArray[i].top  + 1)
                );
            if (!intersect_rect.isempty()) {
                region.subtract_rect(intersect_rect);
            }
        }

        for (const Rect & rect : region.rects) {
            this->rdp_input_invalidate(rect.offset(new_rect.x - old_rect.x, new_rect.y - old_rect.y));
        }
    }

public:
    void set_xy(int16_t x, int16_t y) override {
        Rect old_rect = this->get_rect();

        WidgetParent::set_xy(x, y);

        this->update_rects();

        if (!old_rect.isempty()) {
            Rect new_rect = this->get_rect();

            this->screen_copy(old_rect, new_rect);
        }
        else {
            this->rdp_input_invalidate(this->get_rect());
        }
    }

    void set_wh(uint16_t w, uint16_t h) override {
        Rect old_mod_visible_rect = this->mod_visible_rect;
        Rect old_rect             = this->get_rect();

        if (this->hscroll_added) {
            old_rect.cy -= this->hscroll.cy();
        }
        if (this->vscroll_added) {
            old_rect.cx -= this->vscroll.cx();
        }

        WidgetParent::set_wh(w, h);

        this->update_rects();

        Rect new_mod_visible_rect = this->mod_visible_rect;
        Rect new_rect = this->get_rect();

        Rect intersect_mod_visible_rect = new_mod_visible_rect.intersect(old_mod_visible_rect);

        old_rect.x  += intersect_mod_visible_rect.x - old_mod_visible_rect.x;
        old_rect.y  += intersect_mod_visible_rect.y - old_mod_visible_rect.y;
        old_rect.cx  = intersect_mod_visible_rect.cx;
        old_rect.cy  = intersect_mod_visible_rect.cy;

        new_rect.x  += intersect_mod_visible_rect.x - new_mod_visible_rect.x;
        new_rect.y  += intersect_mod_visible_rect.y - new_mod_visible_rect.y;
        new_rect.cx  = intersect_mod_visible_rect.cx;
        new_rect.cy  = intersect_mod_visible_rect.cy;

        SubRegion region;

        region.rects.push_back(this->get_rect());

        if (!old_rect.isempty()) {
            this->screen_copy(old_rect, new_rect);

            region.subtract_rect(new_rect);
        }

        for (const Rect & rect : region.rects) {
            this->rdp_input_invalidate(rect);
        }

        if (this->hscroll_added) {
            this->hscroll.rdp_input_invalidate(this->hscroll.get_rect());
        }
        if (this->vscroll_added) {
            this->vscroll.rdp_input_invalidate(this->vscroll.get_rect());
        }
    }

    using WidgetParent::set_wh;

    const Pointer* get_pointer() const override {
        return &this->current_pointer;
    }

public:
    // NotifyApi

    void notify(Widget2* /*widget*/, NotifyApi::notify_event_t event) override {
        Widget2* parentWidget = &this->parent;
        while (&parentWidget->parent != &parentWidget->parent.parent) {
            parentWidget = &parentWidget->parent;
        }

        if (event == NOTIFY_HSCROLL) {
            if (this->hscroll_added) {
                int16_t old_mod_visible_rect_x = this->mod_visible_rect.x;

                this->mod_visible_rect.x = this->hscroll.get_current_value();

                Rect visible_vision_rect = this->vision_rect.intersect(parentWidget->get_rect());
                int16_t offset_x = old_mod_visible_rect_x - this->mod_visible_rect.x;

                Rect dest_rect = visible_vision_rect.offset(offset_x, 0).intersect(visible_vision_rect);
                Rect src_rect  = dest_rect.offset(-offset_x, 0);

                if (src_rect.intersect(dest_rect).isempty()) {
                    this->module_holder.rdp_input_invalidate(dest_rect.offset(
                            -this->x() + this->mod_visible_rect.x,
                            -this->y() + this->mod_visible_rect.y
                        ));
                }
                else {
                    this->screen_copy(src_rect, dest_rect);

                    SubRegion region;

                    region.rects.push_back(visible_vision_rect);
                    region.subtract_rect(dest_rect);

                    REDASSERT(region.rects.size() == 1);

                    this->module_holder.rdp_input_invalidate(region.rects[0].offset(
                            -this->x() + this->mod_visible_rect.x,
                            -this->y() + this->mod_visible_rect.y
                        ));
                }
            }
        }
        else if (event == NOTIFY_VSCROLL) {
            if (this->vscroll_added) {
                int16_t old_mod_visible_rect_y = this->mod_visible_rect.y;

                this->mod_visible_rect.y = this->vscroll.get_current_value();

                Rect visible_vision_rect = this->vision_rect.intersect(parentWidget->get_rect());
                int16_t offset_y = old_mod_visible_rect_y - this->mod_visible_rect.y;

                Rect dest_rect = visible_vision_rect.offset(0, offset_y).intersect(visible_vision_rect);
                Rect src_rect  = dest_rect.offset(0, -offset_y);

                if (src_rect.intersect(dest_rect).isempty()) {
                    this->module_holder.rdp_input_invalidate(dest_rect.offset(
                            -this->x() + this->mod_visible_rect.x,
                            -this->y() + this->mod_visible_rect.y
                        ));
                }
                else {
                    this->screen_copy(src_rect, dest_rect);

                    SubRegion region;

                    region.rects.push_back(visible_vision_rect);
                    region.subtract_rect(dest_rect);

                    REDASSERT(region.rects.size() == 1);

                    this->module_holder.rdp_input_invalidate(region.rects[0].offset(
                            -this->x() + this->mod_visible_rect.x,
                            -this->y() + this->mod_visible_rect.y
                        ));
                }
            }
        }
    }

    // RdpInput

    void rdp_input_invalidate(Rect clip) override
    {
        Rect rect_intersect = clip.intersect(this->get_rect());

        if (!rect_intersect.isempty()) {
            this->drawable.begin_update();

            SubRegion region;

            region.rects.push_back(rect_intersect);
            if (!this->mod_visible_rect.isempty()) {
                Rect mod_vision_rect(this->vision_rect.x, this->vision_rect.y,
                    this->mod_visible_rect.cx, this->mod_visible_rect.cy);
                region.subtract_rect(mod_vision_rect);
            }
            if (this->hscroll_added) {
                Rect hscroll_rect = this->hscroll.get_rect();
                if (!hscroll_rect.isempty()) {
                    region.subtract_rect(hscroll_rect);
                }
            }
            if (this->vscroll_added) {
                Rect vscroll_rect = this->vscroll.get_rect();
                if (!vscroll_rect.isempty()) {
                    region.subtract_rect(vscroll_rect);
                }
            }

            ::fill_region(this->drawable, region, BLACK);


            Rect mod_update_rect = clip.intersect(this->vision_rect);
            mod_update_rect = mod_update_rect.offset(-this->x() + this->mod_visible_rect.x, -this->y() + this->mod_visible_rect.y);
            if (!mod_update_rect.isempty()) {
                this->module_holder.rdp_input_invalidate(mod_update_rect);
            }

            if (this->hscroll_added) {
                this->hscroll.rdp_input_invalidate(rect_intersect);
            }
            if (this->vscroll_added) {
                this->vscroll.rdp_input_invalidate(rect_intersect);
            }

            this->drawable.end_update();
        }
    }

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override
    {
        this->current_pointer_pos_x = x;
        this->current_pointer_pos_y = y;

        if (this->vision_rect.contains_pt(x, y)) {
            this->module_holder.rdp_input_mouse(device_flags, x - this->x() + this->mod_visible_rect.x, y - this->y() + this->mod_visible_rect.y, keymap);
        }

        WidgetParent::rdp_input_mouse(device_flags, x, y, keymap);
    }

    void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap) override
    {
        this->module_holder.rdp_input_scancode(param1, param2, param3, param4, keymap);
    }

    void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override
    {
        this->module_holder.rdp_input_synchronize(time, device_flags, param1, param2);
    }

    // Widget2

    void refresh(Rect/* clip*/) override {
        this->update_rects();
    }

private:
    void begin_update() override
    {
        gdi::GraphicApi& drawable_ = this->get_drawable();

        drawable_.begin_update();
    }

    void end_update() override
    {
        gdi::GraphicApi& drawable_ = this->get_drawable();

        drawable_.end_update();
    }

    template<class Cmd>
    void draw_impl(const Cmd& cmd) {
        gdi::GraphicApi& drawable_ = this->get_drawable();

        drawable_.draw(cmd);
    }

    template<class Cmd>
    using cmd_not_implementing_move = std::integral_constant<bool,
            std::is_same<Cmd, RDPBitmapData      >::value ||
            std::is_same<Cmd, RDPEllipseCB       >::value ||
            std::is_same<Cmd, RDPEllipseSC       >::value ||
            std::is_same<Cmd, RDPPolygonCB       >::value ||
            std::is_same<Cmd, RDPPolygonSC       >::value ||
            std::is_same<Cmd, RDPGlyphIndex      >::value
        >;

    template<class Cmd, class... Args, typename std::enable_if<
            !cmd_not_implementing_move<Cmd>::value, bool
        >::type = 1>
    void draw_impl(const Cmd& cmd, const Rect clip, const Args&... args)
    {
        gdi::GraphicApi& drawable_ = this->get_drawable();

        Rect new_clip = clip.offset(this->x() - this->mod_visible_rect.x, this->y() - this->mod_visible_rect.y);
        new_clip = new_clip.intersect(this->vision_rect);
        if (new_clip.isempty()) { return; }

        Cmd new_cmd = cmd;

        new_cmd.move(this->x() - this->mod_visible_rect.x, this->y() - this->mod_visible_rect.y);

        drawable_.draw(new_cmd, new_clip, args...);
    }

    template<class Cmd, class... Args, typename std::enable_if<
            cmd_not_implementing_move<Cmd>::value, bool
        >::type = 1>
    void draw_impl(const Cmd& cmd, const Rect clip, const Args&... args)
    {
        gdi::GraphicApi& drawable_ = this->get_drawable();

        LOG(LOG_INFO, "");
        LOG(LOG_INFO, "");
        LOG(LOG_INFO, "order_id=%u", unsigned(cmd.id()));
        LOG(LOG_INFO, "");
        LOG(LOG_INFO, "");

        Rect new_clip = clip.offset(this->x() - this->mod_visible_rect.x, this->y() - this->mod_visible_rect.y);
        new_clip = new_clip.intersect(this->vision_rect);
        if (new_clip.isempty()) { return; }

        drawable_.draw(cmd, new_clip, args...);
    }

    void draw_impl(const RDPBitmapData& bitmap_data, const Bitmap& bmp)
    {
        Rect boundary(bitmap_data.dest_left,
                      bitmap_data.dest_top,
                      bitmap_data.dest_right - bitmap_data.dest_left + 1,
                      bitmap_data.dest_bottom - bitmap_data.dest_top + 1
                     );

        this->draw_impl(RDPMemBlt(0, boundary, 0xCC, 0, 0, 0), boundary, bmp);
    }
};
