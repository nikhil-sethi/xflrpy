/****************************************************************************

	LECircleDlg Class
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

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include "LECircleDlg.h"
#include "AFoil.h"



LECircleDlg::LECircleDlg(QWidget *pParent): QDialog(pParent)
{
	setWindowTitle(tr("L.E. Circle"));
	SetupLayout();
}


void LECircleDlg::SetupLayout()
{
	QHBoxLayout *LERadius = new QHBoxLayout;
	m_pctrlRadius = new DoubleEdit(0.0,3);
	QLabel *lab0 = new QLabel(tr("r="));
	QLabel *lab1 = new QLabel(tr("% Chord"));
	lab0->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	LERadius->addStretch(1);
	LERadius->addWidget(lab0);
	LERadius->addWidget(m_pctrlRadius);
	LERadius->addWidget(lab1);

	m_pctrlShow = new QCheckBox(tr("Show"));

	QHBoxLayout *CommandButtons = new QHBoxLayout;
	OKButton     = new QPushButton(tr("OK"));
	CancelButton = new QPushButton(tr("Cancel"));

	CommandButtons->addStretch(1);
	CommandButtons->addWidget(OKButton);
	CommandButtons->addStretch(1);
	CommandButtons->addWidget(CancelButton);
	CommandButtons->addStretch(1);

	QVBoxLayout *MainLayout = new QVBoxLayout;
	MainLayout->addWidget(m_pctrlShow);
	MainLayout->addStretch(1);
	MainLayout->addLayout(LERadius);
	MainLayout->addStretch(1);
	MainLayout->addLayout(CommandButtons);
	MainLayout->addStretch(1);

	setLayout(MainLayout);

	connect(OKButton, SIGNAL(clicked()),this, SLOT(OnOK()));
	connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}


void LECircleDlg::InitDialog()
{
	m_pctrlRadius->setValue(m_Radius);
	m_pctrlShow->setChecked(m_bShowRadius);
}


void LECircleDlg::keyPressEvent(QKeyEvent *event)
{
	// Prevent Return Key from closing App
	switch (event->key())
	{
		case Qt::Key_Escape:
		{
			reject();
			return;
		}
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			if(!OKButton->hasFocus() && !CancelButton->hasFocus())
			{
				OKButton->setFocus();
			}
			else
			{
				OnOK();
			}
			break;
		}
		default:
			event->ignore();
			break;
	}
}


void LECircleDlg::OnOK()
{
	m_Radius = m_pctrlRadius->value();
	m_bShowRadius = m_pctrlShow->isChecked();
	accept();
}



