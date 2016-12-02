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

#include "get_ocr_constants.hpp"
#include "ocr_datas_constant.hpp"

#include "ppocr/utils/read_file.hpp"

#include "core/defines.hpp"

#include "utils/log.hpp"

#include <cassert>

rdp_ppocr::OcrDatasConstant const & rdp_ppocr::get_ocr_constants(std::string const & directory_)
{
    try {
        static std::string directory = directory_;
        static OcrDatasConstant const constants(
            ppocr::utils::load_from_file<ppocr::PpOcrDatas>((directory + "/datas.txt").c_str()),
            ppocr::utils::load_from_file<ppocr::ocr2::Glyphs>((directory + "/glyphs.txt").c_str()),
            ppocr::utils::load_from_file<ppocr::spell::Dictionary>((directory + "/dict.trie.txt").c_str()),
            ppocr::utils::load_from_file<ppocr::ocr2::WWordsLines>((directory + "/words_lines.txt").c_str()),
            ppocr::utils::load_from_file<ppocr::ocr2::Replacements>((directory + "/replacements.txt").c_str())
        );
        assert(directory == directory_);
        return constants;
    }
    catch (std::exception const & e) {
        LOG(LOG_ERR, "ppocr initialize: %s", e.what());
        throw;
    }
}

rdp_ppocr::OcrDatasConstant const & rdp_ppocr::get_ocr_constants(std::string directory, ocr::locale::LocaleId const & locale) {
    if (locale == ocr::locale::LocaleId::cyrillic) {
        directory += "/ppocr.latin-cyrillic";
    }
    else {
        directory += "/ppocr.latin";
    }
    return get_ocr_constants(std::move(directory));
}
