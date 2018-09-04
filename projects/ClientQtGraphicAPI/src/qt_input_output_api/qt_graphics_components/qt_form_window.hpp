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
   Copyright (C) Wallix 2010-2013
   Author(s): Clément Moroldo, Jonathan Poelen

*/

#pragma once


#include <stdio.h>
#include <stdlib.h>


#include "utils/log.hpp"
#include "core/RDP/MonitorLayoutPDU.hpp"
#include "core/channel_list.hpp"
#include "client_redemption/client_redemption_api.hpp"



#include "../keymaps/qt_scancode_keymap.hpp"
#include "qt_options_window.hpp"

#include "client_redemption/client_input_output_api/client_mouse_keyboard_api.hpp"

#include <QtGui/QPainter>
#include <QtGui/QKeyEvent>

#if REDEMPTION_QT_VERSION == 4
#   define REDEMPTION_QT_INCLUDE_WIDGET(name) <QtGui/name>
#else
#   define REDEMPTION_QT_INCLUDE_WIDGET(name) <QtWidgets/name>
#endif

#include REDEMPTION_QT_INCLUDE_WIDGET(QApplication)
#include REDEMPTION_QT_INCLUDE_WIDGET(QDesktopWidget)
#include REDEMPTION_QT_INCLUDE_WIDGET(QCheckBox)
#include REDEMPTION_QT_INCLUDE_WIDGET(QComboBox)
#include REDEMPTION_QT_INCLUDE_WIDGET(QPushButton)
#include REDEMPTION_QT_INCLUDE_WIDGET(QLabel)
#include REDEMPTION_QT_INCLUDE_WIDGET(QFormLayout)
#include REDEMPTION_QT_INCLUDE_WIDGET(QLineEdit)
#include REDEMPTION_QT_INCLUDE_WIDGET(QFileDialog)
#include REDEMPTION_QT_INCLUDE_WIDGET(QComboBox)
#include REDEMPTION_QT_INCLUDE_WIDGET(QDialog)
#include REDEMPTION_QT_INCLUDE_WIDGET(QGridLayout)
#include REDEMPTION_QT_INCLUDE_WIDGET(QTabWidget)
#include REDEMPTION_QT_INCLUDE_WIDGET(QTableWidget)
#include REDEMPTION_QT_INCLUDE_WIDGET(QToolTip)
#include REDEMPTION_QT_INCLUDE_WIDGET(QWidget)
#include REDEMPTION_QT_INCLUDE_WIDGET(QScrollArea)

#undef REDEMPTION_QT_INCLUDE_WIDGET



#include <vector>


class IconMovie :  public QWidget
{
REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Winconsistent-missing-override")
Q_OBJECT
REDEMPTION_DIAGNOSTIC_POP

public:
    ClientRedemptionConfig * config;
    ClientInputMouseKeyboardAPI * controllers;

    int _width;
    int _height;

    QPixmap pixmap;
    QRect drop_rect;

    std::string name;
    const std::string path;
    const std::string version;
    const std::string reso;
    const std::string checksum;

    const long int movie_len;


    IconMovie(ClientRedemptionConfig * config, ClientInputMouseKeyboardAPI * controllers
      , const std::string & name
      , const std::string & path
      , const std::string & version,
        const std::string & reso,
        const std::string & checksum,
        const long int movie_len,
        QWidget * parent)
      : QWidget(parent)
      , config(config)
      , controllers(controllers)
      , _width(385)
      , _height(60)
      , pixmap(this->_width, this->_height)
      , drop_rect(24, 95, 240, 160)
      , name(name)
      , path(path)
      , version(version)
      , reso(reso)
      , checksum(checksum)
      , movie_len(movie_len)
    {
        this->setFixedSize(this->_width, this->_height);
    }

    void draw_account() {
        QPen                 pen;
        QPainter             painter(&(this->pixmap));
        painter.setRenderHint(QPainter::Antialiasing);
        pen.setWidth(1);
        pen.setBrush(Qt::black);
        painter.setPen(pen);
        painter.fillRect(0, 0, this->width(), this->height(), Qt::white);
        painter.fillRect(0, 0, this->width(), this->height(), Qt::transparent);
        painter.drawRoundedRect(2, 2, this->height()-5, this->height()-5, 4, 4);

        QFont font = painter.font();
        font.setPixelSize(10);
        painter.setFont(font);

        if (this->name.length() > 40) {
            this->name = this->name.substr(0, 39)+"...";
        }

        QString qname(this->name.c_str());
        QString qversion(this->version.c_str());
        std::string line(this->checksum+"   "+this->reso);
        QString qchecksum(line.c_str());

        painter.drawText(QPoint(this->height()+6, 15), qname);
        painter.drawText(QPoint(this->height()+6, 25), qversion);
        painter.drawText(QPoint(this->height()+6, 35), qchecksum);
        painter.drawText(QPoint(this->height()+6, 45), toQStringData(this->movie_len));

        pen.setBrush(QColor(0xFF, 0x8C, 0x00));
        painter.setPen(pen);
        painter.drawRoundedRect(0, 0, this->width()-20, this->height()-1, 4, 4);

        painter.end();

        this->repaint();
    }

