/****************************************************************************

    WingSelDlg Class
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

#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>

#include "wingseldlg.h"
#include <objects/objects3d/plane.h>
#include <globals/globals.h>
#include <analysis3d/analysis3d_enums.h>

WingSelDlg::WingSelDlg(QWidget *pParent) : QDialog(pParent)
{
    m_pPlane = nullptr;
    setWindowFlags(Qt::Tool);
    setupLayout();
}


void WingSelDlg::setupLayout()
{
    QVBoxLayout *pWingLayout = new QVBoxLayout;
    {
        m_pctrlBody = new QCheckBox(tr("BODY"));
        pWingLayout->addWidget(m_pctrlBody);
        for(int iw=XFLR5::MAINWING; iw!=XFLR5::OTHERWING; iw++)
        {
            XFLR5::enumWingType wType = XFLR5::enumWingType(iw);
            m_pctrlWing[iw] = new QCheckBox(wingType(wType));
            pWingLayout->addWidget(m_pctrlWing[iw]);
        }
        m_pctrlWing[0]->setEnabled(false);
    }
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    QPushButton *pOKButton = new QPushButton(tr("OK"));
    connect(pOKButton, SIGNAL(clicked()), this, SLOT(onOK()));
    pMainLayout->addLayout(pWingLayout);
    pMainLayout->addWidget(pOKButton);
    setLayout(pMainLayout);
}


void WingSelDlg::initDialog(Plane *pPlane)
{
    m_pPlane = pPlane;
    for(int iw=0; iw<MAXWINGS; iw++)
    {
        m_pctrlWing[iw]->setChecked(pPlane->wing(iw));
    }
    m_pctrlBody->setChecked(m_pPlane->hasBody());
}

void WingSelDlg::onOK()
{
    m_pPlane->hasSecondWing() = m_pctrlWing[1]->isChecked();
    m_pPlane->hasElevator()   = m_pctrlWing[2]->isChecked();
    m_pPlane->hasFin()        = m_pctrlWing[3]->isChecked();

    m_pPlane->hasBody() = m_pctrlBody->isChecked();
    accept();
}

