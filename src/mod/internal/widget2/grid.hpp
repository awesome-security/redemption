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
  Copyright (C) Wallix 2014
  Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan
*/

#pragma once

#include "utils/log.hpp"
#include "mod/internal/widget2/widget.hpp"
#include "keyboard/keymap2.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "gdi/graphic_api.hpp"
#include "utils/difftimeval.hpp"

static const uint16_t GRID_NB_COLUMNS_MAX = 10;
static const uint16_t GRID_NB_ROWS_MAX    = 50;


struct WidgetGrid : public Widget2 {
protected:
    Widget2  * widgets[GRID_NB_COLUMNS_MAX][GRID_NB_ROWS_MAX];
    void     * meta_data[GRID_NB_COLUMNS_MAX][GRID_NB_ROWS_MAX];

    uint16_t nb_rows;

public:
    const uint16_t nb_columns;

protected:
    uint16_t column_width[GRID_NB_COLUMNS_MAX];
    uint16_t row_height[GRID_NB_ROWS_MAX];

public:
    const uint32_t bg_color_1;    // Odd
    const uint32_t fg_color_1;

    const uint32_t bg_color_2;    // Even
    const uint32_t fg_color_2;

    const uint32_t bg_color_focus;
    const uint32_t fg_color_focus;

    const uint32_t bg_color_selection;
    const uint32_t fg_color_selection;

    const uint16_t border;    // Width and height of cell's border.

protected:
    uint16_t selection_y;   // Index of seleted row.

    // TODO: see why grid object need a difftimer ?
    struct difftimer {
        uint64_t t;

        explicit difftimer(uint64_t start = 0)
            : t(start)
        {}

        uint64_t tick() {
            uint64_t ret = this->t;
            this->t = ustime();
            return this->t - ret;
        }

        void update() {
            this->t = ustime();
        }
    } click_interval;

public:
    WidgetGrid(gdi::GraphicApi & drawable, const Rect & rect, Widget2 & parent,
               NotifyApi * notifier, uint16_t nb_rows, uint16_t nb_columns,
               uint32_t bg_color_1, uint32_t fg_color_1,
               uint32_t bg_color_2, uint32_t fg_color_2,
               uint32_t bg_color_focus, uint32_t fg_color_focus,
               uint32_t bg_color_selection, uint32_t fg_color_selection,
               uint16_t border = 0, int group_id = 0)
        : Widget2(drawable, rect, parent, notifier, group_id)
        , widgets()
        , meta_data()
        , nb_rows(nb_rows)
        , nb_columns(nb_columns)
        , column_width()
        , row_height()
        , bg_color_1(bg_color_1)
        , fg_color_1(fg_color_1)
        , bg_color_2(bg_color_2)
        , fg_color_2(fg_color_2)
        , bg_color_focus(bg_color_focus)
        , fg_color_focus(fg_color_focus)
        , bg_color_selection(bg_color_selection)
        , fg_color_selection(fg_color_selection)
        , border(border)
        , selection_y(static_cast<uint16_t>(-1u))
        , click_interval()
    {
        REDASSERT(nb_columns <= GRID_NB_COLUMNS_MAX);
    }

    ~WidgetGrid() override {
    }

    virtual void clear() {
        for (uint16_t column_index = 0; column_index < this->nb_columns; column_index++) {
            for (uint16_t row_index = 1; row_index < GRID_NB_ROWS_MAX; row_index++) {
                this->widgets[column_index][row_index]   = nullptr;
                this->meta_data[column_index][row_index] = nullptr;
            }
        }
        this->nb_rows = 0;
        this->selection_y = static_cast<uint16_t>(-1);
    }


    void draw(const Rect & clip) override {
        for (uint16_t row_index = 0; row_index < this->nb_rows; row_index++) {
            this->draw_row(row_index, clip);
        }
    }

