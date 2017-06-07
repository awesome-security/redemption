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
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#pragma once

#include "configs/attributes/spec.hpp"
#include "configs/generators/utils/spec_writer.hpp"
#include "configs/generators/utils/write_template.hpp"
#include "configs/enumeration.hpp"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <chrono>
#include <locale>
#include <vector>
#include <unordered_map>

#include <cerrno>
#include <cstring>


namespace cfg_generators {

namespace python_spec_writer {

using namespace cfg_attributes;

template<class Inherit>
struct PythonSpecWriterBase : ConfigSpecWriterBase<Inherit, spec::name>
{
    using base_type = PythonSpecWriterBase;

    std::ofstream out_file_;
    std::ostringstream out_member_;

    std::ostream & out() { return this->out_member_; }

    PythonSpecWriterBase(char const * filename)
    : out_file_(filename)
    {
        this->out_file_ << "\"## Config file for RDP proxy.\\n\\n\\n\"\n";
    }

    void do_stop_section(std::string const & section_name)
    {
        auto str = this->out_member_.str();
        if (!str.empty()) {
            if (!section_name.empty()) {
                this->out_file_ << "\"[" << section_name << "]\\n\\n\"\n\n";
            }
            this->out_file_ << str;
        }
        this->out_member_.str("");
    }

    template<class Pack>
    void do_member(
        std::string const & /*section_name*/,
        std::string const & member_name,
        Pack const & infos
    ) {
        apply_if_contains<spec::attr>(infos, [&, this](auto && attr, auto && infos) {
            auto type = pack_get<spec::type_>(infos);

            this->write_description(pack_contains<desc>(infos), type, infos);
            this->inherit().write_type_info(type);
            this->write_enumeration_value_description(pack_contains<prefix_value>(infos), type, infos);

            if (bool(attr & spec::attr::iptables_in_gui)) this->out() << "\"#_iptables\\n\"\n";
            if (bool(attr & spec::attr::advanced_in_gui)) this->out() << "\"#_advanced\\n\"\n";
            if (bool(attr & spec::attr::hidden_in_gui))   this->out() << "\"#_hidden\\n\"\n";
            if (bool(attr & spec::attr::hex_in_gui))      this->out() << "\"#_hex\\n\"\n";
            if (bool(attr & spec::attr::password_in_gui)) this->out() << "\"#_password\\n\"\n";

            this->out() << "\"" << member_name << " = ";
            this->inherit().write_type(type, get_default(type, infos));
            this->out() << "\\n\\n\"\n\n";
        }, infos);
    }

    struct macroio {
        const char * name;
        friend std::ostream & operator << (std::ostream & os, macroio const & mio) {
            return os << "\" " << mio.name << " \"";
        }
    };
    macroio get_value(cpp::macro const & m) { return {m.name}; }
    int get_value(types::integer_base) { return 0; }
    int get_value(types::u32) { return 0; }
    int get_value(types::u64) { return 0; }
    template<class T> T const & get_value(T const & x) { return x; }
    template<class T> enable_if_enum_t<T, T const &> get_value(T const & x) { return x; }
    template<class Int, long min, long max, class T> T get_value(types::range<Int, min, max>)
    { static_assert(!min, "unspecified value but 'min' isn't 0"); return {}; }


    macroio quoted2(macroio m) { return m; }
    template<class T> io_quoted2 quoted2(T const & s) { return s; }
    template<class T> char const * quoted2(types::list<T> const &) { return ""; }


    io_prefix_lines comment(char const * s) {
        return io_prefix_lines{s, "\"# ", "\\n\"", 0};
    }


    template<class T, class Pack>
    void write_description(std::true_type, type_<T>, Pack const & pack)
    { this->out() << this->comment(pack_get<desc>(pack).value.c_str()); }

    template<class T, class Pack>
    disable_if_enum_t<T>
    write_description(std::false_type, type_<T>, Pack const &)
    {}

