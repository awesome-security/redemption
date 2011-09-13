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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   common sec layer at core module

*/

#if !defined(__SEC_HPP__)
#define __SEC_HPP__

#include <assert.h>
#include <stdint.h>

#include <iostream>

#include "RDP/x224.hpp"
#include "RDP/rdp.hpp"
#include "client_info.hpp"
#include "rsa_keys.hpp"
#include "constants.hpp"


#warning ssl calls introduce some dependency on ssl system library, injecting it in the sec object would be better.
#include "ssl_calls.hpp"


inline static void sec_make_40bit(uint8_t* key)
{
    key[0] = 0xd1;
    key[1] = 0x26;
    key[2] = 0x9e;
}

// Output a uint32 into a buffer (little-endian)
inline static void buf_out_uint32(uint8_t* buffer, int value)
{
  buffer[0] = value & 0xff;
  buffer[1] = (value >> 8) & 0xff;
  buffer[2] = (value >> 16) & 0xff;
  buffer[3] = (value >> 24) & 0xff;
}


#warning method used by licence, common with basic crypto support code should be made common. pad are also common to several functions.
/* Generate a MAC hash (5.2.3.1), using a combination of SHA1 and MD5 */
inline static void sec_sign(uint8_t* signature, int siglen, uint8_t* session_key, int keylen, uint8_t* data, int datalen)
{
    static uint8_t pad_54[40] = { 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
                                 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
                                 54, 54, 54, 54, 54, 54, 54, 54
                               };
    static uint8_t pad_92[48] = { 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92,
                             92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92,
                             92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92
                           };

    uint8_t shasig[20];
    uint8_t md5sig[16];
    uint8_t lenhdr[4];
    SSL_SHA1 sha1;
    SSL_MD5 md5;

    buf_out_uint32(lenhdr, datalen);

    ssllib ssl;

    ssl.sha1_init(&sha1);
    ssl.sha1_update(&sha1, session_key, keylen);
    ssl.sha1_update(&sha1, pad_54, 40);
    ssl.sha1_update(&sha1, lenhdr, 4);
    ssl.sha1_update(&sha1, data, datalen);
    ssl.sha1_final(&sha1, shasig);

    ssl.md5_init(&md5);
    ssl.md5_update(&md5, session_key, keylen);
    ssl.md5_update(&md5, pad_92, 48);
    ssl.md5_update(&md5, shasig, 20);
    ssl.md5_final(&md5, md5sig);

    memcpy(signature, md5sig, siglen);
}




struct CryptContext
{
    int use_count;
    uint8_t sign_key[16]; // should I call it session_key ?
    uint8_t key[16];
    uint8_t update_key[16];
    int rc4_key_len;
    SSL_RC4 rc4_info;
    int rc4_key_size; /* 1 = 40-bit, 2 = 128-bit */

    CryptContext() : use_count(0)
    {
        memset(this->sign_key, 0, 16);
    }

    /* Encrypt data using RC4 */
    void encrypt(uint8_t* data, int length)
    {
        ssllib ssl;

        if (this->use_count == 4096){
            this->update();
            if (this->rc4_key_len == 8) {
                sec_make_40bit(this->key);
            }
            ssl.rc4_set_key(this->rc4_info, this->key, this->rc4_key_len);
            this->use_count = 0;
        }
        ssl.rc4_crypt(this->rc4_info, data, data, length);
        this->use_count++;
    }

    /* Decrypt data using RC4 */
    void decrypt(uint8_t* data, int len)
    {
        ssllib ssl;

        if (this->use_count == 4096) {
            this->update();
            if (this->rc4_key_len == 8) {
                sec_make_40bit(this->key);
            }
            ssl.rc4_set_key(this->rc4_info, this->key, this->rc4_key_len);
            this->use_count = 0;
        }
        ssl.rc4_crypt(this->rc4_info, data, data, len);
        this->use_count++;
    }

    /* Generate a MAC hash (5.2.3.1), using a combination of SHA1 and MD5 */
    void sign(uint8_t* signature, int siglen, uint8_t* data, int datalen)
    {
        static uint8_t pad_54[40] = { 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
                                     54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
                                     54, 54, 54, 54, 54, 54, 54, 54
                                   };
        static uint8_t pad_92[48] = { 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92,
                                 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92,
                                 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92
                               };

        uint8_t shasig[20];
        uint8_t md5sig[16];
        uint8_t lenhdr[4];
        SSL_SHA1 sha1;
        SSL_MD5 md5;

        buf_out_uint32(lenhdr, datalen);

        ssllib ssl;

        ssl.sha1_init(&sha1);
        ssl.sha1_update(&sha1, this->sign_key, this->rc4_key_len);
        ssl.sha1_update(&sha1, pad_54, 40);
        ssl.sha1_update(&sha1, lenhdr, 4);
        ssl.sha1_update(&sha1, data, datalen);
        ssl.sha1_final(&sha1, shasig);

        ssl.md5_init(&md5);
        ssl.md5_update(&md5, this->sign_key, this->rc4_key_len);
        ssl.md5_update(&md5, pad_92, 48);
        ssl.md5_update(&md5, shasig, 20);
        ssl.md5_final(&md5, md5sig);

        memcpy(signature, md5sig, siglen);
    }

