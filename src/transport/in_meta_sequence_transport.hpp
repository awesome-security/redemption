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
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#ifndef REDEMPTION_TRANSPORT_IN_META_SEQUENCE_TRANSPORT_HPP
#define REDEMPTION_TRANSPORT_IN_META_SEQUENCE_TRANSPORT_HPP

#include <cerrno>
#include <fcntl.h>
#include <snappy-c.h>
#include <stdint.h>
#include <unistd.h>
#include <memory>
#include "openssl_crypto.hpp"
#include "log.hpp"
#include "transport/detail/meta_opener.hpp"
#include "transport/mixin_transport.hpp"
#include "transport/buffer/file_buf.hpp"

#include "transport/cryptofile.hpp"

#include "urandom_read.hpp"

namespace transfil {

    class decrypt_filter
    {
        char           buf[CRYPTO_BUFFER_SIZE]; //
        EVP_CIPHER_CTX ectx;                    // [en|de]cryption context
        uint32_t       pos;                     // current position in buf
        uint32_t       raw_size;                // the unciphered/uncompressed file size
        uint32_t       state;                   // enum crypto_file_state
        unsigned int   MAX_CIPHERED_SIZE;       // = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;

    public:
        decrypt_filter() = default;
        //: pos(0)
        //, raw_size(0)
        //, state(0)
        //, MAX_CIPHERED_SIZE(0)
        //{}

