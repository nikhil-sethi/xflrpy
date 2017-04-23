/****************************************************************************

	ImportWingDlg Class
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


#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QLabel>
#include <QPushButton>

#include <miarex/Objects3D.h>
#include <objects3d/Plane.h>
#include "ImportObjectDlg.h"


ImportObjectDlg::ImportObjectDlg(QWidget *pParent):QDialog(pParent)
{
	m_bWing = m_bBody = false;
	setWindowTitle(tr("Import Object"));
	SetupLayout();
}


void ImportObjectDlg::initDialog(bool bWing)
{
	if(bWing)
	{
		m_bWing = true;
		m_bBody = false;
		m_pctrlQuestion->setText(tr("Select the wing to import"));

		Plane *pPlane;
		for(int ip=0; ip<Objects3D::s_oaPlane.size(); ip++)
		{
			pPlane = (Plane*)Objects3D::s_oaPlane.at(ip);

			if(pPlane->planeName() != m_ObjectName)
				m_pctrlNameList->addItem(pPlane->planeName()+"/Main wing");
		}
	}
	else
	{
		m_bWing = false;
		m_bBody = true;
		m_pctrlQuestion->setText(tr("Select the body to import"));

		//list all bodies not attached to a plane... remnants from versions < 6.09.06
		Body *pBody;
		for(int ib=0; ib<Objects3D::s_oaBody.size(); ib++)
		{
			pBody = (Body*)Objects3D::s_oaBody.at(ib);
			m_pctrlNameList->addItem(pBody->m_BodyName);
		}

		Plane *pPlane;
		for(int ip=0; ip<Objects3D::s_oaPlane.size(); ip++)
		{
			pPlane = (Plane*)Objects3D::s_oaPlane.at(ip);
			if(pPlane->body())
			{
				if(pPlane->m_BodyName != m_ObjectName)
					m_pctrlNameList->addItem(pPlane->planeName()+"/Body");
			}
		}
	}
}


void ImportObjectDlg::SetupLayout()
{
	QDesktopWidget desktop;
	QRect r = desktop.geometry();
	setMinimumHeight(r.height()/3);
	move(r.width()/3, r.height()/6);

	QVBoxLayout *MainLayout = new QVBoxLayout;

	m_pctrlQuestion = new QLabel;

	m_pctrlNameList = new QListWidget;
	m_pctrlNameList->setMinimumHeight(300);
	connect(m_pctrlNameList, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(OnOK()));

	QHBoxLayout *CommandButtons = new QHBoxLayout;
	{
		QPushButton *OKButton = new QPushButton(tr("OK"));
		OKButton->setAutoDefault(false);
		QPushButton *CancelButton = new QPushButton(tr("Cancel"));
		CancelButton->setAutoDefault(false);
		CommandButtons->addStretch(1);
		CommandButtons->addWidget(OKButton);
		CommandButtons->addStretch(1);
		CommandButtons->addWidget(CancelButton);
		CommandButtons->addStretch(1);
		connect(OKButton, SIGNAL(clicked()),this, SLOT(OnOK()));
		connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	}

	MainLayout->addStretch(1);
	MainLayout->addWidget(m_pctrlQuestion);
	MainLayout->addWidget(m_pctrlNameList);
	MainLayout->addStretch(1);
	MainLayout->addLayout(CommandButtons);
	MainLayout->addStretch(1);

	setLayout(MainLayout);
}



void ImportObjectDlg::OnOK()
{
	QListWidgetItem *pItem =  m_pctrlNameList->currentItem();
	m_ObjectName = pItem->text();
	QDialog::accept();
}

