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


#pragma once

#include "core/channel_list.hpp"
#include "core/defines.hpp"
#include "core/FSCC/FileInformation.hpp"
#include "core/RDP/channels/rdpdr.hpp"
#include "core/SMB2/MessageSyntax.hpp"
#include "mod/rdp/channels/rdpdr_asynchronous_task.hpp"
#include "mod/rdp/channels/sespro_launcher.hpp"
#include "mod/rdp/rdp_log.hpp"
#include "transport/in_file_transport.hpp"
#include "utils/fileutils.hpp"
#include "utils/sugar/make_unique.hpp"
#include "utils/virtual_channel_data_sender.hpp"
#include "utils/winpr/pattern.hpp"

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <sys/statvfs.h>

#include <vector>
#include <string>
#include <algorithm>

#define EPOCH_DIFF 11644473600LL

#define FILE_TIME_SYSTEM_TO_RDP(_t) \
    ((static_cast<uint64_t>(_t) + EPOCH_DIFF) * 10000000LL)
#define FILE_TIME_RDP_TO_SYSTEM(_t) \
    (((_t) == 0LL || (_t) == static_cast<uint64_t>(-1LL)) \
    ? 0 : static_cast<time_t>((_t) / 10000000LL - EPOCH_DIFF))

class ManagedFileSystemObject {
protected:
    std::string full_path;

    int fd = -1;

    bool delete_pending = false;

public:
    virtual ~ManagedFileSystemObject() = default;

    static inline const char * get_open_flag_name(int flag) {
        switch (flag) {
            case O_RDONLY: return "O_RDONLY";
            case O_WRONLY: return "O_WRONLY";
            case O_RDWR:   return "O_RDWR";
        }

        return "<unknown>";
    }

    inline int FileDescriptor() const {
        REDASSERT(this->fd > -1);

        return this->fd;
    }

    virtual bool IsDirectory() const = 0;

    virtual bool IsSessionProbeImage() const { return false; }

    virtual void ProcessServerCreateDriveRequest(
        rdpdr::DeviceIORequest const & device_io_request,
        rdpdr::DeviceCreateRequest const & device_create_request,
        int drive_access_mode, const char * path, InStream & in_stream,
        bool & out_drive_created,
        VirtualChannelDataSender & to_server_sender,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
        bool is_session_probe_image,
        RDPVerbose verbose) = 0;

    virtual void ProcessServerCloseDriveRequest(
        rdpdr::DeviceIORequest const & device_io_request,
        const char * path, InStream & in_stream,
        VirtualChannelDataSender & to_server_sender,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
        RDPVerbose verbose) = 0;

    virtual void ProcessServerDriveReadRequest(
        rdpdr::DeviceIORequest const & device_io_request,
        rdpdr::DeviceReadRequest const & device_read_request,
        const char * path, InStream & in_stream,
        VirtualChannelDataSender & to_server_sender,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
        RDPVerbose verbose) = 0;

