#!env python3
# -*- coding: utf-8 -*-
# Dominique Lafages, Jonathan Poelen
# Copyright WALLIX 2018

###############################################################################################
# script extracting a font from a truetype opensource definition and converting it to the FV2
# format used by ReDemPtion.
#
# HINTs:
# - Each RBF1 glyph is sketched in a bitmap whose dimensions are mutiples of 8. As PIL glyphes
#   width are not multiple of 8 they have to be padded. By convention, they are padded to left
#   and bottom.
# - The glyphs are not antialiased.
# - The police is variable sized
# - Thus, each pixel in a sketch is represented by only one bit
#
# FORMATs :
# - the RBF1 file always begins by the label "RBF1"
# - Police global informations are :
#     * name (32 bytes) (ex : Deja Vu Sans)
#     * size (2 bytes)
#     * style (2 bytes) (always '1')
#     * max height (4 bytes)
#     * number of glyph (4 bytes)
#     * total data len (4 bytes)
# - Individual glyph informations are :
#     * value (4 bytes)
#     * offsetx (2 bytes)
#     * offsety (2 bytes)
#     * abcA (left space)
#     * abcB (glyph width)
#     * abcC (roght space)
#     * cx (2 bytes)
#     * cy (2 bytes)
#     * data (the bitmap representing the sketch of the glyph, one bit by pixel, 0 for
#       background, 1 for foreground) (aligned of 4 btyes)
#
# TECHs :
# - struct.pack formats are :
#     * '<' little endian
#     * 'h' [short] for a two bytes emision
#     * 'B' [unsigned char] for a one byte emision
#     * 'L' [unsigned int] for a four bytes emision
# - the data generation loop print each glyph sketch to sdtout, with each bit represented as
#   follow :
#     * '.' for a PIL background bit
#     * '#' for a PIL foreground bit
#     * 'o' for an horizontal end of line paddind bit
#     * '+' for a vertical paddind line of bits
###############################################################################################

import PIL.ImageFont as ImageFont

# import codecs
import os
import struct
import sys

if len(sys.argv) > 1 and (sys.argv[1] == '-h' or sys.argv[1] == '--help') or \
   len(sys.argv) > 2 and (sys.argv[2] == '-h' or sys.argv[2] == '--help'):
    print(sys.argv[0], '[font_name] [font_size=14]')
    exit(0)

def nbbytes(x):
    return (x + 7) // 8

def align4(x):
    return (x+3) & ~3

def count_bit_padding(cx):
    padding = 8 - cx % 8
    if padding == 8:
        padding = 0
    return padding

# python2: sys.stdout = codecs.getwriter("utf-8")(sys.stdout)
# sys.stdout = codecs.getwriter("utf-8")(sys.stdout.buffer)

fontsize = 14
fontpath = "/usr/share/fonts/truetype/lato/Lato-Light.ttf"
fontpath2 = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"

if len(sys.argv) > 1:
    try:
        fontsize = int(sys.argv[1])
    except ValueError:
        fontpath = sys.argv[1]
        if len(sys.argv) > 2:
            try:
                fontsize = int(sys.argv[2])
            except ValueError:
                pass

CHARSET_SIZE = 0x4e00
ichar_gen = range(32, CHARSET_SIZE)
# ichar_gen = range(32, max(ord('p'),ord('l'),ord('o'),ord('?'))+1)

freetypefont_ = ImageFont.truetype(fontpath, fontsize)
fallback_freetypefont = ImageFont.truetype(fontpath2, fontsize)

unknown_glyph = freetypefont_.getmask("◀", mode="1")
unknown_glyph_bbox = unknown_glyph.getbbox()

def is_unknown_glyph(mask):
    if not unknown_glyph:
        return False
    bbox1 = mask.getbbox()
    if bbox1 != unknown_glyph_bbox:
        return False
    for iy in range(bbox1[1], bbox1[3]):
        for ix in range(bbox1[0], bbox1[2]):
            if mask.getpixel((ix, iy)) != unknown_glyph.getpixel((ix, iy)):
                return False
    return True

if not is_unknown_glyph(freetypefont_.getmask("▶", mode="1")):
    unknown_glyph = None

