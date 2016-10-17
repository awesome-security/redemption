//
// ATTENTION -- This file is auto-generated
//

namespace cfg {
    struct globals {
        // AUTHID_GLOBALS_CAPTURE_CHUNK
        // type: bool
        struct capture_chunk {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "capture_chunk"; }
            static constexpr unsigned index() { return 0; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_GLOBALS_AUTH_USER
        // type: std::string
        struct auth_user {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "auth_user"; }
            static constexpr unsigned index() { return 1; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_GLOBALS_HOST
        // type: std::string
        struct host {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "host"; }
            static constexpr unsigned index() { return 2; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_GLOBALS_TARGET
        // type: std::string
        struct target {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "target"; }
            static constexpr unsigned index() { return 3; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_GLOBALS_TARGET_DEVICE
        // type: std::string
        struct target_device {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "target_device"; }
            static constexpr unsigned index() { return 4; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_GLOBALS_DEVICE_ID
        // type: std::string
        struct device_id {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "device_id"; }
            static constexpr unsigned index() { return 5; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_GLOBALS_TARGET_USER
        // type: std::string
        struct target_user {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "target_user"; }
            static constexpr unsigned index() { return 6; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_GLOBALS_TARGET_APPLICATION
        // type: std::string
        struct target_application {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "target_application"; }
            static constexpr unsigned index() { return 7; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_GLOBALS_TARGET_APPLICATION_ACCOUNT
        // type: std::string
        struct target_application_account {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "target_application_account"; }
            static constexpr unsigned index() { return 8; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_GLOBALS_TARGET_APPLICATION_PASSWORD
        // type: std::string
        struct target_application_password {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "target_application_password"; }
            static constexpr unsigned index() { return 9; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // Support of Bitmap Cache.
        // type: bool
        struct bitmap_cache {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "bitmap_cache"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{1};
        };
        // type: bool
        struct glyph_cache {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "glyph_cache"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // type: unsigned int
        struct port {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "port"; }
            using type = unsigned int;
            using sesman_and_spec_type = unsigned int;
            using mapped_type = sesman_and_spec_type;
            type value{3389};
        };
        // type: bool
        struct nomouse {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "nomouse"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // type: bool
        struct notimestamp {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "notimestamp"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // type: Level
        struct encryptionLevel {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "encryptionLevel"; }
            using type = Level;
            using sesman_and_spec_type = Level;
            using mapped_type = sesman_and_spec_type;
            type value{static_cast<type>(0)};
        };
        // type: std::string
        struct authfile {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "authfile"; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value = "/tmp/redemption-sesman-sock";
        };
        // Time out during RDP handshake stage.
        // type: std::chrono::seconds
        struct handshake_timeout {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "handshake_timeout"; }
            using type = std::chrono::seconds;
            using sesman_and_spec_type = std::chrono::seconds;
            using mapped_type = sesman_and_spec_type;
            type value{10};
        };
        // No traffic auto disconnection.
        // type: std::chrono::seconds
        struct session_timeout {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "session_timeout"; }
            using type = std::chrono::seconds;
            using sesman_and_spec_type = std::chrono::seconds;
            using mapped_type = sesman_and_spec_type;
            type value{900};
        };
        // Keepalive.
        // type: std::chrono::seconds
        struct keepalive_grace_delay {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "keepalive_grace_delay"; }
            using type = std::chrono::seconds;
            using sesman_and_spec_type = std::chrono::seconds;
            using mapped_type = sesman_and_spec_type;
            type value{30};
        };
        // Specifies the time to spend on the login screen of proxy RDP before closing client window (0 to desactivate).
        // type: std::chrono::seconds
        struct authentication_timeout {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "authentication_timeout"; }
            using type = std::chrono::seconds;
            using sesman_and_spec_type = std::chrono::seconds;
            using mapped_type = sesman_and_spec_type;
            type value{120};
        };
        // Specifies the time to spend on the close box of proxy RDP before closing client window (0 to desactivate).
        // type: std::chrono::seconds
        struct close_timeout {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "close_timeout"; }
            using type = std::chrono::seconds;
            using sesman_and_spec_type = std::chrono::seconds;
            using mapped_type = sesman_and_spec_type;
            type value{600};
        };
        // AUTHID_GLOBALS_TRACE_TYPE
        // type: TraceType
        struct trace_type {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "trace_type"; }
            static constexpr unsigned index() { return 10; }
            using type = TraceType;
            using sesman_and_spec_type = TraceType;
            using mapped_type = sesman_and_spec_type;
            type value{static_cast<type>(1)};
        };
        // type: std::string
        struct listen_address {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "listen_address"; }
            using type = std::string;
            using sesman_and_spec_type = ::configs::spec_types::ip;
            using mapped_type = sesman_and_spec_type;
            type value = "0.0.0.0";
        };
        // Allow Transparent mode.
        // type: bool
        struct enable_transparent_mode {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "enable_transparent_mode"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // Proxy certificate password.
        // type: char[255]
        struct certificate_password {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "certificate_password"; }
            using type = char[255];
            using sesman_and_spec_type = ::configs::spec_types::fixed_string;
            using mapped_type = sesman_and_spec_type;
            type value = "inquisition";
        };
        // type: ::configs::spec_types::directory_path
        struct png_path {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "png_path"; }
            using type = ::configs::spec_types::directory_path;
            using sesman_and_spec_type = ::configs::spec_types::directory_path;
            using mapped_type = sesman_and_spec_type;
            type value = PNG_PATH;
        };
        // type: ::configs::spec_types::directory_path
        struct wrm_path {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "wrm_path"; }
            using type = ::configs::spec_types::directory_path;
            using sesman_and_spec_type = ::configs::spec_types::directory_path;
            using mapped_type = sesman_and_spec_type;
            type value = WRM_PATH;
        };
        // AUTHID_GLOBALS_IS_REC
        // type: bool
        struct is_rec {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "is_rec"; }
            static constexpr unsigned index() { return 11; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // AUTHID_GLOBALS_MOVIE_PATH
        // type: std::string
        struct movie_path {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "movie_path"; }
            static constexpr unsigned index() { return 12; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // Support of Bitmap Update.
        // type: bool
        struct enable_bitmap_update {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "enable_bitmap_update"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{1};
        };
        // Show close screen.
        // type: bool
        struct enable_close_box {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "enable_close_box"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{1};
        };
        // type: bool
        struct enable_osd {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "enable_osd"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{1};
        };
        // type: bool
        struct enable_osd_display_remote_target {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "enable_osd_display_remote_target"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{1};
        };
        // type: ::configs::spec_types::directory_path
        struct persistent_path {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "persistent_path"; }
            using type = ::configs::spec_types::directory_path;
            using sesman_and_spec_type = ::configs::spec_types::directory_path;
            using mapped_type = sesman_and_spec_type;
            type value = PERSISTENT_PATH;
        };
        // type: bool
        struct enable_wab_integration {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "enable_wab_integration"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // type: bool
        struct allow_using_multiple_monitors {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "allow_using_multiple_monitors"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // Needed to refresh screen of Windows Server 2012.
        // type: bool
        struct bogus_refresh_rect {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "bogus_refresh_rect"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{1};
        };
    };

    struct session_log {
        // type: bool
        struct enable_session_log {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "session_log"; }
            static constexpr char const * name() { return "enable_session_log"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{1};
        };
        // Log redirection in a file
        // type: bool
        struct session_log_redirection {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "session_log"; }
            static constexpr char const * name() { return "session_log_redirection"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // type: KeyboardInputMaskingLevel
        struct keyboard_input_masking_level {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "session_log"; }
            static constexpr char const * name() { return "keyboard_input_masking_level"; }
            using type = KeyboardInputMaskingLevel;
            using sesman_and_spec_type = KeyboardInputMaskingLevel;
            using mapped_type = sesman_and_spec_type;
            type value{static_cast<type>(3)};
        };
    };

    struct client {
        // AUTHID_CLIENT_KEYBOARD_LAYOUT
        // type: unsigned int
        struct keyboard_layout {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "client"; }
            static constexpr char const * name() { return "keyboard_layout"; }
            static constexpr unsigned index() { return 13; }
            using type = unsigned int;
            using sesman_and_spec_type = unsigned int;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // cs-CZ, da-DK, de-DE, el-GR, en-US, es-ES, fi-FI.finnish, fr-FR, is-IS, it-IT, nl-NL, nb-NO, pl-PL.programmers, pt-BR.abnt, ro-RO, ru-RU, hr-HR, sk-SK, sv-SE, tr-TR.q, uk-UA, sl-SI, et-EE, lv-LV, lt-LT.ibm, mk-MK, fo-FO, mt-MT.47, se-NO, kk-KZ, ky-KG, tt-RU, mn-MN, cy-GB, lb-LU, mi-NZ, de-CH, en-GB, es-MX, fr-BE.fr, nl-BE, pt-PT, sr-La, se-SE, uz-Cy, iu-La, fr-CA, sr-Cy, en-CA.fr, fr-CH, bs-Cy, bg-BG.latin, cs-CZ.qwerty, en-IE.irish, de-DE.ibm, el-GR.220, es-ES.variation, hu-HU, en-US.dvorak, it-IT.142, pl-PL, pt-BR.abnt2, ru-RU.typewriter, sk-SK.qwerty, tr-TR.f, lv-LV.qwerty, lt-LT, mt-MT.48, se-NO.ext_norway, fr-BE, se-SE, en-CA.multilingual, en-IE, cs-CZ.programmers, el-GR.319, en-US.international, se-SE.ext_finland_sweden, bg-BG, el-GR.220_latin, en-US.dvorak_left, el-GR.319_latin, en-US.dvorak_right, el-GR.latin, el-GR.polytonic
        // type: std::string
        struct keyboard_layout_proposals {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "client"; }
            static constexpr char const * name() { return "keyboard_layout_proposals"; }
            using type = std::string;
            using sesman_and_spec_type = ::configs::spec_types::list<std::string>;
            using mapped_type = sesman_and_spec_type;
            type value = "en-US, fr-FR, de-DE, ru-RU";
        };
        // If true, ignore password provided by RDP client, user need do login manually.
        // type: bool
        struct ignore_logon_password {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "client"; }
            static constexpr char const * name() { return "ignore_logon_password"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // Enable font smoothing (0x80).
        // type: uint32_t
        struct performance_flags_default {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "client"; }
            static constexpr char const * name() { return "performance_flags_default"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{128};
        };
        // Disable theme (0x8).
// Disable mouse cursor shadows (0x20).
        // type: uint32_t
        struct performance_flags_force_present {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "client"; }
            static constexpr char const * name() { return "performance_flags_force_present"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{40};
        };
        // type: uint32_t
        struct performance_flags_force_not_present {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "client"; }
            static constexpr char const * name() { return "performance_flags_force_not_present"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // If enabled, avoid automatically font smoothing in recorded session.
        // type: bool
        struct auto_adjust_performance_flags {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "client"; }
            static constexpr char const * name() { return "auto_adjust_performance_flags"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{1};
        };
        // Fallback to RDP Legacy Encryption if client does not support TLS.
        // type: bool
        struct tls_fallback_legacy {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "client"; }
            static constexpr char const * name() { return "tls_fallback_legacy"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // type: bool
        struct tls_support {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "client"; }
            static constexpr char const * name() { return "tls_support"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{1};
        };
        // Needed to connect with jrdp, based on bogus X224 layer code.
        // type: bool
        struct bogus_neg_request {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "client"; }
            static constexpr char const * name() { return "bogus_neg_request"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // Needed to connect with Remmina 0.8.3 and freerdp 0.9.4, based on bogus MCS layer code.
        // type: bool
        struct bogus_user_id {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "client"; }
            static constexpr char const * name() { return "bogus_user_id"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{1};
        };
        // If enabled, ignore CTRL+ALT+DEL and CTRL+SHIFT+ESCAPE (or the equivalents) keyboard sequences.
        // AUTHID_CLIENT_DISABLE_TSK_SWITCH_SHORTCUTS
        // type: bool
        struct disable_tsk_switch_shortcuts {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "client"; }
            static constexpr char const * name() { return "disable_tsk_switch_shortcuts"; }
            static constexpr unsigned index() { return 14; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // type: RdpCompression
        struct rdp_compression {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "client"; }
            static constexpr char const * name() { return "rdp_compression"; }
            using type = RdpCompression;
            using sesman_and_spec_type = RdpCompression;
            using mapped_type = sesman_and_spec_type;
            type value{static_cast<type>(4)};
        };
        // type: ColorDepth
        struct max_color_depth {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "client"; }
            static constexpr char const * name() { return "max_color_depth"; }
            using type = ColorDepth;
            using sesman_and_spec_type = ColorDepth;
            using mapped_type = sesman_and_spec_type;
            type value{static_cast<type>(24)};
        };
        // Persistent Disk Bitmap Cache on the front side.
        // type: bool
        struct persistent_disk_bitmap_cache {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "client"; }
            static constexpr char const * name() { return "persistent_disk_bitmap_cache"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{1};
        };
        // Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled).
        // type: bool
        struct cache_waiting_list {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "client"; }
            static constexpr char const * name() { return "cache_waiting_list"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // If enabled, the contents of Persistent Bitmap Caches are stored on disk.
        // type: bool
        struct persist_bitmap_cache_on_disk {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "client"; }
            static constexpr char const * name() { return "persist_bitmap_cache_on_disk"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // Support of Bitmap Compression.
        // type: bool
        struct bitmap_compression {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "client"; }
            static constexpr char const * name() { return "bitmap_compression"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{1};
        };
        // Enables support of Clent Fast-Path Input Event PDUs.
        // type: bool
        struct fast_path {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "client"; }
            static constexpr char const * name() { return "fast_path"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{1};
        };
        // type: bool
        struct enable_suppress_output {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "client"; }
            static constexpr char const * name() { return "enable_suppress_output"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{1};
        };
        // [Not configured]: Compatible with more RDP clients (less secure)
// HIGH:!ADH:!3DES:!SHA: Compatible only with MS Windows 2008 R2 client or more recent (more secure)
        // type: std::string
        struct ssl_cipher_list {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "client"; }
            static constexpr char const * name() { return "ssl_cipher_list"; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: bool
        struct show_target_user_in_f12_message {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "client"; }
            static constexpr char const * name() { return "show_target_user_in_f12_message"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
    };

    struct mod_rdp {
        // type: RdpCompression
        struct rdp_compression {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "rdp_compression"; }
            using type = RdpCompression;
            using sesman_and_spec_type = RdpCompression;
            using mapped_type = sesman_and_spec_type;
            type value{static_cast<type>(4)};
        };
        // type: bool
        struct disconnect_on_logon_user_change {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "disconnect_on_logon_user_change"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // type: std::chrono::seconds
        struct open_session_timeout {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "open_session_timeout"; }
            using type = std::chrono::seconds;
            using sesman_and_spec_type = std::chrono::seconds;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // Enables support of additional drawing orders:
//   15: MultiDstBlt
//   16: MultiPatBlt
//   17: MultiScrBlt
//   18: MultiOpaqueRect
//   22: Polyline
        // type: std::string
        struct extra_orders {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "extra_orders"; }
            using type = std::string;
            using sesman_and_spec_type = ::configs::spec_types::list<unsigned int>;
            using mapped_type = sesman_and_spec_type;
            type value = "15,16,17,18,22";
        };
        // NLA authentication in secondary target.
        // type: bool
        struct enable_nla {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "enable_nla"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{1};
        };
        // If enabled, NLA authentication will try Kerberos before NTLM.
// (if enable_nla is disabled, this value is ignored).
        // type: bool
        struct enable_kerberos {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "enable_kerberos"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // Persistent Disk Bitmap Cache on the mod side.
        // type: bool
        struct persistent_disk_bitmap_cache {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "persistent_disk_bitmap_cache"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{1};
        };
        // Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled).
        // type: bool
        struct cache_waiting_list {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "cache_waiting_list"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{1};
        };
        // If enabled, the contents of Persistent Bitmap Caches are stored on disk.
        // type: bool
        struct persist_bitmap_cache_on_disk {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "persist_bitmap_cache_on_disk"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // Enables channels names (example: channel1,channel2,etc). Character * only, activate all with low priority.
        // type: std::string
        struct allow_channels {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "allow_channels"; }
            using type = std::string;
            using sesman_and_spec_type = ::configs::spec_types::list<std::string>;
            using mapped_type = sesman_and_spec_type;
            type value = "*";
        };
        // Disable channels names (example: channel1,channel2,etc). Character * only, deactivate all with low priority.
        // type: std::string
        struct deny_channels {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "deny_channels"; }
            using type = std::string;
            using sesman_and_spec_type = ::configs::spec_types::list<std::string>;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // Enables support of Server Fast-Path Update PDUs.
        // type: bool
        struct fast_path {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "fast_path"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{1};
        };
        // Enables Server Redirection Support.
        // type: bool
        struct server_redirection_support {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "server_redirection_support"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // type: RedirectionInfo
        struct redir_info {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "redir_info"; }
            using type = RedirectionInfo;
            using mapped_type = type;
            type value{};
        };
        // Needed to connect with VirtualBox, based on bogus TS_UD_SC_NET data block.
        // AUTHID_MOD_RDP_BOGUS_SC_NET_SIZE
        // type: bool
        struct bogus_sc_net_size {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "bogus_sc_net_size"; }
            static constexpr unsigned index() { return 15; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{1};
        };
        // type: BogusLinuxCursor
        struct bogus_linux_cursor {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "bogus_linux_cursor"; }
            using type = BogusLinuxCursor;
            using sesman_and_spec_type = BogusLinuxCursor;
            using mapped_type = sesman_and_spec_type;
            type value{static_cast<type>(2)};
        };
        // AUTHID_MOD_RDP_PROXY_MANAGED_DRIVES
        // type: std::string
        struct proxy_managed_drives {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "proxy_managed_drives"; }
            static constexpr unsigned index() { return 16; }
            using type = std::string;
            using sesman_and_spec_type = ::configs::spec_types::list<std::string>;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_MOD_RDP_IGNORE_AUTH_CHANNEL
        // type: bool
        struct ignore_auth_channel {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "ignore_auth_channel"; }
            static constexpr unsigned index() { return 17; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // Authentication channel used by Auto IT scripts. May be '*' to use default name. Keep empty to disable virtual channel.
        // type: char[8]
        struct auth_channel {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "auth_channel"; }
            using type = char[8];
            using sesman_and_spec_type = ::configs::spec_types::fixed_string;
            using mapped_type = sesman_and_spec_type;
            type value = "*";
        };
        // AUTHID_MOD_RDP_ALTERNATE_SHELL
        // type: std::string
        struct alternate_shell {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "alternate_shell"; }
            static constexpr unsigned index() { return 18; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_MOD_RDP_SHELL_WORKING_DIRECTORY
        // type: std::string
        struct shell_working_directory {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "shell_working_directory"; }
            static constexpr unsigned index() { return 19; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_MOD_RDP_USE_CLIENT_PROVIDED_ALTERNATE_SHELL
        // type: bool
        struct use_client_provided_alternate_shell {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "use_client_provided_alternate_shell"; }
            static constexpr unsigned index() { return 20; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // AUTHID_MOD_RDP_ENABLE_SESSION_PROBE
        // type: bool
        struct enable_session_probe {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "enable_session_probe"; }
            static constexpr unsigned index() { return 21; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // Minimum supported server : Windows Server 2008.
// Clipboard redirection should be remain enabled on Terminal Server.
        // AUTHID_MOD_RDP_SESSION_PROBE_USE_CLIPBOARD_BASED_LAUNCHER
        // type: bool
        struct session_probe_use_clipboard_based_launcher {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "session_probe_use_clipboard_based_launcher"; }
            static constexpr unsigned index() { return 22; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{1};
        };
        // AUTHID_MOD_RDP_ENABLE_SESSION_PROBE_LAUNCH_MASK
        // type: bool
        struct enable_session_probe_launch_mask {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "enable_session_probe_launch_mask"; }
            static constexpr unsigned index() { return 23; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{1};
        };
        // AUTHID_MOD_RDP_SESSION_PROBE_ON_LAUNCH_FAILURE
        // type: SessionProbeOnLaunchFailure
        struct session_probe_on_launch_failure {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "session_probe_on_launch_failure"; }
            static constexpr unsigned index() { return 24; }
            using type = SessionProbeOnLaunchFailure;
            using sesman_and_spec_type = SessionProbeOnLaunchFailure;
            using mapped_type = sesman_and_spec_type;
            type value{static_cast<type>(2)};
        };
        // This parameter is used if session_probe_on_launch_failure is 1 (disconnect user).
// 0 to disable timeout.
        // AUTHID_MOD_RDP_SESSION_PROBE_LAUNCH_TIMEOUT
        // type: std::chrono::milliseconds
        struct session_probe_launch_timeout {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "session_probe_launch_timeout"; }
            static constexpr unsigned index() { return 25; }
            using type = std::chrono::milliseconds;
            using sesman_and_spec_type = std::chrono::milliseconds;
            using mapped_type = sesman_and_spec_type;
            type value{20000};
        };
        // This parameter is used if session_probe_on_launch_failure is 0 (ignore failure and continue) or 2 (reconnect without Session Probe).
// 0 to disable timeout.
        // AUTHID_MOD_RDP_SESSION_PROBE_LAUNCH_FALLBACK_TIMEOUT
        // type: std::chrono::milliseconds
        struct session_probe_launch_fallback_timeout {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "session_probe_launch_fallback_timeout"; }
            static constexpr unsigned index() { return 26; }
            using type = std::chrono::milliseconds;
            using sesman_and_spec_type = std::chrono::milliseconds;
            using mapped_type = sesman_and_spec_type;
            type value{7000};
        };
        // Minimum supported server : Windows Server 2008.
        // AUTHID_MOD_RDP_SESSION_PROBE_START_LAUNCH_TIMEOUT_TIMER_ONLY_AFTER_LOGON
        // type: bool
        struct session_probe_start_launch_timeout_timer_only_after_logon {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "session_probe_start_launch_timeout_timer_only_after_logon"; }
            static constexpr unsigned index() { return 27; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{1};
        };
        // AUTHID_MOD_RDP_SESSION_PROBE_KEEPALIVE_TIMEOUT
        // type: std::chrono::milliseconds
        struct session_probe_keepalive_timeout {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "session_probe_keepalive_timeout"; }
            static constexpr unsigned index() { return 28; }
            using type = std::chrono::milliseconds;
            using sesman_and_spec_type = std::chrono::milliseconds;
            using mapped_type = sesman_and_spec_type;
            type value{5000};
        };
        // AUTHID_MOD_RDP_SESSION_PROBE_ON_KEEPALIVE_TIMEOUT_DISCONNECT_USER
        // type: bool
        struct session_probe_on_keepalive_timeout_disconnect_user {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "session_probe_on_keepalive_timeout_disconnect_user"; }
            static constexpr unsigned index() { return 29; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{1};
        };
        // End automatically a disconnected session
        // AUTHID_MOD_RDP_SESSION_PROBE_END_DISCONNECTED_SESSION
        // type: bool
        struct session_probe_end_disconnected_session {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "session_probe_end_disconnected_session"; }
            static constexpr unsigned index() { return 30; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // type: bool
        struct session_probe_customize_executable_name {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "session_probe_customize_executable_name"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // This policy setting allows you to configure a time limit for disconnected application sessions.
// 0 to disable timeout.
        // AUTHID_MOD_RDP_SESSION_PROBE_DISCONNECTED_APPLICATION_LIMIT
        // type: std::chrono::milliseconds
        struct session_probe_disconnected_application_limit {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "session_probe_disconnected_application_limit"; }
            static constexpr unsigned index() { return 31; }
            using type = std::chrono::milliseconds;
            using sesman_and_spec_type = std::chrono::milliseconds;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // This policy setting allows you to configure a time limit for disconnected Terminal Services sessions.
// 0 to disable timeout.
        // AUTHID_MOD_RDP_SESSION_PROBE_DISCONNECTED_SESSION_LIMIT
        // type: std::chrono::milliseconds
        struct session_probe_disconnected_session_limit {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "session_probe_disconnected_session_limit"; }
            static constexpr unsigned index() { return 32; }
            using type = std::chrono::milliseconds;
            using sesman_and_spec_type = std::chrono::milliseconds;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // This parameter allows you to specify the maximum amount of time that an active Terminal Services session can be idle (without user input) before it is automatically locked by Session Probe.
// 0 to disable timeout.
        // AUTHID_MOD_RDP_SESSION_PROBE_IDLE_SESSION_LIMIT
        // type: std::chrono::milliseconds
        struct session_probe_idle_session_limit {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "session_probe_idle_session_limit"; }
            static constexpr unsigned index() { return 33; }
            using type = std::chrono::milliseconds;
            using sesman_and_spec_type = std::chrono::milliseconds;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // type: char[512]
        struct session_probe_alternate_shell {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "session_probe_alternate_shell"; }
            using type = char[512];
            using sesman_and_spec_type = ::configs::spec_types::fixed_string;
            using mapped_type = sesman_and_spec_type;
            type value = "cmd /k";
        };
        // Keep known server certificates on WAB
        // AUTHID_MOD_RDP_SERVER_CERT_STORE
        // type: bool
        struct server_cert_store {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "server_cert_store"; }
            static constexpr unsigned index() { return 34; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{1};
        };
        // AUTHID_MOD_RDP_SERVER_CERT_CHECK
        // type: ServerCertCheck
        struct server_cert_check {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "server_cert_check"; }
            static constexpr unsigned index() { return 35; }
            using type = ServerCertCheck;
            using sesman_and_spec_type = ServerCertCheck;
            using mapped_type = sesman_and_spec_type;
            type value{static_cast<type>(1)};
        };
        // Warn if check allow connexion to server.
        // AUTHID_MOD_RDP_SERVER_ACCESS_ALLOWED_MESSAGE
        // type: ServerNotification
        struct server_access_allowed_message {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "server_access_allowed_message"; }
            static constexpr unsigned index() { return 36; }
            using type = ServerNotification;
            using sesman_and_spec_type = ServerNotification;
            using mapped_type = sesman_and_spec_type;
            type value{static_cast<type>(1)};
        };
        // Warn that new server certificate file was created.
        // AUTHID_MOD_RDP_SERVER_CERT_CREATE_MESSAGE
        // type: ServerNotification
        struct server_cert_create_message {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "server_cert_create_message"; }
            static constexpr unsigned index() { return 37; }
            using type = ServerNotification;
            using sesman_and_spec_type = ServerNotification;
            using mapped_type = sesman_and_spec_type;
            type value{static_cast<type>(1)};
        };
        // Warn that server certificate file was successfully checked.
        // AUTHID_MOD_RDP_SERVER_CERT_SUCCESS_MESSAGE
        // type: ServerNotification
        struct server_cert_success_message {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "server_cert_success_message"; }
            static constexpr unsigned index() { return 38; }
            using type = ServerNotification;
            using sesman_and_spec_type = ServerNotification;
            using mapped_type = sesman_and_spec_type;
            type value{static_cast<type>(1)};
        };
        // Warn that server certificate file checking failed.
        // AUTHID_MOD_RDP_SERVER_CERT_FAILURE_MESSAGE
        // type: ServerNotification
        struct server_cert_failure_message {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "server_cert_failure_message"; }
            static constexpr unsigned index() { return 39; }
            using type = ServerNotification;
            using sesman_and_spec_type = ServerNotification;
            using mapped_type = sesman_and_spec_type;
            type value{static_cast<type>(1)};
        };
        // Warn that server certificate check raised some internal error.
        // AUTHID_MOD_RDP_SERVER_CERT_ERROR_MESSAGE
        // type: ServerNotification
        struct server_cert_error_message {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "server_cert_error_message"; }
            static constexpr unsigned index() { return 40; }
            using type = ServerNotification;
            using sesman_and_spec_type = ServerNotification;
            using mapped_type = sesman_and_spec_type;
            type value{static_cast<type>(1)};
        };
        // Do not transmit client machine name or RDP server.
        // type: bool
        struct hide_client_name {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_rdp"; }
            static constexpr char const * name() { return "hide_client_name"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
    };

    struct mod_vnc {
        // Enable or disable the clipboard from client (client to server).
        // AUTHID_MOD_VNC_CLIPBOARD_UP
        // type: bool
        struct clipboard_up {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_vnc"; }
            static constexpr char const * name() { return "clipboard_up"; }
            static constexpr unsigned index() { return 41; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // Enable or disable the clipboard from server (server to client).
        // AUTHID_MOD_VNC_CLIPBOARD_DOWN
        // type: bool
        struct clipboard_down {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_vnc"; }
            static constexpr char const * name() { return "clipboard_down"; }
            static constexpr unsigned index() { return 42; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // Sets the encoding types in which pixel data can be sent by the VNC server:
//   0: Raw
//   1: CopyRect
//   2: RRE
//   16: ZRLE
//   -239 (0xFFFFFF11): Cursor pseudo-encoding
        // type: std::string
        struct encodings {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_vnc"; }
            static constexpr char const * name() { return "encodings"; }
            using type = std::string;
            using sesman_and_spec_type = ::configs::spec_types::list<int>;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: bool
        struct allow_authentification_retries {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_vnc"; }
            static constexpr char const * name() { return "allow_authentification_retries"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // VNC server clipboard data encoding type.
        // AUTHID_MOD_VNC_SERVER_CLIPBOARD_ENCODING_TYPE
        // type: ClipboardEncodingType
        struct server_clipboard_encoding_type {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_vnc"; }
            static constexpr char const * name() { return "server_clipboard_encoding_type"; }
            static constexpr unsigned index() { return 43; }
            using type = ClipboardEncodingType;
            using sesman_and_spec_type = ClipboardEncodingType;
            using mapped_type = sesman_and_spec_type;
            type value{static_cast<type>(1)};
        };
        // AUTHID_MOD_VNC_BOGUS_CLIPBOARD_INFINITE_LOOP
        // type: VncBogusClipboardInfiniteLoop
        struct bogus_clipboard_infinite_loop {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_vnc"; }
            static constexpr char const * name() { return "bogus_clipboard_infinite_loop"; }
            static constexpr unsigned index() { return 44; }
            using type = VncBogusClipboardInfiniteLoop;
            using sesman_and_spec_type = VncBogusClipboardInfiniteLoop;
            using mapped_type = sesman_and_spec_type;
            type value{static_cast<type>(0)};
        };
    };

    struct mod_replay {
        // 0 - Wait for Escape, 1 - End session
        // type: bool
        struct on_end_of_data {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "mod_replay"; }
            static constexpr char const * name() { return "on_end_of_data"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
    };

    struct video {
        // type: unsigned int
        struct capture_groupid {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "video"; }
            static constexpr char const * name() { return "capture_groupid"; }
            using type = unsigned int;
            using sesman_and_spec_type = unsigned int;
            using mapped_type = sesman_and_spec_type;
            type value{33};
        };
        // type: CaptureFlags
        struct capture_flags {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "video"; }
            static constexpr char const * name() { return "capture_flags"; }
            using type = CaptureFlags;
            using sesman_and_spec_type = CaptureFlags;
            using mapped_type = sesman_and_spec_type;
            type value{static_cast<type>(3)};
        };
        // Frame interval.
        // type: std::chrono::duration<unsigned int, std::ratio<1, 10>>
        struct png_interval {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "video"; }
            static constexpr char const * name() { return "png_interval"; }
            using type = std::chrono::duration<unsigned int, std::ratio<1, 10>>;
            using sesman_and_spec_type = std::chrono::duration<unsigned int, std::ratio<1, 10>>;
            using mapped_type = sesman_and_spec_type;
            type value{10};
        };
        // Frame interval.
        // type: std::chrono::duration<unsigned int, std::ratio<1, 100>>
        struct frame_interval {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "video"; }
            static constexpr char const * name() { return "frame_interval"; }
            using type = std::chrono::duration<unsigned int, std::ratio<1, 100>>;
            using sesman_and_spec_type = std::chrono::duration<unsigned int, std::ratio<1, 100>>;
            using mapped_type = sesman_and_spec_type;
            type value{40};
        };
        // Time between 2 wrm movies.
        // type: std::chrono::seconds
        struct break_interval {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "video"; }
            static constexpr char const * name() { return "break_interval"; }
            using type = std::chrono::seconds;
            using sesman_and_spec_type = std::chrono::seconds;
            using mapped_type = sesman_and_spec_type;
            type value{600};
        };
        // Number of png captures to keep.
        // type: unsigned int
        struct png_limit {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "video"; }
            static constexpr char const * name() { return "png_limit"; }
            using type = unsigned int;
            using sesman_and_spec_type = unsigned int;
            using mapped_type = sesman_and_spec_type;
            type value{5};
        };
        // type: ::configs::spec_types::directory_path
        struct replay_path {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "video"; }
            static constexpr char const * name() { return "replay_path"; }
            using type = ::configs::spec_types::directory_path;
            using sesman_and_spec_type = ::configs::spec_types::directory_path;
            using mapped_type = sesman_and_spec_type;
            type value = "/tmp/";
        };
        // type: ::configs::spec_types::directory_path
        struct hash_path {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "video"; }
            static constexpr char const * name() { return "hash_path"; }
            using type = ::configs::spec_types::directory_path;
            using sesman_and_spec_type = ::configs::spec_types::directory_path;
            using mapped_type = sesman_and_spec_type;
            type value = HASH_PATH;
        };
        // type: ::configs::spec_types::directory_path
        struct record_tmp_path {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "video"; }
            static constexpr char const * name() { return "record_tmp_path"; }
            using type = ::configs::spec_types::directory_path;
            using sesman_and_spec_type = ::configs::spec_types::directory_path;
            using mapped_type = sesman_and_spec_type;
            type value = RECORD_TMP_PATH;
        };
        // type: ::configs::spec_types::directory_path
        struct record_path {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "video"; }
            static constexpr char const * name() { return "record_path"; }
            using type = ::configs::spec_types::directory_path;
            using sesman_and_spec_type = ::configs::spec_types::directory_path;
            using mapped_type = sesman_and_spec_type;
            type value = RECORD_PATH;
        };
        // type: bool
        struct inactivity_pause {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "video"; }
            static constexpr char const * name() { return "inactivity_pause"; }
            using type = bool;
            using mapped_type = type;
            type value{0};
        };
        // type: std::chrono::seconds
        struct inactivity_timeout {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "video"; }
            static constexpr char const * name() { return "inactivity_timeout"; }
            using type = std::chrono::seconds;
            using mapped_type = type;
            type value{300};
        };
        // Disable keyboard log:
        // type: KeyboardLogFlags
        struct disable_keyboard_log {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "video"; }
            static constexpr char const * name() { return "disable_keyboard_log"; }
            using type = KeyboardLogFlags;
            using sesman_and_spec_type = KeyboardLogFlags;
            using mapped_type = sesman_and_spec_type;
            type value{static_cast<type>(1)};
        };
        // Disable clipboard log:
        // type: ClipboardLogFlags
        struct disable_clipboard_log {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "video"; }
            static constexpr char const * name() { return "disable_clipboard_log"; }
            using type = ClipboardLogFlags;
            using sesman_and_spec_type = ClipboardLogFlags;
            using mapped_type = sesman_and_spec_type;
            type value{static_cast<type>(1)};
        };
        // Disable (redirected) file system log:
        // type: FileSystemLogFlags
        struct disable_file_system_log {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "video"; }
            static constexpr char const * name() { return "disable_file_system_log"; }
            using type = FileSystemLogFlags;
            using sesman_and_spec_type = FileSystemLogFlags;
            using mapped_type = sesman_and_spec_type;
            type value{static_cast<type>(1)};
        };
        // AUTHID_VIDEO_RT_DISPLAY
        // type: unsigned int
        struct rt_display {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "video"; }
            static constexpr char const * name() { return "rt_display"; }
            static constexpr unsigned index() { return 45; }
            using type = unsigned int;
            using sesman_and_spec_type = unsigned int;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // type: ColorDepthSelectionStrategy
        struct wrm_color_depth_selection_strategy {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "video"; }
            static constexpr char const * name() { return "wrm_color_depth_selection_strategy"; }
            using type = ColorDepthSelectionStrategy;
            using sesman_and_spec_type = ColorDepthSelectionStrategy;
            using mapped_type = sesman_and_spec_type;
            type value{static_cast<type>(1)};
        };
        // type: WrmCompressionAlgorithm
        struct wrm_compression_algorithm {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "video"; }
            static constexpr char const * name() { return "wrm_compression_algorithm"; }
            using type = WrmCompressionAlgorithm;
            using sesman_and_spec_type = WrmCompressionAlgorithm;
            using mapped_type = sesman_and_spec_type;
            type value{static_cast<type>(1)};
        };
    };

    struct crypto {
        // AUTHID_CRYPTO_KEY0
        // type: std::array<unsigned char, 32>
        struct key0 {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "crypto"; }
            static constexpr char const * name() { return "key0"; }
            static constexpr unsigned index() { return 46; }
            using type = std::array<unsigned char, 32>;
            using sesman_and_spec_type = ::configs::spec_types::fixed_binary;
            using mapped_type = sesman_and_spec_type;
            type value{{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, }};
        };
        // AUTHID_CRYPTO_KEY1
        // type: std::array<unsigned char, 32>
        struct key1 {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "crypto"; }
            static constexpr char const * name() { return "key1"; }
            static constexpr unsigned index() { return 47; }
            using type = std::array<unsigned char, 32>;
            using sesman_and_spec_type = ::configs::spec_types::fixed_binary;
            using mapped_type = sesman_and_spec_type;
            type value{{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, }};
        };
    };

    struct debug {
        // type: uint32_t
        struct x224 {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "debug"; }
            static constexpr char const * name() { return "x224"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: uint32_t
        struct mcs {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "debug"; }
            static constexpr char const * name() { return "mcs"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: uint32_t
        struct sec {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "debug"; }
            static constexpr char const * name() { return "sec"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: uint32_t
        struct rdp {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "debug"; }
            static constexpr char const * name() { return "rdp"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: uint32_t
        struct primary_orders {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "debug"; }
            static constexpr char const * name() { return "primary_orders"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: uint32_t
        struct secondary_orders {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "debug"; }
            static constexpr char const * name() { return "secondary_orders"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: uint32_t
        struct bitmap {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "debug"; }
            static constexpr char const * name() { return "bitmap"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: uint32_t
        struct capture {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "debug"; }
            static constexpr char const * name() { return "capture"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: uint32_t
        struct auth {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "debug"; }
            static constexpr char const * name() { return "auth"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: uint32_t
        struct session {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "debug"; }
            static constexpr char const * name() { return "session"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: uint32_t
        struct front {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "debug"; }
            static constexpr char const * name() { return "front"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: uint32_t
        struct mod_rdp {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "debug"; }
            static constexpr char const * name() { return "mod_rdp"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: uint32_t
        struct mod_vnc {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "debug"; }
            static constexpr char const * name() { return "mod_vnc"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: uint32_t
        struct mod_int {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "debug"; }
            static constexpr char const * name() { return "mod_int"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: uint32_t
        struct mod_xup {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "debug"; }
            static constexpr char const * name() { return "mod_xup"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: uint32_t
        struct widget {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "debug"; }
            static constexpr char const * name() { return "widget"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: uint32_t
        struct input {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "debug"; }
            static constexpr char const * name() { return "input"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: uint32_t
        struct password {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "debug"; }
            static constexpr char const * name() { return "password"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: uint32_t
        struct compression {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "debug"; }
            static constexpr char const * name() { return "compression"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: uint32_t
        struct cache {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "debug"; }
            static constexpr char const * name() { return "cache"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: uint32_t
        struct bitmap_update {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "debug"; }
            static constexpr char const * name() { return "bitmap_update"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: uint32_t
        struct performance {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "debug"; }
            static constexpr char const * name() { return "performance"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: uint32_t
        struct pass_dialog_box {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "debug"; }
            static constexpr char const * name() { return "pass_dialog_box"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: uint32_t
        struct mod_internal {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "debug"; }
            static constexpr char const * name() { return "mod_internal"; }
            using type = uint32_t;
            using sesman_and_spec_type = uint32_t;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: unsigned int
        struct config {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "debug"; }
            static constexpr char const * name() { return "config"; }
            using type = unsigned int;
            using sesman_and_spec_type = unsigned int;
            using mapped_type = sesman_and_spec_type;
            type value{2};
        };
    };

    struct translation {
        // AUTHID_TRANSLATION_LANGUAGE
        // type: Language
        struct language {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "translation"; }
            static constexpr char const * name() { return "language"; }
            static constexpr unsigned index() { return 48; }
            using type = Language;
            using sesman_and_spec_type = Language;
            using mapped_type = sesman_and_spec_type;
            type value{static_cast<type>(0)};
        };
        // AUTHID_TRANSLATION_PASSWORD_EN
        // type: std::string
        struct password_en {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "translation"; }
            static constexpr char const * name() { return "password_en"; }
            static constexpr unsigned index() { return 49; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_TRANSLATION_PASSWORD_FR
        // type: std::string
        struct password_fr {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "translation"; }
            static constexpr char const * name() { return "password_fr"; }
            static constexpr unsigned index() { return 50; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
    };

    struct internal_mod {
        // type: std::string
        struct theme {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "internal_mod"; }
            static constexpr char const * name() { return "theme"; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
    };

    struct context {
        // type: ::configs::spec_types::directory_path
        struct movie {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "movie"; }
            using type = ::configs::spec_types::directory_path;
            using mapped_type = type;
            type value{};
        };
        // AUTHID_CONTEXT_OPT_BITRATE
        // type: unsigned int
        struct opt_bitrate {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "opt_bitrate"; }
            static constexpr unsigned index() { return 51; }
            using type = unsigned int;
            using sesman_and_spec_type = unsigned int;
            using mapped_type = sesman_and_spec_type;
            type value{40000};
        };
        // AUTHID_CONTEXT_OPT_FRAMERATE
        // type: unsigned int
        struct opt_framerate {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "opt_framerate"; }
            static constexpr unsigned index() { return 52; }
            using type = unsigned int;
            using sesman_and_spec_type = unsigned int;
            using mapped_type = sesman_and_spec_type;
            type value{5};
        };
        // AUTHID_CONTEXT_OPT_QSCALE
        // type: unsigned int
        struct opt_qscale {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "opt_qscale"; }
            static constexpr unsigned index() { return 53; }
            using type = unsigned int;
            using sesman_and_spec_type = unsigned int;
            using mapped_type = sesman_and_spec_type;
            type value{15};
        };
        // AUTHID_CONTEXT_OPT_BPP
        // type: unsigned int
        struct opt_bpp {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "opt_bpp"; }
            static constexpr unsigned index() { return 54; }
            using type = unsigned int;
            using sesman_and_spec_type = unsigned int;
            using mapped_type = sesman_and_spec_type;
            type value{24};
        };
        // AUTHID_CONTEXT_OPT_HEIGHT
        // type: unsigned int
        struct opt_height {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "opt_height"; }
            static constexpr unsigned index() { return 55; }
            using type = unsigned int;
            using sesman_and_spec_type = unsigned int;
            using mapped_type = sesman_and_spec_type;
            type value{600};
        };
        // AUTHID_CONTEXT_OPT_WIDTH
        // type: unsigned int
        struct opt_width {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "opt_width"; }
            static constexpr unsigned index() { return 56; }
            using type = unsigned int;
            using sesman_and_spec_type = unsigned int;
            using mapped_type = sesman_and_spec_type;
            type value{800};
        };
        // type: std::string
        struct auth_error_message {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "auth_error_message"; }
            using type = std::string;
            using mapped_type = type;
            type value{};
        };
        // AUTHID_CONTEXT_SELECTOR
        // type: bool
        struct selector {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "selector"; }
            static constexpr unsigned index() { return 57; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // AUTHID_CONTEXT_SELECTOR_CURRENT_PAGE
        // type: unsigned int
        struct selector_current_page {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "selector_current_page"; }
            static constexpr unsigned index() { return 58; }
            using type = unsigned int;
            using sesman_and_spec_type = unsigned int;
            using mapped_type = sesman_and_spec_type;
            type value{1};
        };
        // AUTHID_CONTEXT_SELECTOR_DEVICE_FILTER
        // type: std::string
        struct selector_device_filter {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "selector_device_filter"; }
            static constexpr unsigned index() { return 59; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_SELECTOR_GROUP_FILTER
        // type: std::string
        struct selector_group_filter {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "selector_group_filter"; }
            static constexpr unsigned index() { return 60; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_SELECTOR_PROTO_FILTER
        // type: std::string
        struct selector_proto_filter {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "selector_proto_filter"; }
            static constexpr unsigned index() { return 61; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_SELECTOR_LINES_PER_PAGE
        // type: unsigned int
        struct selector_lines_per_page {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "selector_lines_per_page"; }
            static constexpr unsigned index() { return 62; }
            using type = unsigned int;
            using sesman_and_spec_type = unsigned int;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // AUTHID_CONTEXT_SELECTOR_NUMBER_OF_PAGES
        // type: unsigned int
        struct selector_number_of_pages {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "selector_number_of_pages"; }
            static constexpr unsigned index() { return 63; }
            using type = unsigned int;
            using sesman_and_spec_type = unsigned int;
            using mapped_type = sesman_and_spec_type;
            type value{1};
        };
        // AUTHID_CONTEXT_TARGET_PASSWORD
        // type: std::string
        struct target_password {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "target_password"; }
            static constexpr unsigned index() { return 64; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_TARGET_HOST
        // type: std::string
        struct target_host {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "target_host"; }
            static constexpr unsigned index() { return 65; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_TARGET_SERVICE
        // type: std::string
        struct target_service {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "target_service"; }
            static constexpr unsigned index() { return 66; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_TARGET_PORT
        // type: unsigned int
        struct target_port {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "target_port"; }
            static constexpr unsigned index() { return 67; }
            using type = unsigned int;
            using sesman_and_spec_type = unsigned int;
            using mapped_type = sesman_and_spec_type;
            type value{3389};
        };
        // AUTHID_CONTEXT_TARGET_PROTOCOL
        // type: std::string
        struct target_protocol {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "target_protocol"; }
            static constexpr unsigned index() { return 68; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value = "RDP";
        };
        // AUTHID_CONTEXT_PASSWORD
        // type: std::string
        struct password {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "password"; }
            static constexpr unsigned index() { return 69; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_REPORTING
        // type: std::string
        struct reporting {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "reporting"; }
            static constexpr unsigned index() { return 70; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_AUTH_CHANNEL_ANSWER
        // type: std::string
        struct auth_channel_answer {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "auth_channel_answer"; }
            static constexpr unsigned index() { return 71; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_AUTH_CHANNEL_TARGET
        // type: std::string
        struct auth_channel_target {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "auth_channel_target"; }
            static constexpr unsigned index() { return 72; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_MESSAGE
        // type: std::string
        struct message {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "message"; }
            static constexpr unsigned index() { return 73; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_ACCEPT_MESSAGE
        // type: bool
        struct accept_message {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "accept_message"; }
            static constexpr unsigned index() { return 74; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_DISPLAY_MESSAGE
        // type: bool
        struct display_message {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "display_message"; }
            static constexpr unsigned index() { return 75; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_REJECTED
        // type: std::string
        struct rejected {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "rejected"; }
            static constexpr unsigned index() { return 76; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_AUTHENTICATED
        // type: bool
        struct authenticated {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "authenticated"; }
            static constexpr unsigned index() { return 77; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // AUTHID_CONTEXT_KEEPALIVE
        // type: bool
        struct keepalive {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "keepalive"; }
            static constexpr unsigned index() { return 78; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // AUTHID_CONTEXT_SESSION_ID
        // type: std::string
        struct session_id {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "session_id"; }
            static constexpr unsigned index() { return 79; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_END_DATE_CNX
        // type: unsigned int
        struct end_date_cnx {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "end_date_cnx"; }
            static constexpr unsigned index() { return 80; }
            using type = unsigned int;
            using sesman_and_spec_type = unsigned int;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // AUTHID_CONTEXT_END_TIME
        // type: std::string
        struct end_time {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "end_time"; }
            static constexpr unsigned index() { return 81; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_MODE_CONSOLE
        // type: std::string
        struct mode_console {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "mode_console"; }
            static constexpr unsigned index() { return 82; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value = "allow";
        };
        // AUTHID_CONTEXT_TIMEZONE
        // type: int
        struct timezone {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "timezone"; }
            static constexpr unsigned index() { return 83; }
            using type = int;
            using sesman_and_spec_type = int;
            using mapped_type = sesman_and_spec_type;
            type value{-3600};
        };
        // AUTHID_CONTEXT_REAL_TARGET_DEVICE
        // type: std::string
        struct real_target_device {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "real_target_device"; }
            static constexpr unsigned index() { return 84; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_AUTHENTICATION_CHALLENGE
        // type: bool
        struct authentication_challenge {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "authentication_challenge"; }
            static constexpr unsigned index() { return 85; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_TICKET
        // type: std::string
        struct ticket {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "ticket"; }
            static constexpr unsigned index() { return 86; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_COMMENT
        // type: std::string
        struct comment {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "comment"; }
            static constexpr unsigned index() { return 87; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_DURATION
        // type: std::string
        struct duration {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "duration"; }
            static constexpr unsigned index() { return 88; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_WAITINFORETURN
        // type: std::string
        struct waitinforeturn {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "waitinforeturn"; }
            static constexpr unsigned index() { return 89; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_SHOWFORM
        // type: bool
        struct showform {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "showform"; }
            static constexpr unsigned index() { return 90; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // AUTHID_CONTEXT_FORMFLAG
        // type: unsigned int
        struct formflag {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "formflag"; }
            static constexpr unsigned index() { return 91; }
            using type = unsigned int;
            using sesman_and_spec_type = unsigned int;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // AUTHID_CONTEXT_MODULE
        // type: std::string
        struct module {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "module"; }
            static constexpr unsigned index() { return 92; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value = "login";
        };
        // AUTHID_CONTEXT_FORCEMODULE
        // type: bool
        struct forcemodule {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "forcemodule"; }
            static constexpr unsigned index() { return 93; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // AUTHID_CONTEXT_PROXY_OPT
        // type: std::string
        struct proxy_opt {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "proxy_opt"; }
            static constexpr unsigned index() { return 94; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_PATTERN_KILL
        // type: std::string
        struct pattern_kill {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "pattern_kill"; }
            static constexpr unsigned index() { return 95; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_PATTERN_NOTIFY
        // type: std::string
        struct pattern_notify {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "pattern_notify"; }
            static constexpr unsigned index() { return 96; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_OPT_MESSAGE
        // type: std::string
        struct opt_message {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "opt_message"; }
            static constexpr unsigned index() { return 97; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_OUTBOUND_CONNECTION_MONITORING_RULES
        // type: std::string
        struct outbound_connection_monitoring_rules {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "outbound_connection_monitoring_rules"; }
            static constexpr unsigned index() { return 98; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_PROCESS_MONITORING_RULES
        // type: std::string
        struct process_monitoring_rules {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "process_monitoring_rules"; }
            static constexpr unsigned index() { return 99; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // type: std::string
        struct manager_disconnect_reason {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "manager_disconnect_reason"; }
            using type = std::string;
            using mapped_type = type;
            type value{};
        };
        // AUTHID_CONTEXT_DISCONNECT_REASON
        // type: std::string
        struct disconnect_reason {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "disconnect_reason"; }
            static constexpr unsigned index() { return 100; }
            using type = std::string;
            using sesman_and_spec_type = std::string;
            using mapped_type = sesman_and_spec_type;
            type value{};
        };
        // AUTHID_CONTEXT_DISCONNECT_REASON_ACK
        // type: bool
        struct disconnect_reason_ack {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "disconnect_reason_ack"; }
            static constexpr unsigned index() { return 101; }
            using type = bool;
            using sesman_and_spec_type = bool;
            using mapped_type = sesman_and_spec_type;
            type value{0};
        };
        // type: std::string
        struct ip_target {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "context"; }
            static constexpr char const * name() { return "ip_target"; }
            using type = std::string;
            using mapped_type = type;
            type value{};
        };
    };

    // type: Theme
    struct theme {
        static constexpr bool is_readable() { return 0; }
        static constexpr bool is_writable() { return 0; }
        static constexpr char const * section() { return ""; }
        static constexpr char const * name() { return "theme"; }
        using type = Theme;
        using mapped_type = type;
        type value{};
    };
    // type: Font
    struct font {
        static constexpr bool is_readable() { return 0; }
        static constexpr bool is_writable() { return 0; }
        static constexpr char const * section() { return ""; }
        static constexpr char const * name() { return "font"; }
        using type = Font;
        using mapped_type = type;
        type value{};
    };

}

namespace cfg_section {
struct globals
: cfg::globals::capture_chunk
, cfg::globals::auth_user
, cfg::globals::host
, cfg::globals::target
, cfg::globals::target_device
, cfg::globals::device_id
, cfg::globals::target_user
, cfg::globals::target_application
, cfg::globals::target_application_account
, cfg::globals::target_application_password
, cfg::globals::bitmap_cache
, cfg::globals::glyph_cache
, cfg::globals::port
, cfg::globals::nomouse
, cfg::globals::notimestamp
, cfg::globals::encryptionLevel
, cfg::globals::authfile
, cfg::globals::handshake_timeout
, cfg::globals::session_timeout
, cfg::globals::keepalive_grace_delay
, cfg::globals::authentication_timeout
, cfg::globals::close_timeout
, cfg::globals::trace_type
, cfg::globals::listen_address
, cfg::globals::enable_transparent_mode
, cfg::globals::certificate_password
, cfg::globals::png_path
, cfg::globals::wrm_path
, cfg::globals::is_rec
, cfg::globals::movie_path
, cfg::globals::enable_bitmap_update
, cfg::globals::enable_close_box
, cfg::globals::enable_osd
, cfg::globals::enable_osd_display_remote_target
, cfg::globals::persistent_path
, cfg::globals::enable_wab_integration
, cfg::globals::allow_using_multiple_monitors
, cfg::globals::bogus_refresh_rect
{ static constexpr bool is_section = true; };

struct session_log
: cfg::session_log::enable_session_log
, cfg::session_log::session_log_redirection
, cfg::session_log::keyboard_input_masking_level
{ static constexpr bool is_section = true; };

struct client
: cfg::client::keyboard_layout
, cfg::client::keyboard_layout_proposals
, cfg::client::ignore_logon_password
, cfg::client::performance_flags_default
, cfg::client::performance_flags_force_present
, cfg::client::performance_flags_force_not_present
, cfg::client::auto_adjust_performance_flags
, cfg::client::tls_fallback_legacy
, cfg::client::tls_support
, cfg::client::bogus_neg_request
, cfg::client::bogus_user_id
, cfg::client::disable_tsk_switch_shortcuts
, cfg::client::rdp_compression
, cfg::client::max_color_depth
, cfg::client::persistent_disk_bitmap_cache
, cfg::client::cache_waiting_list
, cfg::client::persist_bitmap_cache_on_disk
, cfg::client::bitmap_compression
, cfg::client::fast_path
, cfg::client::enable_suppress_output
, cfg::client::ssl_cipher_list
, cfg::client::show_target_user_in_f12_message
{ static constexpr bool is_section = true; };

struct mod_rdp
: cfg::mod_rdp::rdp_compression
, cfg::mod_rdp::disconnect_on_logon_user_change
, cfg::mod_rdp::open_session_timeout
, cfg::mod_rdp::extra_orders
, cfg::mod_rdp::enable_nla
, cfg::mod_rdp::enable_kerberos
, cfg::mod_rdp::persistent_disk_bitmap_cache
, cfg::mod_rdp::cache_waiting_list
, cfg::mod_rdp::persist_bitmap_cache_on_disk
, cfg::mod_rdp::allow_channels
, cfg::mod_rdp::deny_channels
, cfg::mod_rdp::fast_path
, cfg::mod_rdp::server_redirection_support
, cfg::mod_rdp::redir_info
, cfg::mod_rdp::bogus_sc_net_size
, cfg::mod_rdp::bogus_linux_cursor
, cfg::mod_rdp::proxy_managed_drives
, cfg::mod_rdp::ignore_auth_channel
, cfg::mod_rdp::auth_channel
, cfg::mod_rdp::alternate_shell
, cfg::mod_rdp::shell_working_directory
, cfg::mod_rdp::use_client_provided_alternate_shell
, cfg::mod_rdp::enable_session_probe
, cfg::mod_rdp::session_probe_use_clipboard_based_launcher
, cfg::mod_rdp::enable_session_probe_launch_mask
, cfg::mod_rdp::session_probe_on_launch_failure
, cfg::mod_rdp::session_probe_launch_timeout
, cfg::mod_rdp::session_probe_launch_fallback_timeout
, cfg::mod_rdp::session_probe_start_launch_timeout_timer_only_after_logon
, cfg::mod_rdp::session_probe_keepalive_timeout
, cfg::mod_rdp::session_probe_on_keepalive_timeout_disconnect_user
, cfg::mod_rdp::session_probe_end_disconnected_session
, cfg::mod_rdp::session_probe_customize_executable_name
, cfg::mod_rdp::session_probe_disconnected_application_limit
, cfg::mod_rdp::session_probe_disconnected_session_limit
, cfg::mod_rdp::session_probe_idle_session_limit
, cfg::mod_rdp::session_probe_alternate_shell
, cfg::mod_rdp::server_cert_store
, cfg::mod_rdp::server_cert_check
, cfg::mod_rdp::server_access_allowed_message
, cfg::mod_rdp::server_cert_create_message
, cfg::mod_rdp::server_cert_success_message
, cfg::mod_rdp::server_cert_failure_message
, cfg::mod_rdp::server_cert_error_message
, cfg::mod_rdp::hide_client_name
{ static constexpr bool is_section = true; };

struct mod_vnc
: cfg::mod_vnc::clipboard_up
, cfg::mod_vnc::clipboard_down
, cfg::mod_vnc::encodings
, cfg::mod_vnc::allow_authentification_retries
, cfg::mod_vnc::server_clipboard_encoding_type
, cfg::mod_vnc::bogus_clipboard_infinite_loop
{ static constexpr bool is_section = true; };

struct mod_replay
: cfg::mod_replay::on_end_of_data
{ static constexpr bool is_section = true; };

struct video
: cfg::video::capture_groupid
, cfg::video::capture_flags
, cfg::video::png_interval
, cfg::video::frame_interval
, cfg::video::break_interval
, cfg::video::png_limit
, cfg::video::replay_path
, cfg::video::hash_path
, cfg::video::record_tmp_path
, cfg::video::record_path
, cfg::video::inactivity_pause
, cfg::video::inactivity_timeout
, cfg::video::disable_keyboard_log
, cfg::video::disable_clipboard_log
, cfg::video::disable_file_system_log
, cfg::video::rt_display
, cfg::video::wrm_color_depth_selection_strategy
, cfg::video::wrm_compression_algorithm
{ static constexpr bool is_section = true; };

struct crypto
: cfg::crypto::key0
, cfg::crypto::key1
{ static constexpr bool is_section = true; };

struct debug
: cfg::debug::x224
, cfg::debug::mcs
, cfg::debug::sec
, cfg::debug::rdp
, cfg::debug::primary_orders
, cfg::debug::secondary_orders
, cfg::debug::bitmap
, cfg::debug::capture
, cfg::debug::auth
, cfg::debug::session
, cfg::debug::front
, cfg::debug::mod_rdp
, cfg::debug::mod_vnc
, cfg::debug::mod_int
, cfg::debug::mod_xup
, cfg::debug::widget
, cfg::debug::input
, cfg::debug::password
, cfg::debug::compression
, cfg::debug::cache
, cfg::debug::bitmap_update
, cfg::debug::performance
, cfg::debug::pass_dialog_box
, cfg::debug::mod_internal
, cfg::debug::config
{ static constexpr bool is_section = true; };

struct translation
: cfg::translation::language
, cfg::translation::password_en
, cfg::translation::password_fr
{ static constexpr bool is_section = true; };

struct internal_mod
: cfg::internal_mod::theme
{ static constexpr bool is_section = true; };

struct context
: cfg::context::movie
, cfg::context::opt_bitrate
, cfg::context::opt_framerate
, cfg::context::opt_qscale
, cfg::context::opt_bpp
, cfg::context::opt_height
, cfg::context::opt_width
, cfg::context::auth_error_message
, cfg::context::selector
, cfg::context::selector_current_page
, cfg::context::selector_device_filter
, cfg::context::selector_group_filter
, cfg::context::selector_proto_filter
, cfg::context::selector_lines_per_page
, cfg::context::selector_number_of_pages
, cfg::context::target_password
, cfg::context::target_host
, cfg::context::target_service
, cfg::context::target_port
, cfg::context::target_protocol
, cfg::context::password
, cfg::context::reporting
, cfg::context::auth_channel_answer
, cfg::context::auth_channel_target
, cfg::context::message
, cfg::context::accept_message
, cfg::context::display_message
, cfg::context::rejected
, cfg::context::authenticated
, cfg::context::keepalive
, cfg::context::session_id
, cfg::context::end_date_cnx
, cfg::context::end_time
, cfg::context::mode_console
, cfg::context::timezone
, cfg::context::real_target_device
, cfg::context::authentication_challenge
, cfg::context::ticket
, cfg::context::comment
, cfg::context::duration
, cfg::context::waitinforeturn
, cfg::context::showform
, cfg::context::formflag
, cfg::context::module
, cfg::context::forcemodule
, cfg::context::proxy_opt
, cfg::context::pattern_kill
, cfg::context::pattern_notify
, cfg::context::opt_message
, cfg::context::outbound_connection_monitoring_rules
, cfg::context::process_monitoring_rules
, cfg::context::manager_disconnect_reason
, cfg::context::disconnect_reason
, cfg::context::disconnect_reason_ack
, cfg::context::ip_target
{ static constexpr bool is_section = true; };

}

namespace configs {
struct VariablesConfiguration
: cfg_section::globals
, cfg_section::session_log
, cfg_section::client
, cfg_section::mod_rdp
, cfg_section::mod_vnc
, cfg_section::mod_replay
, cfg_section::video
, cfg_section::crypto
, cfg_section::debug
, cfg_section::translation
, cfg_section::internal_mod
, cfg_section::context
, cfg::theme
, cfg::font
{};

using VariablesAclPack = Pack<
  cfg::globals::capture_chunk
, cfg::globals::auth_user
, cfg::globals::host
, cfg::globals::target
, cfg::globals::target_device
, cfg::globals::device_id
, cfg::globals::target_user
, cfg::globals::target_application
, cfg::globals::target_application_account
, cfg::globals::target_application_password
, cfg::globals::trace_type
, cfg::globals::is_rec
, cfg::globals::movie_path
, cfg::client::keyboard_layout
, cfg::client::disable_tsk_switch_shortcuts
, cfg::mod_rdp::bogus_sc_net_size
, cfg::mod_rdp::proxy_managed_drives
, cfg::mod_rdp::ignore_auth_channel
, cfg::mod_rdp::alternate_shell
, cfg::mod_rdp::shell_working_directory
, cfg::mod_rdp::use_client_provided_alternate_shell
, cfg::mod_rdp::enable_session_probe
, cfg::mod_rdp::session_probe_use_clipboard_based_launcher
, cfg::mod_rdp::enable_session_probe_launch_mask
, cfg::mod_rdp::session_probe_on_launch_failure
, cfg::mod_rdp::session_probe_launch_timeout
, cfg::mod_rdp::session_probe_launch_fallback_timeout
, cfg::mod_rdp::session_probe_start_launch_timeout_timer_only_after_logon
, cfg::mod_rdp::session_probe_keepalive_timeout
, cfg::mod_rdp::session_probe_on_keepalive_timeout_disconnect_user
, cfg::mod_rdp::session_probe_end_disconnected_session
, cfg::mod_rdp::session_probe_disconnected_application_limit
, cfg::mod_rdp::session_probe_disconnected_session_limit
, cfg::mod_rdp::session_probe_idle_session_limit
, cfg::mod_rdp::server_cert_store
, cfg::mod_rdp::server_cert_check
, cfg::mod_rdp::server_access_allowed_message
, cfg::mod_rdp::server_cert_create_message
, cfg::mod_rdp::server_cert_success_message
, cfg::mod_rdp::server_cert_failure_message
, cfg::mod_rdp::server_cert_error_message
, cfg::mod_vnc::clipboard_up
, cfg::mod_vnc::clipboard_down
, cfg::mod_vnc::server_clipboard_encoding_type
, cfg::mod_vnc::bogus_clipboard_infinite_loop
, cfg::video::rt_display
, cfg::crypto::key0
, cfg::crypto::key1
, cfg::translation::language
, cfg::translation::password_en
, cfg::translation::password_fr
, cfg::context::opt_bitrate
, cfg::context::opt_framerate
, cfg::context::opt_qscale
, cfg::context::opt_bpp
, cfg::context::opt_height
, cfg::context::opt_width
, cfg::context::selector
, cfg::context::selector_current_page
, cfg::context::selector_device_filter
, cfg::context::selector_group_filter
, cfg::context::selector_proto_filter
, cfg::context::selector_lines_per_page
, cfg::context::selector_number_of_pages
, cfg::context::target_password
, cfg::context::target_host
, cfg::context::target_service
, cfg::context::target_port
, cfg::context::target_protocol
, cfg::context::password
, cfg::context::reporting
, cfg::context::auth_channel_answer
, cfg::context::auth_channel_target
, cfg::context::message
, cfg::context::accept_message
, cfg::context::display_message
, cfg::context::rejected
, cfg::context::authenticated
, cfg::context::keepalive
, cfg::context::session_id
, cfg::context::end_date_cnx
, cfg::context::end_time
, cfg::context::mode_console
, cfg::context::timezone
, cfg::context::real_target_device
, cfg::context::authentication_challenge
, cfg::context::ticket
, cfg::context::comment
, cfg::context::duration
, cfg::context::waitinforeturn
, cfg::context::showform
, cfg::context::formflag
, cfg::context::module
, cfg::context::forcemodule
, cfg::context::proxy_opt
, cfg::context::pattern_kill
, cfg::context::pattern_notify
, cfg::context::opt_message
, cfg::context::outbound_connection_monitoring_rules
, cfg::context::process_monitoring_rules
, cfg::context::disconnect_reason
, cfg::context::disconnect_reason_ack
>;
}