    void update()
    {
        static uint8_t pad_54[40] = {
            54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
            54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
            54, 54, 54, 54, 54, 54, 54, 54
        };

        static uint8_t pad_92[48] = {
            92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92,
            92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92,
            92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92
        };

        uint8_t shasig[20];
        SSL_SHA1 sha1;
        SSL_MD5 md5;
        SSL_RC4 update;

        ssllib ssl;

        ssl.sha1_init(&sha1);
        ssl.sha1_update(&sha1, this->update_key, this->rc4_key_len);
        ssl.sha1_update(&sha1, pad_54, 40);
        ssl.sha1_update(&sha1, this->key, this->rc4_key_len);
        ssl.sha1_final(&sha1, shasig);

        ssl.md5_init(&md5);
        ssl.md5_update(&md5, this->update_key, this->rc4_key_len);
        ssl.md5_update(&md5, pad_92, 48);
        ssl.md5_update(&md5, shasig, 20);
        ssl.md5_final(&md5, key);

        ssl.rc4_set_key(update, this->key, this->rc4_key_len);
        ssl.rc4_crypt(update, this->key, this->key, this->rc4_key_len);
    }

};


// 2.2.1.1.1   RDP Negotiation Request (RDP_NEG_REQ)
// =================================================
//  The RDP Negotiation Request structure is used by a client to advertise the
//  security protocols which it supports.

// type (1 byte): An 8-bit, unsigned integer. Negotiation packet type. This
//   field MUST be set to 0x01 (TYPE_RDP_NEG_REQ) to indicate that the packet
//   is a Negotiation Request.

// flags (1 byte): An 8-bit, unsigned integer. Negotiation packet flags. There
//   are currently no defined flags so the field MUST be set to 0x00.

// length (2 bytes): A 16-bit, unsigned integer. Indicates the packet size.
//   This field MUST be set to 0x0008 (8 bytes).

// requestedProtocols (4 bytes): A 32-bit, unsigned integer. Flags indicating
//   the supported security protocols.

// +---------------------------------+-----------------------------------------+
// | 0x00000000 PROTOCOL_RDP_FLAG    |  Legacy RDP encryption.                 |
// +---------------------------------+-----------------------------------------+
// | 0x00000001 PROTOCOL_SSL_FLAG    | TLS 1.0 (section 5.4.5.1).              |
// +---------------------------------+-----------------------------------------+
// | 0x00000002 PROTOCOL_HYBRID_FLAG | Credential Security Support Provider    |
// |                                 | protocol (CredSSP) (section 5.4.5.2).   |
// |                                 | If this flag is set, then the           |
// |                                 | PROTOCOL_SSL_FLAG (0x00000001) SHOULD   |
// |                                 | also be set because Transport Layer     |
// |                                 | Security (TLS) is a subset of CredSSP.  |
// +---------------------------------+-----------------------------------------+

// 2.2.1.2.1   RDP Negotiation Response (RDP_NEG_RSP)
// ==================================================

//  The RDP Negotiation Response structure is used by a server to inform the
//  client of the security protocol which it has selected to use for the
//  connection.

// type (1 byte): An 8-bit, unsigned integer. Negotiation packet type. This field MUST be set to
//   0x02 (TYPE_RDP_NEG_RSP) to indicate that the packet is a Negotiation Response.

// flags (1 byte): An 8-bit, unsigned integer. Negotiation packet flags.

// +--------------------------------+------------------------------------------+
// | EXTENDED_CLIENT_DATA_SUPPORTED | The server supports extended client data |
// | 0x00000001                     | blocks in the GCC Conference Create      |
// |                                | Request user data (section 2.2.1.3).     |
// +--------------------------------+------------------------------------------+

// length (2 bytes): A 16-bit, unsigned integer. Indicates the packet size. This
//   field MUST be set to 0x0008 (8 bytes)

// selectedProtocol (4 bytes): A 32-bit, unsigned integer. Field indicating the
//   selected security protocol.

// +---------------------------------------------------------------------------+
// | 0x00000000 PROTOCOL_RDP    | Legacy RDP encryption                        |
// +---------------------------------------------------------------------------+
// | 0x00000001 PROTOCOL_SSL    | TLS 1.0 (section 5.4.5.1)                    |
// +---------------------------------------------------------------------------+
// | 0x00000002 PROTOCOL_HYBRID | CredSSP (section 5.4.5.2)                    |
// +---------------------------------------------------------------------------+

// 2.2.1.2.2   RDP Negotiation Failure (RDP_NEG_FAILURE)
// =====================================================

//  The RDP Negotiation Failure structure is used by a server to inform the
//  client of a failure that has occurred while preparing security for the
//  connection.

