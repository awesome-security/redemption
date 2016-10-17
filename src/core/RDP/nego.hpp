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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Meng Tan

   Early Transport Protocol Security Negotiation stage

*/


#pragma once


#include "core/RDP/nla/nla.hpp"
#include "core/RDP/x224.hpp"

#include "utils/sugar/strutils.hpp"

struct RdpNego
{
    enum {
        EXTENDED_CLIENT_DATA_SUPPORTED = 0x01
    };

    // Protocol Security Negotiation Protocols
    enum RDP_NEG_PROTOCOLS
    {
        PROTOCOL_RDP = 0x00000001,
        PROTOCOL_TLS = 0x00000002,
        PROTOCOL_NLA = 0x00000004
    };

//    int port;
    uint32_t flags;
    bool tls;
    bool nla;
    bool krb;
//    char* hostname;
//    char* cookie;
    bool restricted_admin_mode;

  // NLA : Network Level Authentication (TLS implicit)
  // TLS : TLS Encryption without NLA
  // RDP: Standard Legacy RDP Encryption without TLS nor NLA
  
    enum
    {
        NEGO_STATE_INITIAL,
        NEGO_STATE_NEGOCIATE,
        // NEGO_STATE_FAIL, // Negotiation failure */
        NEGO_STATE_FINAL
    } state;

//    int tcp_connected;
//    struct rdp_blob
//    {
//        void* data;
//        int length;
//    } * routing_token;

    uint32_t selected_protocol;
    uint32_t requested_protocol;
    uint32_t enabled_protocols;
    char username[128];
    Transport & trans;

    uint8_t hostname[16];
    uint8_t user[128];
    uint8_t password[2048];
    uint8_t domain[256];
    const char * target_host;

    uint8_t * current_password;
    Random & rand;
    TimeObj & timeobj;
    const uint32_t verbose;
    char * lb_info;

    RdpNego(const bool tls, Transport & socket_trans, const char * username, bool nla,
            const char * target_host, const char krb, Random & rand, TimeObj & timeobj,
            const uint32_t verbose = 0)
    : flags(0)
    , tls(nla || tls)
    , nla(nla)
    , krb(nla && krb)
    , restricted_admin_mode(false)
    , state(NEGO_STATE_INITIAL)
    , selected_protocol(PROTOCOL_RDP)
    , requested_protocol(PROTOCOL_RDP)
    , trans(socket_trans)
    , target_host(target_host)
    , current_password(nullptr)
    , rand(rand)
    , timeobj(timeobj)
    , verbose(verbose)
    , lb_info(nullptr)
    {
        if (this->tls){
            this->enabled_protocols = RdpNego::PROTOCOL_RDP
                                    | RdpNego::PROTOCOL_TLS;
            if (this->nla) {
                this->enabled_protocols |= RdpNego::PROTOCOL_NLA;
            }
        }
        else {
            this->enabled_protocols = RdpNego::PROTOCOL_RDP;
        }
        strncpy(this->username, username, 127);
        this->username[127] = 0;

        memset(this->hostname, 0, sizeof(this->hostname));
        memset(this->user,     0, sizeof(this->user));
        memset(this->password, 0, sizeof(this->password));
        memset(this->domain,   0, sizeof(this->domain));
    }

    void set_identity(char const * user, char const * domain, char const * pass, char const * hostname) {
        if (this->nla) {
            snprintf(reinterpret_cast<char*>(this->user), sizeof(this->user), "%s", user);
            snprintf(reinterpret_cast<char*>(this->domain), sizeof(this->domain), "%s", domain);

            // Password is a multi-sz!
            MultiSZCopy(reinterpret_cast<char*>(this->password), sizeof(this->password), pass);
            this->current_password = this->password;

            snprintf(reinterpret_cast<char*>(this->hostname), sizeof(this->hostname), "%s", hostname);
        }
    }

