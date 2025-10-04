/****************************************************************************

    BodyScaleDlg Class
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

#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QLabel>

#include "bodyscaledlg.h"
#include "bodydlg.h"
#include <xflcore/xflcore.h>
#include <xflobjects/objects3d/body.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/customwts/doubleedit.h>


BodyScaleDlg::BodyScaleDlg(QWidget *pParent ): QDialog(pParent)
{
    setWindowTitle(tr("Body Scale Dialog"));

    m_XFactor = 1.0;
    m_YFactor = 1.0;
    m_ZFactor = 1.0;
    m_bFrameOnly = false;
    m_FrameID = 0;
    setupLayout();
}



void BodyScaleDlg::initDialog(bool bFrameOnly)
{
    m_pdeXScaleFactor->setValue(m_XFactor);
    m_pdeYScaleFactor->setValue(m_YFactor);
    m_pdeZScaleFactor->setValue(m_ZFactor);

    m_pdeXScaleFactor->setFocus();


    m_pieFrameID->setEnabled(false);
    m_pieFrameID->setValue(m_FrameID+1);

    m_bFrameOnly = bFrameOnly;
    if(!m_bFrameOnly)
    {
        m_prbBody->setChecked(true);
        m_prbFrame->setChecked(false);
    }
    else
    {
        m_prbBody->setChecked(false);
        m_prbFrame->setChecked(true);
        m_pdeXScaleFactor->setEnabled(false);
    }

    enableControls();
}



void BodyScaleDlg::setupLayout()
{
    setWindowTitle("Scale Body");
    //    QDesktopWidget desktop;
    //    QRect r = desktop.geometry();
    //    setMinimumHeight(r.height()/3);
    //    move(r.width()/3, r.height()/6);

    QGridLayout *pTopLayout = new QGridLayout;
    {
        m_prbBody  = new QRadioButton(tr("Whole Body"));
        m_prbFrame = new QRadioButton(tr("Frame Only"));
        m_pieFrameID = new IntEdit(10);
        pTopLayout->addWidget(m_prbBody,1,1);
        pTopLayout->addWidget(m_prbFrame,2,1);
        pTopLayout->addWidget(m_pieFrameID,2,2);
    }

    QGridLayout *pScaleLayout = new QGridLayout;
    {
        m_pdeXScaleFactor = new DoubleEdit(1.0);
        m_pdeYScaleFactor = new DoubleEdit(2.000);
        m_pdeZScaleFactor = new DoubleEdit(3.);
        m_pdeXScaleFactor->setDigits(3);
        m_pdeYScaleFactor->setDigits(3);
        m_pdeZScaleFactor->setDigits(3);
        QLabel *lab0 = new QLabel(tr("Scale Factor"));
        QLabel *lab1 = new QLabel(tr("X Scale"));
        QLabel *lab2 = new QLabel(tr("Y Scale"));
        QLabel *lab3 = new QLabel(tr("Z Scale"));
        pScaleLayout->addWidget(lab0,1,2);
        pScaleLayout->addWidget(lab1,2,1);
        pScaleLayout->addWidget(lab2,3,1);
        pScaleLayout->addWidget(lab3,4,1);
        pScaleLayout->addWidget(m_pdeXScaleFactor,2,2);
        pScaleLayout->addWidget(m_pdeYScaleFactor,3,2);
        pScaleLayout->addWidget(m_pdeZScaleFactor,4,2);
    }

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addLayout(pTopLayout);
        pMainLayout->addLayout(pScaleLayout);
        pMainLayout->addWidget(m_pButtonBox);
        pMainLayout->addStretch(1);
    }
    setLayout(pMainLayout);

    connect(m_prbBody, SIGNAL(clicked()), this, SLOT(onRadio()));
    connect(m_prbFrame, SIGNAL(clicked()), this, SLOT(onRadio()));
}


void BodyScaleDlg::enableControls()
{

}


void BodyScaleDlg::onButton(QAbstractButton *pButton)
{
    if (     m_pButtonBox->button(QDialogButtonBox::Ok)      == pButton)  onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)  reject();
}


void BodyScaleDlg::keyPressEvent(QKeyEvent *event)
{
    // Prevent Return Key from closing App
    switch (event->key())
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
            event->ignore();
    }
}


void BodyScaleDlg::onRadio()
{
    if(m_prbBody->isChecked())
    {
        m_pieFrameID->setEnabled(false);
        m_pdeXScaleFactor->setEnabled(true);
        m_bFrameOnly = false;
    }
    else
    {
        m_pieFrameID->setEnabled(true);
        m_pdeXScaleFactor->setEnabled(false);
        m_bFrameOnly = true;
    }
}


void BodyScaleDlg::onOK()
{
    m_FrameID = m_pieFrameID->value()-1;

    m_XFactor = m_pdeXScaleFactor->value();
    m_YFactor = m_pdeYScaleFactor->value();
    m_ZFactor = m_pdeZScaleFactor->value();

    QDialog::accept();
}