    virtual void ProcessServerDriveControlRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::DeviceControlRequest const & device_control_request,
            const char * path, InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose
      ) {
        (void)path;
        (void)device_control_request;
        (void)in_stream;
        REDASSERT(this->fd > -1);

        StaticOutStream<65536> out_stream;

        this->MakeClientDriveIoResponse(
            out_stream,
            device_io_request,
            "ManagedFileSystemObject::ProcessServerDriveControlRequest",
            0x00000000, // STATUS_SUCCESS
            verbose);

        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            LOG(LOG_INFO,
                "ManagedFileSystemObject::ProcessServerDriveControlRequest: OutputBufferLength=0");
        }
        out_stream.out_uint32_le(0);    // OutputBufferLength(4)

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(), to_server_sender,
            verbose);
    }

    virtual void ProcessServerDriveQueryVolumeInformationRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::ServerDriveQueryVolumeInformationRequest const &
                server_drive_query_volume_information_request,
            const char * path, InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose
      ) {
        (void)in_stream;
        REDASSERT(this->fd > -1);

        StaticOutStream<65536> out_stream;

        switch (server_drive_query_volume_information_request.FsInformationClass()) {
            case rdpdr::FileFsVolumeInformation:
            {
                struct statvfs svfsb;
                ::statvfs(path, &svfsb);
                struct stat64 sb;
                ::stat64(path, &sb);

                this->MakeClientDriveIoResponse(
                    out_stream,
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveQueryVolumeInformationRequest",
                    0x00000000, // STATUS_SUCCESS
                    verbose);

                const fscc::FileFsVolumeInformation file_fs_volume_information(
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_ctime),   // VolumeCreationTime(8)
                        svfsb.f_fsid,                           // VolumeSerialNumber(4)
                        1,                                      // SupportsObjects(1) - FALSE
                        "REDEMPTION"
                    );

                out_stream.out_uint32_le(file_fs_volume_information.size());    // Length(4)

                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "ManagedFileSystemObject::ProcessServerDriveQueryVolumeInformationRequest");
                    file_fs_volume_information.log(LOG_INFO);
                }
                file_fs_volume_information.emit(out_stream);
            }
            break;

            case rdpdr::FileFsSizeInformation:
            {
                struct statvfs svfsb;
                ::statvfs(path, &svfsb);

                this->MakeClientDriveIoResponse(
                    out_stream,
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveQueryVolumeInformationRequest",
                    0x00000000, // STATUS_SUCCESS
                    verbose);

                const fscc::FileFsSizeInformation file_fs_size_information(
                        svfsb.f_blocks, // TotalAllocationUnits(8)
                        svfsb.f_bavail, // AvailableAllocationUnits(8)
                        1,              // SectorsPerAllocationUnit(4)
                        svfsb.f_bsize   // BytesPerSector(4)
                    );

                out_stream.out_uint32_le(file_fs_size_information.size()); // Length(4)

                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "ManagedFileSystemObject::ProcessServerDriveQueryVolumeInformationRequest");
                    file_fs_size_information.log(LOG_INFO);
                }
                file_fs_size_information.emit(out_stream);
            }
            break;

            case rdpdr::FileFsAttributeInformation:
            {
                struct statvfs svfsb;
                ::statvfs(path, &svfsb);
                struct stat64 sb;
                ::stat64(path, &sb);

                this->MakeClientDriveIoResponse(
                    out_stream,
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveQueryVolumeInformationRequest",
                    0x00000000, // STATUS_SUCCESS
                    verbose);

                const fscc::FileFsAttributeInformation file_fs_attribute_information(
                        fscc::FILE_CASE_SENSITIVE_SEARCH |      // FileSystemAttributes(4)
                            fscc::FILE_CASE_PRESERVED_NAMES |
                            //fscc::FILE_READ_ONLY_VOLUME |
                            fscc::FILE_UNICODE_ON_DISK,
                        svfsb.f_namemax,                        // MaximumComponentNameLength(4)
                        "FAT32"                                 // FileSystemName(variable)
                    );

                out_stream.out_uint32_le(file_fs_attribute_information.size()); // Length(4)

                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "ManagedFileSystemObject::ProcessServerDriveQueryVolumeInformationRequest");
                    file_fs_attribute_information.log(LOG_INFO);
                }
                file_fs_attribute_information.emit(out_stream);
            }
            break;

            case rdpdr::FileFsFullSizeInformation:
            {
                struct statvfs svfsb;
                ::statvfs(path, &svfsb);

                this->MakeClientDriveIoResponse(
                    out_stream,
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveQueryVolumeInformationRequest",
                    0x00000000, // STATUS_SUCCESS
                    verbose);

                const fscc::FileFsFullSizeInformation file_fs_full_size_information(
                        svfsb.f_blocks, // TotalAllocationUnits(8)
                        svfsb.f_bavail, // CallerAvailableAllocationUnits(8)
                        svfsb.f_bfree,  // ActualAvailableAllocationUnits(8)
                        1,              // SectorsPerAllocationUnit(4)
                        svfsb.f_bsize   // BytesPerSector(4)
                    );

                out_stream.out_uint32_le(file_fs_full_size_information.size()); // Length(4)

                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "ManagedFileSystemObject::ProcessServerDriveQueryVolumeInformationRequest");
                    file_fs_full_size_information.log(LOG_INFO);
                }
                file_fs_full_size_information.emit(out_stream);
            }
            break;

            case rdpdr::FileFsDeviceInformation:
            {
                LOG(LOG_INFO,
                    "+ + + + + + + + + + ManagedFileSystemObject::ProcessServerDriveQueryVolumeInformationRequest() - FileFsDeviceInformation - Using VirtualChannelDataSender + + + + + + + + + +");

                this->MakeClientDriveIoResponse(
                    out_stream,
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveQueryVolumeInformationRequest",
                    0x00000000, // STATUS_SUCCESS
                    verbose);

                const fscc::FileFsDeviceInformation file_fs_device_information(
                        fscc::FILE_DEVICE_DISK, 0
                    );

                out_stream.out_uint32_le(file_fs_device_information.size());    // Length(4)

                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "ManagedFileSystemObject::ProcessServerDriveQueryVolumeInformationRequest");
                    file_fs_device_information.log(LOG_INFO);
                }
                file_fs_device_information.emit(out_stream);
            }
            break;

            default:
                LOG(LOG_ERR,
                    "ManagedFileSystemObject::ProcessServerDriveQueryVolumeInformationRequest: "
                        "Unknown FsInformationClass(0x%X)",
                    server_drive_query_volume_information_request.FsInformationClass());

                this->MakeClientDriveIoResponse(
                    out_stream,
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveQueryVolumeInformationRequest",
                    0xC0000001, // STATUS_UNSUCCESSFUL
                    verbose);
            break;
        }

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(), to_server_sender,
            verbose);
    }

    virtual void ProcessServerDriveQueryInformationRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::ServerDriveQueryInformationRequest const & server_drive_query_information_request,
            const char * path, InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose
      ) {
        (void)path;
        (void)in_stream;
        REDASSERT(this->fd > -1);

        StaticOutStream<65536> out_stream;

        struct stat64 sb;
        ::fstat64(this->fd, &sb);

        switch (server_drive_query_information_request.FsInformationClass()) {
            case rdpdr::FileBasicInformation:
            {
                this->MakeClientDriveIoResponse(
                    out_stream,
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveQueryInformationRequest",
                    0x00000000, // STATUS_SUCCESS
                    verbose);

                out_stream.out_uint32_le(fscc::FileBasicInformation::size());   // Length(4)

                fscc::FileBasicInformation file_basic_information(
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_mtime),                           // CreationTime(8)
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_atime),                           // LastAccessTime(8)
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_mtime),                           // LastWriteTime(8)
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_ctime),                           // ChangeTime(8)
                        (this->IsDirectory() ? fscc::FILE_ATTRIBUTE_DIRECTORY : 0) |    // FileAttributes(4)
                            ((sb.st_mode & S_IWUSR) ? 0 : fscc::FILE_ATTRIBUTE_READONLY)
                    );

                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "ManagedFileSystemObject::ProcessServerDriveQueryInformationRequest");
                    file_basic_information.log(LOG_INFO);
                }
                file_basic_information.emit(out_stream);
            }
            break;

            case rdpdr::FileStandardInformation:
            {
                this->MakeClientDriveIoResponse(
                    out_stream,
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveQueryInformationRequest",
                    0x00000000, // STATUS_SUCCESS
                    verbose);

                out_stream.out_uint32_le(fscc::FileStandardInformation::size());    // Length(4)

                const size_t block_size = 512;

                fscc::FileStandardInformation file_standard_information(
                        sb.st_blocks * block_size,  // AllocationSize
                        sb.st_size,                 // EndOfFile
                        sb.st_nlink,                // NumberOfLinks
                        0,                          // DeletePending
                        0                           // Directory
                    );

                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "ManagedFileSystemObject::ProcessServerDriveQueryInformationRequest");
                    file_standard_information.log(LOG_INFO);
                }
                file_standard_information.emit(out_stream);
            }
            break;

            case rdpdr::FileAttributeTagInformation:
            {
                this->MakeClientDriveIoResponse(
                    out_stream,
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveQueryInformationRequest",
                    0x00000000, // STATUS_SUCCESS
                    verbose);

                out_stream.out_uint32_le(fscc::FileAttributeTagInformation::size());    // Length(4)

                fscc::FileAttributeTagInformation file_attribute_tag_information(
                        fscc::FILE_ATTRIBUTE_DIRECTORY |                                    // FileAttributes
                            ((sb.st_mode & S_IWUSR) ? 0 : fscc::FILE_ATTRIBUTE_READONLY),
                        0                                                                   // ReparseTag
                    );

                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "ManagedFileSystemObject::ProcessServerDriveQueryInformationRequest");
                    file_attribute_tag_information.log(LOG_INFO);
                }
                file_attribute_tag_information.emit(out_stream);
            }
            break;

            default:
                LOG(LOG_ERR,
                    "ManagedFileSystemObject::ProcessServerDriveQueryInformationRequest: "
                        "Unknown FsInformationClass=%s(0x%X)",
                    server_drive_query_information_request.get_FsInformationClass_name(
                        server_drive_query_information_request.FsInformationClass()),
                    server_drive_query_information_request.FsInformationClass());
                throw Error(ERR_RDP_PROTOCOL);
            //break;
        }

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(), to_server_sender,
            verbose);
    }

    virtual void ProcessServerDriveSetInformationRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::ServerDriveSetInformationRequest const & server_drive_set_information_request,
            const char * path, int drive_access_mode, InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose) {
        REDASSERT(this->fd > -1);

        if ((drive_access_mode != O_RDWR) && (drive_access_mode != O_WRONLY)) {
            this->SendClientDriveIoResponse(
                device_io_request,
                "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest",
                0xC000000D, // STATUS_INVALID_PARAMETER
                to_server_sender,
                out_asynchronous_task,
                verbose);

            return;
        }

        switch (server_drive_set_information_request.FsInformationClass())
        {
            case rdpdr::FileBasicInformation:
            {
                fscc::FileBasicInformation file_basic_information;

                file_basic_information.receive(in_stream);

                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO, "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest");
                    file_basic_information.log(LOG_INFO);
                }

                struct stat64 sb;
                ::fstat64(this->fd, &sb);

                struct timeval times[2] = { { sb.st_atime, 0 }, { sb.st_mtime, 0 } };

                auto file_time_rdp_to_system_timeval = [](uint64_t rdp_time, timeval & out_system_tiem) {
                    if ((rdp_time != 0LL) && (rdp_time != static_cast<uint64_t>(-1LL))) {
                        out_system_tiem.tv_sec  = static_cast<time_t>(rdp_time / 10000000LL - EPOCH_DIFF);
                        out_system_tiem.tv_usec = rdp_time % 10000000LL;
                    }
                };

                file_time_rdp_to_system_timeval(file_basic_information.LastAccessTime(), times[0]);
                file_time_rdp_to_system_timeval(file_basic_information.LastWriteTime(),  times[1]);

                ::futimes(this->fd, times);

                const mode_t mode =
                    ((file_basic_information.FileAttributes() & fscc::FILE_ATTRIBUTE_READONLY) ?
                     (sb.st_mode & (~S_IWUSR)) :
                     (sb.st_mode | S_IWUSR)
                    );
                ::chmod(this->full_path.c_str(), mode);

                this->SendClientDriveSetInformationResponse(
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest",
                    0x00000000, // STATUS_SUCCESS
                    server_drive_set_information_request.Length(),
                    to_server_sender,
                    out_asynchronous_task,
                    verbose);
            }
            break;

            case rdpdr::FileEndOfFileInformation:
            {
                int64_t EndOfFile = in_stream.in_sint64_le();

                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest: "
                            "EndOfFile=%" PRId64,
                        EndOfFile);
                }

                int truncate_result = ::ftruncate(this->fd, EndOfFile);
                (void)truncate_result;

                this->SendClientDriveSetInformationResponse(
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest",
                    0x00000000, // STATUS_SUCCESS
                    server_drive_set_information_request.Length(),
                    to_server_sender,
                    out_asynchronous_task,
                    verbose);
            }
            break;

            case rdpdr::FileDispositionInformation:
                this->delete_pending = true;

                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest: "
                            "DeletePending=yes");
                }

                this->SendClientDriveSetInformationResponse(
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest",
                    0x00000000, // STATUS_SUCCESS
                    server_drive_set_information_request.Length(),
                    to_server_sender,
                    out_asynchronous_task,
                    verbose);
            break;

            case rdpdr::FileRenameInformation:
            {
                rdpdr::RDPFileRenameInformation rdp_file_rename_information;

                //auto in_stream_p = in_stream.get_current();

                rdp_file_rename_information.receive(in_stream);

                //LOG(LOG_INFO, "FileRenameInformation: size=%u",
                //    (unsigned int)(in_stream.get_current() - in_stream_p));
                //hexdump(in_stream_p, in_stream.get_current() - in_stream_p);

                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO, "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest");
                    rdp_file_rename_information.log(LOG_INFO);
                }

                REDASSERT(!rdp_file_rename_information.RootDirectory());

                std::string new_full_path(path);
                new_full_path += rdp_file_rename_information.FileName();

                if (!::access(new_full_path.c_str(), F_OK)) {
                    if (!rdp_file_rename_information.replace_if_exists()) {
                        this->SendClientDriveIoResponse(
                            device_io_request,
                            "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest",
                            (this->IsDirectory() ?
                             0xC0000033 :   // STATUS_OBJECT_NAME_INVALID
                             0xC0000035     // STATUS_OBJECT_NAME_COLLISION
                            ),
                            to_server_sender,
                            out_asynchronous_task,
                            verbose);

                        return;
                    }
                }

                (void)::rename(this->full_path.c_str(), new_full_path.c_str());

                this->SendClientDriveSetInformationResponse(
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest",
                    0x00000000, // STATUS_SUCCESS
                    server_drive_set_information_request.Length(),
                    to_server_sender,
                    out_asynchronous_task,
                    verbose);
            }
            break;

            case rdpdr::FileAllocationInformation:
            {
                int64_t AllocationSize = in_stream.in_sint64_le();

                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest: "
                            "AllocationSize=%" PRId64,
                        AllocationSize);
                }

                int truncate_result = ::ftruncate(this->fd, AllocationSize);
                (void)truncate_result;

                this->SendClientDriveSetInformationResponse(
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest",
                    0x00000000, // STATUS_SUCCESS
                    server_drive_set_information_request.Length(),
                    to_server_sender,
                    out_asynchronous_task,
                    verbose);
            }
            break;

            default:
                LOG(LOG_ERR,
                    "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest: "
                        "Unknown FsInformationClass - %s(0x%X)",
                    server_drive_set_information_request.get_FsInformationClass_name(
                        server_drive_set_information_request.FsInformationClass()),
                    server_drive_set_information_request.FsInformationClass());
                REDASSERT(false);

                SendClientDriveIoUnsuccessfulResponse(device_io_request,
                                                      "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest",
                                                      to_server_sender,
                                                      out_asynchronous_task,
                                                      verbose);

                // Unsupported.
                REDASSERT(false);
            break;
        }
    }

    virtual void ProcessServerDriveWriteRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            const char * path, int drive_access_mode, bool first_chunk,
            InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose
      ) {
        (void)path;
        (void)in_stream;
        (void)drive_access_mode;
        (void)first_chunk;

        SendClientDriveIoUnsuccessfulResponse(
            device_io_request,
            "ManagedFileSystemObject::ProcessServerDriveWriteRequest",
            to_server_sender,
            out_asynchronous_task,
            verbose);

        // Unsupported.
        REDASSERT(false);
    }

    virtual void ProcessServerDriveQueryDirectoryRequest(
        rdpdr::DeviceIORequest const & device_io_request,
        rdpdr::ServerDriveQueryDirectoryRequest const & server_drive_query_directory_request,
        const char * path, InStream & in_stream,
        VirtualChannelDataSender & to_server_sender,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
        RDPVerbose verbose) = 0;