// type (1 byte): An 8-bit, unsigned integer. Negotiation packet type. This
//   field MUST be set to 0x03 (TYPE_RDP_NEG_FAILURE) to indicate that the
//   packet is a Negotiation Failure.

// flags (1 byte): An 8-bit, unsigned integer. Negotiation packet flags. There
//   are currently no defined flags so the field MUST be set to 0x00.

// length (2 bytes): A 16-bit, unsigned integer. Indicates the packet size. This
//   field MUST be set to 0x0008 (8 bytes).

// failureCode (4 bytes): A 32-bit, unsigned integer. Field containing the
//   failure code.

// +---------------------------+-----------------------------------------------+
// | SSL_REQUIRED_BY_SERVER    | The server requires that the client support   |
// | 0x00000001                | Enhanced RDP Security (section 5.4) with      |
// |                           | either TLS 1.0 (section 5.4.5.1) or CredSSP   |
// |                           | (section 5.4.5.2). If only CredSSP was        |
// |                           | requested then the server only supports TLS.  |
// +---------------------------+-----------------------------------------------+
// | SSL_NOT_ALLOWED_BY_SERVER | The server is configured to only use Standard |
// | 0x00000002                | RDP Security mechanisms (section 5.3) and     |
// |                           | does not support any External                 |
// |                           | Security Protocols (section 5.4.5).           |
// +---------------------------+-----------------------------------------------+
// | SSL_CERT_NOT_ON_SERVER    | The server does not possess a valid server    |
// | 0x00000003                | authentication certificate and cannot         |
// |                           | initialize the External Security Protocol     |
// |                           | Provider (section 5.4.5).                     |
// +---------------------------+-----------------------------------------------+
// | INCONSISTENT_FLAGS        | The list of requested security protocols is   |
// | 0x00000004                | not consistent with the current security      |
// |                           | protocol in effect. This error is only        |
// |                           | possible when the Direct Approach (see        |
// |                           | sections 5.4.2.2 and 1.3.1.2) is used and an  |
// |                           | External Security Protocol (section 5.4.5) is |
// |                           | already being used.                           |
// +---------------------------+-----------------------------------------------+
// | HYBRID_REQUIRED_BY_SERVER | The server requires that the client support   |
// | 0x00000005                | Enhanced RDP Security (section 5.4) with      |
// |                           | CredSSP (section 5.4.5.2).                    |
// +---------------------------+-----------------------------------------------+


class SecOut
{
    Stream & stream;
    uint16_t offhdr;
    uint8_t crypt_level;
    CryptContext & crypt;
    public:
    SecOut(Stream & stream, uint8_t crypt_level, uint32_t flags, CryptContext & crypt)
        : stream(stream), offhdr(stream.p - stream.data), crypt_level(crypt_level), crypt(crypt)
    {
        if (crypt_level > 1){
            this->stream.out_uint32_le(flags);
            this->stream.skip_uint8(8);
        }
        else {
            this->stream.out_uint32_le(0);
        }
    }

    void end(){
        if (crypt_level > 1){
            uint8_t * data = this->stream.data + this->offhdr + 12;
            int datalen = this->stream.p - data;
            this->crypt.sign(this->stream.data + this->offhdr + 4, 8, data, datalen);
            this->crypt.encrypt(data, datalen);
        }
    }
};


class SecIn
{
    public:
    uint32_t flags;
    SecIn(Stream & stream, CryptContext & crypt)
    {
        this->flags = stream.in_uint32_le();
        if ((this->flags & SEC_ENCRYPT)  || (this->flags & 0x0400)){
            #warning shouldn't we check signature ?
            stream.skip_uint8(8); /* signature */
            // decrypting to the end of tpdu
            crypt.decrypt(stream.p, stream.end - stream.p);
        }
    }

    void end(){
        #warning put some assertion here to ensure all data has been consumed
    }

};

static inline int recv_sec_tag_sig(Stream & stream, uint16_t len)
{
    stream.skip_uint8(len);
    /* Parse a public key structure */
    #warning is padding always 8 bytes long, may signature length change ? Check in documentation
    #warning we should check the signature is ok (using other provided parameters), this is not yet done today, signature is just dropped
}


static inline void send_sec_tag_sig(Stream & stream, const uint8_t (&pub_sig)[512])
{
    stream.out_uint16_le(SEC_TAG_KEYSIG);
    stream.out_uint16_le(72); /* len */
    stream.out_copy_bytes(pub_sig, 64); /* pub sig */
    stream.out_clear_bytes(8); /* pad */
}

static inline void send_sec_tag_pubkey(Stream & stream, const char (&pub_exp)[4], const uint8_t (&pub_mod)[512])
{
    stream.out_uint16_le(SEC_TAG_PUBKEY);
    stream.out_uint16_le(92); // length
    stream.out_uint32_le(SEC_RSA_MAGIC);
    stream.out_uint32_le(72); /* 72 bytes modulus len */
    stream.out_uint32_be(0x00020000);
    stream.out_uint32_be(0x3f000000);
    stream.out_copy_bytes(pub_exp, 4); /* pub exp */
    stream.out_copy_bytes(pub_mod, 64); /* pub mod */
    stream.out_clear_bytes(8); /* pad */
}


