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

#include <QLabel>
#include <xdirect/xdirect.h>
#include "xfoiladvanceddlg.h"


XFoilAdvancedDlg::XFoilAdvancedDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("XFoil Settings"));
    setupLayout();

    m_IterLimit = 100;
    m_VAccel = 0.001;
    m_bAutoInitBL = true;
    m_bFullReport = false;
}


void XFoilAdvancedDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QHBoxLayout *pVAccelBoxLayout = new QHBoxLayout;
        {
            QLabel *lab1 = new QLabel(tr("VAccel"));
            lab1->setAlignment(Qt::AlignRight);
            m_pdeVAccel = new DoubleEdit(0.0, 3);     // jx-mod allow 3 decimals fo vaccel according to init value '0.001'
            m_pdeVAccel->setAlignment(Qt::AlignRight);
            pVAccelBoxLayout->addStretch(1);
            pVAccelBoxLayout->addWidget(lab1);
            pVAccelBoxLayout->addWidget(m_pdeVAccel);
        }

        QHBoxLayout *pIterBoxLayout = new QHBoxLayout;
        {
            QLabel *lab2 = new QLabel(tr("Iteration Limit"));
            lab2->setAlignment(Qt::AlignRight);
            m_pieIterLimit = new IntEdit;

            pIterBoxLayout->addStretch(1);
            pIterBoxLayout->addWidget(lab2);
            pIterBoxLayout->addWidget(m_pieIterLimit);
        }

        m_pchInitBL = new QCheckBox(tr("Re-initialize BLs after an unconverged iteration"));
        m_pchFullReport = new QCheckBox(tr("Show full log report for an XFoil analysis"));
        m_pchKeepErrorsOpen = new QCheckBox(tr("Keep Xfoil interface open if analysis errors"));

        QHBoxLayout *pTimerLayout = new QHBoxLayout;
        {
            QLabel *pTimerLabel = new QLabel(tr("Time interval between graph updates"));
            QLabel *pTimerUnitLabel = new QLabel("ms");
            m_pieTimerInterval = new IntEdit(XDirect::timeUpdateInterval(), this);
            m_pieTimerInterval->setMin(0);
            pTimerLayout->addStretch();
            pTimerLayout->addWidget(pTimerLabel);
            pTimerLayout->addWidget(m_pieTimerInterval);
            pTimerLayout->addWidget(pTimerUnitLabel);
        }

        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults, this);
        {
            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
        }


        pMainLayout->addStretch();
        pMainLayout->addLayout(pVAccelBoxLayout);
        pMainLayout->addLayout(pIterBoxLayout);
        pMainLayout->addWidget(m_pchInitBL);
        pMainLayout->addWidget(m_pchFullReport);
        pMainLayout->addWidget(m_pchKeepErrorsOpen);
        pMainLayout->addLayout(pTimerLayout);
        pMainLayout->addStretch();
        pMainLayout->addWidget(m_pButtonBox);
    }

    setLayout(pMainLayout);
}


void XFoilAdvancedDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok)              == pButton)   accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel)          == pButton)   reject();
    else if (m_pButtonBox->button(QDialogButtonBox::RestoreDefaults) == pButton)   resetDefaults();
}


void XFoilAdvancedDlg::resetDefaults()
{
    m_IterLimit = 100;
    m_VAccel = 0.001;
    m_bAutoInitBL = true;
    m_bFullReport = false;
    XDirect::setKeepOpenOnErrors(true);
    XDirect::setTimeUpdateInterval(100);
    initDialog();
}


void XFoilAdvancedDlg::initDialog()
{
    m_pdeVAccel->setValue(m_VAccel);
    m_pchInitBL->setChecked(m_bAutoInitBL);
    m_pieIterLimit->setValue(m_IterLimit);
    m_pchFullReport->setChecked(m_bFullReport);
    m_pchKeepErrorsOpen->setChecked(XDirect::bKeepOpenOnErrors());
    m_pieTimerInterval->setValue(XDirect::timeUpdateInterval());
}


void XFoilAdvancedDlg::keyPressEvent(QKeyEvent *pEvent)
{
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
            break;
        }
        case Qt::Key_Escape:
        {
            reject();
            return;
        }
        default:
            pEvent->ignore();
    }
}


void XFoilAdvancedDlg::accept()
{
    m_IterLimit = m_pieIterLimit->value();
    m_VAccel = m_pdeVAccel->value();
    m_bAutoInitBL = m_pchInitBL->isChecked();
    m_bFullReport = m_pchFullReport->isChecked();
    XDirect::setTimeUpdateInterval(m_pieTimerInterval->value());
    XDirect::setKeepOpenOnErrors(m_pchKeepErrorsOpen->isChecked());
    QDialog::accept();
}


