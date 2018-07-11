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
*   Copyright (C) Wallix 2010-2017
*   Author(s): Christophe Grosjean
*/

#pragma once

#include "configs/autogen/enums.hpp"

#include <cstdint>

#include <sys/time.h>


class ReportMessageApi;

struct CaptureParams
{
    timeval now;
    // TODO: basename, record_path and record_tmp_path should be copied, we have no control of these variable lifecycles
    char const * basename;
    char const * record_tmp_path;
    char const * record_path;
    int groupid;
    ReportMessageApi * report_message;

    SmartVideoCropping smart_video_cropping;

    uint32_t verbose;
};
