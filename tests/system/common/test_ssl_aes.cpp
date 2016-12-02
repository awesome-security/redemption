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
   Author(s): Christophe Grosjean, Meng Tan
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include "system/redemption_unit_tests.hpp"

#include "check_mem.hpp"


BOOST_AUTO_TEST_CASE(TestAES128_CBC)
{
    {
        // very secret key
        uint8_t key16[] = {0, 1, 2, 3, 4, 5, 6, 7,
                           8, 9, 10, 11, 12, 13, 14, 15
                          };
        // init vector not secret
        uint8_t iv[] = {
                        0x00, 0x01, 0x02, 0x03,
                        0x04, 0x05, 0x06, 0x07,
                        0x08, 0x09, 0x10, 0x11,
                        0x12, 0x13, 0x14, 0x15
                    };

        uint8_t inbuf[1024]= {3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,

                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,

                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,

                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6

                             }; // message to hide
        uint8_t outbuf[1024] = {};

        SslAes128_CBC aes(key16, iv, AES_direction::SSL_AES_ENCRYPT);
        aes.crypt_cbc(16, inbuf, outbuf);

//        hexdump_d(outbuf, 16);

//        hexdump_d(aes.tiv.iv, 16);

        aes.crypt_cbc(16, inbuf+16, outbuf+16);

//        hexdump_d(aes.tiv.iv, 16);

        uint8_t updated_iv[] = {
            0x32, 0xd4, 0xa1, 0x7b,
            0x6e, 0x80, 0x01, 0x19,
            0xc6, 0x05, 0x6c, 0xb1,
            0xf5, 0xf1, 0xd5, 0x1f
        };
        BOOST_CHECK_EQUAL(memcmp(aes.tiv.iv, updated_iv, 16), 0);

        aes.crypt_cbc(32, inbuf+32, outbuf+32);
        aes.crypt_cbc(32, inbuf+64, outbuf+64);
        aes.crypt_cbc(32, inbuf+96, outbuf+96);

        uint8_t expected[] = {
/* 0000 */ 0x5c, 0xe3, 0x8b, 0xbe, 0xf5, 0xfc, 0x78, 0x86,
           0x98, 0xe2, 0x34, 0xbc, 0x5b, 0xa2, 0xa5, 0x6b,
/* 0010 */ 0x32, 0xd4, 0xa1, 0x7b, 0x6e, 0x80, 0x01, 0x19,
           0xc6, 0x05, 0x6c, 0xb1, 0xf5, 0xf1, 0xd5, 0x1f,
/* 0020 */ 0x8a, 0x9a, 0xbc, 0xa4, 0x6a, 0x5f, 0x05, 0x07,
           0xd1, 0xb4, 0x24, 0x2c, 0x95, 0x7b, 0xdb, 0x01,
/* 0030 */ 0x4e, 0xbb, 0x14, 0x0a, 0xdd, 0x13, 0x09, 0xa3,
           0x39, 0x62, 0x15, 0x58, 0x93, 0xfd, 0xf9, 0x10,
/* 0040 */ 0x03, 0xcc, 0x2b, 0x7b, 0x36, 0x19, 0x40, 0x0b,
           0x31, 0x98, 0x3c, 0x44, 0x2e, 0x75, 0xbc, 0xda,
/* 0050 */ 0x82, 0xb0, 0xb1, 0x15, 0x14, 0x2d, 0xf5, 0x65,
           0xf6, 0x98, 0x79, 0xe9, 0x61, 0x5d, 0xba, 0xee,
/* 0060 */ 0x15, 0x43, 0x37, 0xb8, 0x34, 0x6b, 0x29, 0x41,
           0x28, 0x78, 0x9a, 0xfb, 0xd3, 0xaf, 0x90, 0x72,
/* 0070 */ 0x1a, 0xcd, 0xe3, 0x84, 0x0c, 0x2a, 0xe7, 0x6d,
           0x75, 0xa8, 0x02, 0x0c, 0x74, 0x55, 0x84, 0x50,
        };
        BOOST_CHECK_EQUAL(memcmp(outbuf, expected, 128), 0);

//        hexdump_d(outbuf, 128);

        uint8_t decrypted[1024] = {};

        SslAes128_CBC aes2(key16, iv, AES_direction::SSL_AES_DECRYPT);
        aes2.crypt_cbc(32, outbuf, decrypted);
        aes2.crypt_cbc(32, outbuf+32, decrypted+32);
        aes2.crypt_cbc(32, outbuf+64, decrypted+64);
        aes2.crypt_cbc(32, outbuf+96, decrypted+96);

        BOOST_CHECK_EQUAL(memcmp(inbuf, decrypted, 32), 0);

//        hexdump_d(decrypted, 32);
//        hexdump_d(inbuf, 32);

    }

}

