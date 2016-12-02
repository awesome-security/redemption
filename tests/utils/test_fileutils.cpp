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
#define BOOST_TEST_MODULE TestFileUtils
#include "system/redemption_unit_tests.hpp"

// #define LOGPRINT
#define LOGNULL
#include "utils/log.hpp"

#include "utils/fileutils.hpp"

BOOST_AUTO_TEST_CASE(TestBasename)
{
// basename() change behavior depending if <filegen.h> is included
// or not. The POSIX version chnage it's argument, not the glibc one
// we WANT to use the glibc one. This test below will fail if
// <filegen.h> is included

//     Below expected behavior from the unix man pages
//          path        basename
//          "/usr/lib"  "lib"
//          "/usr/"     ""
//          "usr"       "usr"
//          "/"         "/"
//          "."         "."
//          ".."        ".."
    {
        char path[]= "/usr/lib";
        BOOST_CHECK(0 == strcmp(basename(path), "lib"));
    }
    {
        char path[]= "/usr/lib";
        size_t len = 0;
        char * base = basename_len(path, len);
        BOOST_CHECK_EQUAL(3, len);
        BOOST_CHECK(0 == memcmp(base, "lib", len));

    }

    {
        char path[]= "/usr/lib/";
        BOOST_CHECK(0 == strcmp(basename(path), ""));
    }
    {
        char path[]= "/usr/lib/";
        size_t len = 0;
        char * base = basename_len(path, len);
        BOOST_CHECK_EQUAL(0, len);
        BOOST_CHECK(0 == memcmp(base, "", len));

    }
    {
        char path[]= "/usr/";
        BOOST_CHECK(0 == strcmp(basename(path), ""));
    }
    {
        char path[]= "/usr";
        size_t len = 0;
        char * base = basename_len(path, len);
        BOOST_CHECK_EQUAL(3, len);
        BOOST_CHECK(0 == memcmp(base, "usr", len));

    }
    {
        char path[]= "usr";
        BOOST_CHECK(0 == strcmp(basename(path), "usr"));
    }
    {
        char path[]= "usr";
        size_t len = 0;
        char * base = basename_len(path, len);
        BOOST_CHECK_EQUAL(3, len);
        BOOST_CHECK(0 == memcmp(base, "usr", len));

    }
    {
        char path[]= "/";
        BOOST_CHECK(0 == strcmp(basename(path), ""));
    }
    {
        char path[]= "/";
        size_t len = 0;
        char * base = basename_len(path, len);
        BOOST_CHECK_EQUAL(0, len);
        BOOST_CHECK(0 == memcmp(base, "", len));

    }
    {
        char path[]= ".";
        BOOST_CHECK(0 == strcmp(basename(path), "."));
    }
    {
        char path[]= ".";
        size_t len = 0;
        char * base = basename_len(path, len);
        BOOST_CHECK_EQUAL(1, len);
        BOOST_CHECK(0 == memcmp(base, ".", len));

    }
    {
        char path[]= "..";
        BOOST_CHECK(0 == strcmp(basename(path), ".."));
    }
    {
        const char path[]= "..";
        size_t len = 0;
        const char * base = basename_len(path, len);
        BOOST_CHECK_EQUAL(2, len);
        BOOST_CHECK(0 == memcmp(base, "..", len));

    }

}


#include <unistd.h> // for getgid
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


