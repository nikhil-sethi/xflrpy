/****************************************************************************

    BodyTransDlg Class
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
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QLabel>

class DoubleEdit;
class IntEdit;
class BodyTransDlg : public QDialog
{
    Q_OBJECT
    friend class MainFrame;
    friend class Miarex;
    friend class Body;
    friend class BodyDlg;
    friend class EditBodyDlg;

    public:
        BodyTransDlg(QWidget *pParent);
        void initDialog();

    private slots:
        void onButton(QAbstractButton *pButton);
        void onOK();
        void onFrameOnly();

    private:
        void keyPressEvent(QKeyEvent *pEvent) override;
        void setupLayout();

        DoubleEdit *m_pdeXTransFactor;
        DoubleEdit *m_pdeYTransFactor;
        DoubleEdit *m_pdeZTransFactor;
        IntEdit *m_pieFrameID;
        QCheckBox *m_pchFrameOnly;
        QDialogButtonBox *m_pButtonBox;

        double m_XTrans, m_YTrans, m_ZTrans;
        bool   m_bFrameOnly;
        int    m_FrameID;

};

