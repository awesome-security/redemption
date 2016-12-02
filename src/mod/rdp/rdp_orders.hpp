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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

   rdp module process orders
*/


#pragma once

#include <string.h>
#include <string>

#include <cinttypes>

#include "utils/log.hpp"
#include "core/defines.hpp"
#include "transport/in_file_transport.hpp"
#include "transport/out_file_transport.hpp"
#include "utils/stream.hpp"
#include "utils/fileutils.hpp"

#include "core/RDP/protocol.hpp"

#include "core/RDP/orders/RDPOrdersCommon.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseSC.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryBmpCache.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryFrameMarker.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "core/RDP/orders/AlternateSecondaryWindowing.hpp"

#include "core/RDP/caches/bmpcache.hpp"
#include "core/RDP/caches/bmpcachepersister.hpp"
#include "core/RDP/caches/glyphcache.hpp"

#include "mod/rdp/rdp_log.hpp"

#include "gdi/graphic_api.hpp"

/* orders */
class rdp_orders
{
    // State
    RDPOrderCommon     common;
    RDPMemBlt          memblt;
    RDPMem3Blt         mem3blt;
    RDPOpaqueRect      opaquerect;
    RDPScrBlt          scrblt;
    RDPDestBlt         destblt;
    RDPMultiDstBlt     multidstblt;
    RDPMultiOpaqueRect multiopaquerect;
    RDP::RDPMultiPatBlt multipatblt;
    RDP::RDPMultiScrBlt multiscrblt;
    RDPPatBlt          patblt;
    RDPLineTo          lineto;
    RDPGlyphIndex      glyph_index;
    RDPPolyline        polyline;
    RDPEllipseSC       ellipseSC;

public:
    BGRPalette global_palette;

    BmpCache * bmp_cache;

private:
    GlyphCache gly_cache;

    const implicit_bool_flags<RDPVerbose> verbose;

public:
    size_t recv_bmp_cache_count;
    size_t recv_order_count;

private:
    std::string target_host;
    bool        enable_persistent_disk_bitmap_cache;
    bool        persist_bitmap_cache_on_disk;

public:
    rdp_orders( const char * target_host, bool enable_persistent_disk_bitmap_cache
              , bool persist_bitmap_cache_on_disk, RDPVerbose verbose)
    : common(RDP::PATBLT, Rect(0, 0, 1, 1))
    , memblt(0, Rect(), 0, 0, 0, 0)
    , mem3blt(0, Rect(), 0, 0, 0, 0, 0, RDPBrush(), 0)
    , opaquerect(Rect(), 0)
    , scrblt(Rect(), 0, 0, 0)
    , destblt(Rect(), 0)
    , patblt(Rect(), 0, 0, 0, RDPBrush())
    , lineto(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0))
    , glyph_index( 0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0
                 , reinterpret_cast<const uint8_t *>(""))
    , global_palette(nullptr)
    , bmp_cache(nullptr)
    , verbose(verbose)
    , recv_bmp_cache_count(0)
    , recv_order_count(0)
    , target_host(target_host)
    , enable_persistent_disk_bitmap_cache(enable_persistent_disk_bitmap_cache)
    , persist_bitmap_cache_on_disk(persist_bitmap_cache_on_disk)
    {
    }

    void reset()
    {
        this->common      = RDPOrderCommon(RDP::PATBLT, Rect(0, 0, 1, 1));
        this->memblt      = RDPMemBlt(0, Rect(), 0, 0, 0, 0);
        this->mem3blt     = RDPMem3Blt(0, Rect(), 0, 0, 0, 0, 0, RDPBrush(), 0);
        this->opaquerect  = RDPOpaqueRect(Rect(), 0);
        this->scrblt      = RDPScrBlt(Rect(), 0, 0, 0);
        this->destblt     = RDPDestBlt(Rect(), 0);
        this->multidstblt     = RDPMultiDstBlt();
        this->multiopaquerect = RDPMultiOpaqueRect();
        this->multipatblt     = RDP::RDPMultiPatBlt();
        this->multiscrblt     = RDP::RDPMultiScrBlt();
        this->patblt      = RDPPatBlt(Rect(), 0, 0, 0, RDPBrush());
        this->lineto      = RDPLineTo(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0));
        this->glyph_index = RDPGlyphIndex( 0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1)
                                         , RDPBrush(), 0, 0, 0, reinterpret_cast<const uint8_t *>(""));
        this->polyline        = RDPPolyline();
    }

    ~rdp_orders() {
        if (this->bmp_cache) {
            this->save_persistent_disk_bitmap_cache();
            delete this->bmp_cache;
        }
    }

