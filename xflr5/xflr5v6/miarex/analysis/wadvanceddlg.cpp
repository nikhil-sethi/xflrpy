/****************************************************************************

    WAdvancedDlg Class
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
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>


#include "wadvanceddlg.h"
#include <xflcore/xflcore.h>
#include <xflcore/units.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/customwts/intedit.h>

WAdvancedDlg::WAdvancedDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Wing Analysis Advanced Settings"));

    m_NLLTStation  = 20;
    m_AlphaPrec    = 0.01;
    m_Relax        = 20.;
    m_Iter         = 100;

    m_MaxWakeIter     = 1;
    m_CoreSize        = 0.0001;// 0.1mm
    m_WakeInterNodes  = 6;
    m_MinPanelSize    = .001;

    m_InducedDragPoint = 0;

    m_bDirichlet      = true;
    m_bLogFile        = true;
    m_bKeepOutOpps    = false;

    m_ControlPos = 0.75;
    m_VortexPos  = 0.25;


    setupLayout();
}


void WAdvancedDlg::setupLayout()
{
    QSizePolicy szPolicyMaximum;
    szPolicyMaximum.setHorizontalPolicy(QSizePolicy::Maximum);
    szPolicyMaximum.setVerticalPolicy(QSizePolicy::Maximum);

    QSizePolicy szPolicyMinimum;
    szPolicyMinimum.setHorizontalPolicy(QSizePolicy::Minimum);
    szPolicyMinimum.setVerticalPolicy(QSizePolicy::Minimum);

    QGroupBox *pAllBox = new QGroupBox(tr("All Analysis"));
    {
        QHBoxLayout *pAllLayout = new QHBoxLayout;
        {
            m_pchLogFile     = new QCheckBox(tr("View Log File after errors"));
            m_pchKeepOutOpps = new QCheckBox(tr("Store points outside the polar mesh"));
            pAllLayout->addWidget(m_pchLogFile);
            pAllLayout->addWidget(m_pchKeepOutOpps);
        }
        pAllBox->setLayout(pAllLayout);
    }

    QGroupBox *pVLMPanelBox = new QGroupBox(tr("VLM and Panel Methods"));
    {
        QVBoxLayout *pVLMPanelLayout = new QVBoxLayout;
        {
            QHBoxLayout *pWingPanelLayout = new QHBoxLayout;
            {
                m_pdeMinPanelSize = new DoubleEdit(1.00,5);
                QLabel *plabLength  = new QLabel(Units::lengthUnitLabel());
                QLabel *plab5 = new QLabel(tr("Ignore wing panels with span width <"));
                plab5->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                pWingPanelLayout->addStretch(1);
                pWingPanelLayout->addWidget(plab5);
                pWingPanelLayout->addWidget(m_pdeMinPanelSize);
                pWingPanelLayout->addWidget(plabLength);
            }
            QHBoxLayout *pCoreSizeLayout = new QHBoxLayout;
            {
                QLabel *plabLength  = new QLabel(Units::lengthUnitLabel());
                m_pdeCoreSize = new DoubleEdit(.0001, 4);
                m_pdeCoreSize->setToolTip("The radius of the cylinder around the trailing vortices\n"
                                            "under which the influence of the vortex is ignored, in order\n"
                                            "to prevent numerical errors.");
                QLabel *lab10 = new QLabel(tr("Core Size"));
                lab10->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                pCoreSizeLayout->addStretch(1);
                pCoreSizeLayout->addWidget(lab10);
                pCoreSizeLayout->addWidget(m_pdeCoreSize);
                pCoreSizeLayout->addWidget(plabLength);
            }
            pVLMPanelLayout->addLayout(pWingPanelLayout);
            pVLMPanelLayout->addLayout(pCoreSizeLayout);
        }
        pVLMPanelBox->setLayout(pVLMPanelLayout);
    }

    QGroupBox *pVLMBox = new QGroupBox(tr("VLM Method"));
    {
        QGridLayout *pVLMLayout = new QGridLayout;
        {
            m_pdeVortexPos    = new DoubleEdit(25.0, 2);
            m_pdeControlPos   = new DoubleEdit(75.0, 2);
            QLabel *lab6 = new QLabel(tr("Vortex Position"));
            QLabel *lab7 = new QLabel(tr("Control Point Position"));
            QLabel *lab8 = new QLabel("%");
            QLabel *lab9 = new QLabel("%");
            lab6->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            lab7->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            lab8->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            lab9->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            pVLMLayout->addWidget(lab6,1,1);
            pVLMLayout->addWidget(lab7,2,1);
            pVLMLayout->addWidget(m_pdeVortexPos,1,2);
            pVLMLayout->addWidget(m_pdeControlPos,2,2);
            pVLMLayout->addWidget(lab8,1,3);
            pVLMLayout->addWidget(lab9,2,3);
        }
        pVLMBox->setLayout(pVLMLayout);
    }

    QGroupBox *pLLTBox = new QGroupBox(tr("Lifting Line Method"));
    {
        QGridLayout *pLLTLayout = new QGridLayout;
        {
            m_pieNStation     = new IntEdit(20, this);
            m_pdeRelax        = new DoubleEdit(20,1);
            m_pdeAlphaPrec    = new DoubleEdit(.01, 4);
            m_pieIterMax      = new IntEdit(100, this);
            QLabel *lab1 = new QLabel(tr("Number of spanwise stations"));
            QLabel *lab2 = new QLabel(tr("Relax. factor"));
            QLabel *lab3 = new QLabel(tr("Alpha Precision"));
            QLabel *lab4 = new QLabel(tr("Max. Iterations"));
            lab1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            lab2->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            lab3->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            lab4->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            pLLTLayout->addWidget(lab1,1,1);
            pLLTLayout->addWidget(lab2,2,1);
            pLLTLayout->addWidget(lab3,3,1);
            pLLTLayout->addWidget(lab4,4,1);
            pLLTLayout->addWidget(m_pieNStation,1,2);
            pLLTLayout->addWidget(m_pdeRelax,2,2);
            pLLTLayout->addWidget(m_pdeAlphaPrec,3,2);
            pLLTLayout->addWidget(m_pieIterMax,4,2);
        }
        pLLTBox->setLayout(pLLTLayout);
    }

    QGroupBox *pPanelBCBox = new QGroupBox(tr("3D Panel boundary conditions"));
    {
        QVBoxLayout *pPanelBCLayout = new QVBoxLayout;
        {
            m_prbDirichlet = new QRadioButton("Dirichlet (Recommended)");
            m_prbNeumann = new QRadioButton("Neumann");
            pPanelBCLayout->addWidget(m_prbDirichlet);
            pPanelBCLayout->addWidget(m_prbNeumann);
        }
        pPanelBCBox->setLayout((pPanelBCLayout));
    }

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Discard | QDialogButtonBox::Reset);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
    }

    QHBoxLayout *pBothSides = new QHBoxLayout;
    {
        QVBoxLayout *pLeftSide  = new QVBoxLayout;
        {
            pLeftSide->addWidget(pLLTBox);
            pLeftSide->addStretch(1);
            pLeftSide->addWidget(pPanelBCBox);
            pLeftSide->addStretch(1);
        }
        QVBoxLayout *pRightSide = new QVBoxLayout;
        {
            pRightSide->addWidget(pVLMBox);
            pRightSide->addStretch(1);
            pRightSide->addWidget(pVLMPanelBox);
            pRightSide->addStretch(1);
        }
        pBothSides->addLayout(pLeftSide);
        pBothSides->addLayout(pRightSide);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addLayout(pBothSides);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(pAllBox);
        pMainLayout->addStretch(1);
        pMainLayout->addSpacing(30);
        pMainLayout->addWidget(m_pButtonBox);
    }

    setSizePolicy(szPolicyMaximum);

    setLayout(pMainLayout);
}


void WAdvancedDlg::onButton(QAbstractButton *pButton)
{
    if (     m_pButtonBox->button(QDialogButtonBox::Ok)      == pButton)  onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)  reject();
    else if (m_pButtonBox->button(QDialogButtonBox::Reset)   == pButton)  onResetDefaults();
}


void WAdvancedDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Escape:
        {
            reject();
            return;
        }
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
                onOK();
                return;
            }
        }
        default:
            pEvent->ignore();
            break;
    }
}


void WAdvancedDlg::initDialog()
{
    setParams();

    m_pdeVortexPos->setEnabled(false);
    m_pdeControlPos->setEnabled(false);

    m_bDirichlet = true;
    m_prbDirichlet->setChecked(m_bDirichlet);
    m_prbNeumann->setChecked(!m_bDirichlet);
    m_prbDirichlet->setEnabled(false);
    m_prbNeumann->setEnabled(false);
}


void WAdvancedDlg::onOK()
{
    readParams();
    accept();
}


void WAdvancedDlg::onResetDefaults()
{
    m_Relax            = 20.0;
    m_AlphaPrec        = 0.01;
    m_Iter             = 100;
    m_NLLTStation      = 20;
    m_MaxWakeIter      = 5;
    m_CoreSize         = 0.0001;
    m_MinPanelSize     = .001;
    m_WakeInterNodes   = 6;
    m_bLogFile         = true;
    m_VortexPos        = 0.25;
    m_ControlPos       = 0.75;
    m_bDirichlet       = true;
    m_bTrefftz         = true;
    m_bKeepOutOpps     = false;
    setParams();
}


void WAdvancedDlg::readParams()
{
    m_Relax           = m_pdeRelax->value();
    m_AlphaPrec       = m_pdeAlphaPrec->value();
    m_CoreSize        = m_pdeCoreSize->value() / Units::mtoUnit();
    m_MinPanelSize    = m_pdeMinPanelSize->value() / Units::mtoUnit();
    m_VortexPos       = m_pdeVortexPos->value()/100.0;
    m_ControlPos      = m_pdeControlPos->value()/100.0;
    m_Iter            = m_pieIterMax->value();
    m_NLLTStation     = m_pieNStation->value();
    m_bDirichlet      = m_prbDirichlet->isChecked();
    m_bTrefftz        = true;
    m_bKeepOutOpps    = m_pchKeepOutOpps->isChecked();
    m_bLogFile        = m_pchLogFile->isChecked();
}


void WAdvancedDlg::setParams()
{
    m_pieIterMax->setValue(m_Iter);
    m_pdeRelax->setValue(m_Relax);
    m_pdeAlphaPrec->setValue(m_AlphaPrec);
    m_pieNStation->setValue(m_NLLTStation);

    m_pdeCoreSize->setValue(m_CoreSize* Units::mtoUnit());

    m_pdeMinPanelSize->setValue(m_MinPanelSize * Units::mtoUnit());

    m_pchLogFile->setChecked(m_bLogFile);
    m_pchKeepOutOpps->setChecked(m_bKeepOutOpps);

    m_pdeControlPos->setValue(m_ControlPos*100.0);
    m_pdeVortexPos->setValue(m_VortexPos*100.0);
}




