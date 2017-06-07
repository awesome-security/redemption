/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc.; 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan,
 *              Loïc Michaux
 */

#pragma once

#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolygonSC.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolygonCB.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseSC.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseCB.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryBrushCache.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryFrameMarker.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "utils/bitmap.hpp"
#include "gdi/graphic_api.hpp"

#include "gdi/clip_from_cmd.hpp"

namespace gdi
{

using subrect4_t = std::array<Rect, 4>;
inline subrect4_t subrect4(const Rect rect, const Rect & protected_rect)
{
    const Rect inter = rect.intersect(protected_rect);
    return {{
        // top
        Rect(rect.x, rect.y, rect.cx, inter.y - rect.y),
        // right
        Rect(inter.right(), inter.y, rect.right() - inter.right(), inter.cy),
        // bottom
        Rect(rect.x, inter.bottom(), rect.cx, rect.bottom() - inter.bottom()),
        // left
        Rect(rect.x, inter.y, inter.x - rect.x, inter.cy)
    }};
}

class ProtectedGraphics : public gdi::GraphicApi
{
    Rect protected_rect;
    gdi::GraphicApi & drawable;

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

    void set_pointer(Pointer    const & pointer) override {
        this->get_graphic_proxy().set_pointer(pointer);
    }

    void set_palette(BGRPalette const & palette) override {
        this->get_graphic_proxy().set_palette(palette);
    }

    void sync() override {
        this->get_graphic_proxy().sync();
    }

    void set_row(std::size_t rownum, const uint8_t * data) override {
        this->get_graphic_proxy().set_row(rownum, data);
    }

    void begin_update() override {
        this->get_graphic_proxy().begin_update();
    }

    void end_update() override {
        this->get_graphic_proxy().end_update();
    }

protected:
    template<class... Ts>
    void draw_impl(Ts const & ... args) {
        this->get_graphic_proxy().draw(args...);
    }

public:
    ProtectedGraphics(gdi::GraphicApi & drawable, Rect const rect)
    : protected_rect(rect)
    , drawable(drawable)
    {}

    Rect get_protected_rect() const
    { return this->protected_rect; }

    void set_protected_rect(Rect const rect)
    { this->protected_rect = rect; }

protected:
    virtual void refresh_rects(array_view<Rect const>) = 0;

private:
    gdi::GraphicApi & get_graphic_proxy() const
    { return this->drawable; }

    template<class Command>
    void draw_impl(Command const & cmd)
    { this->drawable.draw(cmd); }

    template<class Command, class... Args>
    void draw_impl(Command const & cmd, Rect clip, Args const &... args)
    {
        auto const & rect = clip_from_cmd(cmd).intersect(clip);
        if (this->protected_rect.contains(rect) || rect.isempty()) {
            //nada
        }
        else if (rect.has_intersection(this->protected_rect)) {
            this->drawable.begin_update();
            // TODO used multi orders
            for (const Rect & subrect : subrect4(rect, this->protected_rect)) {
                if (!subrect.isempty()) {
                    this->drawable.draw(cmd, subrect, args...);
                }
            }
            this->drawable.end_update();
        }
        else {
            this->drawable.draw(cmd, clip, args...);
        }
    }

    void draw_impl(const RDPBitmapData & bitmap_data, const Bitmap & bmp)
    {
        Rect rectBmp( bitmap_data.dest_left, bitmap_data.dest_top
                    , bitmap_data.dest_right - bitmap_data.dest_left + 1
                    , bitmap_data.dest_bottom - bitmap_data.dest_top + 1);

        if (rectBmp.has_intersection(this->protected_rect)) {
            this->drawable.begin_update();
            for (const Rect & subrect : subrect4(rectBmp, this->protected_rect)) {
                if (!subrect.isempty()) {
                    this->drawable.draw(
                        RDPMemBlt(0, subrect, 0xCC, subrect.x - rectBmp.x, subrect.y - rectBmp.y, 0),
                        subrect, bmp
                    );
                }
            }
            this->drawable.end_update();
        }
        else {
            this->drawable.draw(bitmap_data, bmp);
        }
    }

    void draw_impl(const RDPScrBlt & cmd, const Rect clip)
    {
        const Rect drect = cmd.rect.intersect(clip);
        const int deltax = cmd.srcx - cmd.rect.x;
        const int deltay = cmd.srcy - cmd.rect.y;
        const int srcx = drect.x + deltax;
        const int srcy = drect.y + deltay;
        const Rect srect(srcx, srcy, drect.cx, drect.cy);

        const bool has_dest_intersec_fg = drect.has_intersection(this->protected_rect);
        const bool has_src_intersec_fg = srect.has_intersection(this->protected_rect);

        if (!has_dest_intersec_fg && !has_src_intersec_fg) {
            this->drawable.draw(cmd, clip);
        }
        else {
            this->drawable.begin_update();
            subrect4_t rects = subrect4(drect, this->protected_rect);
            auto e = std::remove_if(rects.begin(), rects.end(), [](const Rect & rect) { return rect.isempty(); });
            auto av = make_array_view(rects.begin(), e);
            this->refresh_rects(av);
            this->drawable.end_update();
        }
    }
};

}