    QString toQStringData(long int time) {
        long int s = time;
        int h = s/3600;
        s = s % 3600;
        int m = s/60;
        s = s % 60;
        QString date_str;
        if (h) {
            date_str += QString(std::to_string(h).c_str()) + QString(":");
        }
        if (m < 10) {
            date_str += QString("0");
        }
        date_str += QString(std::to_string(m).c_str()) + QString(":");
        if (s < 10) {
            date_str += QString("0");
        }
        date_str += QString(std::to_string(s).c_str());

        return date_str;
    }

    void enterEvent(QEvent *ev) override {
        Q_UNUSED(ev);
        QPen                 pen;
        QPainter             painter(&(this->pixmap));
        painter.setRenderHint(QPainter::Antialiasing);
        pen.setWidth(1);
        pen.setBrush(QColor(0xFF, 0x8C, 0x00));
        painter.setPen(pen);
        painter.drawRoundedRect(0, 0, this->width()-20, this->height()-1, 4, 4);
        painter.end();
        this->repaint();
    }

    void leaveEvent(QEvent *ev) override {
        Q_UNUSED(ev);
        QPen                 pen;
        QPainter             painter(&(this->pixmap));
        painter.setRenderHint(QPainter::Antialiasing);
        pen.setWidth(1);
        pen.setBrush(Qt::white);
        painter.setPen(pen);
        painter.drawRoundedRect(0, 0, this->width()-20, this->height()-1, 4, 4);
        painter.end();
        this->repaint();
    }

    void mouseDoubleClickEvent( QMouseEvent * e ) override {
        if ( e->button() == Qt::LeftButton )
        {
            auto const last_delimiter_it = std::find(path.rbegin(), path.rend(), '/');
            int pos = path.size() - (last_delimiter_it - path.rbegin());

            std::string const movie_name = (last_delimiter_it == path.rend())
            ? path
            : path.substr(path.size() - (last_delimiter_it - path.rbegin()));

            std::string const movie_dir = path.substr(0, pos);

            this->config->mod_state = ClientRedemptionAPI::MOD_RDP_REPLAY;
            this->controllers->client->replay(movie_name, movie_dir);
        }
    }

    void paintEvent(QPaintEvent * event) override {
        Q_UNUSED(event);
        QPen                 pen;
        QPainter             painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        pen.setWidth(1);
        pen.setBrush(Qt::black);
        painter.setPen(pen);

        painter.drawPixmap(QPoint(0, 0), this->pixmap, QRect(0, 0, this->width(), this->height()));

        painter.end();
    }

};


class QtMoviesPanel : public QWidget
{

REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Winconsistent-missing-override")
Q_OBJECT
REDEMPTION_DIAGNOSTIC_POP

public:
    ClientRedemptionConfig * config;
    ClientInputMouseKeyboardAPI * controllers;
    std::vector<IconMovie *> icons;
    QFormLayout lay;


    QtMoviesPanel(ClientRedemptionConfig * config, ClientInputMouseKeyboardAPI * controllers, QWidget * parent)
      : QWidget(parent)
      , config(config)
      , controllers(controllers)
      , lay(this)
    {
        this->setMinimumSize(395, 250);
        this->setMaximumWidth(395);

        std::vector<ClientRedemptionAPI::IconMovieData> iconData = this->controllers->client->get_icon_movie_data();

        for (size_t i = 0; i < iconData.size(); i++) {
            IconMovie* icon = new IconMovie(this->config, controllers, iconData[i].file_name, iconData[i].file_path, iconData[i].file_version, iconData[i].file_resolution, iconData[i].file_checksum, iconData[i].movie_len, this);
            this->icons.push_back(icon);
            icon->draw_account();
            this->lay.addRow(icon);
        }

        this->setLayout(&(this->lay));
    }

    void paintEvent(QPaintEvent * event) override {
        Q_UNUSED(event);
        QPen                 pen;
        QPainter             painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        pen.setWidth(1);
        pen.setBrush(Qt::black);
        painter.setPen(pen);
        painter.fillRect(0, 0, this->width(), this->height(), Qt::white);
        painter.end();
    }

};



class QtFormReplay : public QWidget
{
REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Winconsistent-missing-override")
Q_OBJECT
REDEMPTION_DIAGNOSTIC_POP

public:
    ClientRedemptionConfig * config;
    ClientInputMouseKeyboardAPI * controllers;

    QFormLayout lay;
    QPushButton buttonReplay;