static inline void recv_sec_tag_pubkey(Stream & stream, uint32_t & server_public_key_len, uint8_t* modulus, uint8_t* exponent)
{
    /* Parse a public key structure */
    uint32_t magic = stream.in_uint32_le();
    if (magic != SEC_RSA_MAGIC) {
        LOG(LOG_WARNING, "RSA magic 0x%x\n", magic);
        throw Error(ERR_SEC_PARSE_PUB_KEY_MAGIC_NOT_OK);
    }
    server_public_key_len = stream.in_uint32_le() - SEC_PADDING_SIZE;

    if ((server_public_key_len < SEC_MODULUS_SIZE)
    ||  (server_public_key_len > SEC_MAX_MODULUS_SIZE)) {
        LOG(LOG_WARNING, "Bad server public key size (%u bits)\n", server_public_key_len * 8);
        throw Error(ERR_SEC_PARSE_PUB_KEY_MODUL_NOT_OK);
    }
    stream.skip_uint8(8); /* modulus_bits, unknown */
    memcpy(exponent, stream.in_uint8p(SEC_EXPONENT_SIZE), SEC_EXPONENT_SIZE);
    memcpy(modulus, stream.in_uint8p(server_public_key_len), server_public_key_len);
    stream.skip_uint8(SEC_PADDING_SIZE);

    if (!stream.check()){
        throw Error(ERR_SEC_PARSE_PUB_KEY_ERROR_CHECKING_STREAM);
    }
    LOG(LOG_DEBUG, "Got Public key, RDP4-style\n");
}


