/****************************************************************************

    InverseOptionsDlg  Classes
    Copyright (C) André Deperrois

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

#include <QDialog>

class XInverse;
class LineBtn;

class InverseOptionsDlg:public QDialog
{
    Q_OBJECT
    

    public:
        InverseOptionsDlg(QWidget *pParent);
        void initDialog();
        static void setXInverse(XInverse *pXInverse) {s_pXInverse = pXInverse;}

    private slots:
        void onRefStyle();
        void onModStyle();
        void onSplineStyle();
        void onReflectedStyle();

    private:
        void setupLayout();

        LineBtn *m_plbRefFoil, *m_plbModFoil, *m_plbSpline, *m_plbReflected;

        static XInverse * s_pXInverse;
};