    void set_lb_info(uint8_t * lb_info, size_t lb_info_length) {
        if (lb_info_length > 0) {
            this->lb_info = reinterpret_cast<char *>(lb_info);
        }
    }



// 2.2.1.2 Server X.224 Connection Confirm PDU
// ===========================================

// The X.224 Connection Confirm PDU is an RDP Connection Sequence PDU sent from
// server to client during the Connection Initiation phase (see section
// 1.3.1.1). It is sent as a response to the X.224 Connection Request PDU
// (section 2.2.1.1).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Ccf (7 bytes): An X.224 Class 0 Connection Confirm TPDU, as specified in
// [X224] section 13.4.

// rdpNegData (8 bytes): Optional RDP Negotiation Response (section 2.2.1.2.1)
// structure or an optional RDP Negotiation Failure (section 2.2.1.2.2)
// structure. The length of the negotiation structure is include " in the X.224
// Connection Confirm Length Indicator field.

// 2.2.1.2.1 RDP Negotiation Response (RDP_NEG_RSP)
// ================================================

// The RDP Negotiation Response structure is used by a server to inform the
// client of the security protocol which it has selected to use for the
// connection.

// type (1 byte): An 8-bit, unsigned integer. Negotiation packet type. This
// field MUST be set to 0x02 (TYPE_RDP_NEG_RSP) to indicate that the packet is
// a Negotiation Response.

// flags (1 byte): An 8-bit, unsigned integer. Negotiation packet flags.

// +-------------------------------------+-------------------------------------+
// | 0x01 EXTENDED_CLIENT_DATA_SUPPORTED | The server supports Extended Client |
// |                                     | Data Blocks in the GCC Conference   |
// |                                     | Create Request user data (section   |
// |                                     | 2.2.1.3).                           |
// +-------------------------------------+-------------------------------------+

// length (2 bytes): A 16-bit, unsigned integer. Indicates the packet size. This field MUST be set to 0x0008 (8 bytes)

// selectedProtocol (4 bytes): A 32-bit, unsigned integer. Field indicating the selected security protocol.

// +----------------------------+----------------------------------------------+
// | 0x00000000 PROTOCOL_RDP    | Standard RDP Security (section 5.3)          |
// +----------------------------+----------------------------------------------+
// | 0x00000001 PROTOCOL_SSL    | TLS 1.0 (section 5.4.5.1)                    |
// +----------------------------+----------------------------------------------+
// | 0x00000002 PROTOCOL_HYBRID | CredSSP (section 5.4.5.2)                    |
// +----------------------------+----------------------------------------------+


// 2.2.1.2.2 RDP Negotiation Failure (RDP_NEG_FAILURE)
// ===================================================

// The RDP Negotiation Failure structure is used by a server to inform the
// client of a failure that has occurred while preparing security for the
// connection.

// type (1 byte): An 8-bit, unsigned integer. Negotiation packet type. This
// field MUST be set to 0x03 (TYPE_RDP_NEG_FAILURE) to indicate that the packet
// is a Negotiation Failure.

// flags (1 byte): An 8-bit, unsigned integer. Negotiation packet flags. There
// are currently no defined flags so the field MUST be set to 0x00.

// length (2 bytes): A 16-bit, unsigned integer. Indicates the packet size. This
// field MUST be set to 0x0008 (8 bytes).

// failureCode (4 bytes): A 32-bit, unsigned integer. Field containing the
// failure code.

// +--------------------------------------+------------------------------------+
// | 0x00000001 SSL_REQUIRED_BY_SERVER    | The server requires that the       |
// |                                      | client support Enhanced RDP        |
// |                                      | Security (section 5.4) with either |
// |                                      | TLS 1.0 (section 5.4.5.1) or       |
// |                                      | CredSSP (section 5.4.5.2). If only |
// |                                      | CredSSP was requested then the     |
// |                                      | server only supports TLS.          |
// +--------------------------------------+------------------------------------+
// | 0x00000002 SSL_NOT_ALLOWED_BY_SERVER | The server is configured to only   |
// |                                      | use Standard RDP Security          |
// |                                      | mechanisms (section 5.3) and does  |
// |                                      | not support any External Security  |
// |                                      | Protocols (section 5.4.5).         |
// +--------------------------------------+------------------------------------+
// | 0x00000003 SSL_CERT_NOT_ON_SERVER    | The server does not possess a valid|
// |                                      | authentication certificate and     |
// |                                      | cannot initialize the External     |
// |                                      | Security Protocol Provider         |
// |                                      | (section 5.4.5).                   |
// +--------------------------------------+------------------------------------+
// | 0x00000004 INCONSISTENT_FLAGS        | The list of requested security     |
// |                                      | protocols is not consistent with   |
// |                                      | the current security protocol in   |
// |                                      | effect. This error is only possible|
// |                                      | when the Direct Approach (see      |
// |                                      | sections 5.4.2.2 and 1.3.1.2) is   |
// |                                      | used and an External Security      |
// |                                      | Protocol (section 5.4.5) is already|
// |                                      | being used.                        |
// +--------------------------------------+------------------------------------+
// | 0x00000005 HYBRID_REQUIRED_BY_SERVER | The server requires that the client|
// |                                      | support Enhanced RDP Security      |
// |                                      | (section 5.4) with CredSSP (section|
// |                                      | 5.4.5.2).                          |
// +--------------------------------------+------------------------------------+

