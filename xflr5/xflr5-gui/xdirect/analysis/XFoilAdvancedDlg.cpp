/****************************************************************************

	XFoilAdvancedDlg Class
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

#include <QLabel>
#include <xdirect/XDirect.h>
#include "XFoilAdvancedDlg.h"


XFoilAdvancedDlg::XFoilAdvancedDlg(QWidget *pParent) : QDialog(pParent)
{
	setWindowTitle(tr("XFoil Settings"));
	SetupLayout();

	m_IterLimit = 100;
	m_VAccel = 0.001;
	m_bAutoInitBL = true;
	m_bFullReport = false;

	connect(m_pctrlDefaults, SIGNAL(clicked()), SLOT(OnDefaults()));
	connect(OKButton, SIGNAL(clicked()),this, SLOT(OnOK()));
	connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}



void XFoilAdvancedDlg::SetupLayout()
{
	QHBoxLayout *pVAccelBoxLayout = new QHBoxLayout;
	{
		QLabel *lab1 = new QLabel(tr("VAccel"));
		lab1->setAlignment(Qt::AlignRight);
		m_pctrlVAccel = new DoubleEdit;
		m_pctrlVAccel->setAlignment(Qt::AlignRight);
		pVAccelBoxLayout->addStretch(1);
		pVAccelBoxLayout->addWidget(lab1);
		pVAccelBoxLayout->addWidget(m_pctrlVAccel);
	}

	QHBoxLayout *pIterBoxLayout = new QHBoxLayout;
	{
		QLabel *lab2 = new QLabel(tr("Iteration Limit"));
		lab2->setAlignment(Qt::AlignRight);
		m_pctrlIterLimit = new IntEdit;

		pIterBoxLayout->addStretch(1);
		pIterBoxLayout->addWidget(lab2);
		pIterBoxLayout->addWidget(m_pctrlIterLimit);
	}

	m_pctrlInitBL = new QCheckBox(tr("Re-initialize BLs after an unconverged iteration"));
	m_pctrlFullReport = new QCheckBox(tr("Show full log report for an XFoil analysis"));
	m_pctrlKeepErrorsOpen = new QCheckBox(tr("Keep Xfoil interface open if analysis errors"));

	QHBoxLayout *pTimerLayout = new QHBoxLayout;
	{
		QLabel *pTimerLabel = new QLabel(tr("Time interval between graph updates"));
		QLabel *pTimerUnitLabel = new QLabel("ms");
		m_pctrlTimerInterval = new IntEdit(XDirect::s_TimeUpdateInterval, this);
		m_pctrlTimerInterval->setMin(0);
		pTimerLayout->addStretch();
		pTimerLayout->addWidget(pTimerLabel);
		pTimerLayout->addWidget(m_pctrlTimerInterval);
		pTimerLayout->addWidget(pTimerUnitLabel);
	}

	QHBoxLayout *pCommandButtonsLayout = new QHBoxLayout;
	{
		m_pctrlDefaults = new QPushButton(tr("Reset Defaults"));
		OKButton      = new QPushButton(tr("OK"));
		CancelButton  = new QPushButton(tr("Cancel"));
		pCommandButtonsLayout->addStretch(1);
		pCommandButtonsLayout->addWidget(m_pctrlDefaults);
		pCommandButtonsLayout->addStretch(1);
		pCommandButtonsLayout->addWidget(OKButton);
		pCommandButtonsLayout->addStretch(1);
		pCommandButtonsLayout->addWidget(CancelButton);
		pCommandButtonsLayout->addStretch(1);
	}

	QVBoxLayout *pMainLayout = new QVBoxLayout;
	{
		pMainLayout->addStretch();
		pMainLayout->addLayout(pVAccelBoxLayout);
		pMainLayout->addLayout(pIterBoxLayout);
		pMainLayout->addWidget(m_pctrlInitBL);
		pMainLayout->addWidget(m_pctrlFullReport);
		pMainLayout->addWidget(m_pctrlKeepErrorsOpen);
		pMainLayout->addLayout(pTimerLayout);
		pMainLayout->addStretch();
		pMainLayout->addSpacing(15);
		pMainLayout->addLayout(pCommandButtonsLayout);
	}

	setLayout(pMainLayout);
}


void XFoilAdvancedDlg::OnDefaults()
{
	m_IterLimit = 100;
	m_VAccel = 0.001;
	m_bAutoInitBL = true;
	m_bFullReport = false;
	XDirect::s_bKeepOpenErrors = true;
	XDirect::s_TimeUpdateInterval = 100;
	initDialog();
}


void XFoilAdvancedDlg::initDialog()
{
	m_pctrlVAccel->setValue(m_VAccel);
	m_pctrlInitBL->setChecked(m_bAutoInitBL);
	m_pctrlIterLimit->setValue(m_IterLimit);
	m_pctrlFullReport->setChecked(m_bFullReport);
	m_pctrlKeepErrorsOpen->setChecked(XDirect::s_bKeepOpenErrors);
	m_pctrlTimerInterval->setValue(XDirect::s_TimeUpdateInterval);
}




void XFoilAdvancedDlg::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			if(!OKButton->hasFocus() && !CancelButton->hasFocus())
			{
				OKButton->setFocus();
			}
			else if (OKButton->hasFocus())
			{
				OnOK();
			}
			break;
		}
		case Qt::Key_Escape:
		{
			reject();
			return;
		}
		default:
			event->ignore();
	}
}



void XFoilAdvancedDlg::OnOK()
{
	m_IterLimit = m_pctrlIterLimit->value();
	m_VAccel = m_pctrlVAccel->value();
	m_bAutoInitBL = m_pctrlInitBL->isChecked();
	m_bFullReport = m_pctrlFullReport->isChecked();
	XDirect::s_TimeUpdateInterval = m_pctrlTimerInterval->value();
	XDirect::s_bKeepOpenErrors = m_pctrlKeepErrorsOpen->isChecked();
	done(1);
}
