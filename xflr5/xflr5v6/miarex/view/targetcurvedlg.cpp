/****************************************************************************

    TargetCurveDlg Class
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

#include "targetcurvedlg.h"

#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>


TargetCurveDlg::TargetCurveDlg(QWidget *pParent) : QDialog(pParent)
{
    m_BellCurveExp = 1;
    m_bMaxCL = true;
    m_bShowEllipticCurve = false;
    m_bShowBellCurve = false;
    setupLayout();
}


void TargetCurveDlg::initDialog(bool bShowElliptic, bool bShowBell, bool bMaxCl, double curveExp)
{
    m_bShowEllipticCurve = bShowElliptic;
    m_bShowBellCurve = bShowBell;
    m_bMaxCL = bMaxCl;
    m_BellCurveExp = curveExp;
    m_pchShowEllipticCurve->setChecked(m_bShowEllipticCurve);
    m_pchShowBellCurve->setChecked(m_bShowBellCurve);
    m_pdeExptEdit->setValue(m_BellCurveExp);
    m_prbRadio1->setChecked(m_bMaxCL);
    m_prbRadio2->setChecked(!m_bMaxCL);
}


void TargetCurveDlg::setupLayout()
{
    QVBoxLayout*pMainLayout = new QVBoxLayout;
    {
        QHBoxLayout *pEllLayout = new QHBoxLayout;
        {
            QLabel *pEllipticLabel = new QLabel(QString::fromUtf8("y=sqrt(1-(2x/b)²)"));
            m_pchShowEllipticCurve = new QCheckBox(tr("Show Elliptic Curve"));
            pEllLayout->addWidget(m_pchShowEllipticCurve);
            pEllLayout->addWidget(pEllipticLabel);
        }
        QHBoxLayout *pBellLayout = new QHBoxLayout;
        {
            QLabel *pBellLabel = new QLabel(QString::fromUtf8("y=(1-(2x/b)²)^p"));
            m_pchShowBellCurve = new QCheckBox(tr("Show Bell Curve"));
            pBellLayout->addWidget(m_pchShowBellCurve);
            pBellLayout->addWidget(pBellLabel);
        }
        QHBoxLayout * pExpLayout = new QHBoxLayout;
        {
            QLabel *pExpLabel = new QLabel(tr("Bell Curve exponent:"));
            m_pdeExptEdit = new DoubleEdit(1, 2);
            pExpLayout->addWidget(pExpLabel);
            pExpLayout->addWidget(m_pdeExptEdit);
        }
        QGroupBox *pctrlCLBox = new QGroupBox(tr("Cl Adjustment"));
        {
            QHBoxLayout *pCLLayout = new QHBoxLayout;
            {
                m_prbRadio1 = new QRadioButton(tr("Max local Cl"));
                m_prbRadio2 = new QRadioButton(tr("Wing CL"));
                pCLLayout->addWidget(m_prbRadio1);
                pCLLayout->addWidget(m_prbRadio2);
            }
            pctrlCLBox->setLayout(pCLLayout);
        }

        QDialogButtonBox *pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
        connect(pButtonBox, &QDialogButtonBox::accepted, this, &TargetCurveDlg::onOK);

        pMainLayout->addLayout(pEllLayout);
        pMainLayout->addLayout(pBellLayout);
        pMainLayout->addLayout(pExpLayout);
        pMainLayout->addWidget(pctrlCLBox);
        pMainLayout->addWidget(pButtonBox);
    }

    setLayout(pMainLayout);
}


void TargetCurveDlg::onOK()
{
    m_bShowEllipticCurve = m_pchShowEllipticCurve->isChecked();
    m_bShowBellCurve = m_pchShowBellCurve->isChecked();
    m_BellCurveExp = m_pdeExptEdit->value();
    m_bMaxCL = m_prbRadio1->isChecked();
    accept();
}


