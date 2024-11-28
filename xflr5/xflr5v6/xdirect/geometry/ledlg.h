/****************************************************************************

    LEDlg Class
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
#include <QDialogButtonBox>

class Foil;
class DoubleEdit;
class XFoil;

class LEDlg : public QDialog
{
    Q_OBJECT
    friend class XDirect;
    friend class AFoil;

    public:
        LEDlg(QWidget *pParent);
        void setupLayout();
        void initDialog();

    private slots:
        void onChanged();
        void onOK();
        void onApply();
        void onButton(QAbstractButton *pButton);

    private:
        void keyPressEvent(QKeyEvent *event);

    public:
        static XFoil* s_pXFoil;

    private:
        QDialogButtonBox *m_pButtonBox;
        DoubleEdit    *m_pdeBlend, *m_pdeLE;
        bool m_bApplied, m_bModified;
        double m_LErfac, m_Blend;

        QWidget *m_pParent;

        Foil* m_pBufferFoil;
        Foil const*m_pMemFoil;
};