BOOST_AUTO_TEST_CASE(TestAES192_CBC)
{
    {
        // very secret key
        uint8_t key24[] = {0, 1, 2, 3, 4, 5, 6, 7,
                           8, 9, 10, 11, 12, 13, 14, 15,
                           16, 17, 18, 19, 20, 21, 22, 23
                          };
        // init vector not secret
        uint8_t iv[] = {
                        0x00, 0x01, 0x02, 0x03,
                        0x04, 0x05, 0x06, 0x07,
                        0x08, 0x09, 0x10, 0x11,
                        0x12, 0x13, 0x14, 0x15
                    };

        uint8_t inbuf[1024]= {3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,

                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,

                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,

                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6

                             }; // message to hide
        uint8_t outbuf[1024] = {};

        SslAes192_CBC aes(key24, iv, AES_direction::SSL_AES_ENCRYPT);
        aes.crypt_cbc(32, inbuf, outbuf);

//        hexdump_d(aes.tiv.iv, 16);

        uint8_t updated_iv[] = {
            0x0c, 0x14, 0xe6, 0xfc,
            0x37, 0xdb, 0x94, 0xa6,
            0x2c, 0x6c, 0x55, 0x38,
            0xd5, 0xa1, 0x34, 0xaf
        };
        BOOST_CHECK_EQUAL(memcmp(aes.tiv.iv, updated_iv, 16), 0);

//        hexdump_d(aes.tiv.iv, 16);

        aes.crypt_cbc(32, inbuf+32, outbuf+32);
        aes.crypt_cbc(32, inbuf+64, outbuf+64);
        aes.crypt_cbc(32, inbuf+96, outbuf+96);

        uint8_t expected[] = {
/* 0000 */ 0x82, 0x50, 0x7e, 0xfd, 0x85, 0x90, 0xb3, 0x20, 0x88, 0x89, 0x16, 0xc5, 0xb8, 0xf5, 0x2e, 0xae,
/* 0010 */ 0x0c, 0x14, 0xe6, 0xfc, 0x37, 0xdb, 0x94, 0xa6, 0x2c, 0x6c, 0x55, 0x38, 0xd5, 0xa1, 0x34, 0xaf,
/* 0020 */ 0x04, 0x55, 0xd3, 0x1c, 0x43, 0x08, 0x72, 0x67, 0x46, 0xf2, 0x59, 0x67, 0x04, 0xf7, 0xdc, 0x8c,
/* 0030 */ 0xa0, 0x69, 0x69, 0xe9, 0x59, 0xb4, 0x2a, 0x04, 0x8a, 0x7b, 0x07, 0x59, 0xf8, 0x4f, 0xcd, 0xb8,
/* 0040 */ 0xd4, 0x0d, 0x28, 0x75, 0x22, 0x8f, 0x7f, 0xcb, 0xb0, 0x1d, 0x95, 0xa3, 0xe3, 0x7f, 0x2a, 0x14,
/* 0050 */ 0x54, 0xa7, 0xb7, 0xb6, 0x56, 0x83, 0xb9, 0x59, 0x32, 0x67, 0xaa, 0x92, 0x1c, 0xfe, 0xc8, 0xe2,
/* 0060 */ 0xb1, 0x91, 0xde, 0xa7, 0x3c, 0xca, 0xc8, 0x3f, 0x2e, 0xf9, 0x66, 0xde, 0x4a, 0x24, 0x40, 0xaa,
/* 0070 */ 0x87, 0x3b, 0xf8, 0x4f, 0x59, 0xbd, 0xf9, 0x4b, 0x60, 0xcb, 0x42, 0x15, 0x56, 0xb7, 0x9d, 0x2f,
        };
        BOOST_CHECK_EQUAL(memcmp(outbuf, expected, 96), 0);

//        hexdump_d(outbuf, 128);

        uint8_t decrypted[1024] = {};

        SslAes192_CBC aes2(key24, iv, AES_direction::SSL_AES_DECRYPT);
        aes2.crypt_cbc(32, outbuf, decrypted);
        aes2.crypt_cbc(32, outbuf+32, decrypted+32);
        aes2.crypt_cbc(32, outbuf+64, decrypted+64);
        aes2.crypt_cbc(32, outbuf+96, decrypted+96);


        BOOST_CHECK_EQUAL(memcmp(inbuf, decrypted, 128), 0);

//        hexdump_d(decrypted, 32);
//        hexdump_d(inbuf, 32);

    }
}