private:
    void save_persistent_disk_bitmap_cache() const {
        if (!this->enable_persistent_disk_bitmap_cache || !this->persist_bitmap_cache_on_disk) {
            return;
        }

        const char * persistent_path = PERSISTENT_PATH "/mod_rdp";

        // Ensures that the directory exists.
        if (::recursive_create_directory( persistent_path, S_IRWXU | S_IRWXG, -1) != 0) {
            LOG( LOG_ERR
               , "rdp_orders::save_persistent_disk_bitmap_cache: failed to create directory \"%s\"."
               , persistent_path);
            throw Error(ERR_BITMAP_CACHE_PERSISTENT, 0);
        }

        // Generates the name of file.
        char filename[2048];
        ::snprintf(filename, sizeof(filename) - 1, "%s/PDBC-%s-%d",
            persistent_path, this->target_host.c_str(), this->bmp_cache->bpp);
        filename[sizeof(filename) - 1] = '\0';

        char filename_temporary[2048];
        ::snprintf(filename_temporary, sizeof(filename_temporary) - 1, "%s/PDBC-%s-%d-XXXXXX.tmp",
            persistent_path, this->target_host.c_str(), this->bmp_cache->bpp);
        filename_temporary[sizeof(filename_temporary) - 1] = '\0';

        int fd = ::mkostemps(filename_temporary, 4, O_CREAT | O_WRONLY);
        if (fd == -1) {
            LOG( LOG_ERR
               , "rdp_orders::save_persistent_disk_bitmap_cache: "
                 "failed to open (temporary) file for writing. filename=\"%s\""
               , filename_temporary);
            throw Error(ERR_PDBC_SAVE);
        }

        try
        {
            OutFileTransport oft(fd);

            BmpCachePersister::save_all_to_disk(*this->bmp_cache, oft, to_verbose_flags(this->verbose));

            ::close(fd);

            if (::rename(filename_temporary, filename) == -1) {
                LOG( LOG_WARNING
                   , "rdp_orders::save_persistent_disk_bitmap_cache: failed to rename the (temporary) file. "
                     "old_filename=\"%s\" new_filename=\"%s\""
                   , filename_temporary, filename);
                ::unlink(filename_temporary);
            }
        }
        catch (...)
        {
            ::close(fd);
            ::unlink(filename_temporary);
        }
    }

public:
    void create_cache_bitmap(const uint8_t bpp,
        uint16_t small_entries, uint16_t small_size, bool small_persistent,
        uint16_t medium_entries, uint16_t medium_size, bool medium_persistent,
        uint16_t big_entries, uint16_t big_size, bool big_persistent,
        bool enable_waiting_list, BmpCache::Verbose verbose)
    {
        if (this->bmp_cache) {
            if (this->bmp_cache->bpp == bpp) {
                return;
            }

            this->save_persistent_disk_bitmap_cache();
            delete this->bmp_cache;
            this->bmp_cache = nullptr;
        }

        this->bmp_cache = new BmpCache(BmpCache::Mod_rdp, bpp, 3, false,
                                       BmpCache::CacheOption(small_entries + (enable_waiting_list ? 1 : 0), small_size, small_persistent),
                                       BmpCache::CacheOption(medium_entries + (enable_waiting_list ? 1 : 0), medium_size, medium_persistent),
                                       BmpCache::CacheOption(big_entries + (enable_waiting_list ? 1 : 0), big_size, big_persistent),
                                       BmpCache::CacheOption(),
                                       BmpCache::CacheOption(),
                                       verbose);

        if (this->enable_persistent_disk_bitmap_cache && this->persist_bitmap_cache_on_disk) {
            // Generates the name of file.
            char filename[2048];
            ::snprintf(filename, sizeof(filename) - 1, "%s/PDBC-%s-%d",
                PERSISTENT_PATH "/mod_rdp", this->target_host.c_str(), this->bmp_cache->bpp);
            filename[sizeof(filename) - 1] = '\0';

            int fd = ::open(filename, O_RDONLY);
            if (fd == -1) {
                return;
            }

            InFileTransport ift(fd);

            try {
                if (this->verbose & RDPVerbose::basic_trace) {
                    LOG(LOG_INFO, "rdp_orders::create_cache_bitmap: filename=\"%s\"", filename);
                }
                BmpCachePersister::load_all_from_disk(*this->bmp_cache, ift, filename, to_verbose_flags(this->verbose));
            }
            catch (...) {
            }

            ::close(fd);
        }
    }

