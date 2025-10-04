/****************************************************************************

    TwoDPanelDlg Class
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
#include <QPushButton>


class IntEdit;
class DoubleEdit;
class Foil;
class XFoil;

class TwoDPanelDlg : public QDialog
{
    Q_OBJECT
    friend class AFoil;
    friend class XDirect;

    private slots:
        void onApply();
        void onOK();
        void onChanged();
        void onButton(QAbstractButton *pButton);

    public:
        TwoDPanelDlg(QWidget *pParent);

        static XFoil *s_pXFoil;

        void initDialog();

    private:
        void keyPressEvent(QKeyEvent *pEvent) override;
        void setupLayout();
        void readParams();

        QDialogButtonBox *m_pButtonBox;

        IntEdit  *m_pieNPanels;
        DoubleEdit *m_pdeCVpar,  *m_pdeCTErat, *m_pdeCTRrat;
        DoubleEdit *m_pdeXsRef1, *m_pdeXsRef2, *m_pdeXpRef1, *m_pdeXpRef2;

        bool m_bApplied;
        bool m_bModified;

        int npan;
        double cvpar;
        double cterat;
        double ctrrat;
        double xsref1;
        double xsref2;
        double xpref1;
        double xpref2;

        Foil *m_pBufferFoil;
        Foil const*m_pMemFoil;

        QWidget *m_pParent;
};