        // transbuf::ifile_buf& src | transbuf::ifile_buf_crypto& src
        template<class Source>
        int open(Source & src, unsigned char * trace_key)
        {
            ::memset(this->buf, 0, sizeof(this->buf));
            ::memset(&this->ectx, 0, sizeof(this->ectx));

            this->pos = 0;
            this->raw_size = 0;
            this->state = 0;
            const size_t MAX_COMPRESSED_SIZE = ::snappy_max_compressed_length(CRYPTO_BUFFER_SIZE);
            this->MAX_CIPHERED_SIZE = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;

            unsigned char tmp_buf[40];

            if (const ssize_t err = this->raw_read(src, tmp_buf, 40)) {
                return err;
            }

            // Check magic
            const uint32_t magic = tmp_buf[0] + (tmp_buf[1] << 8) + (tmp_buf[2] << 16) + (tmp_buf[3] << 24);
            if (magic != WABCRYPTOFILE_MAGIC) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Wrong file type %04x != %04x\n",
                    ::getpid(), magic, WABCRYPTOFILE_MAGIC);
                return -1;
            }
            const int version = tmp_buf[4] + (tmp_buf[5] << 8) + (tmp_buf[6] << 16) + (tmp_buf[7] << 24);
            if (version > WABCRYPTOFILE_VERSION) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Unsupported version %04x > %04x\n",
                    ::getpid(), version, WABCRYPTOFILE_VERSION);
                return -1;
            }

            unsigned char * const iv = tmp_buf + 8;

            const EVP_CIPHER * cipher  = ::EVP_aes_256_cbc();
            const unsigned int salt[]  = { 12345, 54321 };    // suspicious, to check...
            const int          nrounds = 5;
            unsigned char      key[32];
            const int i = ::EVP_BytesToKey(cipher, ::EVP_sha1(), reinterpret_cast<const unsigned char *>(salt),
                                           trace_key, CRYPTO_KEY_LENGTH, nrounds, key, nullptr);
            if (i != 32) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: EVP_BytesToKey size is wrong\n", ::getpid());
                return -1;
            }

            ::EVP_CIPHER_CTX_init(&this->ectx);
            if(::EVP_DecryptInit_ex(&this->ectx, cipher, nullptr, key, iv) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize decrypt context\n", ::getpid());
                return -1;
            }

            return 0;
        }

        template<class Source>
        ssize_t read(Source & src, void * data, size_t len)
        {
            if (this->state & CF_EOF) {
                //printf("cf EOF\n");
                return 0;
            }

            unsigned int requested_size = len;

            while (requested_size > 0) {
                // Check how much we have decoded
                if (!this->raw_size) {
                    // Buffer is empty. Read a chunk from file
                    /*
                     i f (-1 == ::do_chunk_read*(this)) {
                         return -1;
                }
                */
                    // TODO: avoid reading size directly into an integer, performance enhancement is minimal
                    // and it's not portable because of endianness issue => read in a buffer and decode by hand
                    unsigned char tmp_buf[4] = {};
                    if (const int err = this->raw_read(src, tmp_buf, 4)) {
                        return err;
                    }

                    uint32_t ciphered_buf_size = tmp_buf[0] + (tmp_buf[1] << 8) + (tmp_buf[2] << 16) + (tmp_buf[3] << 24);

                    if (ciphered_buf_size == WABCRYPTOFILE_EOF_MAGIC) { // end of file
                        this->state |= CF_EOF;
                        this->pos = 0;
                        this->raw_size = 0;
                    }
                    else {
                        if (ciphered_buf_size > this->MAX_CIPHERED_SIZE) {
                            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Integrity error, erroneous chunk size!\n", ::getpid());
                            return -1;
                        }
                        else {
                            uint32_t compressed_buf_size = ciphered_buf_size + AES_BLOCK_SIZE;
                            //char ciphered_buf[ciphered_buf_size];
                            unsigned char ciphered_buf[65536];
                            //char compressed_buf[compressed_buf_size];
                            unsigned char compressed_buf[65536];

                            if (const ssize_t err = this->raw_read(src, ciphered_buf, ciphered_buf_size)) {
                                return err;
                            }

                            if (this->xaes_decrypt(ciphered_buf, ciphered_buf_size, compressed_buf, &compressed_buf_size)) {
                                return -1;
                            }

                            size_t chunk_size = CRYPTO_BUFFER_SIZE;
                            const snappy_status status = snappy_uncompress(reinterpret_cast<char *>(compressed_buf),
                                                                           compressed_buf_size, this->buf, &chunk_size);

                            switch (status)
                            {
                                case SNAPPY_OK:
                                    break;
                                case SNAPPY_INVALID_INPUT:
                                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Snappy decompression failed with status code INVALID_INPUT!\n", getpid());
                                    return -1;
                                case SNAPPY_BUFFER_TOO_SMALL:
                                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Snappy decompression failed with status code BUFFER_TOO_SMALL!\n", getpid());
                                    return -1;
                                default:
                                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Snappy decompression failed with unknown status code (%d)!\n", getpid(), status);
                                    return -1;
                            }

                            this->pos = 0;
                            // When reading, raw_size represent the current chunk size
                            this->raw_size = chunk_size;
                        }
                    }

                    // TODO: check that
                    if (!this->raw_size) { // end of file reached
                        break;
                    }
                }
                // remaining_size is the amount of data available in decoded buffer
                unsigned int remaining_size = this->raw_size - this->pos;
                // Check how much we can copy
                unsigned int copiable_size = MIN(remaining_size, requested_size);
                // Copy buffer to caller
                ::memcpy(static_cast<char*>(data) + (len - requested_size), this->buf + this->pos, copiable_size);
                this->pos      += copiable_size;
                requested_size -= copiable_size;
                // Check if we reach the end
                if (this->raw_size == this->pos) {
                    this->raw_size = 0;
                }
            }
            return len - requested_size;
        }

    private:
        ///\return 0 if success, otherwise a negatif number
        template<class Source>
        ssize_t raw_read(Source & src, void * data, size_t len)
        {
            ssize_t err = src.read(data, len);
            return err < ssize_t(len) ? (err < 0 ? err : -1) : 0;
        }

        int xaes_decrypt(const unsigned char *src_buf, uint32_t src_sz, unsigned char *dst_buf, uint32_t *dst_sz)
        {
            int safe_size = *dst_sz;
            int remaining_size = 0;

            /* allows reusing of ectx for multiple encryption cycles */
            if (EVP_DecryptInit_ex(&this->ectx, nullptr, nullptr, nullptr, nullptr) != 1){
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not prepare decryption context!\n", getpid());
                return -1;
            }
            if (EVP_DecryptUpdate(&this->ectx, dst_buf, &safe_size, src_buf, src_sz) != 1){
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not decrypt data!\n", getpid());
                return -1;
            }
            if (EVP_DecryptFinal_ex(&this->ectx, dst_buf + safe_size, &remaining_size) != 1){
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not finish decryption!\n", getpid());
                return -1;
            }
            *dst_sz = safe_size + remaining_size;
            return 0;
        }
    };

}

namespace transbuf {

    class ifile_buf
    {
    public:
        transfil::decrypt_filter decrypt;
        CryptoContext * cctx;
        int encryption;

        int fd;

        ifile_buf(CryptoContext * cctx, int encryption) : fd(-1) {}
        
        ~ifile_buf()
        {
            this->close();
        }

        int open(const char * filename)
        {
            this->close();
            this->fd = ::open(filename, O_RDONLY);
            return this->fd;
        }

        int open(const char * filename, mode_t /*mode*/)
        {
            TODO("see why mode is ignored even if it's provided as a parameter?");
            this->close();
            this->fd = ::open(filename, O_RDONLY);
            return this->fd;
        }

        int close()
        {
            if (this->is_open()) {
                const int ret = ::close(this->fd);
                this->fd = -1;
                return ret;
            }
            return 0;
        }

        bool is_open() const noexcept
        { return -1 != this->fd; }

        ssize_t read(void * data, size_t len)
        {
            TODO("this is blocking read, add support for timeout reading");
            TODO("add check for O_WOULDBLOCK, as this is is blockig it would be bad");
            size_t remaining_len = len;
            while (remaining_len) {
                ssize_t ret = ::read(this->fd, static_cast<char*>(data) + (len - remaining_len), remaining_len);
                if (ret < 0){
                    if (errno == EINTR){
                        continue;
                    }
                    // Error should still be there next time we try to read
                    if (remaining_len != len){
                        return len - remaining_len;
                    }
                    return ret;
                }
                // We must exit loop or we will enter infinite loop
                if (ret == 0){
                    break;
                }
                remaining_len -= ret;
            }
            return len - remaining_len;        
        }

