/****************************************************************************

    QXDirectStyleDlg Class
    Copyright (C) 2009 André Deperrois

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

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

#include "xdirectstyledlg.h"
#include <xdirect/xdirect.h>
#include <xdirect/oppointwt.h>
#include <xflwidgets/line/linemenu.h>

#include <xflwidgets/line/linebtn.h>


XDirectStyleDlg::XDirectStyleDlg(OpPointWt *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("XDirect Styles"));
    m_pOpPointWt = pParent;

    m_NeutralStyle  = m_pOpPointWt->m_NeutralStyle;
    m_BLStyle  = m_pOpPointWt->m_BLStyle;
    m_PressureStyle  = m_pOpPointWt->m_PressureStyle;

    setupLayout();

    m_plbNeutral->setTheStyle(m_NeutralStyle);
    m_plbBL->setTheStyle(m_BLStyle);
    m_plbPressure->setTheStyle(m_PressureStyle);
}


void XDirectStyleDlg::setupLayout()
{
    QGridLayout *pStyleLayout = new QGridLayout;
    {
        m_plbNeutral  = new LineBtn(this);
        m_plbBL       = new LineBtn(this);
        m_plbPressure = new LineBtn(this);
        QLabel *plab1 = new QLabel(tr("Neutral Line"));
        QLabel *plab2 = new QLabel(tr("Boundary Layer"));
        QLabel *plab3 = new QLabel(tr("Pressure"));
        pStyleLayout->addWidget(plab1,1,1);
        pStyleLayout->addWidget(plab2,2,1);
        pStyleLayout->addWidget(plab3,3,1);
        pStyleLayout->addWidget(m_plbNeutral,1,2);
        pStyleLayout->addWidget(m_plbBL,2,2);
        pStyleLayout->addWidget(m_plbPressure,3,2);
        connect(m_plbNeutral,  SIGNAL(clickedLB(LineStyle)), SLOT(onNeutralStyle()));
        connect(m_plbBL,       SIGNAL(clickedLB(LineStyle)), SLOT(onBLStyle()));
        connect(m_plbPressure, SIGNAL(clickedLB(LineStyle)), SLOT(onPressureStyle()));
    }


    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close | QDialogButtonBox::RestoreDefaults);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addLayout(pStyleLayout);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_pButtonBox);
        pMainLayout->addStretch(1);
    }

    setLayout(pMainLayout);
}


void XDirectStyleDlg::onButton(QAbstractButton *pButton)
{
    if (     m_pButtonBox->button(QDialogButtonBox::Close)           == pButton)  accept();
    else if (m_pButtonBox->button(QDialogButtonBox::RestoreDefaults) == pButton)  onRestoreDefaults();
}


void XDirectStyleDlg::onNeutralStyle()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(m_NeutralStyle);
    lm.exec(QCursor::pos());

    m_NeutralStyle = lm.theStyle();
    m_plbNeutral->setTheStyle(m_NeutralStyle);
}


void XDirectStyleDlg::onPressureStyle()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(m_PressureStyle);
    lm.exec(QCursor::pos());

    m_PressureStyle = lm.theStyle();
    m_plbPressure->setTheStyle(m_PressureStyle);
}


void XDirectStyleDlg::onBLStyle()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(m_BLStyle);
    lm.exec(QCursor::pos());

    m_BLStyle = lm.theStyle();
    m_plbBL->setTheStyle(m_BLStyle);
}


void XDirectStyleDlg::onRestoreDefaults()
{
    m_NeutralStyle = {true, Line::DASHDOT, 1, QColor(155,155,155), Line::NOSYMBOL};
    m_plbNeutral->setTheStyle(m_NeutralStyle);

    m_BLStyle      = {true, Line::DASH, 1, QColor(205,55,55), Line::NOSYMBOL};
    m_plbBL->setTheStyle(m_BLStyle);

    m_PressureStyle = {true, Line::DASH, 1, QColor(55,155,55), Line::NOSYMBOL};
    m_plbPressure->setTheStyle(m_PressureStyle);
}


void XDirectStyleDlg::keyPressEvent(QKeyEvent *pEvent)
{
    // Prevent Return Key from closing App
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
                m_pButtonBox->setFocus();
                return;
            }
            else
            {
                accept();
                return;
            }
        }
        case Qt::Key_Escape:
        {
            reject();
            break;
        }
    }
}

