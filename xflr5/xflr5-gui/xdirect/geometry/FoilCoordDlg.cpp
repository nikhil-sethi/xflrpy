/****************************************************************************

	FoilCoordDlg Class
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

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStringList>
#include <QHeaderView>

#include "FoilCoordDlg.h"
#include <misc/Settings.h>



FoilCoordDlg::FoilCoordDlg(QWidget *pParent) : QDialog(pParent)
{
	setWindowTitle(tr("Foil Coordinates"));

	m_pParent = pParent;

	m_pBufferFoil = NULL;
	m_pMemFoil    = NULL;
	setupLayout();

	m_bApplied  = true;
	m_bModified = false;
}


FoilCoordDlg::~FoilCoordDlg()
{
	if(m_precision) delete [] m_precision;
}

void FoilCoordDlg::fillList()
{
	m_pCoordModel->setRowCount(m_pBufferFoil->n);
	m_pCoordModel->setColumnCount(2);
	for (int i=0; i<m_pMemFoil->n; i++)
	{
		QModelIndex Xindex = m_pCoordModel->index(i, 0, QModelIndex());
		m_pCoordModel->setData(Xindex, m_pBufferFoil->x[i]);

		QModelIndex Yindex =m_pCoordModel->index(i, 1, QModelIndex());
		m_pCoordModel->setData(Yindex, m_pBufferFoil->y[i]);
	}
}


void FoilCoordDlg::initDialog()
{
	if(!m_pMemFoil || !m_pBufferFoil) return;

	int w = m_pctrlCoordTable->width();
	m_pctrlCoordTable->setColumnWidth(0,(int)(w/2));
	m_pctrlCoordTable->setColumnWidth(1,(int)(w/2));
	QHeaderView *HorizontalHeader = m_pctrlCoordTable->horizontalHeader();
	HorizontalHeader->setStretchLastSection(true);

	m_pCoordModel = new QStandardItemModel(this);
	m_pCoordModel->setRowCount(10);//temporary
	m_pCoordModel->setColumnCount(2);
	m_pCoordModel->setHeaderData(0, Qt::Horizontal, QObject::tr("X"));
	m_pCoordModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Y"));

	m_pctrlCoordTable->setModel(m_pCoordModel);

	m_pFloatDelegate = new FloatEditDelegate(this);
	m_pctrlCoordTable->setItemDelegate(m_pFloatDelegate);

	m_precision = new int[2];
	m_precision[0] = 5;//five digits for x and y coordinates
	m_precision[1] = 5;
	m_pFloatDelegate->setPrecision(m_precision);

//void QAbstractItemDelegate::closeEditor ( QWidget * editor, QAbstractItemDelegate::EndEditHint hint = NoHint )
	connect(m_pFloatDelegate, SIGNAL(closeEditor(QWidget *)), this, SLOT(onCellChanged(QWidget *)));

	QItemSelectionModel *selectionModel = new QItemSelectionModel(m_pCoordModel);
	m_pctrlCoordTable->setSelectionModel(selectionModel);
	connect(selectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));


//void QAbstractItemView::activated ( const QModelIndex & index )   [signal]
//void itemChanged ( QStandardItem * item )
	connect(m_pctrlApply, SIGNAL(clicked()),this, SLOT(onApply()));
	connect(m_pctrlDeletePoint, SIGNAL(clicked()),this, SLOT(onDeletePoint()));
	connect(m_pctrlInsertPoint, SIGNAL(clicked()),this, SLOT(onInsertPoint()));
	connect(m_pctrlRestore, SIGNAL(clicked()),this, SLOT(onRestore()));

	connect(OKButton, SIGNAL(clicked()),this, SLOT(accept()));
	connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	fillList();
}



void FoilCoordDlg::resizeEvent(QResizeEvent *event)
{
	int w2 = (int)((double)m_pctrlCoordTable->width()*.7/2);

	m_pctrlCoordTable->setColumnWidth(0,w2);
	m_pctrlCoordTable->setColumnWidth(1,w2);
	event->accept();
}


void FoilCoordDlg::keyPressEvent(QKeyEvent *event)
{
	// Prevent Return Key from closing App
	switch (event->key())
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			if(!OKButton->hasFocus() && !CancelButton->hasFocus())
			{
				OKButton->setFocus();
			}
			else
			{
				QDialog::accept();
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


void FoilCoordDlg::onApply()
{
	if(m_bApplied) return;
	m_bApplied = true;
	m_bModified = true;

	m_pParent->update();
}


void FoilCoordDlg::onDeletePoint()
{
	int i, sel;
	QModelIndex index = m_pctrlCoordTable->currentIndex();
	sel = index.row();

	if(sel<0) return;

	for (i=sel;i<m_pBufferFoil->nb-1; i++)
	{
		m_pBufferFoil->xb[i] = m_pBufferFoil->xb[i+1];
		m_pBufferFoil->yb[i] = m_pBufferFoil->yb[i+1];
	}
	for (i=sel;i<m_pBufferFoil->n-1; i++)
	{
		m_pBufferFoil->x[i] = m_pBufferFoil->x[i+1];
		m_pBufferFoil->y[i] = m_pBufferFoil->y[i+1];
	}
	m_pBufferFoil->nb--;
	m_pBufferFoil->n--;

	fillList();
	setSelection(sel);
	m_bModified = true;

	m_pParent->update();
}


void FoilCoordDlg::onInsertPoint()
{
	int i, sel;
	sel = m_pctrlCoordTable->currentIndex().row();


	if(sel<=0) return;

	for (i=m_pBufferFoil->nb; i>sel; i--)
	{
		m_pBufferFoil->xb[i] = m_pBufferFoil->xb[i-1];
		m_pBufferFoil->yb[i] = m_pBufferFoil->yb[i-1];
	}
	m_pBufferFoil->xb[sel] = (m_pBufferFoil->xb[sel-1] + m_pBufferFoil->xb[sel+1])/2.0;
	m_pBufferFoil->yb[sel] = (m_pBufferFoil->yb[sel-1] + m_pBufferFoil->yb[sel+1])/2.0 ;

	for (i=m_pBufferFoil->n; i>sel; i--)
	{
		m_pBufferFoil->x[i] = m_pBufferFoil->x[i-1];
		m_pBufferFoil->y[i] = m_pBufferFoil->y[i-1];
	}
	m_pBufferFoil->x[sel] = (m_pBufferFoil->x[sel-1] + m_pBufferFoil->x[sel+1])/2.;
	m_pBufferFoil->y[sel] = (m_pBufferFoil->y[sel-1] + m_pBufferFoil->y[sel+1])/2.;

	m_pBufferFoil->nb++;
	m_pBufferFoil->n++;

	fillList();
	setSelection(sel);

	m_bModified = true;

	m_pParent->update();
}




void FoilCoordDlg::onCellChanged(QWidget *)
{
	double X,Y;

	int  sel = m_pctrlCoordTable->currentIndex().row();

	QModelIndex Xindex = m_pCoordModel->index(sel, 0);
	X = Xindex.data().toDouble();
	m_pBufferFoil->x[sel]  = X;
	m_pBufferFoil->xb[sel] = X;

	QModelIndex Yindex = m_pCoordModel->index(sel, 1);
	Y = Yindex.data().toDouble();
	m_pBufferFoil->y[sel]  = Y;
	m_pBufferFoil->yb[sel] = Y;

	m_bApplied = false;

	onApply();
}


void FoilCoordDlg::onItemClicked(QModelIndex)
{
	int sel = m_pctrlCoordTable->currentIndex().row();
	if(m_pBufferFoil)	m_pBufferFoil->setHighLight(sel);

	m_pParent->update();
}


void FoilCoordDlg::onRestore()
{
	int i;

	for (i=0;i<m_pMemFoil->nb; i++)
	{
		m_pBufferFoil->xb[i] = m_pMemFoil->xb[i];
		m_pBufferFoil->yb[i] = m_pMemFoil->yb[i];
	}
	m_pBufferFoil->nb = m_pMemFoil->n;
	for (i=0;i<m_pMemFoil->n; i++)
	{
		m_pBufferFoil->x[i]  = m_pMemFoil->x[i];
		m_pBufferFoil->y[i]  = m_pMemFoil->y[i];
	}
	m_pBufferFoil->n = m_pMemFoil->n;


	fillList();
	m_bApplied = true;
	m_bModified = false;

	setSelection(0);
	m_pParent->update();
}


void FoilCoordDlg::readSectionData(int sel, double &X, double &Y)
{
	QModelIndex XIndex =m_pCoordModel->index(sel, 0, QModelIndex());
	X = XIndex.data().toDouble();
	QModelIndex YIndex =m_pCoordModel->index(sel, 0, QModelIndex());
	Y = YIndex.data().toDouble();

}

void FoilCoordDlg::setSelection(int sel)
{
	if(sel>=0)
	{
		m_pctrlCoordTable->selectRow(sel);
	}
}


void FoilCoordDlg::setupLayout()
{
	QVBoxLayout *pCommandButtonsLayout = new QVBoxLayout;
	{
		m_pctrlInsertPoint	= new QPushButton(tr("Insert Point"));
		m_pctrlDeletePoint	= new QPushButton(tr("Delete Point"));
		m_pctrlRestore      = new QPushButton(tr("Restore"));
		m_pctrlApply        = new QPushButton(tr("Apply"));
		OKButton            = new QPushButton(tr("OK"));
		CancelButton        = new QPushButton(tr("Cancel"));
		pCommandButtonsLayout->addStretch(1);
		pCommandButtonsLayout->addWidget(m_pctrlInsertPoint);
		pCommandButtonsLayout->addWidget(m_pctrlDeletePoint);
		pCommandButtonsLayout->addWidget(m_pctrlRestore);
		pCommandButtonsLayout->addWidget(m_pctrlApply);
		pCommandButtonsLayout->addStretch(2);
		pCommandButtonsLayout->addWidget(OKButton);
		pCommandButtonsLayout->addWidget(CancelButton);
		pCommandButtonsLayout->addStretch(1);
	}

	m_pctrlCoordTable = new QTableView(this);
	m_pctrlCoordTable->setFont(Settings::s_TableFont);
	m_pctrlCoordTable->setMinimumHeight(500);
	m_pctrlCoordTable->setMinimumWidth(150);

	QHBoxLayout * pMainLayout = new QHBoxLayout(this);
	{
		pMainLayout->addWidget(m_pctrlCoordTable);
		pMainLayout->addLayout(pCommandButtonsLayout);
	}
	setLayout(pMainLayout);
}


void FoilCoordDlg::showEvent(QShowEvent *event)
{
    //setWindowModality(Qt::NonModal);
    setModal(true);
//	Qt::WindowFlags flags = windowFlags();
//	flags = Qt::Dialog | Qt::WindowStaysOnTopHint;
//	setWindowFlags(flags);
	event->accept();
}