        off64_t seek(off64_t offset, int whence) const
        { return lseek64(this->fd, offset, whence); }
    };

// =============================================================================

    class ifile_buf_crypto
    {
    public:
        transfil::decrypt_filter decrypt;
        CryptoContext * cctx;
        class ifile_buf
        {
        public:
            transfil::decrypt_filter decrypt;
            CryptoContext * cctx;
            int encryption;

            int fd;

            ifile_buf(CryptoContext * cctx, int encryption) : fd(-1) {}
            
            ~ifile_buf()
            {
                this->close();
            }

            int open(const char * filename)
            {
                this->close();
                this->fd = ::open(filename, O_RDONLY);
                return this->fd;
            }

            int open(const char * filename, mode_t /*mode*/)
            {
                TODO("see why mode is ignored even if it's provided as a parameter?");
                this->close();
                this->fd = ::open(filename, O_RDONLY);
                return this->fd;
            }

            int close()
            {
                if (this->is_open()) {
                    const int ret = ::close(this->fd);
                    this->fd = -1;
                    return ret;
                }
                return 0;
            }

            bool is_open() const noexcept
            { return -1 != this->fd; }

            ssize_t read(void * data, size_t len)
            {
                TODO("this is blocking read, add support for timeout reading");
                TODO("add check for O_WOULDBLOCK, as this is is blockig it would be bad");
                size_t remaining_len = len;
                while (remaining_len) {
                    ssize_t ret = ::read(this->fd, static_cast<char*>(data) + (len - remaining_len), remaining_len);
                    if (ret < 0){
                        if (errno == EINTR){
                            continue;
                        }
                        // Error should still be there next time we try to read
                        if (remaining_len != len){
                            return len - remaining_len;
                        }
                        return ret;
                    }
                    // We must exit loop or we will enter infinite loop
                    if (ret == 0){
                        break;
                    }
                    remaining_len -= ret;
                }
                return len - remaining_len;        
            }

            off64_t seek(off64_t offset, int whence) const
            {
                return lseek64(this->fd, offset, whence);
            }
        } file;

        int encryption;

    public:
        explicit ifile_buf_crypto(CryptoContext * cctx, int encryption)
        : cctx(cctx)
        , file(cctx, 0/*encryption*/)
        {}

        int open(const char * filename, mode_t mode = 0600)
        {
            if (encryption){

                int err = this->file.open(filename, mode);
                if (err < 0) {
                    return err;
                }

                unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
                unsigned char derivator[DERIVATOR_LENGTH];

                this->cctx->get_derivator(filename, derivator, DERIVATOR_LENGTH);
                if (-1 == this->cctx->compute_hmac(trace_key, derivator)) {
                    return -1;
                }

                return this->decrypt.open(this->file, trace_key);
            }
            else {
                return this->file.open(filename, mode);
            }
        }

        ssize_t read(void * data, size_t len)
        {
            if (encryption){
                return this->decrypt.read(this->file, data, len);
            }
            else {
                return this->file.read(data, len);
            }
        }

        int close()
        {
            return this->file.close();
        }

        bool is_open() const noexcept
        { 
            return this->file.is_open();
        }

        off64_t seek(off64_t offset, int whence) const
        {
            return this->file.seek(offset, whence);
        }
    };
}

// =============================================================================
namespace detail {

    class in_meta_sequence_buf_crypto
    {
        transfil::decrypt_filter cfb_decrypt;
        CryptoContext * cfb_cctx;
        transbuf::ifile_buf cfb_file;

    public:
        int cfb_open(const char * filename, mode_t mode = 0600)
        {
            unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
            unsigned char derivator[DERIVATOR_LENGTH];

            this->cfb_cctx->get_derivator(filename, derivator, DERIVATOR_LENGTH);
            if (-1 == this->cfb_cctx->compute_hmac(trace_key, derivator)) {
                return -1;
            }

            int err = this->cfb_file.open(filename, mode);
            if (err < 0) {
                return err;
            }

            return this->cfb_decrypt.open(this->cfb_file, trace_key);
        }

        ssize_t cfb_read(void * data, size_t len)
        { return this->cfb_decrypt.read(this->cfb_file, data, len); }

        int close()
        { return this->cfb_file.close(); }

        bool cfb_is_open() const noexcept
        { return this->cfb_file.is_open(); }

        char rl_buf[1024];
        char * rl_eof;
        char * rl_cur;

        transfil::decrypt_filter buf_meta_decrypt;
        CryptoContext * buf_meta_cctx;
        transbuf::ifile_buf buf_meta_file;