    template<class T, class Pack>
    enable_if_enum_t<T>
    write_description(std::false_type, type_<T>, Pack const &)
    {
        apply_enumeration_for<T>(this->enums, [this](auto const & e) {
            if (e.desc) {
                this->out() << this->comment(e.desc);
            }
        });
    }


    template<class T>
    void write_type_info(type_<T>)
    {}

    void write_type_info(type_<std::chrono::hours>)
    { this->out() << "\"# (is in hour)\\n\"\n"; }

    void write_type_info(type_<std::chrono::minutes>)
    { this->out() << "\"# (is in minute)\\n\"\n"; }

    void write_type_info(type_<std::chrono::seconds>)
    { this->out() << "\"# (is in second)\\n\"\n"; }

    void write_type_info(type_<std::chrono::milliseconds>)
    { this->out() << "\"# (is in millisecond)\\n\"\n"; }

    template<class T, class Ratio>
    void write_type_info(type_<std::chrono::duration<T, Ratio>>)
    { this->out() << "\"# (is in " << Ratio::num << "/" << Ratio::den << " second)\\n\"\n"; }


    template<class T, class V>
    void write_value_(T const & name, V const & v, char const * prefix)
    {
        this->out() << "\"#   " << name;
        if (v.desc) {
            this->out() << ": ";
            if (prefix) {
                this->out() << prefix << " ";
            }
            this->out() << v.desc << "\\n";
        }
        else if (std::is_integral<T>::value) {
            this->out() << ": " << io_replace(v.name, '_', ' ') << "\\n";
        }
        this->out() << "\"\n";
    }

    void write_desc_value(type_enumeration const & e, char const * prefix)
    {
        if (e.is_icase_parser) {
            if (std::none_of(begin(e.values), end(e.values), [](type_enumeration::Value const & v) {
                return v.desc;
            })) {
                return ;
            }
        }

        unsigned d = 0;
        bool const is_autoinc = e.flag == type_enumeration::autoincrement;
        for (type_enumeration::Value const & v : e.values) {
            if (e.is_icase_parser) {
                this->write_value_((v.alias ? v.alias : v.name), v, prefix);
            }
            else {
                this->write_value_((is_autoinc ? d : (1 << d >> 1)), v, prefix);
            }
            ++d;
        }

        if (type_enumeration::flags == e.flag) {
            this->out() << "\"# (note: values can be added (everyone: 1+2+4=7, mute: 0))\\n\"\n";
        }
    }

    void write_desc_value(type_enumeration_set const & e, char const * prefix)
    {
        for (type_enumeration_set::Value const & v : e.values) {
            this->write_value_(v.val, v, prefix);
        }
    }

    template<class Pack>
    std::nullptr_t get_prefix(std::false_type, Pack const &)
    { return nullptr; }

    template<class Pack>
    char const * get_prefix(std::true_type, Pack const & pack)
    { return pack_get<prefix_value>(pack).value; }

    template<bool HasPrefix, class T, class Pack>
    enable_if_enum_t<T>
    write_enumeration_value_description(std::integral_constant<bool, HasPrefix>, type_<T>, Pack const & pack)
    {
        apply_enumeration_for<T>(this->enums, [this, &pack](auto const & e) {
            this->write_desc_value(e, this->get_prefix(pack_contains<prefix_value>(pack), pack));
            if (e.info) {
                this->out() << this->comment(e.info);
            }
        });
    }

    template<bool HasPrefix, class T, class Pack>
    void write_enumeration_value_description(std::integral_constant<bool, HasPrefix>, T, Pack const &)
    { static_assert(!HasPrefix, "prefix_value only with enums type"); }


    template<class T>
    void write_type(type_<bool>, T x)
    { this->out() << "boolean(default=" << (bool(x) ? "True" : "False") << ")"; }

    template<class T>
    void write_type(type_<std::string>, T const & s)
    { this->out() << "string(default='" << quoted2(s) << "')"; }