def get_freetypefont_mask(char):
    mask = freetypefont_.getmask(char, mode="1")
    if is_unknown_glyph(mask):
        return (fallback_freetypefont, fallback_freetypefont.getmask(char, mode="1"))
    return (freetypefont_, mask)


f = open(f"{os.path.splitext(os.path.basename(fontpath))[0]}_{fontsize}.rbf", u'wb')

# Magic number
f.write(u"RBF1".encode('utf-8'))

# Name of font
name = freetypefont_.getname()[0].encode('utf-8')
f.write(name)
f.write(b'\0'*(max(0,32-len(name))))

max_height = 0
total_data_len = 0
freetypefont_masks = []
for i in ichar_gen:
    char = chr(i)
    freetypefont, mask = get_freetypefont_mask(char)
    freetypefont_masks.append((freetypefont, mask))
    bbox = mask.getbbox()
    if bbox is None:
        abc = freetypefont.font.getabc(char)
        total_data_len += align4(nbbytes(int(abc[0]) + int(abc[1]) + int(abc[2])))
    else:
        x1 = bbox[0]
        y1 = bbox[1]
        x2 = bbox[2]
        y2 = bbox[3]
        cx = x2 - x1
        cy = y2 - y1
        # offsetx, offsety = freetypefont.getoffset(char)
        # max_height = max(max_height, offsety + cy)
        max_height = max(max_height, freetypefont.getsize(char)[1])
        total_data_len += align4(nbbytes(cx) * cy)

f.write(struct.pack('<H', fontsize))
f.write(struct.pack('<h', 1))
f.write(struct.pack('<H', max_height))
f.write(struct.pack('<I', len(ichar_gen)))
f.write(struct.pack('<I', total_data_len))

ituple = 0
for i in ichar_gen:
    char = chr(i)
    freetypefont, mask = freetypefont_masks[ituple]
    ituple += 1
    abc = freetypefont.font.getabc(char)
    abc = (int(abc[0]), int(abc[1]), int(abc[2]))
    w, h = freetypefont.getsize(char)
    #x w, h = mask.size # freetypefont.getsize(char)
    offsetx, offsety = freetypefont.getoffset(char)
    bbox = mask.getbbox()
    if bbox is None:
        class mask:
            def getpixel(self):
                return 0
        bbox = (0, 0, abc[0]+abc[1]+abc[2], 1)
        abc = (0, bbox[2], 0)
    x1 = bbox[0]
    y1 = bbox[1]
    x2 = bbox[2]
    y2 = bbox[3]
    cx = x2 - x1
    cy = y2 - y1

    print(f"{i:#x} CHR: {char}  SIZE: {w},{h}  CX/Y: {cx},{cy}  OFFSET: {offsetx},{offsety}  ABC: {abc}  BBOX: {bbox}")

    f.write(struct.pack('<I', i))
    f.write(struct.pack('<h', offsetx))
    f.write(struct.pack('<h', offsety))
    f.write(struct.pack('<h', abc[0]))
    f.write(struct.pack('<h', abc[1]))
    f.write(struct.pack('<h', abc[2]))
    f.write(struct.pack('<H', cx))
    f.write(struct.pack('<H', cy))
    # f.write(struct.pack('<H', x1))
    # f.write(struct.pack('<H', y1))

    padding = count_bit_padding(cx)
    empty_line = '+'*(w+padding) + '\n'
    padding_line = 'o'*(padding)
    left_empty_line = '+'*x1
    right_empty_line = '+'*(w-x1-cx)
    line = empty_line*y1 + '\n'
    for iy in range(y1, y1+cy):
        line += left_empty_line
        byte = 0
        counter = 0

        for ix in range(x1, x1+cx):
            pix = mask.getpixel((ix, iy))
            byte <<= 1
            if pix == 255:
                line += '#'
                byte |= 1
            else:
                line += '.'

            counter += 1
            if counter == 8:
                f.write(struct.pack('<B', byte))
                counter = 0
                byte = 0

        if counter != 0:
            line += padding_line
            f.write(struct.pack('<B', byte << padding))

        line += right_empty_line+'\n'

    f.write(b'\0'*(align4(nbbytes(cx)*cy)-nbbytes(cx)*cy))

    print(line+empty_line*(h-y2)+'\n')
