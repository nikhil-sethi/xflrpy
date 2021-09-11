/****************************************************************************

    FoilCoordDlg Class
    Copyright (C) 2009-2016 André Deperrois 

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

#include "foilcoorddlg.h"
#include <xflcore/displayoptions.h>
#include <xflobjects/objects2d/foil.h>
#include <xflwidgets/customwts/floateditdelegate.h>



FoilCoordDlg::FoilCoordDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Foil Coordinates"));

    m_pParent = pParent;

    m_pBufferFoil = nullptr;
    m_pMemFoil    = nullptr;
    m_bApplied  = true;
    m_bModified = false;

    setupLayout();

    connect(m_ppbDeletePoint, SIGNAL(clicked()), SLOT(onDeletePoint()));
    connect(m_ppbInsertPoint, SIGNAL(clicked()), SLOT(onInsertPoint()));
}


FoilCoordDlg::~FoilCoordDlg()
{
}


void FoilCoordDlg::setupLayout()
{
    QHBoxLayout *pTopLayout = new QHBoxLayout;
    {
        m_ptvCoordTable = new QTableView(this);
        {
            m_ptvCoordTable->setFont(DisplayOptions::tableFont());

            m_pCoordModel = new QStandardItemModel(this);
            m_pCoordModel->setRowCount(10);//temporary
            m_pCoordModel->setColumnCount(2);
            m_pCoordModel->setHeaderData(0, Qt::Horizontal, QObject::tr("X"));
            m_pCoordModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Y"));

            m_ptvCoordTable->setModel(m_pCoordModel);

            m_pFloatDelegate = new FloatEditDelegate(this);
            m_ptvCoordTable->setItemDelegate(m_pFloatDelegate);

            QVector<int> precision = {5,5};
            m_pFloatDelegate->setPrecision(precision);

            connect(m_pFloatDelegate, SIGNAL(closeEditor(QWidget *)), this, SLOT(onCellChanged(QWidget *)));

            QItemSelectionModel *pSelectionModel = new QItemSelectionModel(m_pCoordModel);
            m_ptvCoordTable->setSelectionModel(pSelectionModel);
            connect(pSelectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));
        }

        QVBoxLayout *pCommandButtonsLayout = new QVBoxLayout;
        {
            m_ppbInsertPoint  = new QPushButton(tr("Insert Point"));
            m_ppbDeletePoint  = new QPushButton(tr("Delete Point"));

            pCommandButtonsLayout->addStretch(1);
            pCommandButtonsLayout->addWidget(m_ppbInsertPoint);
            pCommandButtonsLayout->addWidget(m_ppbDeletePoint);
            pCommandButtonsLayout->addStretch(1);
        }

        pTopLayout->addWidget(m_ptvCoordTable);
        pTopLayout->addLayout(pCommandButtonsLayout);
    }

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Reset);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout * pMainLayout = new QVBoxLayout(this);
    {
        pMainLayout->addLayout(pTopLayout);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void FoilCoordDlg::fillList()
{
    m_pCoordModel->setRowCount(m_pBufferFoil->m_n);
    m_pCoordModel->setColumnCount(2);
    for (int i=0; i<m_pMemFoil->m_n; i++)
    {
        QModelIndex Xindex = m_pCoordModel->index(i, 0, QModelIndex());
        m_pCoordModel->setData(Xindex, m_pBufferFoil->m_x[i]);

        QModelIndex Yindex =m_pCoordModel->index(i, 1, QModelIndex());
        m_pCoordModel->setData(Yindex, m_pBufferFoil->m_y[i]);
    }
}


void FoilCoordDlg::initDialog()
{
    if(!m_pMemFoil || !m_pBufferFoil) return;

    int w = m_ptvCoordTable->width();
    m_ptvCoordTable->setColumnWidth(0,int(w/2));
    m_ptvCoordTable->setColumnWidth(1,int(w/2));
    QHeaderView *HorizontalHeader = m_ptvCoordTable->horizontalHeader();
    HorizontalHeader->setStretchLastSection(true);

    fillList();
}


void FoilCoordDlg::resizeEvent(QResizeEvent *pEvent)
{
    int w2 = int(double(m_ptvCoordTable->width())*.7/2);

    m_ptvCoordTable->setColumnWidth(0,w2);
    m_ptvCoordTable->setColumnWidth(1,w2);
    pEvent->accept();
}


void FoilCoordDlg::keyPressEvent(QKeyEvent *pEvent)
{
    // Prevent Return Key from closing App
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
                m_pButtonBox->setFocus();
                return;
            }
            else
            {
                accept();
                return;
            }
        }
        case Qt::Key_Escape:
        {
            reject();
            return;
        }
        default:
            pEvent->ignore();
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
    QModelIndex index = m_ptvCoordTable->currentIndex();
    sel = index.row();

    if(sel<0) return;

    for (i=sel;i<m_pBufferFoil->m_nb-1; i++)
    {
        m_pBufferFoil->m_xb[i] = m_pBufferFoil->m_xb[i+1];
        m_pBufferFoil->m_yb[i] = m_pBufferFoil->m_yb[i+1];
    }
    for (i=sel;i<m_pBufferFoil->m_n-1; i++)
    {
        m_pBufferFoil->m_x[i] = m_pBufferFoil->m_x[i+1];
        m_pBufferFoil->m_y[i] = m_pBufferFoil->m_y[i+1];
    }
    m_pBufferFoil->m_nb--;
    m_pBufferFoil->m_n--;

    fillList();
    setSelection(sel);
    m_bModified = true;

    m_pParent->update();
}


void FoilCoordDlg::onInsertPoint()
{
    int sel = m_ptvCoordTable->currentIndex().row();

    if(sel<=0) return;

    for (int i=m_pBufferFoil->m_nb; i>sel; i--)
    {
        m_pBufferFoil->m_xb[i] = m_pBufferFoil->m_xb[i-1];
        m_pBufferFoil->m_yb[i] = m_pBufferFoil->m_yb[i-1];
    }
    m_pBufferFoil->m_xb[sel] = (m_pBufferFoil->m_xb[sel-1] + m_pBufferFoil->m_xb[sel+1])/2.0;
    m_pBufferFoil->m_yb[sel] = (m_pBufferFoil->m_yb[sel-1] + m_pBufferFoil->m_yb[sel+1])/2.0 ;

    for (int i=m_pBufferFoil->m_n; i>sel; i--)
    {
        m_pBufferFoil->m_x[i] = m_pBufferFoil->m_x[i-1];
        m_pBufferFoil->m_y[i] = m_pBufferFoil->m_y[i-1];
    }
    m_pBufferFoil->m_x[sel] = (m_pBufferFoil->m_x[sel-1] + m_pBufferFoil->m_x[sel+1])/2.;
    m_pBufferFoil->m_y[sel] = (m_pBufferFoil->m_y[sel-1] + m_pBufferFoil->m_y[sel+1])/2.;

    m_pBufferFoil->m_nb++;
    m_pBufferFoil->m_n++;

    fillList();
    setSelection(sel);

    m_bModified = true;

    m_pParent->update();
}


void FoilCoordDlg::onCellChanged(QWidget *)
{
    int  sel = m_ptvCoordTable->currentIndex().row();

    QModelIndex Xindex = m_pCoordModel->index(sel, 0);
    double X = Xindex.data().toDouble();
    m_pBufferFoil->m_x[sel]  = X;
    m_pBufferFoil->m_xb[sel] = X;

    QModelIndex Yindex = m_pCoordModel->index(sel, 1);
    double Y = Yindex.data().toDouble();
    m_pBufferFoil->m_y[sel]  = Y;
    m_pBufferFoil->m_yb[sel] = Y;

    m_bApplied = false;

    onApply();
}


void FoilCoordDlg::onItemClicked(QModelIndex)
{
    int sel = m_ptvCoordTable->currentIndex().row();
    if(m_pBufferFoil)    m_pBufferFoil->setHighLight(sel);

    m_pParent->update();
}


void FoilCoordDlg::onRestore()
{
    for (int i=0; i<m_pMemFoil->m_nb; i++)
    {
        m_pBufferFoil->m_xb[i] = m_pMemFoil->m_xb[i];
        m_pBufferFoil->m_yb[i] = m_pMemFoil->m_yb[i];
    }
    m_pBufferFoil->m_nb = m_pMemFoil->m_n;
    for (int i=0; i<m_pMemFoil->m_n; i++)
    {
        m_pBufferFoil->m_x[i]  = m_pMemFoil->m_x[i];
        m_pBufferFoil->m_y[i]  = m_pMemFoil->m_y[i];
    }
    m_pBufferFoil->m_n = m_pMemFoil->m_n;


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
        m_ptvCoordTable->selectRow(sel);
    }
}


void FoilCoordDlg::onButton(QAbstractButton *pButton)
{
    if (     m_pButtonBox->button(QDialogButtonBox::Ok)     == pButton)  accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)  reject();
    else if (m_pButtonBox->button(QDialogButtonBox::Reset)  == pButton)  onRestore();
}


void FoilCoordDlg::showEvent(QShowEvent *pEvent)
{
    //setWindowModality(Qt::NonModal);
    setModal(true);
    pEvent->accept();
}



