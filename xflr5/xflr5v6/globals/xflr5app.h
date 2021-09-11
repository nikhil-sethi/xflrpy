/****************************************************************************

        XFLR5App Class

    Copyright (C) 2008 André Deperrois 
                       Francesco Meschia francesco.meschia@gmail.com

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

#include <QApplication>


class XFLR5App : public QApplication
{
    Q_OBJECT

    public:
        XFLR5App(int&, char**);
        bool done() const {return m_bDone;}

    private:
        bool event(QEvent *pEvent) override;
        void addStandardBtnStrings();
        void parseCmdLine(XFLR5App &xflapp, QString &scriptfilename, bool &bScript, bool &bShowProgress, int &OGLVersion);

        bool m_bDone;
};