protected:
    static void MakeClientDriveIoResponse(
            OutStream & out_stream,
            rdpdr::DeviceIORequest const & device_io_request,
            const char * message,
            uint32_t IoStatus,
            RDPVerbose verbose) {
        const rdpdr::SharedHeader shared_header(
                rdpdr::Component::RDPDR_CTYP_CORE,
                rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION
            );
        shared_header.emit(out_stream);

        const rdpdr::DeviceIOResponse device_io_response(
                device_io_request.DeviceId(),
                device_io_request.CompletionId(),
                IoStatus
            );
        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            LOG(LOG_INFO, "%s", message);
            device_io_response.log(LOG_INFO);
        }
        device_io_response.emit(out_stream);
    }

    static inline void SendClientDriveIoResponse(
            rdpdr::DeviceIORequest const & device_io_request,
            const char * message,
            uint32_t IoStatus,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose) {
        StaticOutStream<65536> out_stream;

        MakeClientDriveIoResponse(out_stream,
            device_io_request, message, IoStatus, verbose);

        uint32_t out_flags =
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(),
            to_server_sender, verbose);
    }

    static void SendClientDriveSetInformationResponse(
            rdpdr::DeviceIORequest const & device_io_request,
            const char * message,
            uint32_t IoStatus,
            uint32_t Length,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose) {
        StaticOutStream<65536> out_stream;

        MakeClientDriveIoResponse(out_stream,
            device_io_request, message, IoStatus, verbose);

        out_stream.out_uint32_le(Length);   // Length(4)

        // Padding(1), optional

        uint32_t out_flags =
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(),
            to_server_sender, verbose);
    }

public:
    static void SendClientDriveLockControlResponse(
            rdpdr::DeviceIORequest const & device_io_request,
            const char * message,
            uint32_t IoStatus,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose) {
        StaticOutStream<65536> out_stream;

        MakeClientDriveIoResponse(out_stream,
            device_io_request, message, IoStatus, verbose);

        out_stream.out_clear_bytes(5);  // Padding(5)

        uint32_t out_flags =
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(),
            to_server_sender, verbose);
    }

    static inline void SendClientDriveIoUnsuccessfulResponse(
            rdpdr::DeviceIORequest const & device_io_request,
            const char * message,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose) {
        SendClientDriveIoResponse(
            device_io_request,
            message,
            0xC0000001, // STATUS_UNSUCCESSFUL
            to_server_sender,
            out_asynchronous_task,
            verbose);
    }
};  // ManagedFileSystemObject

class ManagedDirectory : public ManagedFileSystemObject {
    DIR * dir = nullptr;

    std::string pattern;

public:
    //ManagedDirectory() {
    //    LOG(LOG_INFO, "ManagedDirectory::ManagedDirectory() : <%p>", this);
    //}

    ~ManagedDirectory() override {
        //LOG(LOG_INFO, "ManagedDirectory::~ManagedDirectory(): <%p> fd=%d",
        //    this, (this->dir ? ::dirfd(this->dir) : -1));

        if (this->dir) {
            ::closedir(this->dir);
        }

        if (this->delete_pending) {
            ::recursive_delete_directory(this->full_path.c_str());
        }
    }

    bool IsDirectory() const override { return true; }

    void ProcessServerCreateDriveRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::DeviceCreateRequest const & device_create_request,
            int drive_access_mode, const char * path, InStream & in_stream,
            bool & out_drive_created,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            bool is_session_probe_image,
            RDPVerbose verbose
      ) override {
        (void)in_stream;
        (void)is_session_probe_image;
        REDASSERT(!this->dir);

        out_drive_created = false;

        this->full_path =  path;
        this->full_path += device_create_request.Path();

        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            LOG(LOG_INFO,
                "ManagedDirectory::ProcessServerCreateDriveRequest: "
                    "<%p> full_path=\"%s\" drive_access_mode=%s(%d)",
                static_cast<void*>(this), this->full_path.c_str(), get_open_flag_name(drive_access_mode),
                drive_access_mode);
        }

        const uint32_t DesiredAccess     = device_create_request.DesiredAccess();
        const uint32_t CreateDisposition = device_create_request.CreateDisposition();

        const int last_error = [] (const char * path,
                                   uint32_t DesiredAccess,
                                   uint32_t CreateDisposition,
                                   int drive_access_mode,
                                   DIR *& out_dir) -> int {
            if (((drive_access_mode != O_RDWR) && (drive_access_mode != O_RDONLY) &&
                 smb2::read_access_is_required(DesiredAccess, /*strict_check = */false)) ||
                ((drive_access_mode != O_RDWR) && (drive_access_mode != O_WRONLY) &&
                 smb2::write_access_is_required(DesiredAccess, /*strict_check = */false))) {
                out_dir = nullptr;
                return EACCES;
            }

            if ((::access(path, F_OK) != 0) &&
                (CreateDisposition == smb2::FILE_CREATE)) {
                if ((drive_access_mode != O_RDWR) && (drive_access_mode != O_WRONLY)) {
                    out_dir = nullptr;
                    return EACCES;
                }

                ::mkdir(path, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP);
            }

            out_dir = ::opendir(path);
            return ((out_dir != nullptr) ? 0 : errno);
        } (full_path.c_str(), DesiredAccess, CreateDisposition, drive_access_mode, this->dir);

        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            LOG(LOG_INFO,
                "ManagedDirectory::ProcessServerCreateDriveRequest: <%p> dir=<%p> FileId=%d errno=%d",
                static_cast<void*>(this),
                static_cast<void*>(this->dir),
                (this->dir ? ::dirfd(this->dir) : -1),
                (this->dir ? 0 : last_error));
        }

        const uint32_t IoStatus = [] (const DIR * const dir, int last_error) -> uint32_t {
            if (dir) { return 0x00000000 /* STATUS_SUCCESS */; }

            switch (last_error) {
                case ENOENT:
                    return 0xC000000F;  // STATUS_NO_SUCH_FILE

                case EACCES:
                    return 0xC0000022;  // STATUS_ACCESS_DENIED
            }

            return 0xC0000001;  // STATUS_UNSUCCESSFUL
        } (this->dir, last_error);

        StaticOutStream<65536> out_stream;

        this->MakeClientDriveIoResponse(
            out_stream,
            device_io_request,
            "ManagedDirectory::ProcessServerCreateDriveRequest",
            IoStatus,
            verbose);

        const rdpdr::DeviceCreateResponse device_create_response(
                static_cast<uint32_t>(this->dir ? ::dirfd(this->dir) : -1),
                0x0
            );
        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            LOG(LOG_INFO, "ManagedDirectory::ProcessServerCreateDriveRequest");
            device_create_response.log(LOG_INFO);
        }
        device_create_response.emit(out_stream);

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(), to_server_sender,
            verbose);

        //if (this->dir) {
        //    LOG(LOG_INFO, "ManagedDirectory::ProcessServerCreateDriveRequest(): <%p> fd=%d",
        //        this, ::dirfd(this->dir));
        //}

        if (this->dir != nullptr) {
            this->fd = ::dirfd(this->dir);

            out_drive_created = true;
        }
    }

    void ProcessServerCloseDriveRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            const char * path, InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose
      ) override {
        (void)path;
        (void)in_stream;

        REDASSERT(this->dir);

        //LOG(LOG_INFO, "ManagedDirectory::ProcessServerCloseDriveRequest(): <%p> fd=%d",
        //    this, ::dirfd(this->dir));

        ::closedir(this->dir);

        this->dir = nullptr;
        this->fd  = -1;

        StaticOutStream<65536> out_stream;

        this->MakeClientDriveIoResponse(
            out_stream,
            device_io_request,
            "ManagedDirectory::ProcessServerCloseDriveRequest",
            0x00000000, /* STATUS_SUCCESS */
            verbose);

        // Device Close Response (DR_CLOSE_RSP)
        out_stream.out_clear_bytes(5);  // Padding(5);

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(), to_server_sender,
            verbose);

        REDASSERT(!this->dir);
    }

    void ProcessServerDriveReadRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::DeviceReadRequest const & device_read_request,
            const char * path, InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose
      ) override {
        (void)device_read_request;
        (void)path;
        (void)in_stream;
        REDASSERT(this->dir);

        StaticOutStream<65536> out_stream;

        this->MakeClientDriveIoResponse(
            out_stream,
            device_io_request,
            "ManagedDirectory::ProcessServerDriveReadRequest",
            0x00000000, // STATUS_SUCCESS
            verbose);

        out_stream.out_uint32_le(0);    // Length(4)

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(), to_server_sender,
            verbose);
    }

    void ProcessServerDriveQueryDirectoryRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::ServerDriveQueryDirectoryRequest const & server_drive_query_directory_request,
            const char * path, InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose
      ) override {
        (void)path;
        (void)in_stream;

        if (server_drive_query_directory_request.InitialQuery()) {
            ::rewinddir(this->dir);

            const char * separator = strrchr(server_drive_query_directory_request.Path(), '/');
            REDASSERT(separator);
            this->pattern = (++separator);
        }

        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            LOG(LOG_INFO,
                "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest: "
                    "full_path=\"%s\" pattern=\"%s\"",
                this->full_path.c_str(), this->pattern.c_str());
        }

        long     name_max = pathconf(this->full_path.c_str(), _PC_NAME_MAX);
        size_t   len      = offsetof(struct dirent, d_name) + name_max + 1;
        auto     uptr     = std::make_unique<char[]>(len);
        dirent * entry    = reinterpret_cast<dirent *>(uptr.get());
        dirent * result   = nullptr;

        do {
            if (::readdir_r(this->dir, entry, &result) || !result) { break; }

            if (::FilePatternMatchA(result->d_name, this->pattern.c_str())) {
                break;
            }
        }
        while (true);

        StaticOutStream<65536> out_stream;

        if (!result) {
            this->MakeClientDriveIoResponse(
                out_stream,
                device_io_request,
                "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest",
                0x80000006, // STATUS_NO_MORE_FILES
                verbose);

            out_stream.out_uint32_le(0);    // Length(4)
            out_stream.out_clear_bytes(1);  // Padding(1)
        }
        else {
            std::string file_full_path = this->full_path;
            if ((file_full_path.back() != '/') && (result->d_name[0] != '/')) {
                file_full_path += '/';
            }
            file_full_path += result->d_name;
            if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                LOG(LOG_INFO,
                    "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest: "
                        "<%p> full_path=\"%s\"",
                    static_cast<void*>(this), file_full_path.c_str());
            }

            struct stat64 sb;

            ::stat64(file_full_path.c_str(), &sb);

            switch (server_drive_query_directory_request.FsInformationClass()) {
                case rdpdr::FileFullDirectoryInformation:
                {
                    this->MakeClientDriveIoResponse(
                        out_stream,
                        device_io_request,
                        "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest",
                        0x00000000, // STATUS_SUCCESS
                        verbose);

                    const fscc::FileFullDirectoryInformation file_full_directory_information(
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_mtime),
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_atime),
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_mtime),
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_ctime),
                        sb.st_size, sb.st_blocks * 512 /* Block size */,
                        (S_ISDIR(sb.st_mode) ? fscc::FILE_ATTRIBUTE_DIRECTORY : 0) |
                            ((sb.st_mode & S_IWUSR) ? 0 : fscc::FILE_ATTRIBUTE_READONLY),
                        result->d_name
                        );
                    if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                        LOG(LOG_INFO,
                            "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest");
                        file_full_directory_information.log(LOG_INFO);
                    }

                    out_stream.out_uint32_le(file_full_directory_information.size());   // Length(4)

                    file_full_directory_information.emit(out_stream);
                }
                break;

                case rdpdr::FileBothDirectoryInformation:
                {
                    this->MakeClientDriveIoResponse(
                        out_stream,
                        device_io_request,
                        "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest",
                        0x00000000, // STATUS_SUCCESS
                        verbose);

                    const fscc::FileBothDirectoryInformation file_both_directory_information(
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_mtime),
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_atime),
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_mtime),
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_ctime),
                        sb.st_size, sb.st_blocks * 512 /* Block size */,
                        (S_ISDIR(sb.st_mode) ? fscc::FILE_ATTRIBUTE_DIRECTORY : 0) |
                            ((sb.st_mode & S_IWUSR) ? 0 : fscc::FILE_ATTRIBUTE_READONLY),
                        result->d_name
                        );
                    if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                        LOG(LOG_INFO,
                            "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest");
                        file_both_directory_information.log(LOG_INFO);
                    }

                    out_stream.out_uint32_le(file_both_directory_information.size());   // Length(4)

                    file_both_directory_information.emit(out_stream);
                }
                break;

                case rdpdr::FileNamesInformation:
                {
                    this->MakeClientDriveIoResponse(
                        out_stream,
                        device_io_request,
                        "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest",
                        0x00000000, // STATUS_SUCCESS
                        verbose);

                    const fscc::FileNamesInformation file_name_information(result->d_name);
                    if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                        LOG(LOG_INFO,
                            "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest");
                        file_name_information.log(LOG_INFO);
                    }

                    out_stream.out_uint32_le(file_name_information.size()); // Length(4)

