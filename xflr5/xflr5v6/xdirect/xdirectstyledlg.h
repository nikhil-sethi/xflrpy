/****************************************************************************

    QXDirectStyleDlg Class
    Copyright (C) 2009 André Deperrois

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
#include <QKeyEvent>
#include <QDialogButtonBox>

#include <xflcore/linestyle.h>


class LineBtn;
class OpPointWt;

class XDirectStyleDlg : public QDialog
{
    Q_OBJECT
    friend class OpPointWt;

    public:
        XDirectStyleDlg(OpPointWt *pParent=nullptr);

    private slots:
        void onButton(QAbstractButton *pButton);
        void onRestoreDefaults();
        void onNeutralStyle();
        void onBLStyle();
        void onPressureStyle();

    private:
        void keyPressEvent(QKeyEvent *pEvent) override;
        void setupLayout();



    private:
        OpPointWt *m_pOpPointWt;
        LineBtn *m_plbBL, *m_plbPressure, *m_plbNeutral;

        LineStyle m_FoilStyle;
        LineStyle m_BLStyle;
        LineStyle m_PressureStyle;
        LineStyle m_NeutralStyle;


        QDialogButtonBox *m_pButtonBox;
};

