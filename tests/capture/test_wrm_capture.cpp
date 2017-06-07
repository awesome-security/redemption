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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to capture interface to video recording to flv or mp4frecv
*/

#define RED_TEST_MODULE TestWrmCapture
#include "system/redemption_unit_tests.hpp"


// #define LOGNULL
#define LOGPRINT

#include "utils/log.hpp"

#include <snappy-c.h>
#include <memory>

#include "utils/png.hpp"
#include "utils/drawable.hpp"

#include "transport/transport.hpp"
#include "test_only/transport/test_transport.hpp"
#include "transport/out_file_transport.hpp"
#include "transport/in_file_transport.hpp"

#include "test_only/check_sig.hpp"
#include "test_only/get_file_contents.hpp"
#include "utils/bitmap_shrink.hpp"

#include "capture/wrm_capture.hpp"
#include "transport/in_meta_sequence_transport.hpp"

#include "test_only/lcg_random.hpp"
#include "test_only/fake_stat.hpp"

template<class Writer>
void wrmcapture_write_meta_headers(Writer & writer, uint16_t width, uint16_t height, bool has_checksum)
{
    char header1[3 + ((std::numeric_limits<unsigned>::digits10 + 1) * 2 + 2) + (10 + 1) + 2 + 1];
    const int len = sprintf(
        header1,
        "v2\n"
        "%u %u\n"
        "%s\n"
        "\n\n",
        unsigned(width),
        unsigned(height),
        has_checksum  ? "checksum" : "nochecksum"
    );
    RED_CHECK_EQ(writer.write(header1, len), len);
}