    QScrollArea scroller;
    QtMoviesPanel movie_panel;


    QtFormReplay(ClientRedemptionConfig * config, ClientInputMouseKeyboardAPI * controllers, QWidget * parent)
    : QWidget(parent)
    , config(config)
    , controllers(controllers)
    , lay(this)
    , buttonReplay("Select a mwrm file", this)
    , scroller(this)
    , movie_panel(config, controllers, this)
    {
        this->scroller.setFixedSize(410,  250);
        this->scroller.setStyleSheet("background-color: #C4C4C3; border: 1px solid #FFFFFF;"
            "border-bottom-color: #FF8C00;");
        this->scroller.setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
        this->scroller.setWidget(&(this->movie_panel));
        this->lay.addRow(&(this->scroller));

        this->buttonReplay.setToolTip(this->buttonReplay.text());
        this->buttonReplay.setFixedSize(220, 24);
        this->buttonReplay.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->buttonReplay)   , SIGNAL (pressed()),  this, SLOT (replayPressed()));
        this->buttonReplay.setFocusPolicy(Qt::StrongFocus);
        this->buttonReplay.setAutoDefault(true);
        this->lay.addRow(&(this->buttonReplay));
    }

private Q_SLOTS:
    void replayPressed() {
        QString filePath("");
        filePath = QFileDialog::getOpenFileName(this, tr("Open a Movie"),
                                                this->controllers->client->REPLAY_DIR.c_str(),
                                                tr("Movie Files(*.mwrm)"));
        std::string str_movie_path(filePath.toStdString());

        auto const last_delimiter_it = std::find(str_movie_path.rbegin(), str_movie_path.rend(), '/');
        int pos = str_movie_path.size() - (last_delimiter_it - str_movie_path.rbegin());

        std::string const movie_name = (last_delimiter_it == str_movie_path.rend())
        ? str_movie_path
        : str_movie_path.substr(str_movie_path.size() - (last_delimiter_it - str_movie_path.rbegin()));

        std::string const movie_dir = str_movie_path.substr(0, pos);

        this->config->mod_state = ClientRedemptionAPI::MOD_RDP_REPLAY;
        this->controllers->client->replay(movie_name, movie_dir);
    }

};






class FormTabAPI : public QWidget
{
public:
     int account_index_to_drop;

    FormTabAPI(QWidget * parent)
      : QWidget(parent)
      , account_index_to_drop(-1)
      {}

    virtual void targetPicked(int ) {}
    virtual void drop_account() {}
    virtual void check_password_box() {}
    virtual void delete_account(int index) { (void)index; }
    virtual ~FormTabAPI() = default;
};



class ConnectionFormQt  : public QWidget
{

REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Winconsistent-missing-override")
Q_OBJECT
REDEMPTION_DIAGNOSTIC_POP

public:
    uint8_t protocol_type;

    QFormLayout line_edit_layout;

    FormTabAPI * main_panel;

    QComboBox  _IPCombobox;

    QLineEdit _IPField;
    QLineEdit _userNameField;
    QLineEdit _PWDField;
    QLineEdit _portField;

    QLabel    _IPLabel;
    QLabel    _userNameLabel;
    QLabel    _PWDLabel;
    QLabel    _portLabel;


    ConnectionFormQt(FormTabAPI * main_panel, uint8_t protocol_type, QWidget * parent)
      : QWidget(parent)
      , protocol_type(protocol_type)
      , line_edit_layout(this)
      , main_panel(main_panel)
      , _IPCombobox(this)
      , _IPField("", this)
      , _userNameField("", this)
      , _PWDField("", this)
      , _portField((protocol_type == ClientRedemptionAPI::MOD_RDP) ? "3389" : "5900", this)
      , _IPLabel(      QString("IP server :"), this)
      , _userNameLabel(QString("User name : "), this)
      , _PWDLabel(     QString("Password :  "), this)
      , _portLabel(    QString("Port :      "), this)
    {
        this->setFixedSize(240, 160);
        this->_IPCombobox.setFixedWidth(140);
        this->_IPCombobox.setLineEdit(&(this->_IPField));
        this->QObject::connect(&(this->_IPCombobox), SIGNAL(currentIndexChanged(int)) , this, SLOT(targetPicked(int)));

        this->setLayout(&(this->line_edit_layout));

        this->_PWDField.setEchoMode(QLineEdit::Password);
        this->_PWDField.setInputMethodHints(Qt::ImhHiddenText | Qt::ImhNoPredictiveText | Qt::ImhNoAutoUppercase);
        this->_userNameField.setFixedWidth(140);
        this->_PWDField.setFixedWidth(140);
        this->_portField.setFixedWidth(140);

        this->line_edit_layout.addRow(&(this->_IPLabel)      , &(this->_IPCombobox));
        this->line_edit_layout.addRow(&(this->_userNameLabel), &(this->_userNameField));
        this->line_edit_layout.addRow(&(this->_PWDLabel)     , &(this->_PWDField));
        this->line_edit_layout.addRow(&(this->_portLabel)    , &(this->_portField));

        if (this->protocol_type == ClientRedemptionAPI::MOD_VNC) {
            this->_userNameField.hide();
            this->_userNameLabel.hide();
            this->_portField.setText("5900");
        } else  {
            this->_portField.setText("3389");
        }
    }

private Q_SLOTS:
    void targetPicked(int index) {
        if (this->main_panel) {
            this->main_panel->targetPicked(this->_IPCombobox.itemData(index).toInt());
        }
    }
};



