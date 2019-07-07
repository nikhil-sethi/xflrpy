/****************************************************************************

    TargetCurveDlg Class
    Copyright (C) 2015 Andre Deperrois 

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

#ifndef TARGETCURVEDLG_H
#define TARGETCURVEDLG_H

#include <QDialog>
#include <misc/text/doubleedit.h>
#include <QRadioButton>
#include <QCheckBox>

class TargetCurveDlg : public QDialog
{
    Q_OBJECT
public:
    TargetCurveDlg(QWidget *pParent=NULL);
    void initDialog(bool bShowElliptic, bool bShowBell, bool bMaxCl, double curveExp);

private:
    void setupLayout();

private slots:
    void onOK();

public:
    double m_BellCurveExp;
    bool m_bMaxCL, m_bShowBellCurve, m_bShowEllipticCurve;

private:
    DoubleEdit *m_pCtrlExptEdit;
    QRadioButton *m_pCtrlRadio1, *m_pCtrlRadio2;
    QCheckBox *m_pctrlShowBellCurve, *m_pctrlShowEllipticCurve;
};

#endif // TARGETCURVEDLG_H