/* Parse a crypto information structure */
static inline void rdp_sec_parse_crypt_info(Stream & stream, uint32_t *rc4_key_size,
                              uint8_t * server_random,
                              uint8_t* modulus, uint8_t* exponent,
                              uint32_t & server_public_key_len,
                              uint8_t & crypt_level)
{
    uint32_t random_len;
    uint32_t rsa_info_len;
    uint32_t cacert_len;
    uint32_t cert_len;
    uint32_t flags;
    SSL_CERT *cacert;
    SSL_CERT *server_cert;
    SSL_RKEY *server_public_key;
    uint16_t tag;
    uint16_t length;
    uint8_t* next_tag;
    uint8_t* end;

    *rc4_key_size = stream.in_uint32_le(); /* 1 = 40-bit, 2 = 128-bit */
    crypt_level = stream.in_uint32_le(); /* 1 = low, 2 = medium, 3 = high */
    if (crypt_level == 0) { /* no encryption */
        LOG(LOG_INFO, "No encryption");
        throw Error(ERR_SEC_PARSE_CRYPT_INFO_ENCRYPTION_REQUIRED);
    }
    random_len = stream.in_uint32_le();
    rsa_info_len = stream.in_uint32_le();
    if (random_len != SEC_RANDOM_SIZE) {
        LOG(LOG_ERR,
            "parse_crypt_info_error: random len %d, expected %d\n",
            random_len, SEC_RANDOM_SIZE);
        throw Error(ERR_SEC_PARSE_CRYPT_INFO_BAD_RANDOM_LEN);
    }
    memcpy(server_random, stream.in_uint8p(random_len), random_len);

    /* RSA info */
    end = stream.p + rsa_info_len;
    if (end > stream.end) {
        throw Error(ERR_SEC_PARSE_CRYPT_INFO_BAD_RSA_LEN);
    }

    flags = stream.in_uint32_le(); /* 1 = RDP4-style, 0x80000002 = X.509 */
    LOG(LOG_INFO, "crypt flags %x\n", flags);
    if (flags & 1) {

        LOG(LOG_DEBUG, "We're going for the RDP4-style encryption\n");
        stream.skip_uint8(8); /* unknown */

        while (stream.p < end) {
            tag = stream.in_uint16_le();
            length = stream.in_uint16_le();
            #warning this should not be necessary any more as received tag are fully decoded (but we should check length does not lead accessing data out of buffer)
            next_tag = stream.p + length;

            switch (tag) {
            case SEC_TAG_PUBKEY:
                recv_sec_tag_pubkey(stream, server_public_key_len, modulus, exponent);
            break;
            case SEC_TAG_KEYSIG:
                LOG(LOG_DEBUG, "SEC_TAG_KEYSIG RDP4-style\n");
                recv_sec_tag_sig(stream, length);
                break;
            default:
                LOG(LOG_DEBUG, "unimplemented: crypt tag 0x%x\n", tag);
                throw Error(ERR_SEC_PARSE_CRYPT_INFO_UNIMPLEMENTED_TAG);
                break;
            }
            stream.p = next_tag;
        }
    }
    else {
        LOG(LOG_DEBUG, "We're going for the RDP5-style encryption\n");
        LOG(LOG_DEBUG, "RDP5-style encryption with certificates not available\n");
        uint32_t certcount = stream.in_uint32_le();
        if (certcount < 2){
            LOG(LOG_DEBUG, "Server didn't send enough X509 certificates\n");
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_CERT_NOK);
        }
        for (; certcount > 2; certcount--){
            /* ignore all the certificates between the root and the signing CA */
            LOG(LOG_WARNING, " Ignored certs left: %d\n", certcount);
            uint32_t ignorelen = stream.in_uint32_le();
            LOG(LOG_WARNING, "Ignored Certificate length is %d\n", ignorelen);
            SSL_CERT *ignorecert = ssl_cert_read(stream.p, ignorelen);
            stream.skip_uint8(ignorelen);
            if (ignorecert == NULL){
                LOG(LOG_WARNING,
                    "got a bad cert: this will probably screw up"
                    " the rest of the communication\n");
            }
        }

        /* Do da funky X.509 stuffy

       "How did I find out about this?  I looked up and saw a
       bright light and when I came to I had a scar on my forehead
       and knew about X.500"
       - Peter Gutman in a early version of
       http://www.cs.auckland.ac.nz/~pgut001/pubs/x509guide.txt
       */

        /* Loading CA_Certificate from server*/
        cacert_len = stream.in_uint32_le();
        LOG(LOG_DEBUG, "CA Certificate length is %d\n", cacert_len);
        cacert = ssl_cert_read(stream.p, cacert_len);
        stream.skip_uint8(cacert_len);
        if (NULL == cacert){
            LOG(LOG_DEBUG, "Couldn't load CA Certificate from server\n");
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_CACERT_NULL);
        }

        ssllib ssl;

        /* Loading Certificate from server*/
        cert_len = stream.in_uint32_le();
        LOG(LOG_DEBUG, "Certificate length is %d\n", cert_len);
        server_cert = ssl_cert_read(stream.p, cert_len);
        stream.skip_uint8(cert_len);
        if (NULL == server_cert){
            ssl_cert_free(cacert);
            LOG(LOG_DEBUG, "Couldn't load Certificate from server\n");
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_CACERT_NOT_LOADED);
        }
        /* Matching certificates */
        if (!ssl_certs_ok(server_cert, cacert)){
            ssl_cert_free(server_cert);
            ssl_cert_free(cacert);
            LOG(LOG_DEBUG, "Security error CA Certificate invalid\n");
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_CACERT_NOT_MATCH);
        }
        ssl_cert_free(cacert);
        stream.skip_uint8(16); /* Padding */
        server_public_key = ssl_cert_to_rkey(server_cert, server_public_key_len);
        if (NULL == server_public_key){
            LOG(LOG_DEBUG, "Didn't parse X509 correctly\n");
            ssl_cert_free(server_cert);
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_X509_NOT_PARSED);

        }
        ssl_cert_free(server_cert);
        LOG(LOG_INFO, "server_public_key_len=%d, MODULUS_SIZE=%d MAX_MODULUS_SIZE=%d\n", server_public_key_len, SEC_MODULUS_SIZE, SEC_MAX_MODULUS_SIZE);
        if ((server_public_key_len < SEC_MODULUS_SIZE) ||
            (server_public_key_len > SEC_MAX_MODULUS_SIZE)){
            LOG(LOG_DEBUG, "Bad server public key size (%u bits)\n",
                server_public_key_len * 8);
            ssl.rkey_free(server_public_key);
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_MOD_SIZE_NOT_OK);
        }
        if (ssl_rkey_get_exp_mod(server_public_key, exponent, SEC_EXPONENT_SIZE,
            modulus, SEC_MAX_MODULUS_SIZE) != 0){
            LOG(LOG_DEBUG, "Problem extracting RSA exponent, modulus");
            ssl.rkey_free(server_public_key);
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_RSA_EXP_NOT_OK);

        }
        ssl.rkey_free(server_public_key);
        #warning this return is ugly find a way to correctly dispose of garbage at end of buffer
        return; /* There's some garbage here we don't care about */
    }
    if (!stream.check_end()) {
        throw Error(ERR_SEC_PARSE_CRYPT_INFO_ERROR_CHECKING_STREAM);
    }
    return;
}

// 48-byte transformation used to generate master secret (6.1) and key material (6.2.2).
// Both SHA1 and MD5 algorithms are used.
static inline void sec_hash_48(uint8_t* out, const uint8_t* in, const uint8_t* salt1, const uint8_t* salt2, const uint8_t salt)
{
    uint8_t shasig[20];
    uint8_t pad[4];
    SSL_SHA1 sha1;
    SSL_MD5 md5;

    ssllib ssl;

    for (int i = 0; i < 3; i++) {
        memset(pad, salt + i, i + 1);

        ssl.sha1_init(&sha1);
        ssl.sha1_update(&sha1, pad, i + 1);
        ssl.sha1_update(&sha1, in, 48);
        ssl.sha1_update(&sha1, salt1, 32);
        ssl.sha1_update(&sha1, salt2, 32);
        ssl.sha1_final(&sha1, shasig);

        ssl.md5_init(&md5);
        ssl.md5_update(&md5, in, 48);
        ssl.md5_update(&md5, shasig, 20);
        ssl.md5_final(&md5, &out[i * 16]);
    }
}


