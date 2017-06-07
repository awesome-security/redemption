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
 *   Copyright (C) Wallix 2010-2014
 *   Author(s): Christophe Grosjean, Meng Tan
 */


#pragma once

#include "utils/colors.hpp"
#include "cfgloader.hpp"
#include "utils/parse.hpp"

#include <string>


struct Theme
{
    struct {
        BGRColor bgcolor = DARK_BLUE_BIS;
        BGRColor fgcolor = WHITE;
        BGRColor separator_color = LIGHT_BLUE;
        BGRColor focus_color = WINBLUE;
        BGRColor error_color = YELLOW;
        bool logo = false;
        std::string logo_path;
    } global {}; // note: fixes -Wmissing-field-initializers on gcc-4.9

    struct {
        BGRColor bgcolor = WHITE;
        BGRColor fgcolor = BLACK;
        BGRColor focus_color = WINBLUE;
    } edit;

    struct {
        BGRColor fgcolor = BLACK;
        BGRColor bgcolor = LIGHT_YELLOW;
        BGRColor border_color = BLACK;
    } tooltip;

    struct {
        BGRColor bgcolor = PALE_BLUE;
        BGRColor fgcolor = BLACK;
    } selector_line1;
    struct {
        BGRColor bgcolor = LIGHT_BLUE;
        BGRColor fgcolor = BLACK;
    } selector_line2;
    struct {
        BGRColor bgcolor = MEDIUM_BLUE;
        BGRColor fgcolor = WHITE;
    } selector_selected;
    struct {
        BGRColor bgcolor = WINBLUE;
        BGRColor fgcolor = WHITE;
    } selector_focus;
    struct {
        BGRColor bgcolor = MEDIUM_BLUE;
        BGRColor fgcolor = WHITE;
    } selector_label;
};

struct ThemeHolder final : public ConfigurationHolder
{
    ThemeHolder(Theme & theme)
    : theme(theme)
    {}

    void set_value(const char * context, const char * key, const char * value) override
    {
        if (0 == strcmp(context, "global")) {
            if (0 == strcmp(key, "bgcolor")){
                this->theme.global.bgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "fgcolor")){
                this->theme.global.fgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "separator_color")){
                this->theme.global.separator_color = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "focus_color")){
                this->theme.global.focus_color = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "error_color")){
                this->theme.global.error_color = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "logo")){
                this->theme.global.logo = Parse(reinterpret_cast<const uint8_t *>(value)).bool_from_cstr();
            }
        }
        else if (0 == strcmp(context, "edit")) {
            if (0 == strcmp(key, "bgcolor")) {
                this->theme.edit.bgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "fgcolor")) {
                this->theme.edit.fgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "focus_color")) {
                this->theme.edit.focus_color = color_from_cstr(value);
            }
        }
        else if (0 == strcmp(context, "tooltip")) {
            if (0 == strcmp(key, "bgcolor")) {
                this->theme.tooltip.bgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "fgcolor")) {
                this->theme.tooltip.fgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "border_color")) {
                this->theme.tooltip.border_color = color_from_cstr(value);
            }
        }
        else if (0 == strcmp(context, "selector")) {
            if (0 == strcmp(key, "line1_fgcolor")) {
                this->theme.selector_line1.fgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "line1_bgcolor")) {
                this->theme.selector_line1.bgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "line2_fgcolor")) {
                this->theme.selector_line2.fgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "line2_bgcolor")) {
                this->theme.selector_line2.bgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "selected_bgcolor")) {
                this->theme.selector_selected.bgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "selected_fgcolor")) {
                this->theme.selector_selected.fgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "focus_bgcolor")) {
                this->theme.selector_focus.bgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "focus_fgcolor")) {
                this->theme.selector_focus.fgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "label_bgcolor")) {
                this->theme.selector_label.bgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "label_fgcolor")) {
                this->theme.selector_label.fgcolor = color_from_cstr(value);
            }
        }
    }

private:
    Theme & theme;
};