    void draw_row(uint16_t row_index, const Rect & clip) {
        uint32_t bg_color;
        uint32_t fg_color;

        if (this->selection_y == row_index) {
            bg_color = (this->has_focus ? this->bg_color_focus : this->bg_color_selection);
            fg_color = (this->has_focus ? this->fg_color_focus : this->fg_color_selection);
        }
        else {
            const bool odd = row_index & 1;
            bg_color = (odd ? this->bg_color_1 : this->bg_color_2);
            fg_color = (odd ? this->fg_color_1 : this->fg_color_2);
        }

        uint16_t y = this->y();
        for (uint16_t r_index = 0, r_count = std::min<uint16_t>(row_index, this->nb_rows);
             r_index < r_count; r_index++) {
            y += this->row_height[r_index] + this->border * 2;
        }

        uint16_t x = this->x();
        Rect rectRow(x, y, this->cx(), this->row_height[row_index] + this->border * 2);
        this->drawable.draw(RDPOpaqueRect(rectRow, bg_color), clip);

        x += this->border;
        y += this->border;

        for (uint16_t column_index = 0; column_index < this->nb_columns; column_index++) {
            Widget2 * w = this->widgets[column_index][row_index];
            Rect rectCell(x, y, this->column_width[column_index], this->row_height[row_index]);
            if (w) {
                w->set_xy(rectCell.x, rectCell.y);
                w->set_wh(rectCell.cx, rectCell.cy);

                w->set_color(bg_color, fg_color);

                Rect destRect = clip.intersect(rectCell);
                if (!destRect.isempty()) {
                    w->draw(destRect);
                }
            }

            x += this->column_width[column_index] + this->border * 2;
        }
    }

    uint16_t get_column_width(uint16_t column_index) const {
        REDASSERT(column_index < this->nb_columns);

        return this->column_width[column_index];
    }
    void set_column_width(uint16_t column_index, uint16_t width) {
        REDASSERT(column_index < this->nb_columns);

        this->column_width[column_index] = width;
    }

    void * get_meta_data(uint16_t row_index, uint16_t column_index) const {
        REDASSERT(column_index <= this->nb_columns);
        REDASSERT(row_index <= GRID_NB_ROWS_MAX);
        return this->meta_data[column_index][row_index];
    }
    void * set_meta_data(uint16_t row_index, uint16_t column_index, void * meta_data) {
        REDASSERT(column_index <= this->nb_columns);
        REDASSERT(row_index <= GRID_NB_ROWS_MAX);
        void * res = this->meta_data[column_index][row_index];
        this->meta_data[column_index][row_index] = meta_data;
        return res;
    }

    uint16_t get_nb_rows() const {
        return this->nb_rows;
    }
    uint16_t set_nb_rows(uint16_t nb_rows) {
        REDASSERT(nb_rows < GRID_NB_ROWS_MAX);

        uint16_t old_nb_rows = this->nb_rows;
        this->nb_rows = nb_rows;
        return old_nb_rows;
    }

    uint16_t get_row_height(uint16_t row_index) const {
        REDASSERT(row_index < this->nb_rows);

        return this->row_height[row_index];
    }
    void set_row_height(uint16_t row_index, uint16_t height) {
        REDASSERT(row_index < this->nb_rows);

        this->row_height[row_index] = height;
    }

    Widget2 * get_widget(uint16_t row_index, uint16_t column_index) const {
        REDASSERT(column_index <= this->nb_columns);
        REDASSERT(row_index <= GRID_NB_ROWS_MAX);
        return this->widgets[column_index][row_index];
    }
    Widget2 * set_widget(uint16_t row_index, uint16_t column_index, Widget2 * w,
                         void * meta_data = nullptr) {
        REDASSERT(column_index <= this->nb_columns);
        REDASSERT(row_index <= GRID_NB_ROWS_MAX);
        Widget2 * res = this->widgets[column_index][row_index];
        this->widgets[column_index][row_index]   = w;
        this->meta_data[column_index][row_index] = meta_data;

        return res;
    }

    Widget2 * widget_at_pos(int16_t x, int16_t y) override {
        for (unsigned row_index = 0; row_index < this->nb_rows; row_index++) {
            bool empty_row = true;
            for (unsigned column_index = 0; column_index < this->nb_columns; column_index++) {
                if (this->widgets[column_index][row_index]) {
                    empty_row = false;

                    if (this->widgets[column_index][row_index]->get_rect().contains_pt(x, y)) {
                        return this->widgets[column_index][row_index];
                    }
                }
            }

            if (empty_row) {
                break;
            }
        }

        return nullptr;
    }

