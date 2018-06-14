/****************************************************************************

	FoilSelectionDlg Classes
		Copyright (C) 2009 Andre Deperrois 

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
#include <QHBoxLayout>
#include <QPushButton>
#include <objects/objects2d/Foil.h>
#include "FoilSelectionDlg.h"



FoilSelectionDlg::FoilSelectionDlg(QWidget *pParent) : QDialog(pParent)
{
	setWindowTitle(tr("Foil Selection"));
	m_poaFoil = NULL;
	m_FoilName = "";
	m_FoilList.clear();
	setupLayout();
}


void FoilSelectionDlg::setupLayout()
{
	QVBoxLayout *MainLayout = new QVBoxLayout;
	{
		m_pctrlNameList = new QListWidget;
		m_pctrlNameList->setMinimumHeight(300);
		m_pctrlNameList->setSelectionMode(QAbstractItemView::MultiSelection);

		QHBoxLayout *CommandButtons = new QHBoxLayout;
		{
			QPushButton *pSelectAllBtn = new QPushButton(tr("Select All"));
			QPushButton *pOKButton = new QPushButton(tr("OK"));
			pOKButton->setAutoDefault(false);
			QPushButton *pCancelButton = new QPushButton(tr("Cancel"));
			pCancelButton->setAutoDefault(false);
			CommandButtons->addStretch(1);
			CommandButtons->addWidget(pSelectAllBtn);
			CommandButtons->addStretch(1);
			CommandButtons->addWidget(pOKButton);
			CommandButtons->addStretch(1);
			CommandButtons->addWidget(pCancelButton);
			CommandButtons->addStretch(1);
			connect(pSelectAllBtn, SIGNAL(clicked()),this, SLOT(onSelectAll()));
			connect(pOKButton, SIGNAL(clicked()),this, SLOT(onOK()));
			connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
		}

		MainLayout->addStretch(1);
		MainLayout->addWidget(m_pctrlNameList);
		MainLayout->addStretch(1);
		MainLayout->addLayout(CommandButtons);
		MainLayout->addStretch(1);
	}

	connect(m_pctrlNameList, SIGNAL(itemClicked(QListWidgetItem *)),       this, SLOT(onSelChangeList(QListWidgetItem *)));
	connect(m_pctrlNameList, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(onDoubleClickList(QListWidgetItem *)));

	setLayout(MainLayout);
}



void FoilSelectionDlg::onSelectAll()
{
	m_pctrlNameList->selectAll();
}


void FoilSelectionDlg::onOK()
{
	QListWidgetItem *pItem =  m_pctrlNameList->currentItem();
	m_FoilName = pItem->text();
	
	m_FoilList.clear();
	for(int i=0; i<m_pctrlNameList->count();i++)
	{
		pItem = m_pctrlNameList->item(i);
		if(pItem->isSelected())
		{
			m_FoilList.append(pItem->text());
		}
	}

	accept();
}


void FoilSelectionDlg::onSelChangeList(QListWidgetItem *pItem)
{
	m_FoilName = pItem->text();
}


void FoilSelectionDlg::onDoubleClickList(QListWidgetItem *)
{
	onOK();
}


void FoilSelectionDlg::initDialog()
{
	if(!m_poaFoil) return;
	Foil *pFoil;

	for (int i=0; i<m_poaFoil->size(); i++)
	{
		pFoil = (Foil*)m_poaFoil->at(i);
		m_pctrlNameList->addItem(pFoil->foilName());
		m_pctrlNameList->setItemSelected(m_pctrlNameList->item(i), false);
		for(int j=0; j<m_FoilList.size();j++)
		{
			if(m_FoilList.at(j)==pFoil->foilName())
			{
				m_pctrlNameList->setItemSelected(m_pctrlNameList->item(i), true);
				break;
			}
		}
//		if(pFoil->foilName()==m_FoilName) m_pctrlNameList->setItemSelected(m_pctrlNameList->item(i), true);
//		else                              m_pctrlNameList->setItemSelected(m_pctrlNameList->item(i), false);
	}
}





