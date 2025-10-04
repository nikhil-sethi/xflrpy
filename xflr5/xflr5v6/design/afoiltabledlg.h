/****************************************************************************

    AFoilTableDlg Class
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
#include <QPushButton>
#include <QCheckBox>
#include <QDialogButtonBox>


class AFoilTableDlg : public QDialog
{
    Q_OBJECT

    friend class AFoil;

    public:
        AFoilTableDlg(QWidget *pParent);
        void initDialog();

    private slots:
        void onOK();


    private:
        void setupLayout();
        void keyPressEvent(QKeyEvent *pEvent);

        QDialogButtonBox *m_pButtonBox;

        QCheckBox *m_pchFoilName;
        QCheckBox *m_pchThickness;
        QCheckBox *m_pchThicknessAt;
        QCheckBox *m_pchCamber;
        QCheckBox *m_pchCamberAt;
        QCheckBox *m_pchPoints;
        QCheckBox *m_pchTEFlapAngle;
        QCheckBox *m_pchTEXHinge;
        QCheckBox *m_pchTEYHinge;
        QCheckBox *m_pchLEFlapAngle;
        QCheckBox *m_pchLEXHinge;
        QCheckBox *m_pchLEYHinge;

        bool m_bFoilName, m_bPoints;
        bool m_bThickness, m_bThicknessAt, m_bCamber, m_bCamberAt;
        bool m_bTEFlapAngle, m_bTEXHinge, m_bTEYHinge, m_bLEFlapAngle, m_bLEXHinge, m_bLEYHinge;
};

