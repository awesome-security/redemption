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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean, Dominique Lafages
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   header file. Keylayout object, used by keymap managers
*/

#if !defined(__KEYLAYOUT_X00000409_HPP__)
#define __KEYLAYOUT_X00000409_HPP__

#include "keylayout.hpp"

namespace x00000409 {    // English (United States)

const static int LCID = 0x00000409;

const Keylayout::KeyLayout_t noMod = {
    /* x00 - x07 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x08 - x0F */    0x0000, 0x001B,    '1',    '2',    '3',    '4',    '5',    '6',
    /* x10 - x17 */       '7',    '8',    '9',    '0', 0x002D, 0x003D, 0x0008, 0x0009,
    /* x18 - x1F */       'q',    'w',    'e',    'r',    't',    'y',    'u',    'i',
    /* x20 - x27 */       'o',    'p', 0x005B, 0x005D, 0x000D, 0x0000,    'a',    's',
    /* x28 - x2F */       'd',    'f',    'g',    'h',    'j',    'k',    'l', 0x003B,
    /* x30 - x37 */    0x0027, 0x0060, 0x0000, 0x005C,    'z',    'x',    'c',    'v',
    /* x38 - x3F */       'b',    'n',    'm', 0x002C, 0x002E, 0x002F, 0x0000, 0x002A,
    /* x40 - x47 */    0x0000, 0x0020, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x48 - x4F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0037,
    /* x50 - x57 */    0x0038, 0x0039, 0x002D, 0x0034, 0x0035, 0x0036, 0x002B, 0x0031,
    /* x58 - x5F */    0x0032, 0x0033, 0x0030, 0x002E, 0x0000, 0x0000, 0x005C, 0x0000,
    /* x60 - x67 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x68 - x6F */    0x0000, 0x0000, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x70 - x77 */    0x002F, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

const Keylayout::KeyLayout_t shift = {
    /* x00 - x07 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x08 - x0F */    0x0000, 0x001B, 0x0021, 0x0040, 0x0023, 0x0024, 0x0025, 0x005E,
    /* x10 - x17 */    0x0026, 0x002A, 0x0028, 0x0029, 0x005F, 0x002B, 0x0008, 0x0000,
    /* x18 - x1F */       'Q',    'W',    'E',    'R',    'T',    'Y',    'U',    'I',
    /* x20 - x27 */       'O',    'P', 0x007B, 0x007D, 0x000D, 0x0000,    'A',    'S',
    /* x28 - x2F */       'D',    'F',    'G',    'H',    'J',    'K',    'L', 0x003A,
    /* x30 - x37 */    0x0022, 0x007E, 0x0000, 0x007C,    'Z',    'X',    'C',    'V',
    /* x38 - x3F */       'B',    'N',    'M', 0x003C, 0x003E, 0x003F, 0x0000, 0x002A,
    /* x40 - x47 */    0x0000, 0x0020, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x48 - x4F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x50 - x57 */    0x0000, 0x0000, 0x002D, 0x0000, 0x0000, 0x0000, 0x002B, 0x0000,
    /* x58 - x5F */    0x0000, 0x0000, 0x0000, 0x002E, 0x0000, 0x0000, 0x007C, 0x0000,
    /* x60 - x67 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x68 - x6F */    0x0000, 0x0000, 0x0000, 0x007F, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x70 - x77 */    0x002F, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

const Keylayout::KeyLayout_t altGr = {
    /* x00 - x07 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x08 - x0F */    0x0000, 0x001B, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x10 - x17 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0008, 0x0009,
    /* x18 - x1F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x20 - x27 */    0x0000, 0x0000, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x28 - x2F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x30 - x37 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x38 - x3F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x002A,
    /* x40 - x47 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x48 - x4F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x50 - x57 */    0x0000, 0x0000, 0x002D, 0x0000, 0x0000, 0x0000, 0x002B, 0x0000,
    /* x58 - x5F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x60 - x67 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x68 - x6F */    0x0000, 0x0000, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x70 - x77 */    0x002F, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

const Keylayout::KeyLayout_t shiftAltGr = {
    /* x00 - x07 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x08 - x0F */    0x0000, 0x001B, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x10 - x17 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0008, 0x0009,
    /* x18 - x1F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x20 - x27 */    0x0000, 0x0000, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x28 - x2F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x30 - x37 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x38 - x3F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x002A,
    /* x40 - x47 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x48 - x4F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x50 - x57 */    0x0000, 0x0000, 0x002D, 0x0000, 0x0000, 0x0000, 0x002B, 0x0000,
    /* x58 - x5F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x60 - x67 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x68 - x6F */    0x0000, 0x0000, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x70 - x77 */    0x002F, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

const Keylayout::KeyLayout_t ctrl = {
    /* x00 - x07 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x08 - x0F */    0x0000, 0x001B, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x10 - x17 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0008, 0x0009,
    /* x18 - x1F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x20 - x27 */    0x0000, 0x0000, 0x001B, 0x001D, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x28 - x2F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x30 - x37 */    0x0000, 0x0000, 0x0000, 0x001C, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x38 - x3F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x002A,
    /* x40 - x47 */    0x0000, 0x0020, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x48 - x4F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x50 - x57 */    0x0000, 0x0000, 0x002D, 0x0000, 0x0000, 0x0000, 0x002B, 0x0000,
    /* x58 - x5F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x001C, 0x0000,
    /* x60 - x67 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x68 - x6F */    0x0000, 0x0000, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x70 - x77 */    0x002F, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

const Keylayout::KeyLayout_t capslock_noMod = {
    /* x00 - x07 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x08 - x0F */    0x0000, 0x001B,    '1',    '2',    '3',    '4',    '5',    '6',
    /* x10 - x17 */       '7',    '8',    '9',    '0', 0x002D, 0x003D, 0x0008, 0x0009,
    /* x18 - x1F */       'Q',    'W',    'E',    'R',    'T',    'Y',    'U',    'I',
    /* x20 - x27 */       'O',    'P', 0x005B, 0x005D, 0x000D, 0x0000,    'A',    'S',
    /* x28 - x2F */       'D',    'F',    'G',    'H',    'J',    'K',    'L', 0x003B,
    /* x30 - x37 */    0x0027, 0x0060, 0x0000, 0x005C,    'Z',    'X',    'C',    'V',
    /* x38 - x3F */       'B',    'N',    'M', 0x002C, 0x002E, 0x002F, 0x0000, 0x002A,
    /* x40 - x47 */    0x0000, 0x0020, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x48 - x4F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x50 - x57 */    0x0000, 0x0000, 0x002D, 0x0000, 0x0000, 0x0000, 0x002B, 0x0000,
    /* x58 - x5F */    0x0000, 0x0000, 0x0000, 0x002E, 0x0000, 0x0000, 0x005C, 0x0000,
    /* x60 - x67 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x68 - x6F */    0x0000, 0x0000, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x70 - x77 */    0x002F, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

const Keylayout::KeyLayout_t capslock_shift = {
    /* x00 - x07 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x08 - x0F */    0x0000, 0x001B, 0x0021, 0x0040, 0x0023, 0x0024, 0x0025, 0x005E,
    /* x10 - x17 */    0x0026, 0x002A, 0x0028, 0x0029, 0x005F, 0x002B, 0x0008, 0x0009,
    /* x18 - x1F */       'q',    'w',    'e',    'r',    't',    'y',    'u',    'i',
    /* x20 - x27 */       'o',    'p', 0x007B, 0x007D, 0x000D, 0x0000,    'a',    's',
    /* x28 - x2F */       'd',    'f',    'g',    'h',    'j',    'k',    'l', 0x003A,
    /* x30 - x37 */    0x0022, 0x007E, 0x0000, 0x007C,    'z',    'x',    'c',    'v',
    /* x38 - x3F */       'b',    'n',    'm', 0x003C, 0x003E, 0x003F, 0x0000, 0x002A,
    /* x40 - x47 */    0x0000, 0x0020, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x48 - x4F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x50 - x57 */    0x0000, 0x0000, 0x002D, 0x0000, 0x0000, 0x0000, 0x002B, 0x0000,
    /* x58 - x5F */    0x0000, 0x0000, 0x0000, 0x002E, 0x0000, 0x0000, 0x007C, 0x0000,
    /* x60 - x67 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x68 - x6F */    0x0000, 0x0000, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x70 - x77 */    0x002F, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

const Keylayout::KeyLayout_t capslock_altGr = {
    /* x00 - x07 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x08 - x0F */    0x0000, 0x001B, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x10 - x17 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0008, 0x0009,
    /* x18 - x1F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x20 - x27 */    0x0000, 0x0000, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x28 - x2F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x30 - x37 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x38 - x3F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x002A,
    /* x40 - x47 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x48 - x4F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x50 - x57 */    0x0000, 0x0000, 0x002D, 0x0000, 0x0000, 0x0000, 0x002B, 0x0000,
    /* x58 - x5F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x60 - x67 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x68 - x6F */    0x0000, 0x0000, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x70 - x77 */    0x002F, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

const Keylayout::KeyLayout_t capslock_shiftAltGr = {
    /* x00 - x07 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x08 - x0F */    0x0000, 0x001B, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x10 - x17 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0008, 0x0009,
    /* x18 - x1F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x20 - x27 */    0x0000, 0x0000, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x28 - x2F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x30 - x37 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x38 - x3F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x002A,
    /* x40 - x47 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x48 - x4F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x50 - x57 */    0x0000, 0x0000, 0x002D, 0x0000, 0x0000, 0x0000, 0x002B, 0x0000,
    /* x58 - x5F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x60 - x67 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x68 - x6F */    0x0000, 0x0000, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x70 - x77 */    0x002F, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

const Keylayout::dkey_t deadkeys[] = {
     { 0x00, 0x00,  0, {} },
     { 0x00, 0x00,  0, {} },
     { 0x00, 0x00,  0, {} },
     { 0x00, 0x00,  0, {} },
     { 0x00, 0x00,  0, {} },
};

} // END NAMESPACE - x00000409

static const Keylayout keylayout_x00000409( x00000409::LCID
                                          , x00000409::noMod
                                          , x00000409::shift
                                          , x00000409::altGr
                                          , x00000409::shiftAltGr
                                          , x00000409::ctrl
                                          , x00000409::capslock_noMod
                                          , x00000409::capslock_shift
                                          , x00000409::capslock_altGr
                                          , x00000409::capslock_shiftAltGr
                                          , x00000409::deadkeys
);

#endif
