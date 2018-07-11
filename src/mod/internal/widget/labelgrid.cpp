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

#include "mod/internal/widget/labelgrid.hpp"
#include "mod/internal/widget/label.hpp"
#include "gdi/graphic_api.hpp"
#include "utils/log.hpp"

WidgetLabelGrid::WidgetLabelGrid(
    gdi::GraphicApi & drawable, Widget & parent,
    NotifyApi * notifier, uint16_t nb_lines, uint16_t nb_columns,
    BGRColor bg_color_1, BGRColor fg_color_1,
    BGRColor bg_color_2, BGRColor fg_color_2,
    BGRColor bg_color_focus, BGRColor fg_color_focus,
    BGRColor bg_color_selection, BGRColor fg_color_selection,
    Font const & font, uint16_t border, int group_id
)
    : WidgetGrid(
        drawable, parent, notifier, nb_lines, nb_columns,
        bg_color_1, fg_color_1, bg_color_2, fg_color_2,
        bg_color_focus, fg_color_focus,
        bg_color_selection, fg_color_selection, border, group_id)
    , font(font)
{}

WidgetLabelGrid::~WidgetLabelGrid()
{
    this->clean_labels();
}

void WidgetLabelGrid::clear()
{
    this->clean_labels();
    WidgetGrid::clear();
}

void WidgetLabelGrid::clean_labels()
{
    for (int i = 0; i < this->get_nb_columns(); i++) {
        for (int j = 0; j < this->get_nb_rows(); j++) {
            Widget* w = this->remove_widget(j, i);
            delete w;
        }
    }
}

uint16_t WidgetLabelGrid::add_line(const char ** entries)
{
    uint16_t const  old_nb_row = this->get_nb_rows();

    assert(old_nb_row < GRID_NB_ROWS_MAX);

    this->set_nb_rows(old_nb_row + 1);
    for (int i = 0; i < this->get_nb_columns() && entries[i]; i++) {
        bool odd = this->get_nb_rows() & 1;
        WidgetLabel * label = new WidgetLabel(
            this->drawable, *this, this,
            entries[i], this->group_id,
            odd ? this->fg_color_1 : this->fg_color_2,
            odd ? this->bg_color_1 : this->bg_color_2,
            this->font, x_padding_label, y_padding_label
        );

        Dimension dim = label->get_optimal_dim();
        label->set_wh(dim);

        label->tool = true;
        this->set_widget(old_nb_row, i, label);
    }

    return old_nb_row;
}

const char * WidgetLabelGrid::get_cell_text(uint16_t row_index, uint16_t column_index)
{
    const char * result = "";
    Widget * w = this->get_widget(row_index, column_index);
    if (w) {
        WidgetLabel * label = static_cast<WidgetLabel*>(w);
        result = label->get_text();
    }
    return result;
}
