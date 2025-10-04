/****************************************************************************

    WAdvancedDlg Class
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
#include <QDialogButtonBox>

class DoubleEdit;
class IntEdit;

class WAdvancedDlg : public QDialog
{
    Q_OBJECT
    friend class Miarex;
    friend class MainFrame;

    public:
        WAdvancedDlg(QWidget *pParent);
        void initDialog();

    private slots:
        void onOK();
        void onResetDefaults();
        void onButton(QAbstractButton *pButton);

    private:
        void keyPressEvent(QKeyEvent *event) override;
        void readParams();
        void setParams();
        void setupLayout();

        QDialogButtonBox *m_pButtonBox;

        QCheckBox *m_pchLogFile;
        QCheckBox *m_pchKeepOutOpps;
        QRadioButton *m_prbDirichlet, *m_prbNeumann;
        DoubleEdit *m_pdeRelax;
        DoubleEdit *m_pdeAlphaPrec;
        DoubleEdit *m_pdeMinPanelSize;
        IntEdit *m_pieNStation;
        IntEdit *m_pieIterMax;
        DoubleEdit *m_pdeCoreSize;
        DoubleEdit *m_pdeVortexPos;
        DoubleEdit *m_pdeControlPos;

        bool m_bLogFile;
        bool m_bDirichlet;
        bool m_bTrefftz;
        bool m_bKeepOutOpps;

        int m_Iter;
        int m_NLLTStation;
        int m_WakeInterNodes;
        int m_MaxWakeIter;
        int m_InducedDragPoint;

        double m_ControlPos, m_VortexPos;
        double m_Relax, m_AlphaPrec;
        double m_CoreSize;
        double m_MinPanelSize;

};

