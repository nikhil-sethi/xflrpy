/****************************************************************************

    BodyTransDlg Class
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


#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "bodytransdlg.h"
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/customwts/doubleedit.h>


BodyTransDlg::BodyTransDlg(QWidget *pParent): QDialog(pParent)
{
    setWindowTitle(tr("Body Translation"));
    m_XTrans = m_YTrans = m_ZTrans = 0.0;
    m_bFrameOnly = false;
    m_FrameID = 1;

    setupLayout();
}


void BodyTransDlg::initDialog()
{
    m_pdeXTransFactor->setValue(m_XTrans);
    m_pdeYTransFactor->setValue(m_YTrans);
    m_pdeZTransFactor->setValue(m_ZTrans);

    m_pdeYTransFactor->setEnabled(false);

    m_pchFrameOnly->setChecked(m_bFrameOnly);
    m_pieFrameID->setValue(m_FrameID+1);
    m_pieFrameID->setEnabled(m_bFrameOnly);
}


void BodyTransDlg::keyPressEvent(QKeyEvent *pEvent)
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
            }
            return;
        }
        case Qt::Key_Escape:
        {
            reject();
            break;
        }
        default:
            pEvent->ignore();
    }
}


void BodyTransDlg::onButton(QAbstractButton *pButton)
{
    if (     m_pButtonBox->button(QDialogButtonBox::Ok)      == pButton)  onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)  reject();
}


void BodyTransDlg::onOK()
{
    m_bFrameOnly = m_pchFrameOnly->isChecked();
    m_FrameID    = m_pieFrameID->value()-1;
    m_XTrans     = m_pdeXTransFactor->value() / Units::mtoUnit();
    m_YTrans     = m_pdeYTransFactor->value() / Units::mtoUnit();
    m_ZTrans     = m_pdeZTransFactor->value() / Units::mtoUnit();
    accept();
}



void BodyTransDlg::onFrameOnly()
{
    m_bFrameOnly = m_pchFrameOnly->isChecked();
    m_pieFrameID->setEnabled(m_bFrameOnly);
}



void BodyTransDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QHBoxLayout *pFrameIDLayout = new QHBoxLayout;
        {
            m_pchFrameOnly = new QCheckBox(tr("Frame Only"));
            m_pieFrameID = new IntEdit(0);
            pFrameIDLayout->addWidget(m_pchFrameOnly);
            pFrameIDLayout->addWidget(m_pieFrameID);
        }

        QGridLayout *pTransLayout = new QGridLayout;
        {
            QLabel * XTrans = new QLabel(tr("X Translation"));
            QLabel * YTrans = new QLabel(tr("Y Translation"));
            QLabel * ZTrans = new QLabel(tr("Z Translation"));
            m_pdeXTransFactor = new DoubleEdit(0.0,3);
            m_pdeYTransFactor = new DoubleEdit(0.0,3);
            m_pdeZTransFactor = new DoubleEdit(0.0,3);
            QString length;
            Units::getLengthUnitLabel(length);
            QLabel *plabLength1 = new QLabel(length);
            QLabel *plabLength2 = new QLabel(length);
            QLabel *plabLength3 = new QLabel(length);
            pTransLayout->addWidget(XTrans,              1,1);
            pTransLayout->addWidget(YTrans,              2,1);
            pTransLayout->addWidget(ZTrans,              3,1);
            pTransLayout->addWidget(m_pdeXTransFactor, 1,2);
            pTransLayout->addWidget(m_pdeYTransFactor, 2,2);
            pTransLayout->addWidget(m_pdeZTransFactor, 3,2);
            pTransLayout->addWidget(plabLength1,         1,3);
            pTransLayout->addWidget(plabLength2,         2,3);
            pTransLayout->addWidget(plabLength3,         3,3);
        }

        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        {
            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
        }

        pMainLayout->addLayout(pFrameIDLayout);
        pMainLayout->addLayout(pTransLayout);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_pButtonBox);
    }

    setLayout(pMainLayout);

    connect(m_pchFrameOnly, SIGNAL(clicked()), SLOT(onFrameOnly()));

}