//auto out_stream_p = out_stream.get_current();
                    file_name_information.emit(out_stream);
//LOG(LOG_INFO, "FileNamesInformation: size=%u",
//    static_cast<unsigned int>(out_stream.get_current() - out_stream_p));
//hexdump(out_stream_p, out_stream.get_current() - out_stream_p);
                }
                break;

                default:
                {
                    LOG(LOG_ERR,
                        "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest: "
                            "Unknown FsInformationClass(0x%X)",
                        server_drive_query_directory_request.FsInformationClass());
                    REDASSERT(false);

                    this->MakeClientDriveIoResponse(
                        out_stream,
                        device_io_request,
                        "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest",
                        0xC0000001, // STATUS_UNSUCCESSFUL
                        verbose);
                }
                break;
            }
        }

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(), to_server_sender,
            verbose);
    }
};  // ManagedDirectory

class ManagedFile : public ManagedFileSystemObject {
    std::unique_ptr<InFileSeekableTransport> in_file_transport; // For read operations only.

    bool is_session_probe_image = false;

public:
    //ManagedFile() {
    //    LOG(LOG_INFO, "ManagedFile::ManagedFile(): <%p>", this);
    //}

    ~ManagedFile() override {
        //LOG(LOG_INFO, "ManagedFile::~ManagedFile(): <%p> fd=%d",
        //    this, this->fd);

        // File descriptor will be closed when in_file_transport is destroyed.

        REDASSERT((this->fd <= -1) || in_file_transport);

        if (this->delete_pending) {
            ::unlink(this->full_path.c_str());
        }
    }

    bool IsDirectory() const override { return false; }

    bool IsSessionProbeImage() const override { return this->is_session_probe_image; }

    void ProcessServerCreateDriveRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::DeviceCreateRequest const & device_create_request,
            int drive_access_mode, const char * path, InStream & in_stream,
            bool & out_drive_created,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            bool is_session_probe_image,
            RDPVerbose verbose
      ) override {
        (void)in_stream;
        REDASSERT(this->fd == -1);

        out_drive_created = false;

        this->is_session_probe_image = is_session_probe_image;

        this->full_path = path;
        this->full_path += device_create_request.Path();

        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            LOG(LOG_INFO,
                "ManagedFile::ProcessServerCreateDriveRequest: "
                    "<%p> full_path=\"%s\" drive_access_mode=%s(%d)",
                static_cast<void*>(this), this->full_path.c_str(),
                get_open_flag_name(drive_access_mode), drive_access_mode);
        }

        const uint32_t DesiredAccess     = device_create_request.DesiredAccess();
        const uint32_t CreateDisposition = device_create_request.CreateDisposition();

        const int last_error = [] (const char * path,
                                   uint32_t DesiredAccess,
                                   uint32_t CreateDisposition,
                                   int drive_access_mode,
                                   void * log_this,
                                   RDPVerbose verbose,
                                   int & out_fd) -> int {
            out_fd = -1;

            if (((drive_access_mode != O_RDWR) && (drive_access_mode != O_RDONLY) &&
                 smb2::read_access_is_required(DesiredAccess, /*strict_check = */false)) ||
                ((drive_access_mode != O_RDWR) && (drive_access_mode != O_WRONLY) &&
                 smb2::write_access_is_required(DesiredAccess, /*strict_check = */false))) {
                return EACCES;
            }

            int open_flags = O_LARGEFILE;

            const bool strict_check = true;

            if (smb2::read_access_is_required(DesiredAccess, strict_check) &&
                smb2::write_access_is_required(DesiredAccess, strict_check)) {
                open_flags |= O_RDWR;
            }
            else if (smb2::write_access_is_required(DesiredAccess, strict_check)) {
                open_flags |= O_WRONLY;
            }
            else/* if (smb2::read_access_is_required(DesiredAccess, strict_check))*/ {
                open_flags |= O_RDONLY;
            }

            if ((DesiredAccess & smb2::FILE_APPEND_DATA) &&
                !(DesiredAccess & smb2::FILE_WRITE_DATA)) {
                open_flags |= O_APPEND;
            }

            if (CreateDisposition == smb2::FILE_SUPERSEDE) {
                open_flags |= (O_TRUNC | O_CREAT);
            }
            else if (CreateDisposition == smb2::FILE_CREATE) {
                open_flags |= (O_CREAT | O_EXCL);
            }
            else if (CreateDisposition == smb2::FILE_OPEN_IF) {
                open_flags |= O_CREAT;
            }
            else if (CreateDisposition == smb2::FILE_OVERWRITE) {
                open_flags |= O_TRUNC;
            }
            else if (CreateDisposition == smb2::FILE_OVERWRITE_IF) {
                open_flags |= (O_TRUNC | O_CREAT);
            }

            if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                LOG(LOG_INFO,
                    "ManagedFile::ProcessServerCreateDriveRequest: <%p> open_flags=0x%X",
                    log_this, open_flags);
            }

            out_fd = ::open(path, open_flags, S_IRUSR | S_IWUSR | S_IRGRP);
            return ((out_fd > -1) ? 0 : errno);
        } (this->full_path.c_str(), DesiredAccess, CreateDisposition, drive_access_mode,
           this, verbose, this->fd);

        if (this->fd > -1) {
            this->in_file_transport = std::make_unique<InFileSeekableTransport>(this->fd);
        }

        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            LOG(LOG_INFO,
                "ManagedFile::ProcessServerCreateDriveRequest: <%p> FileId=%d errno=%d",
                static_cast<void*>(this), this->fd, ((this->fd == -1) ? last_error : 0));
        }

        const uint32_t IoStatus = [] (int fd, int last_error) -> uint32_t {
            if (fd > -1) { return 0x00000000 /* STATUS_SUCCESS */; }

            switch (last_error) {
                case ENOENT:
                    return 0xC000000F;  // STATUS_NO_SUCH_FILE

                case EACCES:
                    return 0xC0000022;  // STATUS_ACCESS_DENIED
            }

            return 0xC0000001;  // STATUS_UNSUCCESSFUL
        } (this->fd, last_error);

        StaticOutStream<65536> out_stream;

        this->MakeClientDriveIoResponse(
            out_stream,
            device_io_request,
            "ManagedFile::ProcessServerCreateDriveRequest",
            IoStatus,
            verbose);

        const rdpdr::DeviceCreateResponse device_create_response(
                static_cast<uint32_t>(this->fd),
                0x0
            );
        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            LOG(LOG_INFO, "ManagedFile::ProcessServerCreateDriveRequest");
            device_create_response.log(LOG_INFO);
        }
        device_create_response.emit(out_stream);

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(), to_server_sender,
            verbose);

        //if (this->fd > -1) {
        //    LOG(LOG_INFO, "ManagedFile::ProcessServerCreateDriveRequest(): <%p> fd=%d",
        //        this, this->fd);
        //}

        out_drive_created = (this->fd != -1);
    }   // ProcessServerCreateDriveRequest

    void ProcessServerCloseDriveRequest(
            rdpdr::DeviceIORequest const & device_io_request, const char * path,
            InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose
      ) override {
        (void)path;
        (void)in_stream;
        REDASSERT(this->fd > -1);

        //LOG(LOG_INFO, "ManagedFile::ProcessServerCloseDriveRequest(): <%p> fd=%d",
        //    this, this->fd);

        ::close(this->fd);

        this->fd = -1;

        StaticOutStream<65536> out_stream;

        this->MakeClientDriveIoResponse(
            out_stream,
            device_io_request,
            "ManagedFile::ProcessServerCloseDriveRequest",
            0x00000000, // STATUS_SUCCESS
            verbose);

        // Device Close Response (DR_CLOSE_RSP)
        out_stream.out_clear_bytes(5);  // Padding(5);

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(), to_server_sender,
            verbose);

        REDASSERT(this->fd == -1);
    }

    void ProcessServerDriveReadRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::DeviceReadRequest const & device_read_request,
            const char * path, InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose
      ) override {
        (void)path;
        (void)in_stream;
        REDASSERT(this->fd > -1);

        const uint32_t Length = device_read_request.Length();

        const uint64_t Offset = device_read_request.Offset();

        struct stat64 sb;
        if (::fstat64(this->fd, &sb)) {
            StaticOutStream<512> out_stream;

            ManagedFileSystemObject::MakeClientDriveIoResponse(
                  out_stream
                , device_io_request
                , "ManagedFile::ProcessServerDriveReadRequest"
                , 0xC0000001    // STATUS_UNSUCCESSFUL
                , verbose);

            out_stream.out_uint32_le(0);    // Length(4)

            uint32_t out_flags =
                CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

            out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
                out_flags, out_stream.get_data(), out_stream.get_offset(),
                to_server_sender, verbose);

            return;
        }

        off64_t remaining_number_of_bytes_to_read = std::min<off64_t>(
            sb.st_size - Offset, Length);

        out_asynchronous_task = std::make_unique<RdpdrDriveReadTask>(
            this->in_file_transport.get(),
            this->fd, device_io_request.DeviceId(),
            device_io_request.CompletionId(),
            static_cast<uint32_t>(remaining_number_of_bytes_to_read),
            Offset, to_server_sender, verbose);
    }

    void ProcessServerDriveControlRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::DeviceControlRequest const & device_control_request,
            const char * path, InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose
      ) override {
        (void)device_control_request;
        (void)path;
        (void)in_stream;
        REDASSERT(this->fd > -1);

        StaticOutStream<65536> out_stream;

        this->MakeClientDriveIoResponse(
            out_stream,
            device_io_request,
            "ManagedFile::ProcessServerDriveControlRequest",
            0x00000000, // STATUS_SUCCESS
            verbose);

        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            LOG(LOG_INFO,
                "ManagedFile::ProcessServerDriveControlRequest: OutputBufferLength=0");
        }
        out_stream.out_uint32_le(0);    // OutputBufferLength(4)

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(), to_server_sender,
            verbose);
    }