        int reader_read(int err)
        {
            LOG(LOG_INFO, "reader_read");
 
            ssize_t ret = this->buf_meta_decrypt.read(
                this->buf_meta_file, this->rl_buf, sizeof(this->rl_buf));
            if (ret < 0 && errno != EINTR) {
                LOG(LOG_WARNING, "read failed");
                return -ERR_TRANSPORT_READ_FAILED;
            }
            LOG(LOG_INFO, "reader_read: rl_buf=%s %d %d %d", this->rl_buf
                , static_cast<int>(this->rl_cur-this->rl_buf)
                , static_cast<int>(this->rl_eof-this->rl_buf)
                , static_cast<int>(ret));

            if (ret == 0) {
                LOG(LOG_WARNING, "read failed xxx");
                return -err;
            }
            this->rl_eof = this->rl_buf + ret;
            this->rl_cur = this->rl_buf;
            return 0;
        }

        ssize_t reader_read_line(char * dest, size_t len, int err)
        {
            LOG(LOG_INFO, "reader_read_line");
            ssize_t total_read = 0;
            LOG(LOG_INFO, "rl_buf=%s [%d] len=%d", this->rl_cur, static_cast<int>(this->rl_cur - this->rl_buf), static_cast<int>(len));
            while (1) {
                char * pos = std::find(this->rl_cur, this->rl_eof, '\n');
                LOG(LOG_INFO, "rl_buf (%d) %s [%d]", static_cast<int>(pos - this->rl_buf), this->rl_cur, static_cast<int>(this->rl_cur - this->rl_buf));
                if (len < static_cast<size_t>(pos - this->rl_cur)) {
                    total_read += len;
                    memcpy(dest, this->rl_cur, len);
                    this->rl_cur += len;
                    break;
                }
                total_read += pos - this->rl_cur;
                memcpy(dest, this->rl_cur, pos - this->rl_cur);
                dest += pos - this->rl_cur;
                this->rl_cur = pos + 1;
                if (pos != this->rl_eof) {
                    break;
                }
                if (int e = this->reader_read(err)) {
                    LOG(LOG_INFO, "reader_read result: rl_buf=%s %d %d %d", this->rl_buf
                        , static_cast<int>(this->rl_cur-this->rl_buf)
                        , static_cast<int>(this->rl_eof-this->rl_buf)
                        , e);
                    return e;
                }
            }
            return total_read;
        }

        int reader_next_line()
        {
            LOG(LOG_INFO, "reader_next_line");
            
            char * pos;
            while ((pos = std::find(this->rl_cur, this->rl_eof, '\n')) == this->rl_eof) {
                if (int e = this->reader_read(ERR_TRANSPORT_READ_FAILED)) {
                    return e;
                }
            }
            this->rl_cur = pos+1;
            return 0;
        }

        unsigned meta_header_version;
        bool meta_header_has_checksum;

        MetaLine meta_line;
        char meta_path[2048];
        uint32_t verbose;

    public:
        explicit in_meta_sequence_buf_crypto(const char * meta_filename,
                                             CryptoContext * cctx_buf,
                                             CryptoContext * cctx_meta,
                                             uint32_t verbose)
        : cfb_cctx(cctx_buf)
        , cfb_file(cctx_buf, 1/*encryption*/)
        , rl_eof(rl_buf)
        , rl_cur(rl_buf)
        , buf_meta_decrypt{}
        , buf_meta_cctx(cctx_meta)
        , buf_meta_file(cctx_meta, 1/*encryption*/)
        , meta_header_version(1)
        , meta_header_has_checksum(false)
        , verbose(verbose)
        {
            LOG(LOG_INFO, "in_meta_sequence_buf_crypto v2 %s", meta_filename);
           
            unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
            unsigned char derivator[DERIVATOR_LENGTH];

            this->buf_meta_cctx->get_derivator(meta_filename, derivator, DERIVATOR_LENGTH);
            if (-1 == this->buf_meta_cctx->compute_hmac(trace_key, derivator)) {
                LOG(LOG_WARNING, "read failed 4.0");
                throw Error(ERR_TRANSPORT_OPEN_FAILED);
            }

            LOG(LOG_INFO, "in_meta_sequence_buf_crypto : opening meta_file %s", meta_filename);

            int err = this->buf_meta_file.open(meta_filename, 0600);
            if (err < 0) {
                LOG(LOG_WARNING, "read failed 4.1");
                throw Error(ERR_TRANSPORT_OPEN_FAILED);
            }

            LOG(LOG_INFO, "in_meta_sequence_buf_crypto : decryption of meta_file %s", meta_filename);

            if (this->buf_meta_decrypt.open(this->buf_meta_file, trace_key) < 0){
                LOG(LOG_WARNING, "read failed 4.2");
                throw Error(ERR_TRANSPORT_OPEN_FAILED);
            }

            LOG(LOG_INFO, "in_meta_sequence_buf_crypto : read first line of meta_file %s", meta_filename);

            char line[32];
            auto sz = this->reader_read_line(line, sizeof(line), ERR_TRANSPORT_READ_FAILED);
            if (sz < 0) {
                LOG(LOG_WARNING, "read failed 1");
                throw Error(ERR_TRANSPORT_READ_FAILED, errno);
            }

            // v2
            if (line[0] == 'v') {
                LOG(LOG_INFO, "in_meta_sequence_buf_crypto : first line of meta_file %s is v2", meta_filename);
                if (this->reader_next_line() 
                 || (sz = this->reader_read_line(line, sizeof(line), ERR_TRANSPORT_READ_FAILED)) < 0
                ) {
                    LOG(LOG_WARNING, "read failed 2");
                    throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                }
                this->meta_header_version = 2;
                this->meta_header_has_checksum = (line[0] == 'c');
                LOG(LOG_INFO, "in_meta_sequence_buf_crypto : header of %s is v2 checksum=%s", meta_filename,
                    this->meta_header_has_checksum?"yes":"no");
            }
            // else v1

            LOG(LOG_INFO, "in_meta_sequence_buf_crypto : second line of meta_file %s skipped", meta_filename);
            if (this->reader_next_line()) {
                LOG(LOG_WARNING, "read failed 3.0");
                throw Error(ERR_TRANSPORT_READ_FAILED, errno);
            }

            LOG(LOG_INFO, "in_meta_sequence_buf_crypto : 3rd line of meta_file %s skipped", meta_filename);
            if (this->reader_next_line()) {
                LOG(LOG_WARNING, "read failed 3.1");
                throw Error(ERR_TRANSPORT_READ_FAILED, errno);
            }

            this->meta_line.start_time = 0;
            this->meta_line.stop_time = 0;

            this->meta_path[0] = 0;

            char basename[1024] = {};
            char extension[256] = {};

            canonical_path( meta_filename
                          , this->meta_path, sizeof(this->meta_path)
                          , basename, sizeof(basename)
                          , extension, sizeof(extension)
                          , this->verbose);
            LOG(LOG_INFO, "in_meta_sequence_buf_crypto : end of constructor");

        }