private:
    void process_framemarker( InStream & stream, const RDP::AltsecDrawingOrderHeader & header
                            , gdi::GraphicApi & gd) {
        if (this->verbose & RDPVerbose::graphics) {
            LOG(LOG_INFO, "rdp_orders::process_framemarker");
        }

        RDP::FrameMarker order;

        order.receive(stream, header);

        gd.draw(order);
    }

    void process_windowing( InStream & stream, const RDP::AltsecDrawingOrderHeader & header
                          , gdi::GraphicApi & gd) {
        if (this->verbose & RDPVerbose::graphics) {
            LOG(LOG_INFO, "rdp_orders::process_windowing");
        }

        const uint32_t FieldsPresentFlags = [&]{
            InStream stream2(stream.get_current(), stream.in_remain());
            stream2.in_skip_bytes(2);    // OrderSize(2)
            return stream2.in_uint32_le();
        }();

        switch (FieldsPresentFlags & (  RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
                                      | RDP::RAIL::WINDOW_ORDER_TYPE_NOTIFY
                                      | RDP::RAIL::WINDOW_ORDER_TYPE_DESKTOP)) {
            case RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW:
                this->process_window_information(stream, header, FieldsPresentFlags, gd);
                break;

            case RDP::RAIL::WINDOW_ORDER_TYPE_NOTIFY:
                this->process_notification_icon_information(stream, header, FieldsPresentFlags, gd);
                break;

            case RDP::RAIL::WINDOW_ORDER_TYPE_DESKTOP:
                this->process_desktop_information(stream, header, FieldsPresentFlags, gd);
                break;

            default:
                LOG(LOG_INFO,
                    "rdp_orders::process_windowing: "
                        "unsupported Windowing Alternate Secondary Drawing Orders! "
                        "FieldsPresentFlags=0x%08X",
                    FieldsPresentFlags);
                break;
        }
    }

    void process_window_information( InStream & stream, const RDP::AltsecDrawingOrderHeader &
                                   , uint32_t FieldsPresentFlags, gdi::GraphicApi & gd) {
        if (this->verbose & RDPVerbose::graphics) {
            LOG(LOG_INFO, "rdp_orders::process_window_information");
        }

        switch (FieldsPresentFlags & (  RDP::RAIL::WINDOW_ORDER_STATE_NEW
                                      | RDP::RAIL::WINDOW_ORDER_ICON
                                      | RDP::RAIL::WINDOW_ORDER_CACHEDICON
                                      | RDP::RAIL::WINDOW_ORDER_STATE_DELETED))
        {
            case RDP::RAIL::WINDOW_ORDER_ICON: {
                    RDP::RAIL::WindowIcon order;
                    order.receive(stream);
                    order.log(LOG_INFO);
                    gd.draw(order);
                }
                break;

            case RDP::RAIL::WINDOW_ORDER_CACHEDICON: {
                    RDP::RAIL::CachedIcon order;
                    order.receive(stream);
                    order.log(LOG_INFO);
                    gd.draw(order);
                }
                break;

            case RDP::RAIL::WINDOW_ORDER_STATE_DELETED: {
                    RDP::RAIL::DeletedWindow order;
                    order.receive(stream);
                    order.log(LOG_INFO);
                    gd.draw(order);
                }
                break;

            case 0:
            case RDP::RAIL::WINDOW_ORDER_STATE_NEW: {
                    RDP::RAIL::NewOrExistingWindow order;
                    order.receive(stream);
                    order.log(LOG_INFO);
                    gd.draw(order);
                }
                break;
        }
    }

    void process_notification_icon_information( InStream & stream, const RDP::AltsecDrawingOrderHeader &
                                              , uint32_t FieldsPresentFlags, gdi::GraphicApi & gd) {
        if (this->verbose & RDPVerbose::graphics) {
            LOG(LOG_INFO, "rdp_orders::process_notification_icon_information");
        }

        switch (FieldsPresentFlags & (  RDP::RAIL::WINDOW_ORDER_STATE_NEW
                                      | RDP::RAIL::WINDOW_ORDER_STATE_DELETED))
        {
            case RDP::RAIL::WINDOW_ORDER_STATE_DELETED: {
                    RDP::RAIL::DeletedNotificationIcons order;
                    order.receive(stream);
                    order.log(LOG_INFO);
                    gd.draw(order);
                }
                break;

            case 0:
            case RDP::RAIL::WINDOW_ORDER_STATE_NEW: {
                    RDP::RAIL::NewOrExistingNotificationIcons order;
                    order.receive(stream);
                    order.log(LOG_INFO);
                    gd.draw(order);
                }
                break;
        }
    }

    void process_desktop_information( InStream & stream, const RDP::AltsecDrawingOrderHeader &
                                    , uint32_t FieldsPresentFlags, gdi::GraphicApi & gd) {
        if (this->verbose & RDPVerbose::graphics) {
            LOG(LOG_INFO, "rdp_orders::process_desktop_information");
        }

        if (FieldsPresentFlags & RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_NONE) {
            RDP::RAIL::NonMonitoredDesktop order;
            order.receive(stream);
            order.log(LOG_INFO);
            gd.draw(order);
        }
        else {
            RDP::RAIL::ActivelyMonitoredDesktop order;
            order.receive(stream);
            order.log(LOG_INFO);
            gd.draw(order);
        }
    }

    void process_bmpcache(InStream & stream, const RDPSecondaryOrderHeader & header, uint8_t bpp)
    {
        if (this->verbose & RDPVerbose::graphics) {
            LOG(LOG_INFO, "rdp_orders_process_bmpcache bpp=%u", bpp);
        }
        RDPBmpCache bmp(this->verbose);
        bmp.receive(stream, header, this->global_palette, bpp);

        this->recv_bmp_cache_count++;

        REDASSERT(bmp.bmp.is_valid());

        this->bmp_cache->put(bmp.id, bmp.idx, bmp.bmp, bmp.key1, bmp.key2);
        if (this->verbose & RDPVerbose::graphics) {
            LOG( LOG_ERR
               , "rdp_orders_process_bmpcache bitmap id=%d idx=%d cx=%" PRIu16 " cy=%" PRIu16
                 " bmp_size=%zu original_bpp=%" PRIu8 " bpp=%" PRIu8
               , bmp.id, bmp.idx, bmp.bmp.cx(), bmp.bmp.cy(), bmp.bmp.bmp_size(), bmp.bmp.bpp(), bpp);
        }
    }

    void server_add_char( uint8_t cacheId, uint16_t cacheIndex
                        , int16_t offset, int16_t baseline
                        , uint16_t width, uint16_t height, const uint8_t * data)
    {
        FontChar fi(offset, baseline, width, height, 0);
        memcpy(fi.data.get(), data, fi.datasize());

        this->gly_cache.set_glyph(std::move(fi), cacheId, cacheIndex);
    }

    void process_glyphcache(InStream & stream, const RDPSecondaryOrderHeader &/* header*/) {
        if (this->verbose & RDPVerbose::graphics) {
            LOG(LOG_INFO, "rdp_orders_process_glyphcache");
        }
        const uint8_t cacheId = stream.in_uint8();
        const uint8_t nglyphs = stream.in_uint8();
        for (uint8_t i = 0; i < nglyphs; i++) {
            const uint16_t cacheIndex = stream.in_uint16_le();
            const int16_t  offset     = stream.in_sint16_le();
            const int16_t  baseline   = stream.in_sint16_le();
            const uint16_t width      = stream.in_uint16_le();
            const uint16_t height     = stream.in_uint16_le();

            const unsigned int   datasize = (height * nbbytes(width) + 3) & ~3;
            const uint8_t      * data     = stream.in_uint8p(datasize);

            this->server_add_char(cacheId, cacheIndex, offset, baseline, width, height, data);
        }
        if (this->verbose & RDPVerbose::graphics) {
            LOG(LOG_INFO, "rdp_orders_process_glyphcache done");
        }
    }

    void process_colormap(InStream & stream, const RDPSecondaryOrderHeader & header, gdi::GraphicApi & gd) {
        if (this->verbose & RDPVerbose::graphics) {
            LOG(LOG_INFO, "process_colormap");
        }
        RDPColCache colormap;
        colormap.receive(stream, header);
        RDPColCache cmd(colormap.cacheIndex, colormap.palette);
        gd.draw(cmd);

        if (this->verbose & RDPVerbose::graphics) {
            LOG(LOG_INFO, "process_colormap done");
        }
    }