BOOST_AUTO_TEST_CASE(TestClearTargetFiles)
{
    {
        char tmpdirname[128];
        sprintf(tmpdirname, "/tmp/test_dir_XXXXXX");
        BOOST_CHECK(nullptr != mkdtemp(tmpdirname));

//        int fd = ::mkostemp(tmpdirname, O_WRONLY|O_CREAT);

        char toto_mwrm[512]; sprintf(toto_mwrm, "%s/%s", tmpdirname, "toto.mwrm");
        { int fd = ::creat(toto_mwrm, 0777); BOOST_CHECK_EQUAL(10, write(fd, "toto_mwrm", sizeof("toto_mwrm"))); close(fd); }

        char toto_0_wrm[512]; sprintf(toto_0_wrm, "%s/%s", tmpdirname, "toto_0.mwrm");
        { int fd = ::creat(toto_0_wrm, 0777); BOOST_CHECK_EQUAL(11, write(fd, "toto_0_wrm", sizeof("toto_0_wrm"))); close(fd); }

        char toto_1_wrm[512]; sprintf(toto_1_wrm, "%s/%s", tmpdirname, "toto_1.wrm");
        { int fd = ::creat(toto_1_wrm, 0777); BOOST_CHECK_EQUAL(11, write(fd, "toto_1_wrm", sizeof("toto_1_wrm"))); close(fd); }

        char toto_0_flv[512]; sprintf(toto_0_flv, "%s/%s", tmpdirname, "toto_0.flv");
        { int fd = ::creat(toto_0_flv, 0777); BOOST_CHECK_EQUAL(11, write(fd, "toto_0_flv", sizeof("toto_0_flv"))); close(fd); }

        char toto_1_flv[512]; sprintf(toto_1_flv, "%s/%s", tmpdirname, "toto_1.flv");
        { int fd = ::creat(toto_1_flv, 0777); BOOST_CHECK_EQUAL(11, write(fd, "toto_1_flv", sizeof("toto_1_flv"))); close(fd); }

        char toto_meta[512]; sprintf(toto_meta, "%s/%s", tmpdirname, "toto.meta");
        { int fd = ::creat(toto_meta, 0777); BOOST_CHECK_EQUAL(10, write(fd, "toto_meta", sizeof("toto_meta"))); close(fd); }

        char toto_0_png[512]; sprintf(toto_0_png, "%s/%s", tmpdirname, "toto_0.png");
        { int fd = ::creat(toto_0_png, 0777); BOOST_CHECK_EQUAL(11, write(fd, "toto_0_png", sizeof("toto_0_png"))); close(fd); }

        char toto_1_png[512]; sprintf(toto_1_png, "%s/%s", tmpdirname, "toto_1.png");
        { int fd = ::creat(toto_1_png, 0777); BOOST_CHECK_EQUAL(11, write(fd, "toto_1_png", sizeof("toto_1_png"))); close(fd); }

        char tititi_mwrm[512]; sprintf(tititi_mwrm, "%s/%s", tmpdirname, "tititi.mwrm");
        { int fd = ::creat(tititi_mwrm, 0777); BOOST_CHECK_EQUAL(12, write(fd, "tititi_mwrm", sizeof("tititi_mwrm"))); close(fd); }

        char tititi_0_wrm[512]; sprintf(tititi_0_wrm, "%s/%s", tmpdirname, "tititi_0.mwrm");
        { int fd = ::creat(tititi_0_wrm, 0777); BOOST_CHECK_EQUAL(13, write(fd, "tititi_0_wrm", sizeof("tititi_0_wrm"))); close(fd); }

        char tititi_1_wrm[512]; sprintf(tititi_1_wrm, "%s/%s", tmpdirname, "tititi_1.wrm");
        { int fd = ::creat(tititi_1_wrm, 0777); BOOST_CHECK_EQUAL(13, write(fd, "tititi_1_wrm", sizeof("tititi_1_wrm"))); close(fd); }

        char tititi_0_flv[512]; sprintf(tititi_0_flv, "%s/%s", tmpdirname, "tititi_0.flv");
        { int fd = ::creat(tititi_0_flv, 0777); BOOST_CHECK_EQUAL(13, write(fd, "tititi_0_flv", sizeof("tititi_0_flv"))); close(fd); }

        char tititi_1_flv[512]; sprintf(tititi_1_flv, "%s/%s", tmpdirname, "tititi_1.flv");
        { int fd = ::creat(tititi_1_flv, 0777); BOOST_CHECK_EQUAL(13, write(fd, "tititi_1_flv", sizeof("tititi_1_flv"))); close(fd); }

        char tititi_meta[512]; sprintf(tititi_meta, "%s/%s", tmpdirname, "tititi.meta");
        { int fd = ::creat(tititi_meta, 0777); BOOST_CHECK_EQUAL(12, write(fd, "tititi_meta", sizeof("tititi_meta"))); close(fd); }

        char tititi_0_png[512]; sprintf(tititi_0_png, "%s/%s", tmpdirname, "tititi_0.png");
        { int fd = ::creat(tititi_0_png, 0777); BOOST_CHECK_EQUAL(13, write(fd, "tititi_0_png", sizeof("tititi_0_png"))); close(fd); }

        char tititi_1_png[512]; sprintf(tititi_1_png, "%s/%s", tmpdirname, "tititi_1.png");
        { int fd = ::creat(tititi_1_png, 0777); BOOST_CHECK_EQUAL(13, write(fd, "tititi_1_png", sizeof("tititi_1_png"))); close(fd); }

        BOOST_CHECK_EQUAL(10, filesize(toto_mwrm));
        BOOST_CHECK_EQUAL(11, filesize(toto_0_wrm));
        BOOST_CHECK_EQUAL(11, filesize(toto_1_wrm));
        BOOST_CHECK_EQUAL(11, filesize(toto_0_flv));
        BOOST_CHECK_EQUAL(11, filesize(toto_1_flv));
        BOOST_CHECK_EQUAL(10, filesize(toto_meta));
        BOOST_CHECK_EQUAL(11, filesize(toto_0_png));
        BOOST_CHECK_EQUAL(11, filesize(toto_1_png));
        BOOST_CHECK_EQUAL(12, filesize(tititi_mwrm));
        BOOST_CHECK_EQUAL(13, filesize(tititi_0_wrm));
        BOOST_CHECK_EQUAL(13, filesize(tititi_1_wrm));
        BOOST_CHECK_EQUAL(13, filesize(tititi_0_flv));
        BOOST_CHECK_EQUAL(13, filesize(tititi_1_flv));
        BOOST_CHECK_EQUAL(12, filesize(tititi_meta));
        BOOST_CHECK_EQUAL(13, filesize(tititi_0_png));
        BOOST_CHECK_EQUAL(13, filesize(tititi_1_png));

        clear_files_flv_meta_png(tmpdirname, "ddd");

        BOOST_CHECK_EQUAL(10, filesize(toto_mwrm));
        BOOST_CHECK_EQUAL(11, filesize(toto_0_wrm));
        BOOST_CHECK_EQUAL(11, filesize(toto_1_wrm));
        BOOST_CHECK_EQUAL(11, filesize(toto_0_flv));
        BOOST_CHECK_EQUAL(11, filesize(toto_1_flv));
        BOOST_CHECK_EQUAL(10, filesize(toto_meta));
        BOOST_CHECK_EQUAL(11, filesize(toto_0_png));
        BOOST_CHECK_EQUAL(11, filesize(toto_1_png));
        BOOST_CHECK_EQUAL(12, filesize(tititi_mwrm));
        BOOST_CHECK_EQUAL(13, filesize(tititi_0_wrm));
        BOOST_CHECK_EQUAL(13, filesize(tititi_1_wrm));
        BOOST_CHECK_EQUAL(13, filesize(tititi_0_flv));
        BOOST_CHECK_EQUAL(13, filesize(tititi_1_flv));
        BOOST_CHECK_EQUAL(12, filesize(tititi_meta));
        BOOST_CHECK_EQUAL(13, filesize(tititi_0_png));
        BOOST_CHECK_EQUAL(13, filesize(tititi_1_png));

        clear_files_flv_meta_png(tmpdirname, "toto");

        BOOST_CHECK_EQUAL(10, filesize(toto_mwrm));
        BOOST_CHECK_EQUAL(11, filesize(toto_0_wrm));
        BOOST_CHECK_EQUAL(11, filesize(toto_1_wrm));
        BOOST_CHECK_EQUAL(-1, filesize(toto_0_flv));
        BOOST_CHECK_EQUAL(-1, filesize(toto_1_flv));
        BOOST_CHECK_EQUAL(-1, filesize(toto_meta));
        BOOST_CHECK_EQUAL(-1, filesize(toto_0_png));
        BOOST_CHECK_EQUAL(-1, filesize(toto_1_png));
        BOOST_CHECK_EQUAL(12, filesize(tititi_mwrm));
        BOOST_CHECK_EQUAL(13, filesize(tititi_0_wrm));
        BOOST_CHECK_EQUAL(13, filesize(tititi_1_wrm));
        BOOST_CHECK_EQUAL(13, filesize(tititi_0_flv));
        BOOST_CHECK_EQUAL(13, filesize(tititi_1_flv));
        BOOST_CHECK_EQUAL(12, filesize(tititi_meta));
        BOOST_CHECK_EQUAL(13, filesize(tititi_0_png));
        BOOST_CHECK_EQUAL(13, filesize(tititi_1_png));

        clear_files_flv_meta_png(tmpdirname, "titititi");

        BOOST_CHECK_EQUAL(10, filesize(toto_mwrm));
        BOOST_CHECK_EQUAL(11, filesize(toto_0_wrm));
        BOOST_CHECK_EQUAL(11, filesize(toto_1_wrm));
        BOOST_CHECK_EQUAL(-1, filesize(toto_0_flv));
        BOOST_CHECK_EQUAL(-1, filesize(toto_1_flv));
        BOOST_CHECK_EQUAL(-1, filesize(toto_meta));
        BOOST_CHECK_EQUAL(-1, filesize(toto_0_png));
        BOOST_CHECK_EQUAL(-1, filesize(toto_1_png));
        BOOST_CHECK_EQUAL(12, filesize(tititi_mwrm));
        BOOST_CHECK_EQUAL(13, filesize(tititi_0_wrm));
        BOOST_CHECK_EQUAL(13, filesize(tititi_1_wrm));
        BOOST_CHECK_EQUAL(13, filesize(tititi_0_flv));
        BOOST_CHECK_EQUAL(13, filesize(tititi_1_flv));
        BOOST_CHECK_EQUAL(12, filesize(tititi_meta));
        BOOST_CHECK_EQUAL(13, filesize(tititi_0_png));
        BOOST_CHECK_EQUAL(13, filesize(tititi_1_png));

        clear_files_flv_meta_png(tmpdirname, "tititi");

        BOOST_CHECK_EQUAL(10, filesize(toto_mwrm));
        BOOST_CHECK_EQUAL(11, filesize(toto_0_wrm));
        BOOST_CHECK_EQUAL(11, filesize(toto_1_wrm));
        BOOST_CHECK_EQUAL(-1, filesize(toto_0_flv));
        BOOST_CHECK_EQUAL(-1, filesize(toto_1_flv));
        BOOST_CHECK_EQUAL(-1, filesize(toto_meta));
        BOOST_CHECK_EQUAL(-1, filesize(toto_0_png));
        BOOST_CHECK_EQUAL(-1, filesize(toto_1_png));
        BOOST_CHECK_EQUAL(12, filesize(tititi_mwrm));
        BOOST_CHECK_EQUAL(13, filesize(tititi_0_wrm));
        BOOST_CHECK_EQUAL(13, filesize(tititi_1_wrm));
        BOOST_CHECK_EQUAL(-1, filesize(tititi_0_flv));
        BOOST_CHECK_EQUAL(-1, filesize(tititi_1_flv));
        BOOST_CHECK_EQUAL(-1, filesize(tititi_meta));
        BOOST_CHECK_EQUAL(-1, filesize(tititi_0_png));
        BOOST_CHECK_EQUAL(-1, filesize(tititi_1_png));

        ::unlink(toto_mwrm);
        ::unlink(toto_0_wrm);
        ::unlink(toto_1_wrm);
        ::unlink(tititi_mwrm);
        ::unlink(tititi_0_wrm);
        ::unlink(tititi_1_wrm);

        ::rmdir(tmpdirname);
    }
}