class QtIconAccount :  public QWidget
{

REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Winconsistent-missing-override")
Q_OBJECT
REDEMPTION_DIAGNOSTIC_POP

public:
    FormTabAPI * main_tab;
    const ClientRedemptionConfig::AccountData accountData;
    QPixmap pixmap;
    QRect drop_rect;



    QtIconAccount(FormTabAPI * main_tab, const ClientRedemptionConfig::AccountData & accountData, QWidget * parent)
      : QWidget(parent)
      , main_tab(main_tab)
      , accountData(accountData)
      , pixmap(137, 50)
      , drop_rect(24, 95, 240, 160)
    {
        this->setFixedSize(137, 50);
    }

    void draw_account() {
        QPen                 pen;
        QPainter             painter(&(this->pixmap));
        painter.setRenderHint(QPainter::Antialiasing);
        pen.setWidth(1);
        pen.setBrush(Qt::black);
        painter.setPen(pen);
        painter.fillRect(0, 0, this->width(), this->height(), Qt::white);
        painter.fillRect(0, 0, this->width(), this->height(), Qt::transparent);
        painter.drawRoundedRect(2, 2, this->height()-5, this->height()-5, 4, 4);

        QFont font = painter.font();
        font.setPixelSize(10);
        painter.setFont(font);

        QString qip(this->accountData.IP.c_str());
        QString qname(this->accountData.name.c_str());

        painter.drawText(QPoint(this->height()+6, 20), qip);
        painter.drawText(QPoint(this->height()+6, 30), qname);

        QString qcross("x");
        painter.drawText(QPoint(128, 8), qcross);
        painter.drawRect(128, 3, 6, 6);

        pen.setBrush(QColor(0xFF, 0x8C, 0x00));
        painter.setPen(pen);
        painter.drawRoundedRect(0, 0, this->width()-1, this->height()-1, 4, 4);
        pen.setBrush(Qt::white);
        painter.setPen(pen);
        painter.drawRoundedRect(0, 0, this->width()-1, this->height()-1, 4, 4);

        painter.end();

        this->repaint();
    }

    void enterEvent(QEvent *ev) override {
        Q_UNUSED(ev);
        QPen                 pen;
        QPainter             painter(&(this->pixmap));
        painter.setRenderHint(QPainter::Antialiasing);
        pen.setWidth(1);
        pen.setBrush(QColor(0xFF, 0x8C, 0x00));
        painter.setPen(pen);
        painter.drawRoundedRect(0, 0, this->width()-1, this->height()-1, 4, 4);
        painter.end();
        this->repaint();
    }

    void leaveEvent(QEvent *ev) override {
        Q_UNUSED(ev);
        QPen                 pen;
        QPainter             painter(&(this->pixmap));
        painter.setRenderHint(QPainter::Antialiasing);
        pen.setWidth(1);
        pen.setBrush(Qt::white);
        painter.setPen(pen);
        painter.drawRoundedRect(0, 0, this->width()-1, this->height()-1, 4, 4);
        painter.end();
        this->repaint();
    }

    void mousePressEvent(QMouseEvent *e) override {

        switch (e->button()) {
            case Qt::LeftButton:
                if (this->main_tab) {

                    if (e->x() > 127 && e->x() < 135 && e->y() > 2 && e->y() < 9) {
                        this->main_tab->delete_account(this->accountData.index);
                    } else {

                        this->main_tab->account_index_to_drop = this->accountData.index;
                        QImage image(this->pixmap.toImage().convertToFormat(QImage::Format_ARGB32));
                        QPixmap map = QPixmap::fromImage(image);

                        QCursor qcursor(map, 10, 10);

                        this->main_tab->setCursor(qcursor);
                    }
                }
                break;

            default:
                break;
        }
    }

    void mouseReleaseEvent(QMouseEvent *e) override {

        switch (e->button()) {
            case Qt::LeftButton:
                if (this->main_tab) {
                    if (this->drop_rect.contains(QPoint(e->globalX() - this->main_tab->nativeParentWidget()->x(), e->globalY() - this->main_tab->nativeParentWidget()->y()))) {
                        this->main_tab->drop_account();
                    }
                    this->main_tab->account_index_to_drop = -1;
                    this->main_tab->setCursor(Qt::ArrowCursor);
                }
                break;

            default:
                break;
        }
    }