// 16-byte transformation used to generate export keys (6.2.2).
static inline void sec_hash_16(uint8_t* out, const uint8_t* in, const uint8_t* salt1, const uint8_t* salt2)
{
    SSL_MD5 md5;

    ssllib ssl;

    ssl.md5_init(&md5);
    ssl.md5_update(&md5, in, 16);
    ssl.md5_update(&md5, salt1, 32);
    ssl.md5_update(&md5, salt2, 32);
    ssl.md5_final(&md5, out);
}


struct Sec
{

// only in server_sec : need cleanup

    uint8_t server_random[32];
    uint8_t client_random[64];

// only in rdp_sec : need cleanup
    uint32_t server_public_key_len;

// shared

    #warning windows 2008 does not write trailer because of overflow of buffer below, checked actual size: 64 bytes on xp, 256 bytes on windows 2008
    uint8_t client_crypt_random[512];

    CryptContext encrypt, decrypt;
    uint8_t crypt_level;
    #warning seems rc4_key_size is redundant with crypt level ?

    Sec(uint8_t crypt_level) :
      crypt_level(crypt_level)
    {
        // from server_sec
        // CGR: see if init has influence for the 3 following fields
        memset(this->server_random, 0, 32);
        memset(this->client_random, 0, 64);

        // from rdp_sec
        memset(this->client_crypt_random, 0, 512);
        this->server_public_key_len = 0;

        // shared
        memset(this->decrypt.key, 0, 16);
        memset(this->encrypt.key, 0, 16);
        memset(this->decrypt.update_key, 0, 16);
        memset(this->encrypt.update_key, 0, 16);
        switch (crypt_level) {
        case 1:
        case 2:
            this->decrypt.rc4_key_size = 1; /* 40 bits */
            this->encrypt.rc4_key_size = 1; /* 40 bits */
            this->decrypt.rc4_key_len = 8; /* 8 = 40 bit */
            this->encrypt.rc4_key_len = 8; /* 8 = 40 bit */
        break;
        default:
        case 3:
            this->decrypt.rc4_key_size = 2; /* 128 bits */
            this->encrypt.rc4_key_size = 2; /* 128 bits */
            this->decrypt.rc4_key_len = 16; /* 16 = 128 bit */
            this->encrypt.rc4_key_len = 16; /* 16 = 128 bit */
        break;
        }

    }

    ~Sec() {}



    /*****************************************************************************/
    void server_sec_disconnect(Transport * trans)
    {
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        stream.out_uint8((MCS_DPUM << 2) | 1);
        stream.out_uint8(0x80);

        tpdu.end();
        tpdu.send(trans);
    }



