/****************************************************************************

    WingScaleDlg Class
    Copyright (C) 2009 Andre Deperrois

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

#ifndef WINGSCALEDLG_H
#define WINGSCALEDLG_H

#include <QDialog>
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

private slots:
    void onOK();
    void onClickedCheckBox();
    void onEditingFinished();


public:

    QCheckBox *m_pctrlSpan, *m_pctrlChord, *m_pctrlSweep, *m_pctrlTwist;
    QCheckBox *m_pctrlScaleArea, *m_pctrlScaleAR, *m_pctrlScaleTR;
    DoubleEdit  *m_pctrlNewSpan, *m_pctrlNewChord, *m_pctrlNewSweep, *m_pctrlNewTwist;
    DoubleEdit *m_pctrlNewArea, *m_pctrlNewAR, *m_pctrlNewTR;
    QLabel *m_pctrlRefSpan, *m_pctrlRefChord, *m_pctrlRefSweep, *m_pctrlRefTwist;
    QLabel *m_pctrlRefArea,*m_pctrlRefAR, *m_pctrlRefTR;
    QLabel *m_pctrlSpanRatio, *m_pctrlChordRatio, *m_pctrlSweepRatio, *m_pctrlTwistRatio;
    QLabel *m_pctrlAreaRatio,*m_pctrlARRatio, *m_pctrlTRRatio;

    bool m_bSweep, m_bSpan, m_bChord, m_bTwist;
    bool m_bArea, m_bAR, m_bTR;

    double m_NewSweep, m_NewChord, m_NewTwist, m_NewSpan, m_NewArea, m_NewAR, m_NewTR;
    double m_RefSweep, m_RefChord, m_RefTwist, m_RefSpan, m_RefArea, m_RefAR, m_RefTR;
};

#endif // WINGSCALEDLG_H