        ssize_t read(void * data, size_t len)
        {
            LOG(LOG_INFO, "read file=%s", this->meta_line.filename);

            if (!this->cfb_is_open()) {
                if (const int e1 = this->next_line()) {
                    return e1 < 0 ? e1 : -1;
                }
                else {
                    const int e2 = this->cfb_open(this->meta_line.filename);
                    if (e2 < 0) {
                        return e2;
                    }
                }
            }

            LOG(LOG_INFO, "read file=%s", this->meta_line.filename);

            ssize_t res = this->cfb_read(data, len);
            if (res < 0) {
                return res;
            }
            if (size_t(res) != len) {
                ssize_t res2 = res;
                do {
                    if (/*const ssize_t err = */this->close()) {
                        return res;
                    }
                    data = static_cast<char*>(data) + res2;
                    if (this->next_line()) {
                        return res;
                    }
                    else {
                        const int e = this->cfb_open(this->meta_line.filename);
                        if (e < 0) {
                            return res;
                        }
                    }
                    len -= res2;
                    res2 = this->cfb_read(data, len);
                    if (res2 < 0) {
                        return res;
                    }
                    res += res2;
                } while (size_t(res2) != len);
            }

            return res;
        }

        /// \return 0 if success
        int next()
        {
            LOG(LOG_INFO, "next");
            if (this->cfb_is_open()) {
                this->close();
            }

            return this->reader_next_line();
        }

        int read_meta_file_v1(MetaLine & meta_line) {
            LOG(LOG_INFO, "read_meta_file_v1");
            char line[1024 + (std::numeric_limits<unsigned>::digits10 + 1) * 2 + 4 + 64 * 2 + 2];
            ssize_t len = this->reader_read_line(line, sizeof(line) - 1, ERR_TRANSPORT_NO_MORE_DATA);
            if (len < 0) {
                return -len;
            }
            line[len] = 0;

            // Line format "fffff sssss eeeee hhhhh HHHHH"
            //                               ^  ^  ^  ^
            //                               |  |  |  |
            //                               |hash1|  |
            //                               |     |  |
            //                           space3    |hash2
            //                                     |
            //                                   space4
            //
            // filename(1 or >) + space(1) + start_sec(1 or >) + space(1) + stop_sec(1 or >) +
            //     space(1) + hash1(64) + space(1) + hash2(64) >= 135
            typedef std::reverse_iterator<char*> reverse_iterator;

            using std::begin;

            reverse_iterator last(line);
            reverse_iterator first(line + len);
            reverse_iterator e1 = std::find(first, last, ' ');
            if (e1 - first == 64) {
                int err = 0;
                auto phash = begin(meta_line.hash2);
                for (char * b = e1.base(), * e = b + 64; e != b; ++b, ++phash) {
                    *phash = (chex_to_int(*b, err) << 4);
                    *phash |= chex_to_int(*++b, err);
                }
                REDASSERT(!err);
            }

            reverse_iterator e2 = (e1 == last) ? e1 : std::find(e1 + 1, last, ' ');
            if (e2 - (e1 + 1) == 64) {
                int err = 0;
                auto phash = begin(meta_line.hash1);
                for (char * b = e2.base(), * e = b + 64; e != b; ++b, ++phash) {
                    *phash = (chex_to_int(*b, err) << 4);
                    *phash |= chex_to_int(*++b, err);
                }
                REDASSERT(!err);
            }

            if (e1 - first == 64 && e2 != last) {
                first = e2 + 1;
                e1 = std::find(first, last, ' ');
                e2 = (e1 == last) ? e1 : std::find(e1 + 1, last, ' ');
            }

            meta_line.stop_time = meta_parse_sec(e1.base(), first.base());
            if (e1 != last) {
                ++e1;
            }
            meta_line.start_time = meta_parse_sec(e2.base(), e1.base());

            if (e2 != last) {
                *e2 = 0;
            }

            auto path_len = std::min(int(e2.base() - line), PATH_MAX);
            memcpy(meta_line.filename, line, path_len);
            meta_line.filename[path_len] = 0;

            return 0;
        }