    void mouseDoubleClickEvent( QMouseEvent * e ) override {
        switch (e->button()) {
            case Qt::LeftButton:
                if (this->main_tab) {
                    this->main_tab->account_index_to_drop = this->accountData.index+1;

                    LOG(LOG_INFO, "this->main_tab->account_index_to_drop = %d", this->main_tab->account_index_to_drop);
                    this->main_tab->drop_account();
                    this->main_tab->account_index_to_drop = -1;
                    this->main_tab->setCursor(Qt::ArrowCursor);
                }
                break;

            default:
                break;
        }
    }

    void paintEvent(QPaintEvent * event) override {
        Q_UNUSED(event);
        QPen                 pen;
        QPainter             painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        pen.setWidth(1);
        pen.setBrush(Qt::black);
        painter.setPen(pen);

        painter.drawPixmap(QPoint(0, 0), this->pixmap, QRect(0, 0, this->width(), this->height()));

        painter.end();
    }

};



class QtAccountPanel : public QWidget
{

REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Winconsistent-missing-override")
Q_OBJECT
REDEMPTION_DIAGNOSTIC_POP

public:
    QtIconAccount * icons[15];
    QFormLayout lay;
//     const std::vector<ClientRedemptionConfig::AccountData>  accountData;
//     const int nb_account;


    QtAccountPanel(FormTabAPI * main_tab, ClientRedemptionConfig * config,  QWidget * parent, int protocol_type)
      : QWidget(parent)
      , lay(this)
//       , accountData(accountData)
//       , nb_account(nb_account < 15 ?  nb_account : 15)
    {
        this->setAttribute(Qt::WA_DeleteOnClose);
        this->setMinimumHeight(160);

        size_t i = 0;
        for (auto const& accountData : config->_accountDatas) {
            if (accountData.protocol ==  protocol_type) {
                this->icons[i] = new QtIconAccount(main_tab, accountData, this);
                LOG(LOG_INFO, "elem account %zu title=%s index=%d", i, accountData.title, accountData.index);
                this->icons[i]->draw_account();
                this->lay.addRow(this->icons[i]);
            }
            if (++i > std::size(this->icons)) {
                break;
            }
        }

        this->setLayout(&(this->lay));
    }

    void paintEvent(QPaintEvent * event) override {
        Q_UNUSED(event);
        QPen                 pen;
        QPainter             painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        pen.setWidth(1);
        pen.setBrush(Qt::black);
        painter.setPen(pen);
        painter.fillRect(0, 0, this->width(), this->height(), Qt::white);
        painter.end();
    }

};



class QtFormAccountConnectionPanel : public QWidget
{

public:
    ClientRedemptionConfig * config;
    ClientRedemptionAPI       * _front;
    ClientInputMouseKeyboardAPI * controllers;
    FormTabAPI * main_panel;
    ConnectionFormQt     line_edit_panel;
    QtAccountPanel  *   account_panel;
    QScrollArea scroller;

    int protocol_type;


    QtFormAccountConnectionPanel(ClientRedemptionConfig * config, ClientInputMouseKeyboardAPI * controllers, FormTabAPI * main_panel,  uint8_t protocol_type, ClientRedemptionAPI * front)
      : QWidget(main_panel)
      , config(config)
      , _front(front)
      , controllers(controllers)
      , main_panel(main_panel)
      , line_edit_panel(main_panel, protocol_type, this)
      , scroller(this)
      , protocol_type(protocol_type)
    {

//         this->scroller.setFixedSize(170,  160);
//         this->scroller.setStyleSheet("/*background-color: #FFFFFF;*/ border: 1px solid #FFFFFF;"
//         "border-bottom-color: #FF8C00;");
//         this->scroller.setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );

        this->set_account_panel();
//         this->account_panel = new QtAccountPanel(main_panel, this->config, this->config->_accountNB, this, protocol_type);




        this->setFixedSize(this->scroller.width() + this->line_edit_panel.width(), this->scroller.height());

        this->line_edit_panel.setGeometry(QRect(0, 0, this->line_edit_panel.width(), this->line_edit_panel.height() ));

        this->scroller.setGeometry(QRect(this->line_edit_panel.width()+1, 0, this->scroller.width(), this->scroller.height() ));
    }

    void set_account_panel() {
//         if (this->account_panel) {
//             this->account_panel->close();
//         }

        this->scroller.setFixedSize(170,  160);
        this->scroller.setStyleSheet("/*background-color: #FFFFFF;*/ border: 1px solid #FFFFFF;"
        "border-bottom-color: #FF8C00;");
        this->scroller.setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
        this->account_panel = new QtAccountPanel(this->main_panel, this->config, this, this->protocol_type);
        this->scroller.setWidget(this->account_panel);

        this->setAccountData();
    }