BOOST_AUTO_TEST_CASE(CanonicalPath)
{
  // check that function that splits a path between canonical parts has expected behavior
  // Parts are:
  // - path : full path absolute or relative to directory containing file
  // - basename : the filename without extension
  // - extension : the extension = part following the last dot, removed from basename
  //  if initial fullpath does not has any dot in it nothing is removed

  char path[4096];
  char basename[4096];
  char extension[128];
  strcpy(path, "no path");
  strcpy(basename, "no basename");
  strcpy(extension, "no extension");

  canonical_path("./titi/result.tmp", path, 4096, basename, 4096, extension, 128);
  BOOST_CHECK_EQUAL("./titi/", path);
  BOOST_CHECK_EQUAL("result", basename);
  BOOST_CHECK_EQUAL(".tmp", extension);


  strcpy(path, "no path");
  strcpy(basename, "no basename");
  strcpy(extension, "no extension");
  canonical_path("result", path, 4096, basename, 4096, extension, 128);
  BOOST_CHECK_EQUAL("no path", path);
  BOOST_CHECK_EQUAL("result", basename);
  BOOST_CHECK_EQUAL("no extension", extension);

  strcpy(path, "no path");
  strcpy(basename, "no basename");
  strcpy(extension, "no extension");
  canonical_path("result/", path, 4096, basename, 4096, extension, 128);
  BOOST_CHECK_EQUAL("result/", path);
  BOOST_CHECK_EQUAL("no basename", basename);
  BOOST_CHECK_EQUAL("no extension", extension);

  strcpy(path, "no path");
  strcpy(basename, "no basename");
  strcpy(extension, "no extension");
  canonical_path("result.tmp", path, 4096, basename, 4096, extension, 128);
  BOOST_CHECK_EQUAL("no path", path);
  BOOST_CHECK_EQUAL("result", basename);
  BOOST_CHECK_EQUAL(".tmp", extension);

  strcpy(path, "no extension");
  strcpy(basename, "no basename");
  strcpy(extension, "no extension");
  canonical_path("tmp/.tmp", path, 4096, basename, 4096, extension, 128);
  BOOST_CHECK_EQUAL("tmp/", path);
  BOOST_CHECK_EQUAL("no basename", basename);
  BOOST_CHECK_EQUAL(".tmp", extension);

  strcpy(path, "no path");
  strcpy(basename, "no basename");
  strcpy(extension, "no extension");
  canonical_path(".tmp", path, 4096, basename, 4096, extension, 128);
  BOOST_CHECK_EQUAL("no path", path);
  BOOST_CHECK_EQUAL("no basename", basename);
  BOOST_CHECK_EQUAL(".tmp", extension);

  strcpy(path, "no path");
  strcpy(basename, "no basename");
  strcpy(extension, "no extension");
  canonical_path("", path, 4096, basename, 4096, extension, 128);
  BOOST_CHECK_EQUAL("no path", path);
  BOOST_CHECK_EQUAL("no basename", basename);
  BOOST_CHECK_EQUAL("no extension", extension);
}