    void recv_connection_confirm(
        bool server_cert_store,
        ServerCertCheck server_cert_check,
        ServerNotifier & server_notifier,
        const char * certif_path)
    {
        LOG(LOG_INFO, "RdpNego::recv_connection_confirm");

        constexpr size_t array_size = AUTOSIZE;
        uint8_t array[array_size];
        uint8_t * end = array;
        X224::RecvFactory f(this->trans, &end, array_size);
        InStream stream(array, end - array);
        X224::CC_TPDU_Recv x224(stream);

        if (x224.rdp_neg_type == X224::RDP_NEG_NONE){
            this->tls = false;
            this->nla = false;
            this->state = NEGO_STATE_FINAL;
            LOG(LOG_INFO, "RdpNego::recv_connection_confirm done (legacy, no TLS)");
            return;
        }
        this->selected_protocol = x224.rdp_neg_code;

        if (this->nla) {
            if (x224.rdp_neg_type == X224::RDP_NEG_RSP
            && x224.rdp_neg_code == X224::PROTOCOL_HYBRID){
                // if (x224.rdp_neg_flags & X224::RESTRICTED_ADMIN_MODE_SUPPORTED) {
                //     LOG(LOG_INFO, "Restricted Admin Mode Supported");
                //     this->restricted_admin_mode = true;
                // }
                LOG(LOG_INFO, "activating SSL");
                this->trans.enable_client_tls(
                        server_cert_store,
                        server_cert_check,
                        server_notifier,
                        certif_path
                    );

                LOG(LOG_INFO, "activating CREDSSP");
                rdpCredssp credssp(this->trans, this->user,
//                                   this->domain, this->password,
                                   this->domain, this->current_password,
                                   this->hostname, this->target_host,
                                   this->krb, this->restricted_admin_mode,
                                   this->rand, this->timeobj,
                                   this->verbose);

                int res = 0;
                bool fallback = false;
                try {
                    res = credssp.credssp_client_authenticate();
                }
                catch (Error const & e) {
                    if ((e.id == ERR_TRANSPORT_NO_MORE_DATA) ||
                        (e.id == ERR_TRANSPORT_WRITE_FAILED)) {
                        LOG(LOG_INFO, "NLA/CREDSSP Authentication Failed");
                        res = 1;
                        fallback = true;
                    }
                    else {
                        LOG(LOG_ERR, "Unknown Exception thrown");
                        throw ;
                    }
                }
                if (res != 1) {
                    LOG(LOG_ERR, "NLA/CREDSSP Authentication Failed");
                    throw Error(ERR_NLA_AUTHENTICATION_FAILED);
                }
                else if (!fallback) {
                    this->state = NEGO_STATE_FINAL;
                    return;
                }
            }
            this->trans.disconnect();
            if (!this->trans.connect()){
                LOG(LOG_ERR, "Failed to disconnect transport");
                throw Error(ERR_SOCKET_CONNECT_FAILED);
            }
            this->current_password += (strlen(reinterpret_cast<char*>(this->current_password)) + 1);
            if (*this->current_password) {
                LOG(LOG_INFO, "try next password");
                this->send_negotiation_request();
            }
            else {
                LOG(LOG_INFO, "Can't activate NLA");
                this->nla = false;
                this->tls = true;
                LOG(LOG_INFO, "falling back to SSL only");
                this->send_negotiation_request();
                this->state = NEGO_STATE_NEGOCIATE;
                this->enabled_protocols = RdpNego::PROTOCOL_TLS | RdpNego::PROTOCOL_RDP;
            }
        }
        else if (this->tls) {
            if (x224.rdp_neg_type == X224::RDP_NEG_RSP
            && x224.rdp_neg_code == X224::PROTOCOL_TLS){
                LOG(LOG_INFO, "activating SSL");
                this->trans.enable_client_tls(
                        server_cert_store,
                        server_cert_check,
                        server_notifier,
                        certif_path
                    );
                this->state = NEGO_STATE_FINAL;
            }
            else if (x224.rdp_neg_type == X224::RDP_NEG_FAILURE
            && (x224.rdp_neg_code == X224::SSL_NOT_ALLOWED_BY_SERVER
            || x224.rdp_neg_code == X224::SSL_CERT_NOT_ON_SERVER)){
                LOG(LOG_INFO, "Can't activate SSL, falling back to RDP legacy encryption");
                this->tls = false;
                this->nla = false;
                this->trans.disconnect();
                if (!this->trans.connect()){
                    throw Error(ERR_SOCKET_CONNECT_FAILED);
                }
                this->send_negotiation_request();
                this->state = NEGO_STATE_NEGOCIATE;
                this->enabled_protocols = RdpNego::PROTOCOL_RDP;
            }
            else if (x224.rdp_neg_type == X224::RDP_NEG_FAILURE
                     && x224.rdp_neg_code == X224::HYBRID_REQUIRED_BY_SERVER) {
                LOG(LOG_INFO, "Enable NLA is probably required");
                this->trans.disconnect();
                throw Error(ERR_NEGO_HYBRID_REQUIRED_BY_SERVER);
            }
            else {
                // "Other cases are errors, set an appropriate error message"
                this->trans.disconnect();
                x224.throw_error();
            }
        }
        else {
            if (x224.rdp_neg_type == X224::RDP_NEG_RSP
            && x224.rdp_neg_code == X224::PROTOCOL_RDP){
                this->state = NEGO_STATE_FINAL;
            }
            else {
                // Other cases are errors, set an appropriate error message
                LOG(LOG_INFO, "Enable TLS is probably required");
                this->trans.disconnect();
                x224.throw_error();
            }
            // TODO Check tpdu has no embedded negotiation code
            this->state = NEGO_STATE_FINAL;
        }
        LOG(LOG_INFO, "RdpNego::recv_connection_confirm done");
    }