public:
    /*****************************************************************************/
    int process_orders(uint8_t bpp, InStream & stream, bool fast_path, gdi::GraphicApi & gd,
                       uint16_t front_width, uint16_t front_height) {
        if (this->verbose & RDPVerbose::graphics) {
            LOG(LOG_INFO, "process_orders bpp=%u", bpp);
        }

        using namespace RDP;

        OrdersUpdate_Recv orders_update(stream, fast_path);

        this->recv_order_count += orders_update.number_orders;

        int processed = 0;
        while (processed < orders_update.number_orders) {
            DrawingOrder_RecvFactory drawing_order(stream);

            uint8_t class_ = (drawing_order.control_flags & (STANDARD | SECONDARY));
            if (class_ == SECONDARY) {
                RDP::AltsecDrawingOrderHeader header(drawing_order.control_flags);
                switch (header.orderType) {
                    case RDP::AltsecDrawingOrderHeader::FrameMarker:
                        this->process_framemarker(stream, header, gd);
                    break;
                    case RDP::AltsecDrawingOrderHeader::Window:
                        this->process_windowing(stream, header, gd);
                    break;
                    default:
                        LOG(LOG_ERR, "unsupported Alternate Secondary Drawing Order (%d)", header.orderType);
                        /* error, unknown order */
                    break;
                }
            }
            else if (class_ == (STANDARD | SECONDARY)) {
                //uint8_t * order_start = stream.p;
                RDPSecondaryOrderHeader header(stream);
                //LOG(LOG_INFO, "secondary order=%d", header.type);
                uint8_t const * next_order = stream.get_current() + header.order_data_length();
                switch (header.type) {
                case TS_CACHE_BITMAP_COMPRESSED:
                case TS_CACHE_BITMAP_UNCOMPRESSED:
                case TS_CACHE_BITMAP_COMPRESSED_REV2:
                case TS_CACHE_BITMAP_UNCOMPRESSED_REV2:
                    this->process_bmpcache(stream, header, bpp);
                    break;
                case TS_CACHE_COLOR_TABLE:
                    this->process_colormap(stream, header, gd);
                    break;
                case TS_CACHE_GLYPH:
                    this->process_glyphcache(stream, header);
                    //hexdump_d(order_start, stream.p - order_start);
                    break;
                case TS_CACHE_BITMAP_COMPRESSED_REV3:
                    LOG( LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_BITMAP_COMPRESSED_REV3 (%d)"
                       , header.type);
                    break;
                default:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER (%d)", header.type);
                    /* error, unknown order */
                    break;
                }
                stream.in_skip_bytes(next_order - stream.get_current());
            }
            else if (class_ == STANDARD) {
                RDPPrimaryOrderHeader header = this->common.receive(stream, drawing_order.control_flags);
                const Rect & cmd_clip = ( (drawing_order.control_flags & BOUNDS)
                                        ? this->common.clip
                                        : Rect(0, 0, front_width, front_height)
                                        );
                //LOG(LOG_INFO, "/* order=%d ordername=%s */", this->common.order, ordernames[this->common.order]);
                switch (this->common.order) {
                case GLYPHINDEX:
                    this->glyph_index.receive(stream, header);
                    //this->glyph_index.log(LOG_INFO, cmd_clip);
                    gd.draw(this->glyph_index, cmd_clip, this->gly_cache);
                    break;
                case DESTBLT:
                    this->destblt.receive(stream, header);
                    gd.draw(this->destblt, cmd_clip);
                    //this->destblt.log(LOG_INFO, cmd_clip);
                    break;
                case MULTIDSTBLT:
                    this->multidstblt.receive(stream, header);
                    gd.draw(this->multidstblt, cmd_clip);
                    //this->multidstblt.log(LOG_INFO, cmd_clip);
                    break;
                case MULTIOPAQUERECT:
                    this->multiopaquerect.receive(stream, header);
                    gd.draw(this->multiopaquerect, cmd_clip);
                    //this->multiopaquerect.log(LOG_INFO, cmd_clip);
                    break;
                case MULTIPATBLT:
                    this->multipatblt.receive(stream, header);
                    gd.draw(this->multipatblt, cmd_clip);
                    //this->multipatblt.log(LOG_INFO, cmd_clip);
                    break;
                case MULTISCRBLT:
                    this->multiscrblt.receive(stream, header);
                    gd.draw(this->multiscrblt, cmd_clip);
                    //this->multiscrblt.log(LOG_INFO, cmd_clip);
                    break;
                case PATBLT:
                    this->patblt.receive(stream, header);
                    gd.draw(this->patblt, cmd_clip);
                    //this->patblt.log(LOG_INFO, cmd_clip);
                    break;
                case SCREENBLT:
                    this->scrblt.receive(stream, header);
                    gd.draw(this->scrblt, cmd_clip);
                    //this->scrblt.log(LOG_INFO, cmd_clip);
                    break;
                case LINE:
                    this->lineto.receive(stream, header);
                    gd.draw(this->lineto, cmd_clip);
                    //this->lineto.log(LOG_INFO, cmd_clip);
                    break;
                case RECT:
                    this->opaquerect.receive(stream, header);
                    gd.draw(this->opaquerect, cmd_clip);
                    //this->opaquerect.log(LOG_INFO, cmd_clip);
                    break;
                case MEMBLT:
                    this->memblt.receive(stream, header);
                    {
                        if ((this->memblt.cache_id >> 8) >= 6) {
                            LOG( LOG_INFO, "colormap out of range in memblt:%x"
                               , (this->memblt.cache_id >> 8));
                            this->memblt.log(LOG_INFO, cmd_clip);
                            assert(false);
                        }
                        const Bitmap & bitmap =
                            this->bmp_cache->get(this->memblt.cache_id & 0x3, this->memblt.cache_idx);
                        // TODO CGR: check if bitmap has the right palette...
                        // TODO CGR: 8 bits palettes should probabily be transmitted to front, not stored in bitmaps
                        if (bitmap.is_valid()) {
                            gd.draw(this->memblt, cmd_clip, bitmap);
                        }
                        else {
                            LOG(LOG_ERR, "rdp_orders::process_orders: MEMBLT - Bitmap is not found in cache! cache_id=%u cache_index=%u",
                                this->memblt.cache_id & 0x3, this->memblt.cache_idx);
                            REDASSERT(false);
                        }
                    }
                    break;
                case MEM3BLT:
                    this->mem3blt.receive(stream, header);
                    {
                        if ((this->mem3blt.cache_id >> 8) >= 6){
                            LOG( LOG_INFO, "colormap out of range in mem3blt: %x"
                               , (this->mem3blt.cache_id >> 8));
                            this->mem3blt.log(LOG_INFO, cmd_clip);
                            assert(false);
                        }
                        const Bitmap & bitmap =
                            this->bmp_cache->get(this->mem3blt.cache_id & 0x3, this->mem3blt.cache_idx);
                        // TODO CGR: check if bitmap has the right palette...
                        // TODO CGR: 8 bits palettes should probabily be transmitted to front, not stored in bitmaps
                        if (bitmap.is_valid()) {
                            gd.draw(this->mem3blt, cmd_clip, bitmap);
                        }
                        else {
                            LOG(LOG_ERR, "rdp_orders::process_orders: MEM3BLT - Bitmap is not found in cache! cache_id=%u cache_index=%u",
                                this->mem3blt.cache_id & 0x3, this->mem3blt.cache_idx);
                            REDASSERT(false);
                        }
                    }
                    break;
                case POLYLINE:
                    this->polyline.receive(stream, header);
                    gd.draw(this->polyline, cmd_clip);
                    //this->polyline.log(LOG_INFO, cmd_clip);
                    break;
                case ELLIPSESC:
                    this->ellipseSC.receive(stream, header);
                    gd.draw(this->ellipseSC, cmd_clip);
                    //this->ellipseSC.log(LOG_INFO, cmd_clip);
                    break;
                default:
                    /* error unknown order */
                    LOG(LOG_ERR, "unsupported PRIMARY ORDER (%d)", this->common.order);
                    break;
                }
            }
            else {
                /* error, this should always be set */
                LOG(LOG_ERR, "Unsupported drawing order detected : protocol error. class=0x%02X", class_);
                REDASSERT(false);
                break;
            }
            processed++;
        }
        if (this->verbose & RDPVerbose::graphics){
            LOG(LOG_INFO, "process_orders done");
        }
        return 0;
    }   // int process_orders(uint8_t bpp, Stream & stream, bool fast_path, gdi::GraphicApi & gd)
};

