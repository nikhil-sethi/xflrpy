/****************************************************************************

    WingScaleDlg Class
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

class WingScaleDlg : public QDialog
{
    Q_OBJECT
    friend class Miarex;
    friend class MainFrame;
    public:
        WingScaleDlg(QWidget *pParent);
        void initDialog(double const &RefSpan, double const &RefChord, double const &RefSweep, double const &RefTwist, const double &RefArea, const double &RefAR, const double &RefTR);

    private:
        void setupLayout();
        void readData();
        void setResults();
        void enableControls();
        void keyPressEvent(QKeyEvent *event) override;

    private slots:
        void onButton(QAbstractButton *pButton);
        void onOK();
        void onClickedCheckBox();
        void onEditingFinished();


    public:

        QCheckBox *m_pchSpan, *m_pchChord, *m_pchSweep, *m_pchTwist;
        QCheckBox *m_pchScaleArea, *m_pchScaleAR, *m_pchScaleTR;
        DoubleEdit *m_pdeNewSpan, *m_pdeNewChord, *m_pdeNewSweep, *m_pdeNewTwist;
        DoubleEdit *m_pdeNewArea, *m_pdeNewAR, *m_pdeNewTR;
        QLabel *m_plabRefSpan, *m_plabRefChord, *m_plabRefSweep, *m_plabRefTwist;
        QLabel *m_plabRefArea,*m_plabRefAR, *m_plabRefTR;
        QLabel *m_plabSpanRatio, *m_plabChordRatio, *m_plabSweepRatio, *m_plabTwistRatio;
        QLabel *m_plabAreaRatio,*m_plabARRatio, *m_plabTRRatio;

        QDialogButtonBox *m_pButtonBox;

        bool m_bSweep, m_bSpan, m_bChord, m_bTwist;
        bool m_bArea, m_bAR, m_bTR;

        double m_NewSweep, m_NewChord, m_NewTwist, m_NewSpan, m_NewArea, m_NewAR, m_NewTR;
        double m_RefSweep, m_RefChord, m_RefTwist, m_RefSpan, m_RefArea, m_RefAR, m_RefTR;
};