    void get_selection(uint16_t & row_index, uint16_t & column_index) const {
        row_index    = this->selection_y;
        column_index = static_cast<uint16_t>(-1);
    }

    void set_selection(uint16_t row_index) {
        if (this->focus_flag == Widget2::IGNORE_FOCUS) {
            return;
        }

        if (row_index != this->selection_y) {
            uint16_t previous_selection_y = this->selection_y;
            this->selection_y = row_index;

            this->drawable.begin_update();
            if (previous_selection_y < this->nb_rows) {
                this->draw_row(previous_selection_y, this->get_rect());
            }
            if (this->selection_y < this->nb_rows) {
                this->draw_row(this->selection_y, this->get_rect());
            }
            this->drawable.end_update();
        }
    }

    void refresh_selected() {
        if (this->selection_y < this->nb_rows) {
            this->drawable.begin_update();
            this->draw_row(this->selection_y, this->get_rect());
            this->drawable.end_update();
        }
    }

    void focus(int reason) override {
        (void)reason;
        if (!this->has_focus){
            this->has_focus = true;
            this->send_notify(NOTIFY_FOCUS_BEGIN);
            this->refresh_selected();
        }
    }

    void blur() override {
        if (this->has_focus){
            this->has_focus = false;
            this->send_notify(NOTIFY_FOCUS_END);
            this->refresh_selected();
        }
    }

    void rdp_input_mouse(int device_flags, int mouse_x, int mouse_y, Keymap2 * keymap) override {
        if (device_flags == (MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN)) {
            uint16_t y = this->y();
            for (uint16_t row_index = 0; row_index < this->nb_rows; row_index++) {
                uint16_t x = this->x();
                Rect rectRow(x, y, this->cx(), this->row_height[row_index] + this->border * 2);

                if (rectRow.contains_pt(mouse_x, mouse_y)) {
                    if (row_index != this->selection_y) {
                        this->click_interval.update();
                        this->set_selection(row_index);
                    }
                    else {
                        if (this->click_interval.tick() <= uint64_t(700000L)) {
                            this->send_notify(NOTIFY_SUBMIT);
                            return;
                        }
                    }
                }

                y += this->row_height[row_index] + this->border * 2;
            }
        }
        else if (device_flags == MOUSE_FLAG_MOVE) {
            Widget2 * wid = this->widget_at_pos(mouse_x, mouse_y);
            if (wid) {
                wid->rdp_input_mouse(device_flags, mouse_x, mouse_y, keymap);
            }
        }

        Widget2::rdp_input_mouse(device_flags, mouse_x, mouse_y, keymap);
    }

    void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2 * keymap) override {
        if (keymap->nb_kevent_available() > 0) {
            switch (keymap->top_kevent()) {
                case Keymap2::KEVENT_LEFT_ARROW:
                case Keymap2::KEVENT_UP_ARROW:
                    keymap->get_kevent();
                    if (this->nb_rows > 1) {
                        this->set_selection((this->selection_y > 0) ? this->selection_y - 1 : this->nb_rows - 1);
                    }
                break;
                case Keymap2::KEVENT_RIGHT_ARROW:
                case Keymap2::KEVENT_DOWN_ARROW:
                    keymap->get_kevent();
                    if (this->nb_rows > 1) {
                        this->set_selection((this->selection_y + 1 != this->nb_rows) ? this->selection_y + 1 : 0);
                    }
                break;
                case Keymap2::KEVENT_END:
                    keymap->get_kevent();
                    if ((this->nb_rows > 1) && (this->nb_rows - 1 != this->selection_y)) {
                        this->set_selection(this->nb_rows - 1);
                    }
                    break;
                case Keymap2::KEVENT_HOME:
                    keymap->get_kevent();
                    if ((this->nb_rows > 1) && this->selection_y) {
                        this->set_selection(0);
                    }
                    break;
                case Keymap2::KEVENT_ENTER:
                    keymap->get_kevent();
                    if (this->nb_rows) {
                        this->send_notify(NOTIFY_SUBMIT);
                    }
                    break;
                default:
                    Widget2::rdp_input_scancode(param1, param2, param3, param4, keymap);
                    break;
            }
        }
    }
};

