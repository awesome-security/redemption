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
   Copyright (C) Wallix 2018
   Author(s): David Fort
*/

#pragma once

#include <chrono>
#include <memory>
#include <vector>
#include <string>

#include "transport/recorder_transport.hpp"
#include "transport/in_file_transport.hpp"
#include "utils/sugar/unique_fd.hpp"


/**
 *    @brief a transport that will replay a full capture
 */
class ReplayTransport : public Transport
{
public:
    enum class FdType : bool { Timer, AlwaysReady };
    enum class UncheckedPacket : uint8_t { None, Send };

    ReplayTransport(
        const char* fname, const char *ip_address, int port,
        FdType fd_type = FdType::Timer,
        UncheckedPacket unchecked_packet = UncheckedPacket::None);

    ~ReplayTransport();

    array_view_const_u8 get_public_key() const override;

    TlsResult enable_client_tls(
        bool server_cert_store, ServerCertCheck server_cert_check,
        ServerNotifier & server_notifier, const char * certif_path) override;

    void enable_server_tls(const char * certificate_password,
        const char * ssl_cipher_list) override;

    bool disconnect() override;
    bool connect() override;

    int get_fd() const override { return this->fd.fd(); }

    std::vector<std::string> const& get_infos() const noexcept { return this->infos; }

private:
    using PacketType = RecorderFile::PacketType;

    /** @brief the result of read_more_chunk */
    void read_more_chunk();

    void reschedule_timer();

    size_t do_partial_read(uint8_t * buffer, size_t len) override;

    Read do_atomic_read(uint8_t * buffer, size_t len) override;

    void do_send(const uint8_t * const buffer, size_t len) override;

    void unchecked_next_current_data(PacketType);
    array_view_const_u8 next_current_data(PacketType);
    void read_timer();

private:
    const std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point record_time;
    InFileTransport in_file;
    const unique_fd fd;
    const FdType fd_type;
    const UncheckedPacket unchecked_packet;

    struct Data
    {
        std::unique_ptr<uint8_t[]> data;
        size_t capacity = 0;
        size_t size;
        PacketType type;

        array_view_const_u8 av() const noexcept;
    };

    std::vector<Data> datas;
    size_t data_pos;
    std::vector<Data> gc_datas;

    struct Key
    {
        std::unique_ptr<uint8_t[]> data;
        size_t size = 0;
    };
    Key public_key;
    // uint64_t record_len = 0;
    bool is_eof = false;

    std::vector<std::string> infos;
};
