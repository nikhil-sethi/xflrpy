/****************************************************************************

	WAdvancedDlg Class
	Copyright (C) 2009-2016 Andre Deperrois 

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

#include <globals/globals.h>
#include <misc/options/Units.h>
#include "WAdvancedDlg.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>



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
	m_bKeepOutOpps    = true;

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
			m_pctrlLogFile     = new QCheckBox(tr("View Log File after errors"));
			m_pctrlKeepOutOpps = new QCheckBox(tr("Store points outside the polar mesh"));
			pAllLayout->addWidget(m_pctrlLogFile);
			pAllLayout->addWidget(m_pctrlKeepOutOpps);
		}
		pAllBox->setLayout(pAllLayout);
	}

	QGroupBox *pVLMPanelBox = new QGroupBox(tr("VLM and Panel Methods"));
	{
		QVBoxLayout *pVLMPanelLayout = new QVBoxLayout;
		{
			QHBoxLayout *pWingPanelLayout = new QHBoxLayout;
			{
				m_pctrlMinPanelSize = new DoubleEdit(1.00,5);
				m_pctrlLength  = new QLabel("");
				QLabel *lab5 = new QLabel(tr("Ignore wing panels with span width <"));
				lab5->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
				pWingPanelLayout->addStretch(1);
				pWingPanelLayout->addWidget(lab5);
				pWingPanelLayout->addWidget(m_pctrlMinPanelSize);
				pWingPanelLayout->addWidget(m_pctrlLength);
			}
			QHBoxLayout *pCoreSizeLayout = new QHBoxLayout;
			{
				m_pctrlLength2 = new QLabel("");
				m_pctrlCoreSize     = new DoubleEdit(.0001, 4);
				m_pctrlCoreSize->setToolTip("The radius of the cylinder around the trailing vortices\n"
											"under which the influence of the vortex is ignored, in order\n"
											" to prevent numerical errors.");
				QLabel *lab10 = new QLabel(tr("Core Size"));
				lab10->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
				pCoreSizeLayout->addStretch(1);
				pCoreSizeLayout->addWidget(lab10);
				pCoreSizeLayout->addWidget(m_pctrlCoreSize);
				pCoreSizeLayout->addWidget(m_pctrlLength2);
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
			m_pctrlVortexPos    = new DoubleEdit(25.0, 2);
			m_pctrlControlPos   = new DoubleEdit(75.0, 2);
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
			pVLMLayout->addWidget(m_pctrlVortexPos,1,2);
			pVLMLayout->addWidget(m_pctrlControlPos,2,2);
			pVLMLayout->addWidget(lab8,1,3);
			pVLMLayout->addWidget(lab9,2,3);
		}
		pVLMBox->setLayout(pVLMLayout);
	}

	QGroupBox *pLLTBox = new QGroupBox(tr("Lifting Line Method"));
	{
		QGridLayout *pLLTLayout = new QGridLayout;
		{
			m_pctrlNStation     = new IntEdit(20, this);
			m_pctrlRelax        = new DoubleEdit(20,1);
			m_pctrlAlphaPrec    = new DoubleEdit(.01, 4);
			m_pctrlIterMax      = new IntEdit(100, this);
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
			pLLTLayout->addWidget(m_pctrlNStation,1,2);
			pLLTLayout->addWidget(m_pctrlRelax,2,2);
			pLLTLayout->addWidget(m_pctrlAlphaPrec,3,2);
			pLLTLayout->addWidget(m_pctrlIterMax,4,2);
		}
		pLLTBox->setLayout(pLLTLayout);
	}

	QGroupBox *pPanelBCBox = new QGroupBox(tr("3D Panel boundary conditions"));
	{
		QVBoxLayout *pPanelBCLayout = new QVBoxLayout;
		{
			m_pctrlDirichlet = new QRadioButton("Dirichlet (Recommended)");
			m_pctrlNeumann = new QRadioButton("Neumann");
			pPanelBCLayout->addWidget(m_pctrlDirichlet);
			pPanelBCLayout->addWidget(m_pctrlNeumann);
		}
		pPanelBCBox->setLayout((pPanelBCLayout));
	}

	QHBoxLayout *pCommandButtons = new QHBoxLayout;
	{
		OKButton = new QPushButton(tr("OK"));
		CancelButton = new QPushButton(tr("Cancel"));
		QPushButton *ResetButton = new QPushButton(tr("Reset Defaults"));
		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(OKButton);
		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(CancelButton);
		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(ResetButton);
		pCommandButtons->addStretch(1);
		connect(OKButton, SIGNAL(clicked()),this, SLOT(onOK()));
		connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
		connect(ResetButton, SIGNAL(clicked()), this, SLOT(onResetDefaults()));
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
		pMainLayout->addLayout(pCommandButtons);
	}

	setSizePolicy(szPolicyMaximum);

	setLayout(pMainLayout);
}



void WAdvancedDlg::keyPressEvent(QKeyEvent *event)
{
	// Prevent Return Key from closing App
	switch (event->key())
	{
		case Qt::Key_Escape:
		{
			done(0);
			return;
		}
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			if(!OKButton->hasFocus())
			{
				OKButton->setFocus();
				return;
			}
			break;
		}
		default:
			event->ignore();
			break;
	}
}



void WAdvancedDlg::initDialog()
{
	setParams();

	QString len;
	Units::getLengthUnitLabel(len);

	m_pctrlLength->setText(len);
	m_pctrlLength2->setText(len);

	m_pctrlVortexPos->setEnabled(false);
	m_pctrlControlPos->setEnabled(false);

	m_bDirichlet = true;
	m_pctrlDirichlet->setChecked(m_bDirichlet);
	m_pctrlNeumann->setChecked(!m_bDirichlet);
	m_pctrlDirichlet->setEnabled(false);
	m_pctrlNeumann->setEnabled(false);
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
	m_Relax           = m_pctrlRelax->value();
	m_AlphaPrec       = m_pctrlAlphaPrec->value();
	m_CoreSize        = m_pctrlCoreSize->value() / Units::mtoUnit();
	m_MinPanelSize    = m_pctrlMinPanelSize->value() / Units::mtoUnit();
	m_VortexPos       = m_pctrlVortexPos->value()/100.0;
	m_ControlPos      = m_pctrlControlPos->value()/100.0;
	m_Iter            = (int)m_pctrlIterMax->value();;
	m_NLLTStation     = (int)m_pctrlNStation->value();
	m_bDirichlet      = m_pctrlDirichlet->isChecked();
	m_bTrefftz        = true;
	m_bKeepOutOpps    = m_pctrlKeepOutOpps->isChecked();
	m_bLogFile        = m_pctrlLogFile->isChecked();
}


void WAdvancedDlg::setParams()
{
	m_pctrlIterMax->setValue(m_Iter);
	m_pctrlRelax->setValue(m_Relax);
	m_pctrlAlphaPrec->setValue(m_AlphaPrec);
	m_pctrlNStation->setValue(m_NLLTStation);

	m_pctrlCoreSize->setValue(m_CoreSize* Units::mtoUnit());

	m_pctrlMinPanelSize->setValue(m_MinPanelSize * Units::mtoUnit());

	m_pctrlLogFile->setChecked(m_bLogFile);
	m_pctrlKeepOutOpps->setChecked(m_bKeepOutOpps);

	m_pctrlControlPos->setValue(m_ControlPos*100.0);
	m_pctrlVortexPos->setValue(m_VortexPos*100.0);
}