    void send_mcs_connect_response_pdu_with_gcc_conference_create_response(
                                            Transport * trans,
                                            ClientInfo * client_info,
                                            const ChannelList & channel_list,
                                            int rc4_key_size,
                                            uint8_t (&pub_mod)[512],
                                            uint8_t (&pri_exp)[512]
                                        ) throw(Error)
    {
        Rsakeys rsa_keys(CFG_PATH "/" RSAKEYS_INI);
        memset(this->server_random, 0x44, 32);
        int fd = open("/dev/urandom", O_RDONLY);
        if (fd == -1) {
            fd = open("/dev/random", O_RDONLY);
        }
        if (fd != -1) {
            if (read(fd, this->server_random, 32) != 32) {
            }
            close(fd);
        }

        uint8_t pub_sig[512];

        memcpy(pub_mod, rsa_keys.pub_mod, 64);
        memcpy(pub_sig, rsa_keys.pub_sig, 64);
        memcpy(pri_exp, rsa_keys.pri_exp, 64);

        Stream stream(8192);

        X224Out tpdu(X224Packet::DT_TPDU, stream);

        stream.out_uint16_be(BER_TAG_MCS_CONNECT_RESPONSE);
        uint32_t offset_len_mcs_connect_response = stream.p - stream.data;
        stream.out_ber_len(1024); // placeholder to force len >= 128 (3 bytes)

        stream.out_uint8(BER_TAG_RESULT);
        stream.out_uint8(1);
        stream.out_uint8(0);

        stream.out_uint8(BER_TAG_INTEGER);
        stream.out_uint8(1);
        stream.out_uint8(0);

        stream.out_uint8(BER_TAG_MCS_DOMAIN_PARAMS);
        stream.out_uint8(26);
        stream.out_ber_int8(22); // max_channels
        stream.out_ber_int8(3); // max_users
        stream.out_ber_int8(0); // max_tokens
        stream.out_ber_int8(1);
        stream.out_ber_int8(0);
        stream.out_ber_int8(1);
        stream.out_ber_int24(0xfff8); // max_pdu_size
        stream.out_ber_int8(2);

        stream.out_uint8(BER_TAG_OCTET_STRING);
        uint32_t offset_len_mcs_data = stream.p - stream.data;
        stream.out_ber_len(1024); // placeholder to force > 128 offset (3 bytes)

        /* mcs data */
        stream.out_uint16_be(5);
        stream.out_uint16_be(0x14);
        stream.out_uint8(0x7c);
        stream.out_uint16_be(1);
        stream.out_uint8(0x2a);
        stream.out_uint8(0x14);
        stream.out_uint8(0x76);
        stream.out_uint8(0x0a);
        stream.out_uint8(1);
        stream.out_uint8(1);
        stream.out_uint8(0);
        stream.out_uint16_le(0xc001);
        stream.out_uint8(0);
        stream.out_copy_bytes("McDn", 4);

        uint16_t num_channels = channel_list.size();
        uint16_t padchan = num_channels & 1;

        stream.out_uint16_be(0x8000 + 252 + (channel_list.size() + padchan) * 2); // len

        stream.out_uint16_le(SEC_TAG_SRV_INFO);
        // length, including tag and length fields
        stream.out_uint16_le(8); /* len */
        stream.out_uint8(4); /* 4 = rdp5 1 = rdp4 */
        stream.out_uint8(0);
        stream.out_uint8(8);
        stream.out_uint8(0);

        stream.out_uint16_le(SEC_TAG_SRV_CHANNELS);
        // length, including tag and length fields
        stream.out_uint16_le(8 + (num_channels + padchan) * 2);
        stream.out_uint16_le(MCS_GLOBAL_CHANNEL);
        stream.out_uint16_le(num_channels); /* number of other channels */

        for (int index = 0; index < num_channels; index++) {
                stream.out_uint16_le(MCS_GLOBAL_CHANNEL + (index + 1));
        }
        if (padchan){
            stream.out_uint16_le(0);
        }

        stream.out_uint16_le(SEC_TAG_SRV_CRYPT);
        stream.out_uint16_le(236); // length, including tag and length fields
        stream.out_uint32_le(rc4_key_size); // key len 1 = 40 bit 2 = 128 bit
        stream.out_uint32_le(client_info->crypt_level); // crypt level 1 = low 2 = medium
        /* 3 = high */
        stream.out_uint32_le(32);  // random len
        stream.out_uint32_le(184); // len of rsa info(certificate)
        stream.out_copy_bytes(this->server_random, 32);
        /* here to end is certificate */
        /* HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\ */
        /* TermService\Parameters\Certificate */
        stream.out_uint32_le(1);
        stream.out_uint32_le(1);
        stream.out_uint32_le(1);

        // 96 bytes long of sec_tag pubkey
        send_sec_tag_pubkey(stream, rsa_keys.pub_exp, pub_mod);
        // 76 bytes long of sec_tag_pub_sig
        send_sec_tag_sig(stream, pub_sig);
        /* end certificate */

        assert(offset_len_mcs_connect_response - offset_len_mcs_data == 38);

        #warning create a function in stream that sets differed ber_len_offsets
        // set mcs_data len, BER_TAG_OCTET_STRING (some kind of BLOB)
        stream.set_out_ber_len(stream.p - stream.data - offset_len_mcs_data - 3, offset_len_mcs_data);
        // set BER_TAG_MCS_CONNECT_RESPONSE len
        stream.set_out_ber_len(stream.p - stream.data - offset_len_mcs_connect_response - 3, offset_len_mcs_connect_response);

        tpdu.end();
        tpdu.send(trans);
    }

    /*****************************************************************************/
    void rdp_sec_generate_keys(CryptContext & encrypt, CryptContext & decrypt, uint8_t (& sign_key)[16], uint8_t *client_random, uint8_t *server_random, uint32_t rc4_key_size)
    {
        uint8_t pre_master_secret[48];
        uint8_t master_secret[48];
        uint8_t key_block[48];

        /* Construct pre-master secret (session key) */
        memcpy(pre_master_secret, client_random, 24);
        memcpy(pre_master_secret + 24, server_random, 24);

        /* Generate master secret and then key material */
        sec_hash_48(master_secret, pre_master_secret, client_random, server_random, 'A');
        sec_hash_48(key_block, master_secret, client_random, server_random, 'X');

        /* First 16 bytes of key material is MAC secret */
        memcpy(sign_key, key_block, 16);

        /* Generate export keys from next two blocks of 16 bytes */
        sec_hash_16(decrypt.key, &key_block[16], client_random, server_random);
        sec_hash_16(encrypt.key, &key_block[32], client_random, server_random);

        if (rc4_key_size == 1) {
            // LOG(LOG_DEBUG, "40-bit encryption enabled\n");
            sec_make_40bit(sign_key);
            sec_make_40bit(decrypt.key);
            sec_make_40bit(encrypt.key);
            decrypt.rc4_key_len = 8;
            encrypt.rc4_key_len = 8;
        }
        else {
            //LOG(LOG_DEBUG, "rc_4_key_size == %d, 128-bit encryption enabled\n", rc4_key_size);
            decrypt.rc4_key_len = 16;
            encrypt.rc4_key_len = 16;
        }

        /* Save initial RC4 keys as update keys */
        memcpy(decrypt.update_key, decrypt.key, 16);
        memcpy(encrypt.update_key, encrypt.key, 16);

        ssllib ssl;

        ssl.rc4_set_key(decrypt.rc4_info, decrypt.key, decrypt.rc4_key_len);
        ssl.rc4_set_key(encrypt.rc4_info, encrypt.key, encrypt.rc4_key_len);
    }


