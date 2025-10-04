/****************************************************************************

    Naca Foil Dlg
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
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>

class Foil;
class IntEdit;
class XFoil;

class NacaFoilDlg : public QDialog
{
    Q_OBJECT
    friend class XDirect;
    friend class AFoil;
    friend class XFoil;


    private slots:
        void onEditingFinished();
        void onOK();
        void onButton(QAbstractButton *pButton);

    public:
        NacaFoilDlg(QWidget *pParent);
        void setupLayout();
        void generateFoil();
        void keyPressEvent(QKeyEvent *pEvent) override;

        static XFoil *s_pXFoil;

        QWidget *m_pParent;

        Foil *m_pBufferFoil;
        QDialogButtonBox *m_pButtonBox;
        QLineEdit *m_pleNumber;
        IntEdit *m_piePanels;
        QLabel * m_plabMessage;

        bool m_bGenerated;
        static int s_Digits;
        static int s_Panels;
};

