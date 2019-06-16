/****************************************************************************

	FlapDlg class
	Copyright (C) 2004-2009 Andre Deperrois 

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
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

#include "FlapDlg.h"

#include <objects/objects2d/Foil.h>
#include <misc/text/DoubleEdit.h>


FlapDlg::FlapDlg(QWidget *pParent) : QDialog(pParent)
{
	setWindowTitle(tr("Flap Dlg"));

	m_pParent = pParent;

	m_pMemFoil    = nullptr;
	m_pBufferFoil = nullptr;
	m_bTEFlap     = false;
	m_TEFlapAngle = 0.0;
	m_TEXHinge    = 80.0;//%
	m_TEYHinge    = 50.0;//%
	m_bLEFlap     = false;
	m_LEFlapAngle = 0.0;
	m_LEXHinge    = 20.0;//%
	m_LEYHinge    = 50.0;//%

	m_bModified   = false;
	m_bApplied    = true;

	setupLayout();

	connect(ApplyButton, SIGNAL(clicked()),this, SLOT(onApply()));
	connect(OKButton, SIGNAL(clicked()),this, SLOT(OnOK()));
	connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	connect(m_pctrlLEFlapCheck, SIGNAL(stateChanged(int)), this, SLOT(onLEFlapCheck(int)));
	connect(m_pctrlTEFlapCheck, SIGNAL(stateChanged(int)), this, SLOT(onTEFlapCheck(int)));

	connect(m_pctrlLEXHinge, SIGNAL(editingFinished()), this, SLOT(onChanged()));
	connect(m_pctrlLEYHinge, SIGNAL(editingFinished()), this, SLOT(onChanged()));
	connect(m_pctrlTEXHinge, SIGNAL(editingFinished()), this, SLOT(onChanged()));
	connect(m_pctrlTEYHinge, SIGNAL(editingFinished()), this, SLOT(onChanged()));
	connect(m_pctrlLEFlapAngle, SIGNAL(editingFinished()), this, SLOT(onChanged()));
	connect(m_pctrlTEFlapAngle, SIGNAL(editingFinished()), this, SLOT(onChanged()));

}


void FlapDlg::setupLayout()
{
	QGridLayout *pFlapDataLayout = new QGridLayout;
	{
		m_pctrlLEFlapCheck = new QCheckBox(tr("L.E. Flap"));
		m_pctrlTEFlapCheck = new QCheckBox(tr("T.E. Flap"));
		m_pctrlLEXHinge    = new DoubleEdit;
		m_pctrlLEYHinge    = new DoubleEdit;
		m_pctrlTEXHinge    = new DoubleEdit;
		m_pctrlTEYHinge    = new DoubleEdit;
		m_pctrlTEFlapAngle = new DoubleEdit;
		m_pctrlLEFlapAngle = new DoubleEdit;

		QLabel *lab1 = new QLabel(tr("Flap Angle"));
		QLabel *lab2 = new QLabel(QString::fromUtf8("° (")+tr("+ is down") +")");
		QLabel *lab3 = new QLabel(tr("Hinge X Position"));
		QLabel *lab4 = new QLabel(tr("% Chord"));
		QLabel *lab5 = new QLabel(tr("Hinge Y Position"));
		QLabel *lab6 = new QLabel(tr("% Thickness"));

		pFlapDataLayout->addWidget(m_pctrlLEFlapCheck, 1, 2);
		pFlapDataLayout->addWidget(m_pctrlTEFlapCheck, 1, 3);
		pFlapDataLayout->addWidget(lab1, 2, 1);
		pFlapDataLayout->addWidget(m_pctrlLEFlapAngle, 2, 2);
		pFlapDataLayout->addWidget(m_pctrlTEFlapAngle, 2, 3);
		pFlapDataLayout->addWidget(lab2, 2, 4);
		pFlapDataLayout->addWidget(lab3, 3, 1);
		pFlapDataLayout->addWidget(m_pctrlLEXHinge, 3, 2);
		pFlapDataLayout->addWidget(m_pctrlTEXHinge, 3, 3);
		pFlapDataLayout->addWidget(lab4, 3, 4);
		pFlapDataLayout->addWidget(lab5, 4, 1);
		pFlapDataLayout->addWidget(m_pctrlLEYHinge, 4, 2);
		pFlapDataLayout->addWidget(m_pctrlTEYHinge, 4, 3);
		pFlapDataLayout->addWidget(lab6, 4, 4);
	}

	QHBoxLayout *pCommandButtons = new QHBoxLayout;
	{
		OKButton      = new QPushButton(tr("OK"));
		CancelButton  = new QPushButton(tr("Cancel"));
		ApplyButton  = new QPushButton(tr("Apply"));
	}

	pCommandButtons->addStretch(1);
	pCommandButtons->addWidget(ApplyButton);
	pCommandButtons->addStretch(1);
	pCommandButtons->addWidget(OKButton);
	pCommandButtons->addStretch(1);
	pCommandButtons->addWidget(CancelButton);
	pCommandButtons->addStretch(1);

	QVBoxLayout *MainLayout = new QVBoxLayout;
	MainLayout->addLayout(pFlapDataLayout);
	MainLayout->addLayout(pCommandButtons);
	setLayout(MainLayout);
}


void FlapDlg::initDialog()
{
	m_pctrlTEFlapCheck->setChecked(m_pMemFoil->m_bTEFlap);

	enableTEFlap(m_pMemFoil->m_bTEFlap);
	m_pctrlTEFlapAngle->setValue(m_pMemFoil->m_TEFlapAngle);
	m_pctrlTEXHinge->setValue(m_pMemFoil->m_TEXHinge);
	m_pctrlTEYHinge->setValue(m_pMemFoil->m_TEYHinge);

	m_pctrlLEFlapCheck->setChecked(m_pMemFoil->m_bLEFlap);
	enableLEFlap(m_pMemFoil->m_bLEFlap);
	m_pctrlLEFlapAngle->setValue(m_pMemFoil->m_LEFlapAngle);
	m_pctrlLEXHinge->setValue(m_pMemFoil->m_LEXHinge);
	m_pctrlLEYHinge->setValue(m_pMemFoil->m_LEYHinge);
}


void FlapDlg::readParams()
{
	m_bLEFlap = m_pctrlLEFlapCheck->isChecked();
	m_LEFlapAngle = m_pctrlLEFlapAngle->value();
	m_LEXHinge    = m_pctrlLEXHinge->value();
	m_LEYHinge    = m_pctrlLEYHinge->value();

	m_bTEFlap = m_pctrlTEFlapCheck->isChecked();
	m_TEFlapAngle = m_pctrlTEFlapAngle->value();
	m_TEXHinge    = m_pctrlTEXHinge->value();
	m_TEYHinge    = m_pctrlTEYHinge->value();

	if(m_LEXHinge>=m_TEXHinge && m_bLEFlap && m_bTEFlap)
	{
		QMessageBox::information(window(), tr("Warning"), tr("The trailing edge hinge must be downstream of the leading edge hinge"));
		m_pctrlLEXHinge->setFocus();
		m_pctrlLEXHinge->selectAll();
	}
}


void FlapDlg::onApply()
{
	if(m_bApplied) return;
	//reset everything and retry

	readParams();

	m_pBufferFoil->setTEFlapData(m_bTEFlap, m_TEXHinge, m_TEYHinge, m_TEFlapAngle);
	m_pBufferFoil->setLEFlapData(m_bLEFlap, m_LEXHinge, m_LEYHinge, m_LEFlapAngle);
	m_pBufferFoil->setFlap();

	m_bApplied = true;
	m_bModified = true;

	m_pParent->update();
}


void FlapDlg::keyPressEvent(QKeyEvent *event)
{
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
			if(!OKButton->hasFocus() && !CancelButton->hasFocus())
			{
				onApply();
				OKButton->setFocus();
				m_bApplied  = true;
			}
			else
			{
				QDialog::accept();
			}
			break;
		}
		default:
			event->ignore();
			break;
	}
}

void FlapDlg::enableLEFlap(bool bEnable)
{
	m_pctrlLEFlapAngle->setEnabled(bEnable);
	m_pctrlLEXHinge->setEnabled(bEnable);
	m_pctrlLEYHinge->setEnabled(bEnable);
}

void FlapDlg::enableTEFlap(bool bEnable)
{
	m_pctrlTEFlapAngle->setEnabled(bEnable);
	m_pctrlTEXHinge->setEnabled(bEnable);
	m_pctrlTEYHinge->setEnabled(bEnable);
}


void FlapDlg::onTEFlapCheck(int)
{
	if(m_pctrlTEFlapCheck->isChecked())
	{
		enableTEFlap(true);
		m_pctrlTEFlapAngle->setFocus();
	}
	else
		enableTEFlap(false);
	m_bApplied = false;
	onApply();
}


void FlapDlg::onLEFlapCheck(int)
{
	if(m_pctrlLEFlapCheck->isChecked())
	{
		enableLEFlap(true);
		m_pctrlLEFlapAngle->setFocus();
	}
	else
		enableLEFlap(false);
	m_bApplied = false;
	onApply();
}


void FlapDlg::onChanged()
{
	m_bApplied = false;
	onApply();
}


void FlapDlg::OnOK()
{
	if(!m_bApplied)
	{
		onApply();
		accept();
	}
	else done(1);
}