    void setAccountData() {
        if (this->_front) {
            //TODO
            //this->_front->setAccountData();

            if (this->config->_save_password_account) {
                this->main_panel->check_password_box();
            }

            this->line_edit_panel._IPCombobox.clear();
            this->line_edit_panel._IPCombobox.addItem(QString(""), 0);

            QStringList stringList;

            size_t i = 0;
            for (auto const& accountData : config->_accountDatas) {
                if (accountData.protocol == this->protocol_type) {
                    this->line_edit_panel._IPCombobox.addItem(QString(accountData.title.c_str()), int(++i));
                    stringList << accountData.title.c_str();
                }
            }
            //this->_completer = new QCompleter(stringList, this);
        } else {
            LOG(LOG_INFO, "can't open login config file");
        }
    }

};



class QtFormTab : public FormTabAPI
{

REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Winconsistent-missing-override")
Q_OBJECT
REDEMPTION_DIAGNOSTIC_POP


public:
    ClientRedemptionConfig * config;
    uint8_t protocol_type;
    ClientRedemptionAPI         * _front;
    ClientInputMouseKeyboardAPI * controllers;
    const int            _width;
    const int            _height;

    QGridLayout         grid_layout;


    QLabel               _errorLabel;
    QCheckBox            _pwdCheckBox;

    QtFormAccountConnectionPanel formAccountConnectionPanel;

    QPushButton          _buttonConnexion;
    QPushButton          _buttonOptions;

    QtOptions * options;


    QtFormTab(ClientRedemptionConfig * config, ClientInputMouseKeyboardAPI * controllers, ClientRedemptionAPI  * front, uint8_t protocol_type, QWidget * parent)
        : FormTabAPI(parent)
        , config(config)
        , protocol_type(protocol_type)
        , _front(front)
        , controllers(controllers)
        , _width(400)
        , _height(600)
        , grid_layout(this)
        , _errorLabel(   QString(""            ), this)
        , _pwdCheckBox(QString("Save password."), this)
        , formAccountConnectionPanel(config, this->controllers, this, this->protocol_type, front)
        , _buttonConnexion("Connection", this)
        , _buttonOptions("Options", this)
    {
        if (protocol_type & ClientRedemptionAPI::MOD_RDP) {
            this->options = new QtRDPOptions(config, front, this->controllers, this);
        } else {
            this->options = new QtVNCOptions(config, front, this->controllers, this);
        }
//         this->setMinimumHeight(360);
//         this->setAccountData();

        this->grid_layout.addWidget(&(this->formAccountConnectionPanel), 0, 0, 1, 3);

        this->grid_layout.addWidget(&(this->_pwdCheckBox), 1, 0, 1, 2);
        this->_errorLabel.setFixedHeight(this->_errorLabel.height()+10);
        this->grid_layout.addWidget(&(this->_errorLabel), 2, 0, 1, 4);

        this->_buttonConnexion.setToolTip(this->_buttonConnexion.text());
        this->_buttonConnexion.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonConnexion)   , SIGNAL (released()), this, SLOT (connexionReleased()));
        this->_buttonConnexion.setFocusPolicy(Qt::StrongFocus);
        this->_buttonConnexion.setAutoDefault(true);
        this->grid_layout.addWidget(&(this->_buttonConnexion), 3, 2, 1, 2);

        this->_buttonOptions.setToolTip(this->_buttonOptions.text());
        this->_buttonOptions.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonOptions)     , SIGNAL (released()), this, SLOT (optionsReleased()));
        this->_buttonOptions.setFocusPolicy(Qt::StrongFocus);
        this->grid_layout.addWidget(&(this->_buttonOptions), 3, 0, 1, 1);

        this->grid_layout.addWidget(this->options, 4, 0, 3, 3);
        this->options->hide();

        this->setLayout(&(this->grid_layout));
    }


    void check_password_box() override {
        this->_pwdCheckBox.setCheckState(Qt::Checked);
    }

    void set_ErrorMsg(std::string str) {
        this->_errorLabel.clear();
        this->_errorLabel.setText(QString(str.c_str()));
    }

    void set_IPField(std::string str) {
        this->formAccountConnectionPanel.line_edit_panel._IPField.clear();
        this->formAccountConnectionPanel.line_edit_panel._IPField.insert(QString(str.c_str()));
    }

    void set_userNameField(std::string str) {
        this->formAccountConnectionPanel.line_edit_panel._userNameField.clear();
        this->formAccountConnectionPanel.line_edit_panel._userNameField.insert(QString(str.c_str()));
    }

    void set_PWDField(std::string str) {
        this->formAccountConnectionPanel.line_edit_panel._PWDField.clear();
        this->formAccountConnectionPanel.line_edit_panel._PWDField.insert(QString(str.c_str()));
    }

    void set_portField(int str) {
        this->formAccountConnectionPanel.line_edit_panel._portField.clear();
        if (str == 0) {
            this->formAccountConnectionPanel.line_edit_panel._portField.insert(QString(""));
        } else {
            this->formAccountConnectionPanel.line_edit_panel._portField.insert(QString(std::to_string(str).c_str()));
        }
    }

    std::string get_IPField() {
        std::string delimiter(" - ");
        std::string ip_field_content = this->formAccountConnectionPanel.line_edit_panel._IPField.text().toStdString();
        auto pos(ip_field_content.find(delimiter));
        std::string IP  = ip_field_content.substr(0, pos);
        return IP;
    }

    std::string get_userNameField() {
        if (this->formAccountConnectionPanel.line_edit_panel._userNameField.text().toStdString() !=  std::string(""))
            return this->formAccountConnectionPanel.line_edit_panel._userNameField.text().toStdString();

        return std::string(" ");
    }

    std::string get_PWDField() {
        if (this->formAccountConnectionPanel.line_edit_panel._PWDField.text().toStdString() !=  std::string(""))
            return this->formAccountConnectionPanel.line_edit_panel._PWDField.text().toStdString();

        return std::string(" ");
    }

    int get_portField() {
        return this->formAccountConnectionPanel.line_edit_panel._portField.text().toInt();
    }

    void keyPressEvent(QKeyEvent *e) override {
        if (e->key() == Qt::Key_Enter) {
            this->connexionReleased();
        }
    }

    void drop_account() override {
        if (this->account_index_to_drop !=  -1) {
            this->targetPicked(this->account_index_to_drop);
            this->account_index_to_drop = -1;
        }
    }

    void targetPicked(int index) override {
        if (index < 0) {
            return;
        }
        if (index >=  16 ) {
             this->connexionReleased();
             return;
        }
        if (index == 0) {
            this->formAccountConnectionPanel.line_edit_panel._IPField.clear();
            this->formAccountConnectionPanel.line_edit_panel._userNameField.clear();
            this->formAccountConnectionPanel.line_edit_panel._PWDField.clear();
            this->formAccountConnectionPanel.line_edit_panel._portField.clear();

        } else {
            index--;
            this->set_IPField(this->config->_accountDatas[index].IP);
            this->set_userNameField(this->config->_accountDatas[index].name);
            this->set_PWDField(this->config->_accountDatas[index].pwd);
            this->set_portField(this->config->_accountDatas[index].port);

            this->config->current_user_profil = this->config->_accountDatas[index].options_profil;
        }
    }

    void delete_account(int index) override {
//         this->config->_accountDatas.size();
        LOG(LOG_INFO, "this->config->_accountDatas.size() = %zu", this->config->_accountDatas.size());
        this->config->_accountDatas.erase(this->config->_accountDatas.begin()+index);
        for (size_t i = 0; i < this->config->_accountDatas.size(); i++) {
            this->config->_accountDatas[i].index = i;
        }
        LOG(LOG_INFO, "this->config->_accountDatas.size() = %zu", this->config->_accountDatas.size());
        this->formAccountConnectionPanel.set_account_panel();
       // this->show();
    }

