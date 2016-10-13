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
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan, Jennifer Inthavong
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestFlatDialog
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
#include "utils/log.hpp"

#include "core/font.hpp"
#include "mod/internal/widget2/flat_dialog.hpp"
#include "mod/internal/widget2/screen.hpp"
#include "mod/internal/widget2/flat_button.hpp"
#include "check_sig.hpp"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceFlatDialog)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable.gd, 0, 0, 800, 600, parent, notifier, "test1",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, font);

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(flat_dialog.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog1.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x90\x1d\x52\xa3\x11\x2f\xd3\xed\x6e\xe7\xbd\x98\x3d\x57\xe9\xdb\x0a\x72\xe9\x31"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatDialog2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatDialog is a flat_dialog widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable.gd, 0, 0, 640, 480, parent, notifier, "test2",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, font);

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.dx(),
                                      0 + flat_dialog.dy(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog2.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\x75\x16\xab\xf9\x4a\xab\xf8\x20\x06\x9e\x50\x6f\xce\x58\xe8\x36\xf0\x53\xd2\x38"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatDialog3)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatDialog is a flat_dialog widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable.gd, 0, 0, 1280, 1024, parent, notifier, "test3",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, font);

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.dx(),
                                      0 + flat_dialog.dy(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog3.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\x74\xfd\xcc\x8d\x7b\xe8\xe0\xa2\x5f\x23\xa0\x3a\xe5\x04\x30\x93\x87\x73\x1d\xd3"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatDialog4)
{
    TestDraw drawable(1280, 1024);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatDialog is a flat_dialog widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable.gd, 1280, 1024, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable.gd, 0, 0, 1280, 1024, parent, notifier, "test4",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, font);

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.dx(),
                                      0 + flat_dialog.dy(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog4.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\x64\x9c\x88\xe4\xf1\xe7\x64\x81\xec\x12\x3e\x28\x28\x33\xe9\x96\x15\x34\x4b\x3d"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatDialog5)
{
    TestDraw drawable(640, 480);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatDialog is a flat_dialog widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable.gd, 640, 480, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable.gd, 0, 0, 640, 480, parent, notifier, "test5",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, font);

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.dx(),
                                      0 + flat_dialog.dy(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog5.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\x8f\x5a\xdc\xe7\x0d\xb2\xff\x5b\xed\x9d\x9e\xa4\x22\xfa\x36\x95\x46\x33\x05\x16"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatDialog6)
{
    TestDraw drawable(350, 500);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatDialog is a flat_dialog widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, 300, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable.gd, 0, 0, 350, 500, parent, notifier, "test6",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, font);

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.dx(),
                                      0 + flat_dialog.dy(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog6.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\x81\x25\xbb\xbe\xd1\x82\x8f\x02\xb2\x62\xec\xa6\xdb\xfb\x01\xe6\x58\x52\x6e\xdd"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatDialogClip)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatDialog is a flat_dialog widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable.gd, 0, 0, 300, 600, parent, notifier, "test6",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, font);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    flat_dialog.rdp_input_invalidate(Rect(20 + flat_dialog.dx(),
                                      0 + flat_dialog.dy(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog7.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\xf7\x9e\x59\x23\x0b\x84\x1b\x90\x1d\x6e\x3b\xe2\x2f\x0c\x39\x1f\x95\xac\xaf\x4f"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatDialogClip2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatDialog is a flat_dialog widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable.gd, 0, 0, 800, 600, parent, notifier, "test6",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, font);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    flat_dialog.rdp_input_invalidate(Rect(20 + flat_dialog.dx(),
                                      5 + flat_dialog.dy(),
                                      30,
                                      10));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog8.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\x8e\x3b\x71\xe6\x83\xd8\xae\x56\xe6\xea\xd6\x6d\x09\x40\xf7\xe9\x0d\x88\xdb\x1b"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(EventWidgetOkCancel)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    struct Notify : NotifyApi {
        Widget2* sender = nullptr;
        notify_event_t event = 0;

        void notify(Widget2* sender, notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable.gd, 0, 0, 800, 600, parent, &notifier, "test6",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, font);

//    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.sender == &flat_dialog);
    BOOST_CHECK(notifier.event == 0);

    unsigned x = flat_dialog.ok.rect.x + flat_dialog.ok.rect.cx / 2 ;
    unsigned y = flat_dialog.ok.rect.y + flat_dialog.ok.rect.cy / 2 ;
    flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, nullptr);
    // flat_dialog.ok.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, nullptr);
//    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.sender == &flat_dialog);
    BOOST_CHECK(notifier.event == 0);

    flat_dialog.rdp_input_invalidate(flat_dialog.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog-clic-1-button-ok.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\x6e\x22\xbd\x87\x7b\x9c\x93\xb3\x62\xe6\x47\x36\xae\xb0\x2d\x95\x7b\x0d\xf1\xc9"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y, nullptr);
    // flat_dialog.ok.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
    //                                  flat_dialog.ok.dx(), flat_dialog.ok.dy(), nullptr);
    BOOST_CHECK(notifier.sender == &flat_dialog);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = nullptr;
    notifier.event = 0;

    flat_dialog.rdp_input_invalidate(flat_dialog.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog-clic-2-button-ok.png");


    if (!check_sig(drawable.gd.impl(), message,
        "\xa6\x1d\x30\x14\x91\x5b\x81\xa0\xf4\x6d\x44\x30\x88\x3c\xfc\x83\x80\x79\x52\xd8"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    x = flat_dialog.cancel->rect.x + flat_dialog.cancel->rect.cx / 2 ;
    y = flat_dialog.cancel->rect.y + flat_dialog.cancel->rect.cy / 2 ;
    flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, nullptr);
    // flat_dialog.cancel->rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, nullptr);
//    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.sender == &flat_dialog);
    BOOST_CHECK(notifier.event == 0);


    flat_dialog.rdp_input_invalidate(flat_dialog.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog-clic-3-button-cancel.png");


    if (!check_sig(drawable.gd.impl(), message,
        "\xe5\x54\xf4\x28\x42\xd1\xfc\xb5\xf5\x46\xe5\xb9\x18\xa5\x25\x87\x50\x8b\x5b\x97"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y, nullptr);
    BOOST_CHECK(notifier.sender == &flat_dialog);
    BOOST_CHECK(notifier.event == NOTIFY_CANCEL);

    notifier.sender = nullptr;
    notifier.event = 0;

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ESC);
    flat_dialog.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == &flat_dialog);
    BOOST_CHECK(notifier.event == NOTIFY_CANCEL);
}

BOOST_AUTO_TEST_CASE(EventWidgetChallenge)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    struct Notify : NotifyApi {
        Widget2* sender = nullptr;
        notify_event_t event = 0;

        Notify() = default;

        void notify(Widget2* sender, notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable.gd, 0, 0, 800, 600, parent, &notifier, "test6",
                           "Lorem ipsum dolor sit amet, consectetur<br>"
                           "adipiscing elit. Nam purus lacus, luctus sit<br>"
                           "amet suscipit vel, posuere quis turpis. Sed<br>"
                           "venenatis rutrum sem ac posuere. Phasellus<br>"
                           "feugiat dui eu mauris adipiscing sodales.<br>"
                           "Mauris rutrum molestie purus, in tempor lacus<br>"
                           "tincidunt et. Sed eu ligula mauris, a rutrum<br>"
                           "est. Vestibulum in nunc vel massa condimentum<br>"
                           "iaculis nec in arcu. Pellentesque accumsan,<br>"
                           "quam sit amet aliquam mattis, odio purus<br>"
                           "porttitor tortor, sit amet tincidunt odio<br>"
                           "erat ut ligula. Fusce sit amet mauris neque.<br>"
                           "Sed orci augue, luctus in ornare sed,<br>"
                           "adipiscing et arcu.", extra_button, colors, font,
                           "Ok", "Cancel", CHALLENGE_ECHO);

//    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.sender == &flat_dialog);
    BOOST_CHECK(notifier.event == 0);


    flat_dialog.challenge->set_text("challenge_test");

//    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.sender == &flat_dialog);
    BOOST_CHECK(notifier.event == 0);

    flat_dialog.rdp_input_invalidate(flat_dialog.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog-challenge-1.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\xd2\xe8\x8e\x94\xe5\x8f\xcb\x67\xff\xbb\x48\x5c\x2b\x9a\x5b\x8f\xdb\x01\x16\xd9"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    flat_dialog.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == &flat_dialog);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);

    notifier.sender = nullptr;
    notifier.event = 0;
}
