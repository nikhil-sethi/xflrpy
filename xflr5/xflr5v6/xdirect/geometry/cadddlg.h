/****************************************************************************

    Corner Add class
    Copyright (C) 2004-2016 André Deperrois

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

/** @file This file implements the class used to refine locally the points on a foil */


#pragma once

#include <QDialog>
#include <QLabel>
#include <QRadioButton>
#include <QPushButton>
#include <QDialogButtonBox>

class Foil;
class DoubleEdit;
class XFoil;

/**
* @class CAddDlg
* @brief The class which interfaces with the cadd method implemented in XFoil.
*
* This class is used to add panels locally on a foil surface, in the areas of high surface curvature.
* Refer to Xfoil documentation for more documentation
* One of the early classes of this project.
*/
class CAddDlg : public QDialog
{
    Q_OBJECT

    friend class MainFrame;
    friend class XDirect;
    friend class AFoil;

    public:
        CAddDlg(QWidget *pParent);
        void initDialog();

    private:
        void keyPressEvent(QKeyEvent *pEvent) override;
        void setupLayout();

    private slots:
        void onUniform();
        void onButton(QAbstractButton *pButton);
        void onApply();

    private:
        QDialogButtonBox *m_pButtonBox;

        QLabel *m_plabAtPanel;
        QLabel *m_plabTotal;
        QLabel *m_plabAdded;
        QLabel *m_plabMaxAngle;

        QRadioButton  *m_prbUniform;
        QRadioButton  *m_prbArcLength;
        DoubleEdit    *m_pdeTo;
        DoubleEdit    *m_pdeFrom;
        DoubleEdit    *m_pdeAngTol;

        Foil const* m_pMemFoil;
        Foil* m_pBufferFoil;
        QWidget *m_pParent;
        double atol;
        int m_iSplineType;

        static XFoil* s_pXFoil;

};