private Q_SLOTS:
    void connexionReleased() {

        if (! (this->protocol_type == ClientRedemptionAPI::MOD_RDP && this->config->mod_state == ClientRedemptionAPI::MOD_RDP_REMOTE_APP) ){
            this->config->mod_state = this->protocol_type;
        }

        QPoint points = this->mapToGlobal({0, 0});
        this->config->windowsData.form_x = points.x()-14;
        this->config->windowsData.form_y = points.y()-85;
        this->controllers->client->writeWindowsData();

        this->options->getConfigValues();

//         this->_front->rdp_width = 1920;
//         this->_front->rdp_height = 1080;

        this->_front->writeCustomKeyConfig();
        this->_front->writeClientInfo();

        if (this->controllers->connexionReleased()) {
            this->_front->writeAccoundData(
                this->get_IPField(),
                this->get_userNameField(),
                this->get_PWDField(),
                this->get_portField()
            );
        }

        if (this->_pwdCheckBox.isChecked()) {
            this->config->_save_password_account = true;
        } else {
            this->config->_save_password_account = false;
        }
    }

    void optionsReleased() {
        this->controllers->open_options();
    }
};



class QtForm : public QWidget
{

REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Winconsistent-missing-override")
Q_OBJECT
REDEMPTION_DIAGNOSTIC_POP

public:
    ClientRedemptionConfig * config;
    ClientInputMouseKeyboardAPI * controllers;
    const int _width;
    const int _height;

    const int _long_height;

    QFormLayout main_layout;
    QTabWidget  tabs;