private:
    uint32_t Length = 0;

    uint32_t remaining_number_of_bytes_to_write = 0;
    uint32_t current_offset                     = 0;

public:
    void ProcessServerDriveWriteRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            const char * path, int drive_access_mode,
            bool first_chunk, InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose
      ) override {
        (void)path;
        (void)drive_access_mode;
        REDASSERT(this->fd > -1);

        if (first_chunk) {
            Length = in_stream.in_uint32_le();
            uint64_t const
            Offset = in_stream.in_uint64_le();

            remaining_number_of_bytes_to_write = Length;
            current_offset                     = Offset;

            in_stream.in_skip_bytes(20);  // Padding(20)

            if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                LOG(LOG_INFO,
                    "ManagedFile::ProcessServerDriveWriteRequest(): "
                        "Length=%u Offset=%" PRIu64,
                    Length, Offset);
            }
        }

        REDASSERT(remaining_number_of_bytes_to_write >= in_stream.in_remain());

        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            LOG(LOG_INFO,
                "ManagedFile::ProcessServerDriveWriteRequest(): "
                    "CurrentOffset=%" PRIu32
                    " InRemain=%zu RemainingNumberOfBytesToWrite=%" PRIu32,
                current_offset, in_stream.in_remain(),
                remaining_number_of_bytes_to_write);
        }

        off64_t seek_result = ::lseek64(this->fd, current_offset, SEEK_SET);
        (void)seek_result;
        REDASSERT(seek_result == current_offset);
        int write_result = ::write(this->fd, in_stream.get_current(), in_stream.in_remain());
        (void)write_result;

        remaining_number_of_bytes_to_write -= in_stream.in_remain();
        current_offset                     += in_stream.in_remain();


        if (!remaining_number_of_bytes_to_write) {
            StaticOutStream<65536> out_stream;

            this->MakeClientDriveIoResponse(
                out_stream,
                device_io_request,
                "ManagedFile::ProcessServerDriveQueryInformationRequest",
                0x00000000, // STATUS_SUCCESS
                verbose);

            out_stream.out_uint32_le(Length);   // Length(4)
            out_stream.out_uint8(0);            // Padding(1), optional

            uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

            out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
                out_flags, out_stream.get_data(), out_stream.get_offset(), to_server_sender,
                verbose);
        }
    }

    void ProcessServerDriveQueryDirectoryRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::ServerDriveQueryDirectoryRequest const & server_drive_query_directory_request,
            const char * path, InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose
      ) override {
        (void)server_drive_query_directory_request;
        (void)path;
        (void)in_stream;

        StaticOutStream<65536> out_stream;

        this->MakeClientDriveIoResponse(
            out_stream,
            device_io_request,
            "ManagedFile::ProcessServerDriveQueryDirectoryRequest",
            0x80000006, // STATUS_NO_MORE_FILES
            verbose);

        out_stream.out_uint32_le(0);    // Length(4)
        out_stream.out_uint8(0);        // Padding(1)

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(), to_server_sender,
            verbose);
    }
};  // ManagedFile

class FileSystemDriveManager {
    const uint32_t FIRST_MANAGED_DRIVE_ID = 32767;

    const uint32_t INVALID_MANAGED_DRIVE_ID = 0xFFFFFFFF;

    uint32_t next_managed_drive_id = FIRST_MANAGED_DRIVE_ID;

    struct managed_drive_type
    {
        uint32_t device_id;
        std::string name;
        std::string path;
        int access_mode;
    };
    using managed_drive_collection_type = std::vector<managed_drive_type>;
    managed_drive_collection_type managed_drives;

    struct managed_file_system_object_type
    {
        uint32_t file_id;
        std::unique_ptr<ManagedFileSystemObject> object;
    };
    using managed_file_system_object_collection_type = std::vector<managed_file_system_object_type>;
    managed_file_system_object_collection_type managed_file_system_objects;

    uint32_t session_probe_drive_id = INVALID_MANAGED_DRIVE_ID;

    SessionProbeLauncher* session_probe_drive_access_notifier = nullptr;
    SessionProbeLauncher* session_probe_image_read_notifier   = nullptr;

public:
    void AnnounceDrive(bool device_capability_version_02_supported,
            VirtualChannelDataSender& to_server_sender, RDPVerbose verbose) {
        (void)device_capability_version_02_supported;
        uint8_t   virtual_channel_data[CHANNELS::CHANNEL_CHUNK_LENGTH];
        OutStream virtual_channel_stream(virtual_channel_data);

        for (managed_drive_type const & managed_drive : this->managed_drives) {

            rdpdr::SharedHeader client_message_header(
                rdpdr::Component::RDPDR_CTYP_CORE,
                rdpdr::PacketId::PAKID_CORE_DEVICELIST_ANNOUNCE);

            client_message_header.emit(virtual_channel_stream);

            virtual_channel_stream.out_uint32_le(
                 1  // DeviceCount(4)
                );

            rdpdr::DeviceAnnounceHeader device_announce_header(
                    rdpdr::RDPDR_DTYP_FILESYSTEM,   // DeviceType
                    managed_drive.device_id,
                    managed_drive.name.c_str(),     // PreferredDosName
                    reinterpret_cast<uint8_t const *>(managed_drive.name.c_str()),
                    managed_drive.name.length() + 1
                );

            if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                LOG(LOG_INFO, "FileSystemDriveManager::AnnounceDrive");
                device_announce_header.log(LOG_INFO);
            }

            device_announce_header.emit(virtual_channel_stream);

            to_server_sender(virtual_channel_stream.get_offset(),
                CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                virtual_channel_data,
                virtual_channel_stream.get_offset());

            virtual_channel_stream.rewind();
        }
    }

private:
    uint32_t EnableDrive(const char * drive_name, const char * relative_directory_path,
                         bool read_only, RDPVerbose verbose,
                         bool ignore_existence_check__for_test_only) {
        uint32_t drive_id = INVALID_MANAGED_DRIVE_ID;

        std::string absolute_directory_path = DRIVE_REDIRECTION_PATH "/";
        absolute_directory_path += relative_directory_path;

        struct stat sb;

        if (((::stat(absolute_directory_path.c_str(), &sb) == 0) &&
             S_ISDIR(sb.st_mode)) ||
            ignore_existence_check__for_test_only) {
            if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                LOG(LOG_INFO,
                    "FileSystemDriveManager::EnableDrive: "
                        "drive_name=\"%s\" directory_path=\"%s\"",
                    drive_name, absolute_directory_path.c_str());
            }

            drive_id = this->next_managed_drive_id++;

            this->managed_drives.push_back({
                drive_id,
                drive_name,
                absolute_directory_path.c_str(),
                (read_only ? O_RDONLY : O_RDWR)
            });
        }
        else {
            LOG(LOG_WARNING,
                "FileSystemDriveManager::EnableDrive: "
                    "Directory path \"%s\" is not accessible!",
                absolute_directory_path.c_str());
        }

        return drive_id;
    }