struct ColumnWidthStrategy {
    uint16_t min;
    uint16_t max;
};

inline
void compute_format(WidgetGrid & grid, ColumnWidthStrategy * column_width_strategies, uint16_t * row_height, uint16_t * column_width) {
    uint16_t column_width_optimal[GRID_NB_COLUMNS_MAX] = { 0 };

    for (uint16_t row_index = 0; row_index < grid.get_nb_rows(); row_index++) {
        for (uint16_t column_index = 0; column_index < grid.nb_columns; column_index++) {
            Widget2 * w = grid.get_widget(row_index, column_index);
            if (!w) {
                continue;
            }

            Dimension dim = w->get_optimal_dim();
            if (column_width_optimal[column_index] < dim.w) {
                column_width_optimal[column_index] = dim.w + 2;
            }

            if (row_height[row_index] < dim.h) {
                row_height[row_index] = dim.h;
            }
        }
    }


    // TODO Optiomize this
    uint16_t unsatisfied_column_count = 0;
    // min
    uint16_t unused_width = static_cast<int16_t>(grid.cx() - grid.border * 2 * grid.nb_columns);
    for (uint16_t column_index = 0; column_index < grid.nb_columns; column_index++) {
        column_width[column_index] = column_width_strategies[column_index].min;
        unused_width -= static_cast<int16_t>(column_width_strategies[column_index].min);

        if (column_width[column_index] < std::min(column_width_optimal[column_index], column_width_strategies[column_index].max)) {
            unsatisfied_column_count++;
        }
    }
    // optimal
    while ((unused_width > 0) && (unsatisfied_column_count > 0)) {
        const uint16_t part = unused_width / unsatisfied_column_count;
        if (!part) {
            break;
        }
        unsatisfied_column_count = 0;
        for (uint16_t column_index = 0; column_index < grid.nb_columns; column_index++) {
            uint16_t optimal_max = std::min(column_width_optimal[column_index], column_width_strategies[column_index].max);
            if (column_width[column_index] < optimal_max) {
                uint16_t ajusted_part = std::min<uint16_t>(part, optimal_max - column_width[column_index]);
                column_width[column_index] += ajusted_part;
                unused_width -= ajusted_part;

                if (column_width[column_index] < optimal_max) {
                    unsatisfied_column_count++;
                }
            }
        }
    }
    // max
    unsatisfied_column_count = 0;
    for (uint16_t column_index = 0; column_index < grid.nb_columns; column_index++) {
        if (column_width[column_index] < column_width_strategies[column_index].max) {
            unsatisfied_column_count++;
        }
    }
    while ((unused_width > 0) && (unsatisfied_column_count > 0)) {
        const uint16_t part = unused_width / unsatisfied_column_count;
        if (!part) {
            break;
        }
        unsatisfied_column_count = 0;
        for (uint16_t column_index = 0; column_index < grid.nb_columns; column_index++) {
            if (column_width[column_index] < column_width_strategies[column_index].max) {
                uint16_t ajusted_part = std::min<uint16_t>(part, column_width_strategies[column_index].max - column_width[column_index]);
                column_width[column_index] += ajusted_part;
                unused_width -= ajusted_part;

                if (column_width[column_index] < column_width_strategies[column_index].max) {
                    unsatisfied_column_count++;
                }
            }
        }
    }
}

inline
void apply_format(WidgetGrid & grid, uint16_t * row_height, uint16_t * column_width) {
    uint16_t height = 0;
    for (uint16_t row_index = 0; row_index < grid.get_nb_rows(); row_index++) {
        grid.set_row_height(row_index, row_height[row_index]);
        height += row_height[row_index] + grid.border * 2;
    }
    grid.set_cy(height);
    for (uint16_t column_index = 0; column_index < grid.nb_columns; column_index++) {
        grid.set_column_width(column_index, column_width[column_index]);
    }

}