    template<class Int, class T>
    std::enable_if_t<
        std::is_base_of<types::integer_base, Int>::value
        or
        std::is_integral<Int>::value
    >
    write_type(type_<Int>, T i)
    {
        this->out() << "integer(";
        if (std::is_unsigned<Int>::value || std::is_base_of<types::unsigned_base, Int>::value) {
            this->out() << "min=0, ";
        }
        this->out() << "default=" << this->get_value(i) << ")";
    }

    template<class Int, long min, long max, class T>
    void write_type(type_<types::range<Int, min, max>>, T i)
    { this->out() << "integer(min=" << min << ", max=" << max << ", default=" << this->get_value(i) << ")"; }


    template<class T, class Ratio, class U>
    void write_type(type_<std::chrono::duration<T, Ratio>>, U i)
    { this->out() << "integer(min=0, default=" << this->get_value(i) << ")"; }

    template<unsigned N, class T>
    void write_type(type_<types::fixed_binary<N>>, T const & x)
    {
        this->out() << "string(min=" << N*2 << ", max=" << N*2 << ", default='"
          << io_hexkey{this->get_value(x).c_str(), N} << "')";
    }

    template<unsigned N, class T>
    void write_type(type_<types::fixed_string<N>>, T const & x)
    {
        this->out() << "string(max=" << N <<  ", default='"
          << quoted2(this->get_value(x)) << "')";
    }

    template<class T>
    void write_type(type_<types::dirpath>, T const & x)
    { this->write_type(type_<typename types::dirpath::fixed_type>{}, x); }

    template<class T>
    void write_type(type_<types::ip_string>, T const & x)
    {
        this->out() << "ip_addr(default='" << this->get_value(x) << "')";
    }

    template<class T, class L>
    void write_type(type_<types::list<T>>, L const & s)
    {
        if (is_empty(s)) {
            this->out() << "string_list(default=list())";
        }
        else {
            this->out() << "string_list(default=list('" << quoted2(this->get_value(s)) << "'))";
        }
    }

    template<class T, class E>
    enable_if_enum_t<T>
    write_type(type_<T>, E const & x)
    {
        static_assert(std::is_same<T, E>::value, "");
        apply_enumeration_for<T>(this->enums, [&x, this](auto const & e) {
            this->write_enum_value(e, static_cast<std::underlying_type_t<E>>(x));
        });
    }

    template<class T>
    void write_enum_value(type_enumeration const & e, T default_value)
    {
        if (e.flag == type_enumeration::flags) {
            this->out() << "integer(min=0, max=" << e.max() << ", default=" << default_value << ")";
        }
        else if (e.is_icase_parser) {
            this->out() << "option(";
            for (type_enumeration::Value const & v : e.values) {
                this->out() << "'" << (v.alias ? v.alias : v.name) << "', ";
            }
            this->out() << "default='" << e.values[default_value].name << "')";
        }
        else {
            this->out() << "option(";
            for (unsigned i = 0; i < e.count(); ++i) {
                this->out() << i << ", ";
            }
            this->out() << "default=" << default_value << ")";
        }
    }

    template<class T>
    void write_enum_value(type_enumeration_set const & e, T default_value)
    {
        this->out() << "option(";
        for (type_enumeration_set::Value const & v : e.values) {
            this->out() << v.val << ", ";
        }
        this->out() << "default=" << default_value << ")";
    }
};

}


template<class SpecWriter>
int app_write_python_spec(int ac, char const * const * av)
{
    if (ac < 2) {
        std::cerr << av[0] << " out-spec.h\n";
        return 1;
    }

    SpecWriter writer(av[1]);
    writer.evaluate();

    if (!writer.out_file_) {
        std::cerr << av[0] << ": " << av[1] << ": " << strerror(errno) << "\n";
        return 1;
    }
    return 0;
}

}