    /*****************************************************************************/
    /* Process crypto information blob */
    void rdp_sec_process_crypt_info(Stream & stream, uint32_t & server_public_key_len, uint8_t & crypt_level)
    {
        uint8_t server_random[SEC_RANDOM_SIZE];
        uint8_t client_random[SEC_RANDOM_SIZE];
        uint8_t modulus[SEC_MAX_MODULUS_SIZE];
        uint8_t exponent[SEC_EXPONENT_SIZE];
        uint32_t rc4_key_size;

        memset(modulus, 0, sizeof(modulus));
        memset(exponent, 0, sizeof(exponent));
        memset(client_random, 0, sizeof(SEC_RANDOM_SIZE));
        #warning check for the true size
        memset(server_random, 0, SEC_RANDOM_SIZE);
        rdp_sec_parse_crypt_info(stream, &rc4_key_size, server_random, modulus, exponent, server_public_key_len, crypt_level);

        /* Generate a client random, and determine encryption keys */
        memset(client_random, 0x44, SEC_RANDOM_SIZE);
        int fd = open("/dev/urandom", O_RDONLY);
        if (fd == -1) {
            fd = open("/dev/random", O_RDONLY);
        }
        if (fd != -1) {
            if (read(fd, client_random, SEC_RANDOM_SIZE) != SEC_RANDOM_SIZE) {
                LOG(LOG_WARNING, "random source failed to provide random data\n");
            }
            close(fd);
        }
        else {
            LOG(LOG_WARNING, "random source failed to provide random data : couldn't open device\n");
        }
        ssllib ssl;
        ssl.rsa_encrypt(this->client_crypt_random, client_random, SEC_RANDOM_SIZE, server_public_key_len, modulus, exponent);

        this->rdp_sec_generate_keys(this->encrypt, this->decrypt, this->encrypt.sign_key, client_random, server_random, rc4_key_size);
    }




//    /*****************************************************************************/
//    static void rdp_sec_rsa_op(uint8_t* out, uint8_t* in, uint8_t* mod, uint8_t* exp)
//    {
//        ssl_mod_exp(out, SEC_MODULUS_SIZE, /* 64 */
//                    in, SEC_RANDOM_SIZE, /* 32 */
//                    mod, SEC_MODULUS_SIZE, /* 64 */
//                    exp, SEC_EXPONENT_SIZE); /* 4 */
//    }


    /******************************************************************************/

    /* TODO: this function is not working well because it is stopping copy / paste
       what is required is to stop data from server to client. What we need to do is
       to recover clip_flags, send it to rdp_process_redirect_pdu. After that, we
       need to pass this flags to session_send_to_channel and before doing the
       stream.out_uint8a(data, data_len), we need to do stream.out_uint16_le(clip_flags)*/

    int clipboard_check(char* name, bool clipboard)
    {
      if (!clipboard)
      {
        if (strcmp("cliprdr", name) == 0)
        {
          return 1;
        }
      }
      return 0;
    }

    void send_security_exchange_PDU(Transport * trans, int userid)
    {
        LOG(LOG_INFO, "Iso Layer : setting encryption\n");
        /* Send the client random to the server */
        //      if (this->encryption)
        Stream sdrq_stream(8192);
        X224Out sdrq_tpdu(X224Packet::DT_TPDU, sdrq_stream);
        McsOut sdrq_out(sdrq_stream, MCS_SDRQ, userid, MCS_GLOBAL_CHANNEL);

        sdrq_stream.out_uint32_le(SEC_CLIENT_RANDOM);
        sdrq_stream.out_uint32_le(this->server_public_key_len + SEC_PADDING_SIZE);
        LOG(LOG_INFO, "Server public key is %d bytes long", this->server_public_key_len);
        sdrq_stream.out_copy_bytes(this->client_crypt_random, this->server_public_key_len);
        sdrq_stream.out_clear_bytes(SEC_PADDING_SIZE);

        sdrq_out.end();
        sdrq_tpdu.end();
        sdrq_tpdu.send(trans);
    }

};



#endif
