/****************************************************************************

    TwoDWidget Class
    Copyright (C) 2009-2016 André Deperrois

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

/**
*@file
*@brief This file contains the declaration of the class TwoDWidget,
*used for 2d drawing in the central area of the application's MainFrame.
*/
#ifndef INVERSEVIEWDWIDGET_H
#define INVERSEVIEWDWIDGET_H

#include <QWidget>


class XInverse;
class MainFrame;

/**
*@class inverseviewwt
* @brief This class is used for 2d drawing in the central area of the application's MainFrame.

* There is a unique instance of this class, attached to the QStackedWidget of the MainFrame.
* Depending on the active application, this class calls the drawings methods in QAFoil, QXDirect, QXInverse or QMiarex.
* All Qt events received by this widget are sent to the child applications for handling.
*/
class inverseviewwt : public QWidget
{
    Q_OBJECT

    friend class MainFrame;


public:
    inverseviewwt(QWidget *parent = nullptr);

protected:
    void contextMenuEvent (QContextMenuEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mouseDoubleClickEvent (QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent (QResizeEvent *event) override;
    void wheelEvent (QWheelEvent *pEvent) override;

private:
    MainFrame *m_pMainFrame;   /**< A void pointer to the instance of the MainFrame object. */
    XInverse *m_pXInverse;    /**< A void pointer to the instance of the QXInverse object. */

};

#endif
