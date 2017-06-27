/****************************************************************************

	ManageUFOsDlg Class
	Copyright (C) 2009-2016 Andre Deperrois adeperrois@xflr5.com

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
  
#include <QHeaderView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStringList>
#include <QMessageBox>

#include "ManagePlanesDlg.h"
#include <miarex/Objects3D.h>
#include <misc/Units.h>
#include <globals.h>
#include <misc/Settings.h>



ManagePlanesDlg::ManagePlanesDlg(QWidget *pParent) : QDialog(pParent)
{
	setWindowTitle(tr("Plane Object Management"));

	m_pPlane     = NULL;
	m_pSelectionModel = NULL;
	m_pUFODelegate = NULL;
	m_pPrecision = NULL;

	m_bChanged = false;

	setupLayout();

	connect(m_pctrlDelete, SIGNAL(clicked()),this, SLOT(onDelete()));
	connect(m_pctrlRename, SIGNAL(clicked()),this, SLOT(onRename()));
	connect(m_pctrlUFOTable, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(onDoubleClickTable(const QModelIndex &)));
	connect(m_pctrlDescription, SIGNAL(textChanged()), this, SLOT(onDescriptionChanged()));
	connect(CloseButton, SIGNAL(clicked()),this, SLOT(accept()));
}


ManagePlanesDlg::~ManagePlanesDlg()
{
	if(m_pSelectionModel) delete m_pSelectionModel;
	if(m_pUFODelegate)    delete m_pUFODelegate;
	if(m_pPrecision)      delete [] m_pPrecision;
	if(m_pUFOModel)       delete m_pUFOModel;
}


void ManagePlanesDlg::initDialog(QString &UFOName)
{
	fillUFOTable();

	QString strong;
	QString strArea, strLength;
	Units::getLengthUnitLabel(strLength);
	Units::getAreaUnitLabel(strArea);
	m_pUFOModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
	m_pUFOModel->setHeaderData(1, Qt::Horizontal, tr("Span")+" ("+strLength+")");
	m_pUFOModel->setHeaderData(2, Qt::Horizontal, tr("Area")+" ("+strArea+")");
	m_pUFOModel->setHeaderData(3, Qt::Horizontal, tr("M.A.C.")+" ("+strLength+")");
	m_pUFOModel->setHeaderData(4, Qt::Horizontal, tr("AR"));
	m_pUFOModel->setHeaderData(5, Qt::Horizontal, tr("TR"));
	QString str = tr("Rt-Tip Sweep") +QString::fromUtf8(" (°)");
	m_pUFOModel->setHeaderData(6, Qt::Horizontal, str);
	m_pUFOModel->setHeaderData(7, Qt::Horizontal, tr("Tail Volume"));


	if(m_pUFOModel->rowCount())
	{
		if(UFOName.length())
		{
			QModelIndex ind;
			for(int i=0; i< m_pUFOModel->rowCount(); i++)
			{
				ind = m_pUFOModel->index(i, 0, QModelIndex());
				strong = ind.model()->data(ind, Qt::EditRole).toString();

				if(strong == UFOName)
				{
					m_pctrlUFOTable->selectRow(i);
					break;
				}
			}
		}
		else
		{
			m_pctrlUFOTable->selectRow(0);
			QStandardItem *pItem = m_pUFOModel->item(0,0);
			if(pItem) UFOName = pItem->text();
			else      UFOName.clear();
		}
		m_pPlane = Objects3D::getPlane(UFOName);
	}
	else
	{
		m_pPlane = NULL;
	}
}


void ManagePlanesDlg::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			if(!CloseButton->hasFocus()) CloseButton->setFocus();
			else                         accept();

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


void ManagePlanesDlg::setupLayout()
{
	QVBoxLayout *pCommandButtons = new QVBoxLayout;
	{
		m_pctrlDelete     = new QPushButton(tr("Delete"));
		m_pctrlRename     = new QPushButton(tr("Rename"));

		CloseButton     = new QPushButton(tr("Close"));

		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(m_pctrlRename);
		pCommandButtons->addWidget(m_pctrlDelete);
		pCommandButtons->addStretch(2);
		pCommandButtons->addWidget(CloseButton);
		pCommandButtons->addStretch(1);
	}


	QVBoxLayout *pLeftLayout = new QVBoxLayout;
	{
		m_pctrlUFOTable = new QTableView(this);
		m_pctrlUFOTable->setFont(Settings::s_TableFont);

		m_pctrlUFOTable->setSelectionMode(QAbstractItemView::SingleSelection);
		m_pctrlUFOTable->setSelectionBehavior(QAbstractItemView::SelectRows);
		m_pctrlUFOTable->horizontalHeader()->setStretchLastSection(true);

		QSizePolicy szPolicyExpanding;
		szPolicyExpanding.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
		szPolicyExpanding.setVerticalPolicy(QSizePolicy::Expanding);
		m_pctrlUFOTable->setSizePolicy(szPolicyExpanding);
		m_pctrlUFOTable->setMinimumWidth(800);

		m_pctrlDescription = new QTextEdit;
	//	m_pctrlDescription->setEnabled(false);
		QLabel *Description = new QLabel(tr("Description:"));
		pLeftLayout->addWidget(m_pctrlUFOTable);
		pLeftLayout->addWidget(Description);
		pLeftLayout->addWidget(m_pctrlDescription);
		pLeftLayout->setStretchFactor(m_pctrlUFOTable, 5);
		pLeftLayout->setStretchFactor(m_pctrlDescription, 1);
	}

	QHBoxLayout * pMainLayout = new QHBoxLayout(this);
	{
		pMainLayout->addLayout(pLeftLayout);
		pMainLayout->addLayout(pCommandButtons);
	}
	setLayout(pMainLayout);

	m_pUFOModel = new QStandardItemModel;
	m_pUFOModel->setRowCount(10);//temporary
	m_pUFOModel->setColumnCount(8);

	m_pctrlUFOTable->setModel(m_pUFOModel);
	m_pctrlUFOTable->setWindowTitle(tr("UFOs"));

	m_pSelectionModel = new QItemSelectionModel(m_pUFOModel);
	m_pctrlUFOTable->setSelectionModel(m_pSelectionModel);
	connect(m_pSelectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onPlaneClicked(QModelIndex)));

	m_pUFODelegate = new PlaneTableDelegate;

	m_pctrlUFOTable->setItemDelegate(m_pUFODelegate);
	m_pUFODelegate->m_pUFOModel = m_pUFOModel;

	m_pPrecision = new int[12];
	m_pPrecision[0]  = 2;
	m_pPrecision[1]  = 3;
	m_pPrecision[2]  = 3;
	m_pPrecision[3]  = 3;
	m_pPrecision[4]  = 2;
	m_pPrecision[5]  = 2;
	m_pPrecision[6]  = 1;
	m_pPrecision[7]  = 2;
	m_pPrecision[8]  = 3;

	m_pUFODelegate->m_Precision = m_pPrecision;
}



void ManagePlanesDlg::fillUFOTable()
{
	int i;
	m_pUFOModel->setRowCount(Objects3D::s_oaPlane.size());

	for(i=0; i<Objects3D::s_oaPlane.size(); i++)
	{
		fillPlaneRow(i);
	}
}




void ManagePlanesDlg::fillPlaneRow(int row)
{
	QModelIndex ind;

	if(row>=Objects3D::s_oaPlane.size()) return;

	Plane *pPlane = (Plane*)Objects3D::s_oaPlane.at(row);
	if(!pPlane) return;
	Wing *pWing = pPlane->wing();

	ind = m_pUFOModel->index(row, 0, QModelIndex());
	m_pUFOModel->setData(ind,pPlane->planeName());
	if(pPlane->planeDescription().length()) m_pUFOModel->setData(ind, pPlane->planeDescription(), Qt::ToolTipRole);

	ind = m_pUFOModel->index(row, 1, QModelIndex());
	m_pUFOModel->setData(ind, pWing->m_PlanformSpan*Units::mtoUnit());

	ind = m_pUFOModel->index(row, 2, QModelIndex());
	m_pUFOModel->setData(ind, pWing->m_PlanformArea*Units::m2toUnit());

	ind = m_pUFOModel->index(row, 3, QModelIndex());
	m_pUFOModel->setData(ind, pWing->m_MAChord*Units::mtoUnit());

	ind = m_pUFOModel->index(row, 4, QModelIndex());
	m_pUFOModel->setData(ind, pWing->m_AR);

	ind = m_pUFOModel->index(row, 5, QModelIndex());
	m_pUFOModel->setData(ind, pWing->m_TR);

	ind = m_pUFOModel->index(row, 6, QModelIndex());
	m_pUFOModel->setData(ind,pWing->averageSweep());

	ind = m_pUFOModel->index(row, 7, QModelIndex());
	m_pUFOModel->setData(ind,pPlane->tailVolume());
}


void ManagePlanesDlg::onRename()
{
	if(m_pPlane)      Objects3D::renamePlane(m_pPlane->planeName());

	fillUFOTable();
	m_bChanged = true;
}


void ManagePlanesDlg::onDelete()
{
	if(!m_pPlane) return;

	QString strong;
	if(m_pPlane) strong = tr("Are you sure you want to delete the plane :\n") +  m_pPlane->planeName() +"?\n";

	if (QMessageBox::Yes != QMessageBox::question(window(), tr("Question"), strong,
										 QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
										 QMessageBox::Yes)) return;

    if(m_pPlane)
    {
        Objects3D::deletePlaneResults(m_pPlane, true);
        Objects3D::deletePlane(m_pPlane);
    }
	QModelIndex index = m_pctrlUFOTable->currentIndex();
	int sel = qMax(index.row()-1,0);

	fillUFOTable();

	if(m_pUFOModel->rowCount()>0)
	{
		m_pctrlUFOTable->selectRow(sel);
		QString UFOName;

		QStandardItem *pItem = m_pUFOModel->item(sel,0);
		if(pItem) UFOName = pItem->text();
		else      UFOName.clear();

		m_pPlane = Objects3D::getPlane(UFOName);
	}
	else
	{
		m_pPlane = NULL;
	}

	m_bChanged = true;

}


void ManagePlanesDlg::onDoubleClickTable(const QModelIndex &index)
{
	if(index.row()>=0) accept();
}


void ManagePlanesDlg::onDescriptionChanged()
{
	if(m_pPlane)
	{
		m_pPlane->rPlaneDescription() = m_pctrlDescription->toPlainText();
	}
	m_bChanged = true;
}


void ManagePlanesDlg::onPlaneClicked(QModelIndex index)
{
	QStandardItem *pItem = m_pUFOModel->item(index.row(),0);
	QString UFOName;
	if(pItem) UFOName = pItem->text();
	else      UFOName.clear();


	m_pPlane = Objects3D::getPlane(UFOName);
	if(m_pPlane) m_pctrlDescription->setText(m_pPlane->planeDescription());

}


void ManagePlanesDlg::resizeEvent(QResizeEvent *event)
{
	int w = m_pctrlUFOTable->width();
	int w8 = (int)((double)w/12.0);

	m_pctrlUFOTable->setColumnWidth(1,w8);
	m_pctrlUFOTable->setColumnWidth(3,w8);
	m_pctrlUFOTable->setColumnWidth(2,w8);
	m_pctrlUFOTable->setColumnWidth(4,w8);
	m_pctrlUFOTable->setColumnWidth(5,w8);
	m_pctrlUFOTable->setColumnWidth(6,w8);
	m_pctrlUFOTable->setColumnWidth(7,w8);
	
	m_pctrlUFOTable->setColumnWidth(0,w-7*w8-40);
	event->accept();
}


