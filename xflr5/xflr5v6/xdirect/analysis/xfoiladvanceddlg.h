/****************************************************************************

    XFoilAdvancedDlg Class
    Copyright (C) 2009-2016 André Deperrois 

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
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QDialogButtonBox>

#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/customwts/intedit.h>


class XFoilAdvancedDlg : public QDialog
{
    Q_OBJECT

    friend class XDirect;
    friend class BatchThreadDlg;
    friend class BatchDlg;
    friend class BatchAbstractDlg;

    public:
        XFoilAdvancedDlg(QWidget *pParent=nullptr);
        void initDialog();

    private slots:
        void accept() override;
        void onButton(QAbstractButton *pButton);

    private:
        void keyPressEvent(QKeyEvent *pEvent) override;
        void setupLayout();
        void resetDefaults();

    private:
        QCheckBox *m_pchInitBL, *m_pchFullReport;
        QCheckBox *m_pchKeepErrorsOpen;
        IntEdit *m_pieIterLimit, *m_pieTimerInterval;
        DoubleEdit * m_pdeVAccel;

        QDialogButtonBox *m_pButtonBox;


        int m_IterLimit; /** @todo replace with a static variable in XFoilTask */
        double m_VAccel;
        bool m_bAutoInitBL;
        bool m_bFullReport;

};