    // 2.2.1.1 Client X.224 Connection Request PDU
    // ===========================================

    // The X.224 Connection Request PDU is an RDP Connection Sequence PDU sent from
    // client to server during the Connection Initiation phase (see section 1.3.1.1).

    // tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

    // x224Crq (7 bytes): An X.224 Class 0 Connection Request transport protocol
    // data unit (TPDU), as specified in [X224] section 13.3.

    // routingToken (variable): An optional and variable-length routing token
    // (used for load balancing) terminated by a carriage-return (CR) and line-feed
    // (LF) ANSI sequence. For more information about Terminal Server load balancing
    // and the routing token format, see [MSFT-SDLBTS]. The length of the routing
    // token and CR+LF sequence is include " in the X.224 Connection Request Length
    // Indicator field. If this field is present, then the cookie field MUST NOT be
    //  present.

    //cookie (variable): An optional and variable-length ANSI text string terminated
    // by a carriage-return (CR) and line-feed (LF) ANSI sequence. This text string
    // MUST be "Cookie: mstshash=IDENTIFIER", where IDENTIFIER is an ANSI string
    //(an example cookie string is shown in section 4.1.1). The length of the entire
    // cookie string and CR+LF sequence is include " in the X.224 Connection Request
    // Length Indicator field. This field MUST NOT be present if the routingToken
    // field is present.

    // rdpNegData (8 bytes): An optional RDP Negotiation Request (section 2.2.1.1.1)
    // structure. The length of this negotiation structure is include " in the X.224
    // Connection Request Length Indicator field.

    void send_negotiation_request()
    {
        LOG(LOG_INFO, "RdpNego::send_x224_connection_request_pdu");
        char cookie[256];
        snprintf(cookie, 256, "Cookie: mstshash=%s\x0D\x0A", this->username);
        char * cookie_or_token = this->lb_info?this->lb_info:cookie;
        if (this->verbose & 128) {
            LOG(LOG_INFO, "Send %s:", this->lb_info?"load_balance_info":"cookie");
            hexdump_c(cookie_or_token, strlen(cookie_or_token));
        }
        uint32_t rdp_neg_requestedProtocols = X224::PROTOCOL_RDP
                | (this->tls ? X224::PROTOCOL_TLS:0)
                | (this->nla ? X224::PROTOCOL_HYBRID:0);
        
        StaticOutStream<65536> stream;
        X224::CR_TPDU_Send(stream, cookie_or_token,
                           this->tls?(X224::RDP_NEG_REQ):(X224::RDP_NEG_NONE),
                           // X224::RESTRICTED_ADMIN_MODE_REQUIRED,
                           0,
                           rdp_neg_requestedProtocols);
        this->trans.send(stream.get_data(), stream.get_offset());
        LOG(LOG_INFO, "RdpNego::send_x224_connection_request_pdu done");
    }

};