        char const * sread_filename(char * p, char const * e, char const * pline)
        {
            e -= 1;
            for (; p < e && *pline && *pline != ' ' && (*pline == '\\' ? *++pline : true); ++pline, ++p) {
                *p = *pline;
            }
            *p = 0;
            return pline;
        }

        template<bool read_start_stop_time>
        int read_meta_file_v2_impl(bool has_checksum, MetaLine & meta_line) {
            LOG(LOG_INFO, "read_meta_file_v2_impl");
            char line[
                PATH_MAX + 1 + 1 +
                (std::numeric_limits<long long>::digits10 + 1 + 1) * 8 +
                (std::numeric_limits<unsigned long long>::digits10 + 1 + 1) * 2 +
                (1 + MD_HASH_LENGTH*2) * 2 +
                2
            ];
            ssize_t len = this->reader_read_line(line, sizeof(line) - 1, ERR_TRANSPORT_NO_MORE_DATA);
            if (len < 0) {
                return -len;
            }
            line[len] = 0;
            LOG(LOG_INFO, "line=%s", line);

            // Line format "fffff
            // st_size st_mode st_uid st_gid st_dev st_ino st_mtime st_ctime
            // sssss eeeee hhhhh HHHHH"
            //            ^  ^  ^  ^
            //            |  |  |  |
            //            |hash1|  |
            //            |     |  |
            //        space3    |hash2
            //                  |
            //                space4
            //
            // filename(1 or >) + space(1) + stat_info(ll|ull * 8) +
            //     space(1) + start_sec(1 or >) + space(1) + stop_sec(1 or >) +
            //     space(1) + hash1(64) + space(1) + hash2(64) >= 135

            auto pline = line + (this->sread_filename(std::begin(meta_line.filename), std::end(meta_line.filename), line) - line);

            LOG(LOG_INFO, "meta_line.filename=%s", meta_line.filename);

            int err = 0;
            auto pend = pline;                   meta_line.size       = strtoll (pline, &pend, 10);
            err |= (*pend != ' '); pline = pend; meta_line.mode       = strtoull(pline, &pend, 10);
            err |= (*pend != ' '); pline = pend; meta_line.uid        = strtoll (pline, &pend, 10);
            err |= (*pend != ' '); pline = pend; meta_line.gid        = strtoll (pline, &pend, 10);
            err |= (*pend != ' '); pline = pend; meta_line.dev        = strtoull(pline, &pend, 10);
            err |= (*pend != ' '); pline = pend; meta_line.ino        = strtoll (pline, &pend, 10);
            err |= (*pend != ' '); pline = pend; meta_line.mtime      = strtoll (pline, &pend, 10);
            err |= (*pend != ' '); pline = pend; meta_line.ctime      = strtoll (pline, &pend, 10);
            if (read_start_stop_time) {
            err |= (*pend != ' '); pline = pend; meta_line.start_time = strtoll (pline, &pend, 10);
            err |= (*pend != ' '); pline = pend; meta_line.stop_time  = strtoll (pline, &pend, 10);
            }

            TODO("Why do this with lambda ? Is it so important to avoid typing 3 lines of code two times ?");
            if (has_checksum
             && !(err |= (len - (pend - line) != (sizeof(meta_line.hash1) + sizeof(meta_line.hash2)) * 2 + 2))
            ) {
                auto local_read = [&](unsigned char (&hash)[MD_HASH_LENGTH]) {
                    auto phash = std::begin(hash);
                    for (auto e = ++pend + sizeof(hash) * 2u; pend != e; ++pend, ++phash) {
                        *phash = (chex_to_int(*pend, err) << 4);
                        *phash |= chex_to_int(*++pend, err);
                    }
                };
                local_read(meta_line.hash1);
                err |= (*pend != ' ');
                local_read(meta_line.hash2);
            }

            err |= bool(*pend);

            if (err) {
                LOG(LOG_WARNING, "read failed 5");
                throw Error(ERR_TRANSPORT_READ_FAILED);
            }
            LOG(LOG_INFO, "read_meta_file_v2_impl done");
            return 0;
        }