BOOST_AUTO_TEST_CASE(TestParsePath)
{
    {
        std::string directory;
        std::string filename ;
        std::string extension;
        ParsePath("/etc/rdpproxy/rdpproxy.ini", directory, filename, extension);
        BOOST_CHECK_EQUAL("/etc/rdpproxy/", directory);
        BOOST_CHECK_EQUAL("rdpproxy"      , filename );
        BOOST_CHECK_EQUAL(".ini"          , extension);
    }

    {
        std::string directory;
        std::string filename ;
        std::string extension;
        ParsePath("/etc/rdpproxy/rdpproxy", directory, filename, extension);
        BOOST_CHECK_EQUAL("/etc/rdpproxy/", directory);
        BOOST_CHECK_EQUAL("rdpproxy"      , filename );
        BOOST_CHECK_EQUAL(""              , extension);
    }

    {
        std::string directory;
        std::string filename ;
        std::string extension;
        ParsePath("/etc/rdpproxy/", directory, filename, extension);
        BOOST_CHECK_EQUAL("/etc/rdpproxy/", directory);
        BOOST_CHECK_EQUAL(""              , filename );
        BOOST_CHECK_EQUAL(""              , extension);
    }

    {
        std::string directory;
        std::string filename ;
        std::string extension;
        ParsePath("rdpproxy.ini", directory, filename, extension);
        BOOST_CHECK_EQUAL(""        , directory);
        BOOST_CHECK_EQUAL("rdpproxy", filename );
        BOOST_CHECK_EQUAL(".ini"    , extension);
    }

    {
        std::string directory;
        std::string filename ;
        std::string extension;
        ParsePath("rdpproxy.", directory, filename, extension);
        BOOST_CHECK_EQUAL(""        , directory);
        BOOST_CHECK_EQUAL("rdpproxy", filename );
        BOOST_CHECK_EQUAL("."       , extension);
    }

    {
        std::string directory;
        std::string filename ;
        std::string extension;
        ParsePath("rdpproxy", directory, filename, extension);
        BOOST_CHECK_EQUAL(""        , directory);
        BOOST_CHECK_EQUAL("rdpproxy", filename );
        BOOST_CHECK_EQUAL(""        , extension);
    }

    {
        std::string directory;
        std::string filename ;
        std::string extension;
        ParsePath(".rdpproxy", directory, filename, extension);
        BOOST_CHECK_EQUAL(""         , directory);
        BOOST_CHECK_EQUAL(".rdpproxy", filename );
        BOOST_CHECK_EQUAL(""         , extension);
    }

    {
        std::string directory = "./"    ;
        std::string filename  = "sesman";
        std::string extension = ".conf" ;
        ParsePath("/etc/rdpproxy/rdpproxy.ini", directory, filename, extension);
        BOOST_CHECK_EQUAL("/etc/rdpproxy/", directory);
        BOOST_CHECK_EQUAL("rdpproxy"      , filename );
        BOOST_CHECK_EQUAL(".ini"          , extension);
    }

    {
        std::string directory = "./"    ;
        std::string filename  = "sesman";
        std::string extension = ".conf" ;
        ParsePath("/etc/rdpproxy/rdpproxy", directory, filename, extension);
        BOOST_CHECK_EQUAL("/etc/rdpproxy/", directory);
        BOOST_CHECK_EQUAL("rdpproxy"      , filename );
        BOOST_CHECK_EQUAL(".conf"         , extension);
    }

    {
        std::string directory           ;
        std::string filename            ;
        std::string extension = ".conf" ;
        ParsePath("/etc/rdpproxy/rdpproxy", directory, filename, extension);
        BOOST_CHECK_EQUAL("/etc/rdpproxy/", directory);
        BOOST_CHECK_EQUAL("rdpproxy"      , filename );
        BOOST_CHECK_EQUAL(".conf"         , extension);
    }

    {
        std::string directory = "./"    ;
        std::string filename  = "sesman";
        std::string extension = ".conf" ;
        ParsePath("rdpproxy.ini", directory, filename, extension);
        BOOST_CHECK_EQUAL("./"      , directory);
        BOOST_CHECK_EQUAL("rdpproxy", filename );
        BOOST_CHECK_EQUAL(".ini"    , extension);
    }

    {
        std::string directory = "./"    ;
        std::string filename  = "sesman";
        std::string extension = ".conf" ;
        ParsePath("rdpproxy", directory, filename, extension);
        BOOST_CHECK_EQUAL("./"      , directory);
        BOOST_CHECK_EQUAL("rdpproxy", filename );
        BOOST_CHECK_EQUAL(".conf"   , extension);
    }

    {
        std::string directory = "./"    ;
        std::string filename  = "sesman";
        std::string extension = ".conf" ;
        ParsePath(".rdpproxy.ini", directory, filename, extension);
        BOOST_CHECK_EQUAL("./"       , directory);
        BOOST_CHECK_EQUAL(".rdpproxy", filename );
        BOOST_CHECK_EQUAL(".ini"     , extension);
    }

    {
        std::string directory = "./"    ;
        std::string filename  = "sesman";
        std::string extension = ".conf" ;
        ParsePath("", directory, filename, extension);
        BOOST_CHECK_EQUAL("./"    , directory);
        BOOST_CHECK_EQUAL("sesman", filename );
        BOOST_CHECK_EQUAL(".conf" , extension);
    }
}

