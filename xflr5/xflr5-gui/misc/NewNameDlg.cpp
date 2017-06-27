/****************************************************************************

	NewNameDlg Classes
        Copyright (C) 2010 Andre Deperrois adeperrois@xflr5.com

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


#include "NewNameDlg.h"
#include <QHBoxLayout>
#include <QVBoxLayout>



NewNameDlg::NewNameDlg(QWidget *pParent) : QDialog(pParent)
{
	setWindowTitle("Curve Name Dialog");
	SetupLayout();
}

void NewNameDlg::InitDialog()
{
	m_pctrlName->setText(m_OldName);
	m_pctrlName->selectAll();
}

void NewNameDlg::SetupLayout()
{
	QVBoxLayout *MainLayout = new QVBoxLayout;
	m_pctrlName = new QLineEdit(this);
	QHBoxLayout *CommandButtons = new QHBoxLayout;
	OKButton = new QPushButton(tr("OK"));
	OKButton->setAutoDefault(false);
	QPushButton *CancelButton = new QPushButton(tr("Cancel"));
	CancelButton->setAutoDefault(false);
	CommandButtons->addStretch(1);
	CommandButtons->addWidget(OKButton);
	CommandButtons->addStretch(1);
	CommandButtons->addWidget(CancelButton);
	CommandButtons->addStretch(1);
	
	MainLayout->addStretch(1);
	MainLayout->addWidget(m_pctrlName);
	MainLayout->addStretch(1);
	MainLayout->addLayout(CommandButtons);	
	setLayout(MainLayout);
	
	connect(OKButton, SIGNAL(clicked()),this, SLOT(OnOK()));
	connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void NewNameDlg::OnOK()
{
	m_NewName = m_pctrlName->text();
	accept();
}


void NewNameDlg::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			if(!OKButton->hasFocus()) OKButton->setFocus();
			else OnOK();
			 
			break;
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