        int read_meta_file_v2(MetaLine & meta_line) {
            LOG(LOG_INFO, "read_meta_file_v2");
            return read_meta_file_v2_impl<true>(this->meta_header_has_checksum, meta_line);
        }

        int read_meta_file(MetaLine & meta_line)
        {
            LOG(LOG_INFO, "read_meta_file");
            if (this->meta_header_version == 1) {
                return this->read_meta_file_v1(meta_line);
            }
            else {
                return this->read_meta_file_v2(meta_line);
            }
        }

    private:
        int open_next() {
            LOG(LOG_INFO, "open_next");
            if (const int e = this->reader_next_line()) {
                return e < 0 ? e : -1;
            }
            const int e = this->cfb_open(this->meta_line.filename);
            return (e < 0) ? e : 0;
        }

        int next_line()
        {
            LOG(LOG_INFO, "next_line");
            if (auto err = this->read_meta_file(this->meta_line)) {
                return err;
            }

            LOG(LOG_INFO, "check if file exists");

            if (!file_exist(this->meta_line.filename)) {
                char original_path[1024] = {};
                char basename[1024] = {};
                char extension[256] = {};
                char filename[2048] = {};

                canonical_path( this->meta_line.filename
                              , original_path, sizeof(original_path)
                              , basename, sizeof(basename)
                              , extension, sizeof(extension)
                              , this->verbose);
                snprintf(filename, sizeof(filename), "%s%s%s", this->meta_path, basename, extension);

                if (file_exist(filename)) {
                    strcpy(this->meta_line.filename, filename);
                }
            }
            LOG(LOG_INFO, "next_line done");

            return 0;
        }

    public:
        const char * current_path() const noexcept
        {
            LOG(LOG_INFO, "current_path");
            return this->meta_line.filename;
        }

        time_t get_begin_chunk_time() const noexcept
        {
            LOG(LOG_INFO, "get_begin_chunk_time");
            return this->meta_line.start_time;
        }

        time_t get_end_chunk_time() const noexcept
        { return this->meta_line.stop_time; }
    };
    
    
    
    
    class in_meta_sequence_buf_flat
    : public transbuf::ifile_buf
    {
        struct ReaderBuf
        {
            transbuf::ifile_buf & buf;

            explicit ReaderBuf(transbuf::ifile_buf & buf)
            : buf(buf)
            {}

            ssize_t reader_read(char * buf, size_t len) const {
                return this->buf.read(buf, len);
            }
        };

        transbuf::ifile_buf buf_meta;
        ReaderLine<ReaderBuf> reader;
        MetaHeader meta_header;
        MetaLine meta_line;
        char meta_path[2048];
        uint32_t verbose;

    public:
        explicit in_meta_sequence_buf_flat(
            const char * meta_filename,
            CryptoContext * cctx_buf,
            CryptoContext * cctx_meta,
            uint32_t verbose)
        : transbuf::ifile_buf(cctx_buf, 0/*encryption*/)
        , buf_meta(cctx_meta, 0/*encryption*/)
        , reader([&]() {
            if (this->buf_meta.open(meta_filename) < 0) {
                LOG(LOG_WARNING, "read failed 6");
                throw Error(ERR_TRANSPORT_OPEN_FAILED);
            }
            return ReaderBuf{this->buf_meta};
        }())
        , meta_header(read_meta_headers(this->reader))
        , verbose(verbose)
        {
            this->meta_line.start_time = 0;
            this->meta_line.stop_time = 0;

            this->meta_path[0] = 0;

            char basename[1024] = {};
            char extension[256] = {};

            canonical_path( meta_filename
                          , this->meta_path, sizeof(this->meta_path)
                          , basename, sizeof(basename)
                          , extension, sizeof(extension)
                          , this->verbose);
        }

        ssize_t read(void * data, size_t len)
        {
            if (!this->is_open()) {
                if (const int e = this->open_next()) {
                    return e;
                }
            }

            ssize_t res = this->transbuf::ifile_buf::read(data, len);
            if (res < 0) {
                return res;
            }
            if (size_t(res) != len) {
                ssize_t res2 = res;
                do {
                    if (/*const ssize_t err = */this->close()) {
                        return res;
                    }
                    data = static_cast<char*>(data) + res2;
                    if (/*const int err = */this->open_next()) {
                        return res;
                    }
                    len -= res2;
                    res2 = this->transbuf::ifile_buf::read(data, len);
                    if (res2 < 0) {
                        return res;
                    }
                    res += res2;
                } while (size_t(res2) != len);
            }
            return res;
        }

        /// \return 0 if success
        int next()
        {
            if (this->is_open()) {
                this->close();
            }

            return this->next_line();
        }

    private:
        int open_next() {
            if (const int e = this->next_line()) {
                return e < 0 ? e : -1;
            }
            const int e = this->transbuf::ifile_buf::open(this->meta_line.filename);
            return (e < 0) ? e : 0;
        }

        int next_line()
        {
            if (auto err = read_meta_file(this->reader, this->meta_header, this->meta_line)) {
                return err;
            }

            if (!file_exist(this->meta_line.filename)) {
                char original_path[1024] = {};
                char basename[1024] = {};
                char extension[256] = {};
                char filename[2048] = {};

                canonical_path( this->meta_line.filename
                              , original_path, sizeof(original_path)
                              , basename, sizeof(basename)
                              , extension, sizeof(extension)
                              , this->verbose);
                snprintf(filename, sizeof(filename), "%s%s%s", this->meta_path, basename, extension);

                if (file_exist(filename)) {
                    strcpy(this->meta_line.filename, filename);
                }
            }

            return 0;
        }

    public:
        const char * current_path() const noexcept
        { return this->meta_line.filename; }

        time_t get_begin_chunk_time() const noexcept
        { return this->meta_line.start_time; }

        time_t get_end_chunk_time() const noexcept
        { return this->meta_line.stop_time; }
    };    
}

