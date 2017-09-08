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
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#include "gdi/graphic_api.hpp"
#include "core/RDP/orders/RDPOrdersCommon.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "core/RDP/caches/glyphcache.hpp"

#include <sstream>
#include <vector>

namespace gdi {

TextMetrics::TextMetrics(const Font & font, const char * unicode_text)
{
    UTF8toUnicodeIterator unicode_iter(unicode_text);
    uint16_t height_max = 0;
    for (; uint32_t c = *unicode_iter; ++unicode_iter) {
        const FontChar & font_item = font.glyph_or_unknown(c);
        this->width += font_item.incby;
        height_max = std::max(height_max, font_item.height);
    }
    this->height = height_max;
}

MultiLineTextMetrics::MultiLineTextMetrics(const Font& font, const char* unicode_text, int max_width,
    std::string& out_multiline_string_ref)
{
    out_multiline_string_ref.clear();

    int number_of_lines = 1;

    int height_max = 0;

    auto get_text_width = [&font, &height_max](const char* unicode_text) -> int {
        TextMetrics tt(font, unicode_text);

        height_max = std::max(height_max, tt.height);

        return tt.width;
    };

    const int white_space_width = get_text_width(" ");

    std::istringstream  iss(unicode_text);
    std::string         parameter;
    std::ostringstream  oss;
    int                 cumulative_width(0);
    while (std::getline(iss, parameter, ' ')) {
        if (!parameter.length()) {
            continue;
        }

        const int part_width = get_text_width(parameter.c_str());

        if (cumulative_width) {
            if (cumulative_width + white_space_width + part_width > max_width) {
                oss << "<br>" << parameter;

                cumulative_width = part_width;

                this->width = std::max(this->width, cumulative_width);

                number_of_lines++;
            }
            else {
                oss << " " << parameter;

                cumulative_width += (white_space_width + part_width);

                this->width = std::max(this->width, cumulative_width);
            }
        }
        else {
            oss << parameter;

            cumulative_width = part_width;

            this->width = std::max(this->width, cumulative_width);
        }
    }

    this->height = height_max * number_of_lines;

    out_multiline_string_ref = std::move(oss.str());
}


// TODO implementation of the server_draw_text function below is a small subset of possibilities text can be packed (detecting duplicated strings). See MS-RDPEGDI 2.2.2.2.1.1.2.13 GlyphIndex (GLYPHINDEX_ORDER)
// TODO: is it still used ? If yes move it somewhere else. Method from internal mods ?
void server_draw_text(
    GraphicApi & drawable, Font const & font,
    int16_t x, int16_t y, const char * text,
    RDPColor fgcolor, RDPColor bgcolor,
    ColorCtx color_ctx,
    Rect clip
) {
    // BUG TODO static not const is a bad idea
    static GlyphCache mod_glyph_cache;

    UTF8toUnicodeIterator unicode_iter(text);

    while (*unicode_iter) {
        int total_width = 0;
        int total_height = 0;
        uint8_t data[256];
        auto data_begin = std::begin(data);
        const auto data_end = std::end(data)-2;

        const int cacheId = 7;
        int distance_from_previous_fragment = 0;
        while (data_begin != data_end) {
            const uint32_t charnum = *unicode_iter;
            if (!charnum) {
                break ;
            }
            ++unicode_iter;

            int cacheIndex = 0;
            FontChar const * font_item = font.glyph_at(charnum);
            if (!font_item) {
                LOG(LOG_WARNING, "server_draw_text() - character not defined >0x%02x<", charnum);
                font_item = &font.unknown_glyph();
            }

            // TODO avoid passing parameters by reference to get results
            const GlyphCache::t_glyph_cache_result cache_result =
                mod_glyph_cache.add_glyph(*font_item, cacheId, cacheIndex);
            (void)cache_result; // supress warning

            *data_begin = cacheIndex;
            ++data_begin;
            *data_begin = distance_from_previous_fragment;
            ++data_begin;
            distance_from_previous_fragment = font_item->incby;
            total_width += font_item->incby;
            total_height = std::max(uint16_t(total_height), font_item->height);
        }

        const Rect bk(x, y, total_width + 1, total_height + 1);

        RDPGlyphIndex glyphindex(
            cacheId,            // cache_id
            0x03,               // fl_accel
            0x0,                // ui_charinc
            1,                  // f_op_redundant,
            fgcolor,            // BackColor (text color)
            bgcolor,            // ForeColor (color of the opaque rectangle)
            bk,                 // bk
            bk,                 // op
            // brush
            RDPBrush(0, 0, 3, 0xaa,
                reinterpret_cast<const uint8_t *>("\xaa\x55\xaa\x55\xaa\x55\xaa\x55")),
            x,                  // glyph_x
            y + total_height,   // glyph_y
            data_begin - data,  // data_len in bytes
            data                // data
        );

        x += total_width;

        drawable.draw(glyphindex, clip, color_ctx, mod_glyph_cache);
    }
}

}
