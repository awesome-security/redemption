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
    Copyright (C) Wallix 2015
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef _REDEMPTION_UTILS_VIRTUAL_CHANNEL_DATA_SENDER_HPP_
#define _REDEMPTION_UTILS_VIRTUAL_CHANNEL_DATA_SENDER_HPP_

#include "log.hpp"


inline static void msgdump_c(bool send, bool from_or_to_client,
    uint32_t total_length, uint32_t flags, const uint8_t* chunk_data,
    uint32_t chunk_data_length)
{
    if (send) {
        LOG(LOG_INFO, "Sending on channel (-1) n bytes");
    }
    else {
        LOG(LOG_INFO, "Recv done on rdpdr (-1) n bytes");
    }
    const uint32_t dest = (from_or_to_client
                           ? 0  // Client
                           : 1  // Server
                          );
    hexdump_c(reinterpret_cast<const uint8_t*>(&dest),
        sizeof(dest));
    hexdump_c(reinterpret_cast<uint8_t*>(&total_length),
        sizeof(total_length));
    hexdump_c(reinterpret_cast<uint8_t*>(&flags), sizeof(flags));
    hexdump_c(reinterpret_cast<uint8_t*>(&chunk_data_length),
        sizeof(chunk_data_length));
    hexdump_c(chunk_data, chunk_data_length);
    if (send) {
        LOG(LOG_INFO, "Sent dumped on channel (-1) n bytes");
    }
    else {
        LOG(LOG_INFO, "Dump done on rdpdr (-1) n bytes");
    }
}

class VirtualChannelDataSender
{
public:
    virtual ~VirtualChannelDataSender() = default;

    virtual VirtualChannelDataSender& SynchronousSender() {
        return *this;
    }

    virtual void operator()(uint32_t total_length, uint32_t flags,
        const uint8_t * chunk_data, uint32_t chunk_data_length) = 0;
};

#endif // #ifndef _REDEMPTION_UTILS_VIRTUAL_CHANNEL_DATA_SENDER_HPP_
