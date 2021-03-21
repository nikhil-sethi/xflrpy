/****************************************************************************

    Popup Class
    Copyright (C) 2018 Andre Deperrois

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*****************************************************************************/

#pragma once

#include <QWidget>
#include <QLabel>


class Popup : public QWidget
{
    Q_OBJECT
public:
    Popup(QWidget *pParent=NULL);
    Popup(QString const &message, QWidget *pParent);
    void appendTextMessage(QString const &text);
    void setTextMessage(QString const &text);

    void setRed();
    void setGreen();

    static void setWindowPos(QPoint pt) {s_Position=pt;}

protected:
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    void mousePressEvent(QMouseEvent *event);

private:
    void setupLayout();


private:
    QLabel *m_pMessage;

    static QSize  s_WindowSize;
    static QPoint s_Position;
};