struct InMetaSequenceTransport : public Transport
{
    detail::in_meta_sequence_buf_flat buf;

    InMetaSequenceTransport(CryptoContext * cctx, const char * filename, const char * extension, uint32_t verbose = 0)
    : buf(detail::temporary_concat(filename, extension).str, cctx, cctx, verbose)
    {
        this->verbose = verbose;
    }

    explicit InMetaSequenceTransport(CryptoContext * cctx, const char * filename, uint32_t verbose = 0)
    : buf(filename, cctx, cctx, verbose)
    {
        this->verbose = verbose;
    }

    bool disconnect() override {
        return !this->buf.close();
    }

    void do_recv(char ** pbuffer, size_t len) override {
        const ssize_t res = this->buf.read(*pbuffer, len);
        if (res < 0){
            this->status = false;
            throw Error(ERR_TRANSPORT_READ_FAILED, res);
        }
        *pbuffer += res;
        this->last_quantum_received += res;
        if (static_cast<size_t>(res) != len){
            this->status = false;
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
        }
    }

    bool next() override {
        if (this->status == false) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }
        const ssize_t res = this->buffer().next();
        if (res){
            this->status = false;
            if (res < 0) {
                throw Error(ERR_TRANSPORT_READ_FAILED, -res);
            }
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
        }
        ++this->seqno;
        return true;
    }

    unsigned begin_chunk_time() const noexcept
    { return this->buffer().get_begin_chunk_time(); }

    unsigned end_chunk_time() const noexcept
    { return this->buffer().get_end_chunk_time(); }

    const char * path() const noexcept
    { return this->buffer().current_path(); }

    detail::in_meta_sequence_buf_flat & buffer() noexcept
    { return this->buf; }

    const detail::in_meta_sequence_buf_flat & buffer() const noexcept
    { return this->buf; }

};

struct CryptoInMetaSequenceTransport : public Transport
{
    detail::in_meta_sequence_buf_crypto buf;

    CryptoInMetaSequenceTransport(CryptoContext * cctx, const char * filename, const char * extension, uint32_t verbose = 0)
    : buf(detail::temporary_concat(filename, extension).c_str(), cctx, cctx, verbose)
    {}

    CryptoInMetaSequenceTransport(CryptoContext * cctx, const char * filename, uint32_t verbose = 0)
    : buf(filename, cctx, cctx, verbose)
    {}

    bool disconnect() override {
        return !this->buf.close();
    }

    time_t begin_chunk_time() const noexcept
    { return this->buffer().get_begin_chunk_time(); }

    time_t end_chunk_time() const noexcept
    { return this->buffer().get_end_chunk_time(); }

    const char * path() const noexcept
    { return this->buffer().current_path(); }

    bool next() override {
        if (this->status == false) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }
        const ssize_t res = this->buffer().next();
        if (res){
            this->status = false;
            if (res < 0) {
                throw Error(ERR_TRANSPORT_READ_FAILED, -res);
            }
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
        }
        ++this->seqno;
        return true;
    }

    detail::in_meta_sequence_buf_crypto & buffer() noexcept
    { return this->buf; }

    const detail::in_meta_sequence_buf_crypto & buffer() const noexcept
    { return this->buf; }

    void do_recv(char ** pbuffer, size_t len) override {
        const ssize_t res = this->buf.read(*pbuffer, len);
        if (res < 0){
            this->status = false;
            throw Error(ERR_TRANSPORT_READ_FAILED, res);
        }
        *pbuffer += res;
        this->last_quantum_received += res;
        if (static_cast<size_t>(res) != len){
            this->status = false;
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
        }
    }
};

#endif
