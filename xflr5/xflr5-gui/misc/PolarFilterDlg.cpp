/****************************************************************************

	PolarFilterDlg Class
	Copyright (C) 2009 Andre Deperrois adeperrois@xflr5.com

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
#include <QVBoxLayout>
#include "PolarFilterDlg.h"


PolarFilterDlg::PolarFilterDlg(QWidget *pParent) : QDialog(pParent)
{
	setWindowTitle(tr("Polar Filter"));

	m_bType1 = m_bType2 = m_bType3 = m_bType4 = m_bType7 = true;

	m_bMiarex = false;
	SetupLayout();
}


void PolarFilterDlg::SetupLayout()
{
	QVBoxLayout *MainLayout = new QVBoxLayout;
	{
		QLabel *Label = new QLabel(tr("Show polar types"));

		m_pctrlType1 = new QCheckBox(tr("Type 1")+" - "+tr("fixed speed polars"));
		m_pctrlType2 = new QCheckBox(tr("Type 2")+" - "+tr("fixed lift polars"));
		m_pctrlType3 = new QCheckBox(tr("Type 3")+" - "+tr("XFoil rubber chord polars"));
		m_pctrlType4 = new QCheckBox(tr("Type 4")+" - "+tr("fixed a.o.a. polars"));
		m_pctrlType7 = new QCheckBox(tr("Type 7")+" - "+tr("stability polars"));

		QHBoxLayout *CommandButtons = new QHBoxLayout;
		{
			QPushButton *CancelButton = new QPushButton(tr("Cancel"));
			OKButton= new QPushButton(tr("OK"));


			CancelButton->setAutoDefault(false);
			OKButton->setAutoDefault(false);
			connect(OKButton, SIGNAL(clicked()),this, SLOT(OnOK()));
			connect(CancelButton, SIGNAL(clicked()),this, SLOT(reject()));
			CommandButtons->addStretch(1);
			CommandButtons->addWidget(OKButton);
			CommandButtons->addStretch(1);
			CommandButtons->addWidget(CancelButton);
			CommandButtons->addStretch(1);
		}

		MainLayout->addWidget(Label);
		MainLayout->addWidget(m_pctrlType1);
		MainLayout->addWidget(m_pctrlType2);
		MainLayout->addWidget(m_pctrlType3);
		MainLayout->addWidget(m_pctrlType4);
//		MainLayout->addWidget(m_pctrlType5);
//		MainLayout->addWidget(m_pctrlType6);
		MainLayout->addWidget(m_pctrlType7);
		MainLayout->addStretch(1);
		MainLayout->addLayout(CommandButtons);
		MainLayout->addStretch(1);
	}
	setLayout(MainLayout);

}


void PolarFilterDlg::InitDialog()
{
	m_pctrlType1->setChecked(m_bType1);
	m_pctrlType2->setChecked(m_bType2);
	m_pctrlType3->setChecked(m_bType3);
	m_pctrlType4->setChecked(m_bType4);
	m_pctrlType7->setChecked(m_bType7);

	if(m_bMiarex)
	{
		m_pctrlType3->setEnabled(false);
		m_pctrlType3->setChecked(false);
	}
	else
	{
		m_pctrlType7->setChecked(false);
		m_pctrlType7->setEnabled(false);
	}
}


void PolarFilterDlg::OnOK()
{
	m_bType1 = m_pctrlType1->isChecked();
	m_bType2 = m_pctrlType2->isChecked();
	m_bType3 = m_pctrlType3->isChecked();
	m_bType4 = m_pctrlType4->isChecked();
	m_bType7 = m_pctrlType7->isChecked();

	QDialog::accept();
}



void PolarFilterDlg::keyPressEvent(QKeyEvent *event)
{
	// Prevent Return Key from closing App
	switch (event->key())
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			if(!OKButton->hasFocus())
			{
				OKButton->setFocus();
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
	}
}


