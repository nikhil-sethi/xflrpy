/****************************************************************************

    STLExportDialog
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
#include <QCheckBox>
#include <QRadioButton>
#include <QLabel>
#include <QSettings>
#include <QDialogButtonBox>

class Plane;
class IntEdit;

class STLExportDlg : public QDialog
{
    Q_OBJECT
    public:
        STLExportDlg();
        void initDialog(Plane *pPlane);

        void accept();

        static bool loadSettings(QSettings &settings);
        static bool saveSettings(QSettings &settings);

        static bool s_bBinary;
        static int s_NChordPanels;
        static int s_NSpanPanels;
        static int s_iObject;

    private:
        void setupLayout();
        void connectSignals();
        void setLabels();

    private slots:
        void onObjectSelection();
        void onButton(QAbstractButton *pButton);
        void onReadParams();

    private:
        IntEdit *m_pieChordPanels, *m_pieSpanPanels;
        QRadioButton *m_prbBinary, *m_prbASCII;
        QRadioButton *m_prb[5];

        QLabel *m_plabChordLabel, *m_plabSpanLabel;

        QDialogButtonBox *m_pButtonBox;
};




