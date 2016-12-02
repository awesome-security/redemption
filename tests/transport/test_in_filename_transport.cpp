/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestXXX
#include "system/redemption_unit_tests.hpp"


// #define LOGPRINT
// #define LOGNULL

#include "utils/fileutils.hpp"
#include "utils/genrandom.hpp"

#include "transport/in_filename_transport.hpp"
#include "transport/out_meta_sequence_transport.hpp"

template <class Buf>
class OutputTransport
: public Transport
{
    Buf buf;

public:
    OutputTransport() = default;

    template<class T>
    explicit OutputTransport(const T & buf_params)
    : buf(buf_params)
    {}

    bool disconnect() override {
        return !this->buf.close();
    }

private:
    void do_send(const uint8_t * data, size_t len) override {
        const ssize_t res = this->buf.write(data, len);
        if (res < 0) {
            this->status = false;
            if (errno == ENOSPC) {
                LOG(LOG_ERR, "FILESYSTEM_FULL");
                errno = ENOSPC;
            }
            throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
        }
        this->last_quantum_sent += res;
    }

protected:
    Buf & buffer() noexcept
    { return this->buf; }

    const Buf & buffer() const noexcept
    { return this->buf; }

    typedef OutputTransport TransportType;
};

struct OutFilenameTransport
: OutputTransport<transbuf::ofile_buf_out>
{
    explicit OutFilenameTransport(const char * filename)
    {
        if (this->buffer().open(filename, 0440) < 0) {
            LOG(LOG_ERR, "failed opening=%s\n", filename);
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }
    }
};

struct CryptoOutFilenameTransport
: OutputTransport<transbuf::ocrypto_filename_buf>
{
    CryptoOutFilenameTransport(CryptoContext & crypto_ctx, Random & rnd, const char * filename)
    : CryptoOutFilenameTransport::TransportType(transbuf::ocrypto_filename_params{crypto_ctx, rnd})
    {
        if (this->buffer().open(filename, 0440) < 0) {
            LOG(LOG_ERR, "failed opening=%s\n", filename);
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }
    }
};


BOOST_AUTO_TEST_CASE(TestFilename)
{
    const char * const filename = "/tmp/inoufiletest.txt";

    ::unlink(filename);

    {
        OutFilenameTransport out(filename);
        out.send("ABCDE", 5);
    }

    {
        size_t base_len = 0;
        const uint8_t * base = reinterpret_cast<const uint8_t *>(basename_len(filename, base_len));

        int fd = ::open(filename, O_RDONLY);
        if (fd < 0) {
            LOG(LOG_ERR, "failed opening=%s\n", filename);
            BOOST_CHECK(false);
        }

        CryptoContext cctx;
        cctx.set_master_key(cstr_array_view(
            "\x00\x01\x02\x03\x04\x05\x06\x07"
            "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
            "\x10\x11\x12\x13\x14\x15\x16\x17"
            "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
        ));
        cctx.set_hmac_key(cstr_array_view("12345678901234567890123456789012"));
        InFilenameTransport in(cctx, fd, base, base_len);
        char s[5];
        char * sp = s;
        char ** p = &sp;
        in.recv(p, 5);
        BOOST_CHECK_EQUAL(sp-s, 5);
        BOOST_CHECK_EQUAL(strncmp(s, "ABCDE", 5), 0);
        try {
            sp = s;
            p = &sp;
            in.recv(p, 1);
// Behavior changed, first return 0, then exception
//            BOOST_CHECK(false);
        }
        catch (Error & e) {
        }
    }

    ::unlink(filename);
}

BOOST_AUTO_TEST_CASE(TestFilenameCrypto)
{
    OpenSSL_add_all_digests();

    const char * const filename = "/tmp/inoufiletest_crypt.txt";

    ::unlink(filename);

    CryptoContext cctx;
    cctx.set_master_key(cstr_array_view(
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
        "\x10\x11\x12\x13\x14\x15\x16\x17"
        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
    ));
    cctx.set_hmac_key(cstr_array_view("12345678901234567890123456789012"));

    {
        LCGRandom rnd(0);
        CryptoOutFilenameTransport out(cctx, rnd, filename);
        out.send("ABCDE", 5);
    }

    {
        size_t base_len = 0;
        const uint8_t * base = reinterpret_cast<const uint8_t *>(basename_len(filename, base_len));

        int fd = ::open(filename, O_RDONLY);
        if (fd < 0) {
            LOG(LOG_ERR, "failed opening=%s\n", filename);
            BOOST_CHECK(false);
        }

        InFilenameTransport in(cctx, fd, base, base_len);
        char s[5];
        char * sp = s;
        char ** p = &sp;
        in.recv(p, 5);
        BOOST_CHECK_EQUAL(sp-s, 5);
        BOOST_CHECK_EQUAL(strncmp(s, "ABCDE", 5), 0);
        try {
            sp = s;
            p = &sp;
            in.recv(p, 1);
// BEhavior changed. IS it OK ?
            BOOST_CHECK_EQUAL(sp-s, 0);
//            BOOST_CHECK(false);
        }
        catch (Error & e) {
        }
    }

    ::unlink(filename);
}