public:
    bool EnableDrive(const char * relative_directory_path, RDPVerbose verbose,
            bool ignore_existence_check__for_test_only = false) {
        bool read_only = false;
        if (*relative_directory_path == '*') {
            read_only = true;
            relative_directory_path++;
        }

        if (!::strcasecmp(relative_directory_path, "sespro") ||
            !::strcasecmp(relative_directory_path, "wablnch")) {
                LOG(LOG_WARNING,
                    "FileSystemDriveManager::EnableDrive: "
                        "Directory path \"%s\" is reserved!",
                    relative_directory_path);

            return false;
        }

        char drive_name[1024 * 16];
        int result = snprintf(drive_name, sizeof(drive_name), "%s",
            relative_directory_path);
        if ((result < 0) || (result >= static_cast<int>(sizeof(drive_name)))) {
            LOG(LOG_ERR,
                "FileSystemDriveManager::EnableDrive: "
                    "Failed to duplicate relative directory path. result=%d",
                result);

            return false;
        }

        const unsigned relative_directory_path_length = static_cast<unsigned>(result);
        for (unsigned i = 0; i < relative_directory_path_length; i++) {
            if ((drive_name[i] >= 0x61) && (drive_name[i] <= 0x7A)) {
                drive_name[i] -= 0x20;
            }
        }

        return (this->EnableDrive(
                    drive_name,
                    relative_directory_path,
                    read_only,
                    verbose,
                    ignore_existence_check__for_test_only
                ) != INVALID_MANAGED_DRIVE_ID);
    }

    bool EnableSessionProbeDrive(RDPVerbose verbose) {
        if (this->session_probe_drive_id == INVALID_MANAGED_DRIVE_ID) {
            this->session_probe_drive_id = this->EnableDrive(
                    "SESPRO",
                    "sespro",
                    true,       // read-only
                    verbose,
                    false       // ignore existence check
                );
        }

        return (this->session_probe_drive_id != INVALID_MANAGED_DRIVE_ID);
    }

public:
    bool HasManagedDrive() const {
        return (this->managed_drives.size() > 0);
    }

private:
    managed_drive_collection_type::const_iterator
    find_drive_by_id(uint32_t DeviceId) const {
        return std::find_if(
            this->managed_drives.cbegin(),
            this->managed_drives.cend(),
            [DeviceId](managed_drive_type const & managed_drive) {
                return DeviceId == managed_drive.device_id;
            }
        );
    }

public:
    bool IsManagedDrive(uint32_t DeviceId) const {
        return DeviceId >= FIRST_MANAGED_DRIVE_ID
            && this->find_drive_by_id(DeviceId) != this->managed_drives.cend();
    }

private:
    void ProcessServerCreateDriveRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            std::string const & path, int drive_access_mode, InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose) {
        rdpdr::DeviceCreateRequest device_create_request;

        device_create_request.receive(in_stream);
        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            device_create_request.log(LOG_INFO);
        }

        std::string full_path    = path;
        std::string request_path = device_create_request.Path();
        if ((full_path.back() != '/') && (request_path.front() != '/')) {
            full_path += '/';
        }
        full_path += request_path;
        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            LOG(LOG_INFO,
                "FileSystemDriveManager::ProcessServerCreateDriveRequest: "
                    "full_path=\"%s\" drive_access_mode=%s(%d)",
                full_path.c_str(),
                ManagedFileSystemObject::get_open_flag_name(drive_access_mode),
                drive_access_mode);
        }

        bool is_directory = false;

        struct stat sb;
        if (::stat(full_path.c_str(), &sb) == 0) {
            is_directory = ((sb.st_mode & S_IFMT) == S_IFDIR);
        }
        else {
            is_directory =
                (device_create_request.CreateOptions() &
                 smb2::FILE_DIRECTORY_FILE);
        }

        bool is_session_probe_image = false;

        std::unique_ptr<ManagedFileSystemObject> managed_file_system_object;
        if (is_directory) {
            managed_file_system_object = std::make_unique<ManagedDirectory>();
        }
        else {
            is_session_probe_image =
                ((device_io_request.DeviceId() == this->session_probe_drive_id) &&
                 !::strcmp(device_create_request.Path(), "/BIN"));

            managed_file_system_object = std::make_unique<ManagedFile>();
        }
        bool drive_created = false;
        managed_file_system_object->ProcessServerCreateDriveRequest(
                device_io_request, device_create_request, drive_access_mode,
                path.c_str(), in_stream, drive_created, to_server_sender,
                out_asynchronous_task, is_session_probe_image, verbose);
        if (drive_created) {
            this->managed_file_system_objects.push_back({
                static_cast<uint32_t>(managed_file_system_object->FileDescriptor()),
                std::move(managed_file_system_object)
            });
        }
    }