    QtFormTab  RDP_tab;
    QtFormTab  VNC_tab;
    QtFormReplay       replay_tab;

    bool is_option_open;
    bool is_closing;



    QtForm(ClientRedemptionConfig * config, ClientInputMouseKeyboardAPI * controllers, ClientRedemptionAPI  * front)
        : QWidget()
        , config(config)
        , controllers(controllers)
        , _width(460)
        , _height(375)
        , _long_height(690)
        , main_layout(this)
        , tabs(this)
        , RDP_tab(config, controllers, front, ClientRedemptionAPI::MOD_RDP, this)
        , VNC_tab(config, controllers, front, ClientRedemptionAPI::MOD_VNC, this)
        , replay_tab(config, controllers, this)
        , is_option_open(false)
        , is_closing(false)
    {
        this->setWindowTitle("ReDemPtion Client");
        this->setAttribute(Qt::WA_DeleteOnClose);
        this->setFixedSize(this->_width, this->_height);

        QString qss =
            "QTabBar::tab:selected {"
            "    background: #FFFFFF; "
            "    border: 1px solid #C4C4C3;"
            "    border-top: 3px solid #FF8C00; "
            "    border-bottom-color: #FFFFFF; "
            "    border-top-left-radius: 4px;"
            "    border-top-right-radius: 4px;"
//             "    min-width: 8ex;"
            "    padding: 2px;"
            "}"
//             "QTabWidget::pane { /* The tab widget frame */"
//             "    border: 1px solid #C4C4C3;"
//             "    border-top-color: #FFFFFF; "
//             "    background: #C4F4C3; "
//             "}"
        ;

        this->tabs.setStyleSheet( this->tabs.styleSheet().append(qss));
        this->QObject::connect(&(this->tabs)   , SIGNAL(currentChanged(int)), this, SLOT (tab_changed(int)));

        const QString RDP_title("  RDP  ");
        this->tabs.addTab(&(this->RDP_tab), RDP_title);

        const QString VNC_title("  VNC  ");
        this->tabs.addTab(&(this->VNC_tab), VNC_title);

        const QString replay_title(" Replay ");
        this->tabs.addTab(&(this->replay_tab), replay_title);

        this->main_layout.addRow(&(this->tabs));
        this->setLayout(&(this->main_layout));
    }

    ~QtForm() {
        QPoint points = this->mapToGlobal({0, 0});
        this->config->windowsData.form_x = points.x()-1;
        this->config->windowsData.form_y = points.y()-39;
        this->controllers->client->writeWindowsData();
        this->is_closing = true;

        if (this->is_option_open) {
            this->options();
        }
    }


    QtFormTab * get_current_tab() {
        switch (this->tabs.currentIndex()) {
            case 0: return &(this->RDP_tab);
            case 1: return &(this->VNC_tab);
            default: return nullptr;
        }
    }


    void set_ErrorMsg(std::string str) {

        this->get_current_tab()->set_ErrorMsg(str);
    }

    void set_IPField(std::string str) {
         this->get_current_tab()->set_IPField(str);
    }

    void set_userNameField(std::string str) {
        this->get_current_tab()->set_userNameField(str);
    }

    void set_PWDField(std::string str) {
        this->get_current_tab()->set_PWDField(str);
    }

    void set_portField(int str) {
        this->get_current_tab()->set_portField(str);
    }

    std::string get_IPField() {
        return this->get_current_tab()->get_IPField();
    }

    std::string get_userNameField() {
        return this->get_current_tab()->get_userNameField();
    }

    std::string get_PWDField() {
        return this->get_current_tab()->get_PWDField();
    }

    int get_portField() {
        return this->get_current_tab()->get_portField();
    }

    void init_form() {
        if (this->controllers->client->is_no_win_data()) {
            QDesktopWidget* desktop = QApplication::desktop();
            this->config->windowsData.form_x = (desktop->width()/2)  - (this->_width/2);
            this->config->windowsData.form_y = (desktop->height()/2) - (this->_height/2);
        }
        this->move(this->config->windowsData.form_x, this->config->windowsData.form_y);
    }

    void options() {
        if (this->is_option_open) {
            this->RDP_tab.options->hide();
            this->RDP_tab._buttonOptions.setText("Options v");
            this->VNC_tab.options->hide();
            this->VNC_tab._buttonOptions.setText("Options v");
            this->setFixedHeight(this->_height);
            this->is_option_open = false;
        } else {
            this->setFixedHeight(this->_long_height);
            this->RDP_tab.options->show();
            this->RDP_tab._buttonOptions.setText("Options ^");
            this->VNC_tab.options->show();
            this->VNC_tab._buttonOptions.setText("Options ^");
            this->is_option_open = true;
        }
    }

    void call_add_row() {

    }

private Q_SLOTS:
    void tab_changed(int) {
        if (this->is_option_open) {
            this->options();
        }
    }

};