BOOST_AUTO_TEST_CASE(TestMakePath)
{
    {
        std::string fullpath;
        MakePath(fullpath, nullptr, nullptr, nullptr);
        BOOST_CHECK_EQUAL("", fullpath);
    }

    {
        std::string fullpath;
        MakePath(fullpath, "", "", "");
        BOOST_CHECK_EQUAL("", fullpath);
    }

    {
        std::string fullpath;
        MakePath(fullpath, "/etc/rdpproxy/", "rdpproxy", ".ini");
        BOOST_CHECK_EQUAL("/etc/rdpproxy/rdpproxy.ini", fullpath);
    }

    {
        std::string fullpath;
        MakePath(fullpath, "/etc/rdpproxy", "rdpproxy", "ini");
        BOOST_CHECK_EQUAL("/etc/rdpproxy/rdpproxy.ini", fullpath);
    }
}

BOOST_AUTO_TEST_CASE(TestRecursiveCreateDirectory)
{
    char tmpdirname[128];
    sprintf(tmpdirname, "/tmp/test_dir_XXXXXX");
    BOOST_CHECK(nullptr != mkdtemp(tmpdirname));
    BOOST_CHECK_EQUAL(true, file_exist(tmpdirname));

    recursive_delete_directory(tmpdirname);

    BOOST_CHECK_EQUAL(false, file_exist(tmpdirname));

    recursive_create_directory(tmpdirname, 0777, getgid());

    BOOST_CHECK_EQUAL(true, file_exist(tmpdirname));

    char tmpfilename[128];
    strcpy(tmpfilename, tmpdirname);
    strcat(tmpfilename, "/test_file_XXXXXX");
    close(mkstemp(tmpfilename));

    recursive_delete_directory(tmpdirname);
    BOOST_CHECK_EQUAL(false, file_exist(tmpdirname));

}

BOOST_AUTO_TEST_CASE(TestRecursiveCreateDirectoryTrailingSlash)
{
    char tmpdirname[128];
    sprintf(tmpdirname, "/tmp/test_dir_XXXXXX");
    BOOST_CHECK(nullptr != mkdtemp(tmpdirname));
    BOOST_CHECK_EQUAL(true, file_exist(tmpdirname));

    // Add a trailing slash to tmpdirname
    strcat(tmpdirname, "/");
    recursive_delete_directory(tmpdirname);

    BOOST_CHECK_EQUAL(false, file_exist(tmpdirname));

    recursive_create_directory(tmpdirname, 0777, getgid());

    BOOST_CHECK_EQUAL(true, file_exist(tmpdirname));

    char tmpfilename[128];
    strcpy(tmpfilename, tmpdirname);
    strcat(tmpfilename, "/test_file_XXXXXX");
    close(mkstemp(tmpfilename));

    recursive_delete_directory(tmpdirname);
    BOOST_CHECK_EQUAL(false, file_exist(tmpdirname));
}