RED_AUTO_TEST_CASE(TestWrmCapture)
{
    const struct CheckFiles {
        const char * filename;
        int size;
    } fileinfos[] = {
        {"./capture-000000.wrm", 1646},
        {"./capture-000001.wrm", 3508},
        {"./capture-000002.wrm", 3463},
        {"./capture-000003.wrm", -1},
        {"./capture.mwrm", 165},
        {"/tmp/capture-000000.wrm", 40},
        {"/tmp/capture-000001.wrm", 40},
        {"/tmp/capture-000002.wrm", 40},
        {"/tmp/capture-000003.wrm", -1},
        {"/tmp/capture.mwrm", 34},
    };
    for (auto const & d : fileinfos) {
        unlink(d.filename);
    }

    {
        // Timestamps are applied only when flushing
        timeval now;
        now.tv_usec = 0;
        now.tv_sec = 1000;

        Rect scr(0, 0, 800, 600);

        LCGRandom rnd(0);
        FakeFstat fstat;
        CryptoContext cctx;

        GraphicToFile::Verbose wrm_verbose = to_verbose_flags(0)
    //     |GraphicToFile::Verbose::primary_orders)
    //     |GraphicToFile::Verbose::secondary_orders)
    //     |GraphicToFile::Verbose::bitmap_update)
        ;

        WrmCompressionAlgorithm wrm_compression_algorithm = WrmCompressionAlgorithm::no_compression;
        std::chrono::duration<unsigned int, std::ratio<1l, 100l> > wrm_frame_interval = std::chrono::seconds{1};
        std::chrono::seconds wrm_break_interval = std::chrono::seconds{3};
        TraceType wrm_trace_type = TraceType::localfile;

        const char * record_path = "./";
        const int groupid = 0; // www-data
        const char * hash_path = "/tmp/";

        char path[1024];
        char basename[1024];
        char extension[128];
        strcpy(path, WRM_PATH "/");     // default value, actual one should come from movie_path
        strcpy(basename, "capture");
        strcpy(extension, "");          // extension is currently ignored

        WrmParams wrm_params(
            24,
            wrm_trace_type,
            cctx,
            rnd,
            fstat,
            record_path,
            hash_path,
            basename,
            groupid,
            wrm_frame_interval,
            wrm_break_interval,
            wrm_compression_algorithm,
            int(wrm_verbose)
        );

        RDPDrawable gd_drawable(scr.cx, scr.cy);

        WrmCaptureImpl wrm(now, wrm_params, nullptr /* authentifier */, gd_drawable);

        auto const color_cxt = gdi::ColorCtx::depth24();
        bool ignore_frame_in_timeval = false;

        gd_drawable.draw(RDPOpaqueRect(scr, encode_color24()(GREEN)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(scr, encode_color24()(GREEN)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        gd_drawable.draw(RDPOpaqueRect(Rect(1, 50, 700, 30), encode_color24()(BLUE)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(1, 50, 700, 30), encode_color24()(BLUE)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        gd_drawable.draw(RDPOpaqueRect(Rect(2, 100, 700, 30), encode_color24()(WHITE)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(2, 100, 700, 30), encode_color24()(WHITE)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        // ------------------------------ BREAKPOINT ------------------------------

        gd_drawable.draw(RDPOpaqueRect(Rect(3, 150, 700, 30), encode_color24()(RED)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(3, 150, 700, 30), encode_color24()(RED)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        gd_drawable.draw(RDPOpaqueRect(Rect(4, 200, 700, 30), encode_color24()(BLACK)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(4, 200, 700, 30), encode_color24()(BLACK)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        gd_drawable.draw(RDPOpaqueRect(Rect(5, 250, 700, 30), encode_color24()(PINK)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(5, 250, 700, 30), encode_color24()(PINK)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        // ------------------------------ BREAKPOINT ------------------------------

        gd_drawable.draw(RDPOpaqueRect(Rect(6, 300, 700, 30), encode_color24()(WABGREEN)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(6, 300, 700, 30), encode_color24()(WABGREEN)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);
        // The destruction of capture object will finalize the metafile content
    }

    for (auto x : fileinfos) {
        auto fsize = filesize(x.filename);
        RED_CHECK_MESSAGE(
            x.size == fsize,
            "check " << x.size << " == filesize(\"" << x.filename
            << "\") failed [" << x.size << " != " << fsize << "]"
        );
        ::unlink(x.filename);
    }
}

RED_AUTO_TEST_CASE(TestWrmCaptureLocalHashed)
{
    const char * filesinfo[] = {
        "./capture-000000.wrm",
        "./capture-000001.wrm",
        "./capture-000002.wrm",
        "./capture-000003.wrm",
        "./capture.mwrm",
        "/tmp/capture.mwrm",
    };
    for (auto x: filesinfo) {
        ::unlink(x);
    }

    {
        // Timestamps are applied only when flushing
        timeval now;
        now.tv_usec = 0;
        now.tv_sec = 1000;

        Rect scr(0, 0, 800, 600);

        LCGRandom rnd(0);
        FakeFstat fstat;

        CryptoContext cctx;
        cctx.set_master_key(cstr_array_view(
            "\x61\x1f\xd4\xcd\xe5\x95\xb7\xfd"
            "\xa6\x50\x38\xfc\xd8\x86\x51\x4f"
            "\x59\x7e\x8e\x90\x81\xf6\xf4\x48"
            "\x9c\x77\x41\x51\x0f\x53\x0e\xe8"
        ));
        cctx.set_hmac_key(cstr_array_view(
             "\x86\x41\x05\x58\xc4\x95\xcc\x4e"
             "\x49\x21\x57\x87\x47\x74\x08\x8a"
             "\x33\xb0\x2a\xb8\x65\xcc\x38\x41"
             "\x20\xfe\xc2\xc9\xb8\x72\xc8\x2c"
        ));

        RED_CHECK(true);

        WrmParams wrm_params(
            24,
            TraceType::localfile_hashed,
            cctx,
            rnd,
            fstat,
            "./",
            "/tmp/",
            "capture",
            1000, // ini.get<cfg::video::capture_groupid>()
            std::chrono::seconds{1},
            std::chrono::seconds{3},
            WrmCompressionAlgorithm::no_compression,
            0 //0xFFFF VERBOSE
        );

        RED_CHECK(true);

        RDPDrawable gd_drawable(scr.cx, scr.cy);

        WrmCaptureImpl wrm(now, wrm_params, nullptr /* authentifier */, gd_drawable);

        RED_CHECK(true);

        auto const color_cxt = gdi::ColorCtx::depth24();
        bool ignore_frame_in_timeval = false;

        gd_drawable.draw(RDPOpaqueRect(scr, encode_color24()(GREEN)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(scr, encode_color24()(GREEN)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        RED_CHECK(true);

        gd_drawable.draw(RDPOpaqueRect(Rect(1, 50, 700, 30), encode_color24()(BLUE)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(1, 50, 700, 30), encode_color24()(BLUE)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        RED_CHECK(true);

        gd_drawable.draw(RDPOpaqueRect(Rect(2, 100, 700, 30), encode_color24()(WHITE)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(2, 100, 700, 30), encode_color24()(WHITE)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        RED_CHECK(true);

        // ------------------------------ BREAKPOINT ------------------------------

        gd_drawable.draw(RDPOpaqueRect(Rect(3, 150, 700, 30), encode_color24()(RED)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(3, 150, 700, 30), encode_color24()(RED)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        RED_CHECK(true);

        gd_drawable.draw(RDPOpaqueRect(Rect(4, 200, 700, 30), encode_color24()(BLACK)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(4, 200, 700, 30), encode_color24()(BLACK)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        RED_CHECK(true);

        gd_drawable.draw(RDPOpaqueRect(Rect(5, 250, 700, 30), encode_color24()(PINK)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(5, 250, 700, 30), encode_color24()(PINK)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        RED_CHECK(true);

        // ------------------------------ BREAKPOINT ------------------------------

        gd_drawable.draw(RDPOpaqueRect(Rect(6, 300, 700, 30), encode_color24()(WABGREEN)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(6, 300, 700, 30), encode_color24()(WABGREEN)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);
        // The destruction of capture object will finalize the metafile content
        RED_CHECK(true);

    }
    RED_CHECK(true);

    // TODO: we may have several mwrm sizes as it contains varying length numbers
    // the right solution would be to inject predictable fstat in test environment
    struct CheckFiles {
        const char * filename;
        size_t size;
        size_t altsize;
    } fileinfo[] = {
        {"./capture-000000.wrm", 1646, 0},
        {"./capture-000001.wrm", 3508, 0},
        {"./capture-000002.wrm", 3463, 0},
        {"./capture-000003.wrm", static_cast<size_t>(-1), static_cast<size_t>(-1)},
        {"./capture.mwrm", 676, 553},
    };
    for (auto x: fileinfo) {
        size_t fsize = filesize(x.filename);
        if (x.size != fsize){
            RED_CHECK_EQUAL(x.altsize, fsize);
        }
        ::unlink(x.filename);
    }
}

inline char * wrmcapture_swrite_hash(char * p, uint8_t const * hash)
{
    auto write = [&p](uint8_t const * hash) {
        *p++ = ' ';                // 1 octet
        for (unsigned c : iter(hash, MD_HASH::DIGEST_LENGTH)) {
            sprintf(p, "%02x", c); // 64 octets (hash)
            p += 2;
        }
    };
    write(hash);
    write(hash + MD_HASH::DIGEST_LENGTH);
    return p;
}


//RED_AUTO_TEST_CASE(TestOSumBuf)
//{
//    CryptoContext cctx;
//    cctx.set_master_key(cstr_array_view(
//        "\x00\x01\x02\x03\x04\x05\x06\x07"
//        "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
//        "\x10\x11\x12\x13\x14\x15\x16\x17"
//        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
//    ));
//    cctx.set_hmac_key(cstr_array_view("12345678901234567890123456789012"));
//    wrmcapture_ochecksum_buf_null_buf buf(cctx.get_hmac_key());
//    buf.open();
//    RED_CHECK_EQUAL(buf.write("ab", 2), 2);
//    RED_CHECK_EQUAL(buf.write("cde", 3), 3);

//    MD_HASH hash;
//    buf.close(hash);

//    char hash_str[(MD_HASH::DIGEST_LENGTH*2+1)*2 + 1];
//    *wrmcapture_swrite_hash(hash_str, hash) = 0;
//    RED_CHECK_EQUAL(
//        hash_str,
//        " 03cb482c5a6af0d37b74d0a8b1facf6a02b619068e92495f469e0098b662fe3f"
//        " 03cb482c5a6af0d37b74d0a8b1facf6a02b619068e92495f469e0098b662fe3f"
//    );
//}

template<class Writer>
int wrmcapture_write_filename(Writer & writer, const char * filename)
{
    OutBufferHashLineCtx buf;
    buf.write_filename(filename);

    ssize_t res = writer.write(buf.mes, buf.len);
    if (res < static_cast<ssize_t>(buf.len)) {
        return res < 0 ? res : 1;
    }

    return 0;
}


template<class Writer>
int wrmcapture_write_meta_file(
    Writer & writer, Fstat & fstat, const char * filename,
    time_t start_sec, time_t stop_sec
) {
    struct stat stat;
    int err = fstat.stat(filename, stat);
    if (err){
        return err;
    }

    OutBufferHashLineCtx buf;
    buf.write_filename(filename);
    buf.write_stat(stat);
    buf.write_start_and_stop(start_sec, stop_sec);
    buf.write_newline();

    ssize_t res = writer.write(buf.mes, buf.len);
    if (res < static_cast<ssize_t>(buf.len)) {
        return res < 0 ? res : 1;
    }

    return 0;
}

#include <string>

RED_AUTO_TEST_CASE(TestWriteFilename)
{
    struct {
        std::string s;

        int write(char const * data, std::size_t len) {
            s.append(data, len);
            return len;
        }
    } writer;

#define TEST_WRITE_FILENAME(origin_filename, wrote_filename) \
    wrmcapture_write_filename(writer, origin_filename);      \
    RED_CHECK_EQUAL(writer.s, wrote_filename);             \
    writer.s.clear()

    TEST_WRITE_FILENAME("abcde.txt", "abcde.txt");

    TEST_WRITE_FILENAME(R"(\abcde.txt)", R"(\\abcde.txt)");
    TEST_WRITE_FILENAME(R"(abc\de.txt)", R"(abc\\de.txt)");
    TEST_WRITE_FILENAME(R"(abcde.txt\)", R"(abcde.txt\\)");
    TEST_WRITE_FILENAME(R"(abc\\de.txt)", R"(abc\\\\de.txt)");
    TEST_WRITE_FILENAME(R"(\\\\)", R"(\\\\\\\\)");

    TEST_WRITE_FILENAME(R"( abcde.txt)", R"(\ abcde.txt)");
    TEST_WRITE_FILENAME(R"(abc de.txt)", R"(abc\ de.txt)");
    TEST_WRITE_FILENAME(R"(abcde.txt )", R"(abcde.txt\ )");
    TEST_WRITE_FILENAME(R"(abc  de.txt)", R"(abc\ \ de.txt)");
    TEST_WRITE_FILENAME(R"(    )", R"(\ \ \ \ )");
}

RED_AUTO_TEST_CASE(TestOutmetaTransport)
{
    ::unlink("./xxx-000000.wrm");
    ::unlink("./xxx-000001.wrm");
    ::unlink("./xxx.mwrm");
    ::unlink("./hash-xxx-000000.wrm");
    ::unlink("./hash-xxx-000001.wrm");
    ::unlink("./hash-xxx.mwrm");

    unsigned sec_start = 1352304810;
    {
        CryptoContext cctx;
        cctx.set_master_key(cstr_array_view(
            "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
            "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        ));
        cctx.set_hmac_key(cstr_array_view("12345678901234567890123456789012"));
        LCGRandom rnd(0);
        FakeFstat fstat;

        timeval now;
        now.tv_sec = sec_start;
        now.tv_usec = 0;
        const int groupid = 0;
        wrmcapture_OutMetaSequenceTransport wrm_trans(false, false, cctx, rnd, fstat, "./", "./hash-", "xxx", now, 800, 600, groupid, nullptr);
        wrm_trans.send("AAAAX", 5);
        wrm_trans.send("BBBBX", 5);
        wrm_trans.next();
        wrm_trans.send("CCCCX", 5);
    } // brackets necessary to force closing sequence

    struct {
        size_t len = 0;
        ssize_t write(char const *, size_t len) {
            this->len += len;
            return len;
        }
    } meta_len_writer;

    const char * meta_path = "./xxx.mwrm";
    const char * filename = meta_path + 2;
    const char * meta_hash_path = "./hash-xxx.mwrm";
    meta_len_writer.len = 5; // header
    struct stat stat;
    RED_CHECK(!FakeFstat{}.stat(meta_path, stat));

    wrmcapture_write_filename(meta_len_writer, filename);
    meta_len_writer.len += 17; // " 0 0 0 0 0 0 0 0\n"

    RED_CHECK_EQUAL(meta_len_writer.len, filesize(meta_hash_path));
    RED_CHECK_EQUAL(0, ::unlink(meta_hash_path));


    meta_len_writer.len = 0;

    wrmcapture_write_meta_headers(meta_len_writer, 800, 600, false);

    FakeFstat fstat;
    const char * file1 = "./xxx-000000.wrm";
    RED_CHECK(!wrmcapture_write_meta_file(meta_len_writer, fstat, file1, sec_start, sec_start+1));
    RED_CHECK_EQUAL(10, filesize(file1));
    RED_CHECK_EQUAL(0, ::unlink(file1));
    RED_CHECK_EQUAL(36, filesize("./hash-xxx-000000.wrm"));
    RED_CHECK_EQUAL(0, ::unlink("./hash-xxx-000000.wrm"));

    const char * file2 = "./xxx-000001.wrm";
    RED_CHECK(!wrmcapture_write_meta_file(meta_len_writer, fstat, file2, sec_start, sec_start+1));
    RED_CHECK_EQUAL(5, filesize(file2));
    RED_CHECK_EQUAL(0, ::unlink(file2));
    RED_CHECK_EQUAL(36, filesize("./hash-xxx-000001.wrm"));
    RED_CHECK_EQUAL(0, ::unlink("./hash-xxx-000001.wrm"));

    RED_CHECK_EQUAL(meta_len_writer.len, filesize(meta_path));
    RED_CHECK_EQUAL(0, ::unlink(meta_path));
}


RED_AUTO_TEST_CASE(TestOutmetaTransportWithSum)
{
    ::unlink("/tmp/xxx-000000.wrm");
    ::unlink("/tmp/xxx-000001.wrm");
    ::unlink("/tmp/xxx.mwrm");

    unsigned sec_start = 1352304810;
    {
        CryptoContext cctx;
        cctx.set_master_key(cstr_array_view(
            "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
            "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        ));
        cctx.set_hmac_key(cstr_array_view("12345678901234567890123456789012"));

        LCGRandom rnd(0);
        FakeFstat fstat;

        timeval now;
        now.tv_sec = sec_start;
        now.tv_usec = 0;
        const int groupid = 0;
        wrmcapture_OutMetaSequenceTransport wrm_trans(false, true, cctx, rnd, fstat, "./", "/tmp/", "xxx", now, 800, 600, groupid, nullptr);
        wrm_trans.send("AAAAX", 5);
        wrm_trans.send("BBBBX", 5);
        wrm_trans.next();
        wrm_trans.send("CCCCX", 5);
    } // brackets necessary to force closing sequence

    struct {
        size_t len = 0;
        ssize_t write(char const *, size_t len) {
            this->len += len;
            return len;
        }
    } meta_len_writer;
    wrmcapture_write_meta_headers(meta_len_writer, 800, 600, true);

    const unsigned hash_size = (1 + MD_HASH::DIGEST_LENGTH*2) * 2;

    FakeFstat fstat;

//    char file1[1024];
//    snprintf(file1, 1024, "./xxx-%06u-%06u.wrm", getpid(), 0);
    const char * file1 = "./xxx-000000.wrm";
    wrmcapture_write_meta_file(meta_len_writer, fstat, file1, sec_start, sec_start+1);
    meta_len_writer.len += hash_size;
    RED_CHECK_EQUAL(10, filesize(file1));
    RED_CHECK_EQUAL(0, ::unlink(file1));

//    char file2[1024];
//    snprintf(file2, 1024, "./xxx-%06u-%06u.wrm", getpid(), 1);
    const char * file2 = "./xxx-000001.wrm";
    wrmcapture_write_meta_file(meta_len_writer, fstat, file2, sec_start, sec_start+1);
    meta_len_writer.len += hash_size;
    RED_CHECK_EQUAL(5, filesize(file2));
    RED_CHECK_EQUAL(0, ::unlink(file2));

//    char meta_path[1024];
//    snprintf(meta_path, 1024, "./xxx-%06u.mwrm", getpid());
    const char * meta_path = "./xxx.mwrm";
    RED_CHECK_EQUAL(meta_len_writer.len, filesize(meta_path));
    RED_CHECK_EQUAL(0, ::unlink(meta_path));

    RED_CHECK_EQUAL(166, filesize("/tmp/xxx-000000.wrm"));
    RED_CHECK_EQUAL(0, ::unlink("/tmp/xxx-000000.wrm"));
    RED_CHECK_EQUAL(166, filesize("/tmp/xxx-000001.wrm"));
    RED_CHECK_EQUAL(0, ::unlink("/tmp/xxx-000001.wrm"));
    RED_CHECK_EQUAL(160, filesize("/tmp/xxx.mwrm"));
    RED_CHECK_EQUAL(0, ::unlink("/tmp/xxx.mwrm"));
}