public:
    void ProcessDeviceIORequest(
            rdpdr::DeviceIORequest const & device_io_request,
            bool first_chunk,
            InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose) {
        uint32_t DeviceId = device_io_request.DeviceId();
        if (DeviceId < FIRST_MANAGED_DRIVE_ID) {
            return;
        }
        auto drive_iter = this->find_drive_by_id(DeviceId);
        if (drive_iter == this->managed_drives.end()) {
            LOG(LOG_WARNING,
                "FileSystemDriveManager::ProcessDeviceIORequest: "
                    "Unknown device. DeviceId=%u",
                DeviceId);
            return;
        }

        std::string const & path              = drive_iter->path;
        int const           drive_access_mode = drive_iter->access_mode;

        managed_file_system_object_collection_type::const_iterator file_iter;
        if (device_io_request.MajorFunction() != rdpdr::IRP_MJ_CREATE) {
            file_iter = std::find_if(
                this->managed_file_system_objects.begin(),
                this->managed_file_system_objects.end(),
                [&device_io_request](managed_file_system_object_type const & file) {
                    return device_io_request.FileId() == file.file_id;
                }
            );
            if (file_iter == this->managed_file_system_objects.end()) {
                LOG(LOG_WARNING,
                    "FileSystemDriveManager::ProcessDeviceIORequest: "
                        "Unknown file. FileId=%u",
                    device_io_request.FileId());
                return;
            }
        }

        switch (device_io_request.MajorFunction()) {
            case rdpdr::IRP_MJ_CREATE:
                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::ProcessDeviceIORequest: "
                            "Server Create Drive Request");
                }

                if (this->session_probe_drive_access_notifier) {
                    if (DeviceId == this->session_probe_drive_id) {
                        if (!this->session_probe_drive_access_notifier->on_drive_access()) {
                            this->session_probe_drive_access_notifier = nullptr;
                        }
                    }
                }

                this->ProcessServerCreateDriveRequest(device_io_request,
                    path, drive_access_mode, in_stream,
                    to_server_sender, out_asynchronous_task, verbose);
            break;

            case rdpdr::IRP_MJ_CLOSE:
                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::ProcessDeviceIORequest: "
                            "Server Close Drive Request");
                }

                file_iter->object->ProcessServerCloseDriveRequest(
                    device_io_request, path.c_str(), in_stream,
                    to_server_sender, out_asynchronous_task, verbose);
                if(file_iter + 1 != this->managed_file_system_objects.end()) {
                    this->managed_file_system_objects[
                        file_iter - this->managed_file_system_objects.begin()
                    ] = std::move(this->managed_file_system_objects.back());
                }
                this->managed_file_system_objects.pop_back();
            break;

            case rdpdr::IRP_MJ_READ:
                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::ProcessDeviceIORequest: "
                            "Server Drive Read Request");
                }

                {
                    rdpdr::DeviceReadRequest device_read_request;

                    device_read_request.receive(in_stream);
                    if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                        device_read_request.log(LOG_INFO);
                    }
                    if (this->session_probe_image_read_notifier) {
                        if (file_iter->object->IsSessionProbeImage()) {
                            if (!this->session_probe_image_read_notifier->on_image_read(
                                    device_read_request.Offset(),
                                    device_read_request.Length())) {
                                this->session_probe_image_read_notifier = nullptr;
                            }
                        }
                    }

                    file_iter->object->ProcessServerDriveReadRequest(
                        device_io_request, device_read_request, path.c_str(),
                        in_stream, to_server_sender, out_asynchronous_task,
                        verbose);
                }
            break;

            case rdpdr::IRP_MJ_WRITE:
                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::ProcessDeviceIORequest: "
                            "Server Drive Write Request");
                }

                file_iter->object->ProcessServerDriveWriteRequest(
                    device_io_request, path.c_str(), drive_access_mode,
                    first_chunk, in_stream, to_server_sender,
                    out_asynchronous_task, verbose);
            break;

            case rdpdr::IRP_MJ_DEVICE_CONTROL:
                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::ProcessDeviceIORequest: "
                            "Server Drive Control Request");
                }

                {
                    rdpdr::DeviceControlRequest device_control_request;

                    device_control_request.receive(in_stream);
                    if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                        device_control_request.log(LOG_INFO);
                    }

                    file_iter->object->ProcessServerDriveControlRequest(
                        device_io_request, device_control_request,
                        path.c_str(), in_stream, to_server_sender,
                        out_asynchronous_task, verbose);
                }
            break;

            case rdpdr::IRP_MJ_QUERY_VOLUME_INFORMATION:
                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::ProcessDeviceIORequest: "
                            "Server Drive Query Volume Information Request");
                }

                {
                    rdpdr::ServerDriveQueryVolumeInformationRequest
                        server_drive_query_volume_information_request;

                    server_drive_query_volume_information_request.receive(
                        in_stream);
                    if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                        server_drive_query_volume_information_request.log(
                            LOG_INFO);
                    }

                    file_iter->object->ProcessServerDriveQueryVolumeInformationRequest(
                        device_io_request,
                        server_drive_query_volume_information_request,
                        path.c_str(), in_stream, to_server_sender,
                        out_asynchronous_task, verbose);
                }
            break;

            case rdpdr::IRP_MJ_QUERY_INFORMATION:
                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::ProcessDeviceIORequest: "
                            "Server Drive Query Information Request");
                }

                {
                    rdpdr::ServerDriveQueryInformationRequest
                        server_drive_query_information_request;

                    server_drive_query_information_request.receive(in_stream);
                    if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                        server_drive_query_information_request.log(LOG_INFO);
                    }

                    file_iter->object->ProcessServerDriveQueryInformationRequest(
                        device_io_request,
                        server_drive_query_information_request, path.c_str(),
                        in_stream, to_server_sender, out_asynchronous_task,
                        verbose);
                }
            break;

            case rdpdr::IRP_MJ_SET_INFORMATION:
                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::ProcessDeviceIORequest: "
                            "Server Drive Set Information Request");
                }

                {
                    rdpdr::ServerDriveSetInformationRequest
                        server_drive_set_information_request;

                    server_drive_set_information_request.receive(in_stream);
                    if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                        server_drive_set_information_request.log(LOG_INFO);
                    }

                    file_iter->object->ProcessServerDriveSetInformationRequest(
                        device_io_request,
                        server_drive_set_information_request, path.c_str(),
                        drive_access_mode, in_stream, to_server_sender,
                        out_asynchronous_task, verbose);
                }
            break;

            case rdpdr::IRP_MJ_DIRECTORY_CONTROL:
                switch (device_io_request.MinorFunction()) {
                    case rdpdr::IRP_MN_QUERY_DIRECTORY:
                        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                            LOG(LOG_INFO,
                                "FileSystemDriveManager::ProcessDeviceIORequest: "
                                    "Directory control request - "
                                    "Query directory request");
                        }

                        {
                            rdpdr::ServerDriveQueryDirectoryRequest
                                server_drive_query_directory_request;

                            //auto in_stream_p = in_stream.get_current();

                            server_drive_query_directory_request.receive(
                                in_stream);

                            //LOG(LOG_INFO,
                            //    "ServerDriveQueryDirectoryRequest: size=%u",
                            //    (unsigned int)(in_stream.get_current() - in_stream_p));
                            //hexdump(in_stream_p,
                            //    in_stream.get_current() - in_stream_p);

                            if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                                server_drive_query_directory_request.log(
                                    LOG_INFO);
                            }

                            file_iter->object->ProcessServerDriveQueryDirectoryRequest(
                                device_io_request,
                                server_drive_query_directory_request,
                                path.c_str(), in_stream, to_server_sender,
                                out_asynchronous_task, verbose);
                        }
                    break;

                    case rdpdr::IRP_MN_NOTIFY_CHANGE_DIRECTORY:
                        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                            LOG(LOG_INFO,
                                "FileSystemDriveManager::ProcessDeviceIORequest: "
                                    "Directory control request - "
                                    "Notify change directory request");
                        }

                        // Not yet supported!
                    break;

                    default:
                        LOG(LOG_ERR,
                            "FileSystemDriveManager::ProcessDeviceIORequest: "
                                "Unknown Directory control request - "
                                "MinorFunction=0x%X",
                            device_io_request.MinorFunction());
                        throw Error(ERR_RDP_PROTOCOL);
                    //break;
                }
            break;

            case rdpdr::IRP_MJ_LOCK_CONTROL:
                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::ProcessDeviceIORequest: "
                            "Server Drive Lock Control Request");
                }

                ManagedFileSystemObject::SendClientDriveLockControlResponse(
                    device_io_request,
                    "FileSystemDriveManager::ProcessDeviceIORequest",
                    0x00000000, // STATUS_SUCCESS
                    to_server_sender,
                    out_asynchronous_task,
                    verbose);

            break;

            default:
                LOG(LOG_ERR,
                    "FileSystemDriveManager::ProcessDeviceIORequest: "
                        "Undecoded Device I/O Request - "
                        "MajorFunction=%s(0x%X)",
                    rdpdr::DeviceIORequest::get_MajorFunction_name(
                        device_io_request.MajorFunction()),
                    device_io_request.MajorFunction());
                REDASSERT(false);

                ManagedFileSystemObject::SendClientDriveIoUnsuccessfulResponse(
                    device_io_request,
                    "FileSystemDriveManager::ProcessDeviceIORequest",
                    to_server_sender,
                    out_asynchronous_task,
                    verbose);
            break;
        }
    }

    void RemoveSessionProbeDrive(RDPVerbose verbose) {
        if (this->session_probe_drive_id == INVALID_MANAGED_DRIVE_ID) {
            return;
        }

        const uint32_t old_session_probe_drive_id = this->session_probe_drive_id;

        this->session_probe_drive_id = INVALID_MANAGED_DRIVE_ID;

        auto iter = this->find_drive_by_id(old_session_probe_drive_id);
        if (iter != this->managed_drives.end()) {
            if(iter + 1 != this->managed_drives.end()) {
                this->managed_drives[
                    iter - this->managed_drives.begin()
                ] = std::move(this->managed_drives.back());
            }
            this->managed_drives.pop_back();
            if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                LOG(LOG_INFO,
                    "FileSystemDriveManager::RemoveSessionProbeDrive: Drive removed.");
            }
        }
    }

    void DisableSessionProbeDrive(VirtualChannelDataSender & to_server_sender,
            RDPVerbose verbose) {
        if (this->session_probe_drive_id == INVALID_MANAGED_DRIVE_ID) {
            return;
        }

        const uint32_t old_session_probe_drive_id = this->session_probe_drive_id;

        this->session_probe_drive_id = INVALID_MANAGED_DRIVE_ID;

        StaticOutStream<1024> out_stream;

        const rdpdr::SharedHeader sh_s(rdpdr::Component::RDPDR_CTYP_CORE,
                                       rdpdr::PacketId::PAKID_CORE_DEVICELIST_REMOVE);
        sh_s.emit(out_stream);

        out_stream.out_uint32_le(1);                            // DeviceCount(4)
        out_stream.out_uint32_le(old_session_probe_drive_id);   // DeviceIds(variable)

        to_server_sender(
                out_stream.get_offset(),
                CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                out_stream.get_data(),
                out_stream.get_offset()
            );

        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            LOG(LOG_INFO,
                "FileSystemDriveManager::DisableSessionProbeDrive: Remove request sent.");
        }
    }

    void set_session_probe_launcher(SessionProbeLauncher* launcher) {
        this->session_probe_drive_access_notifier = launcher;
        this->session_probe_image_read_notifier   = launcher;
    }
};  // FileSystemDriveManager