BOOST_AUTO_TEST_CASE(TestAES256_CBC)
{
    {
        // very secret key
        uint8_t key32[] = {0, 1, 2, 3, 4, 5, 6, 7,
                           8, 9, 10, 11, 12, 13, 14, 15,
                           16, 17, 18, 19, 20, 21, 22, 23,
                           24, 25, 26, 27, 28, 29, 30, 31,
                          };
        // init vector not secret
        uint8_t iv[] = {
                        0x00, 0x01, 0x02, 0x03,
                        0x04, 0x05, 0x06, 0x07,
                        0x08, 0x09, 0x10, 0x11,
                        0x12, 0x13, 0x14, 0x15
                    };

        uint8_t inbuf[1024]= {3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,

                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,

                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,

                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6,
                              3, 1, 4, 1, 5, 9, 2, 6

                             }; // message to hide
        uint8_t outbuf[1024] = {};

        SslAes256_CBC aes(key32, iv, AES_direction::SSL_AES_ENCRYPT);
        aes.crypt_cbc(32, inbuf, outbuf);

//        hexdump_d(aes.tiv.iv, 16);

        uint8_t updated_iv[] = {
            0x04, 0x33, 0x3b, 0xe8, 0x66, 0x7a, 0x64, 0x35, 0xb5, 0x0d, 0x94, 0xb0, 0x92, 0x33, 0xb8, 0x0b
        };
        CHECK_MEM(aes.tiv.iv, 16, updated_iv);

//        hexdump_d(aes.tiv.iv, 16);

        aes.crypt_cbc(32, inbuf+32, outbuf+32);
        aes.crypt_cbc(32, inbuf+64, outbuf+64);
        aes.crypt_cbc(32, inbuf+96, outbuf+96);

        uint8_t expected[] = {
/* 0000 */ 0x6c, 0x47, 0x19, 0xee, 0xb0, 0x70, 0x40, 0x3e, 0x2e, 0x2c, 0x2a, 0xbd, 0x5c, 0xa8, 0x4f, 0xfb,
/* 0010 */ 0x04, 0x33, 0x3b, 0xe8, 0x66, 0x7a, 0x64, 0x35, 0xb5, 0x0d, 0x94, 0xb0, 0x92, 0x33, 0xb8, 0x0b,
/* 0020 */ 0xac, 0x2c, 0xe9, 0x07, 0x93, 0xdf, 0x5e, 0x51, 0x72, 0xa9, 0xf0, 0xbc, 0x4d, 0x79, 0x10, 0x1f,
/* 0030 */ 0xe9, 0x72, 0xba, 0x53, 0xbf, 0xb3, 0x5c, 0x2b, 0x9c, 0x05, 0x5c, 0x14, 0x38, 0x77, 0x14, 0xfc,
/* 0040 */ 0xe0, 0xf2, 0xf3, 0x78, 0x32, 0xdf, 0xc1, 0x77, 0xaa, 0xf7, 0xfb, 0xa1, 0x5f, 0xc4, 0x2b, 0xdc,
/* 0050 */ 0xd8, 0x74, 0x5b, 0x34, 0x50, 0xb9, 0xea, 0xfc, 0x20, 0x1a, 0x18, 0xde, 0xaa, 0x73, 0xbd, 0x19,
/* 0060 */ 0x12, 0xdf, 0x8a, 0x47, 0x32, 0x9f, 0xc0, 0xcd, 0x11, 0xd6, 0x8b, 0x2d, 0x7e, 0xc9, 0xe2, 0x01,
/* 0070 */ 0xa5, 0x52, 0x2f, 0xb6, 0x53, 0x61, 0x9e, 0xdd, 0xba, 0x9c, 0x39, 0x48, 0xd7, 0xac, 0x89, 0xb6,
        };
        CHECK_MEM(outbuf, 32, expected);

//        hexdump_d(outbuf, 128);

        uint8_t decrypted[1024] = {};

        SslAes256_CBC aes2(key32, iv, AES_direction::SSL_AES_DECRYPT);
        aes2.crypt_cbc(32, outbuf, decrypted);
        aes2.crypt_cbc(32, outbuf+32, decrypted+32);
        aes2.crypt_cbc(32, outbuf+64, decrypted+64);
        aes2.crypt_cbc(32, outbuf+96, decrypted+96);


        BOOST_CHECK_EQUAL(memcmp(inbuf, decrypted, 64), 0);

//        hexdump_d(decrypted, 128);
//        hexdump_d(inbuf, 32);

    }
}
