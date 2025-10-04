/****************************************************************************

    EditPlrDlg Class
    Copyright (C) André Deperrois

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


#include "editplrdlg.h"
#include <miarex/miarex.h>

#include <xdirect/xdirect.h>
#include <xflcore/displayoptions.h>
#include <xflobjects/objects2d/polar.h>
#include <xflobjects/objects3d/wpolar.h>
#include <xflwidgets/customwts/floateditdelegate.h>

QByteArray EditPlrDlg::s_Geometry;

EditPlrDlg::EditPlrDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Polar Points Edition"));

    m_pXDirect    = nullptr;
    m_pMiarex     = nullptr;

    m_pPolar      = nullptr;
    m_pWPolar     = nullptr;

    m_ptvPoints = nullptr;
    m_pPointModel     = nullptr;
    m_pFloatDelegate  = nullptr;

    setupLayout();
}

EditPlrDlg::~EditPlrDlg()
{
}

void EditPlrDlg::initDialog(XDirect *pXDirect, Polar* pPolar, Miarex *pMiarex, WPolar*pWPolar)
{
    m_pXDirect = pXDirect;
    m_pMiarex = pMiarex;
    m_pPolar = pPolar;
    m_pWPolar = pWPolar;


    m_pPointModel = new QStandardItemModel(this);
    m_pPointModel->setRowCount(10);//temporary
    m_pPointModel->setColumnCount(14);

    m_pPointModel->setHeaderData(0, Qt::Horizontal, "Alpha");
    m_pPointModel->setHeaderData(1, Qt::Horizontal, "Cl");
    m_pPointModel->setHeaderData(2, Qt::Horizontal, "Cd");
    m_pPointModel->setHeaderData(3, Qt::Horizontal, "Cm");
    m_pPointModel->setHeaderData(4, Qt::Horizontal, "XTr_top");
    m_pPointModel->setHeaderData(5, Qt::Horizontal, "XTr_bot");
    m_pPointModel->setHeaderData(6, Qt::Horizontal, "Cl/Cd");
    m_pPointModel->setHeaderData(7, Qt::Horizontal, "Cl^3/2/Cd");
    m_pPointModel->setHeaderData(8, Qt::Horizontal, "sqrt(Cl)");
    m_pPointModel->setHeaderData(9, Qt::Horizontal, "XCp");
    m_pPointModel->setHeaderData(10, Qt::Horizontal, "HMom");
    m_pPointModel->setHeaderData(11, Qt::Horizontal, "Cdp");
    m_pPointModel->setHeaderData(12, Qt::Horizontal, "Cpmn");
    m_pPointModel->setHeaderData(13, Qt::Horizontal, "Re");

    m_ptvPoints->setModel(m_pPointModel);


    QHeaderView *HorizontalHeader = m_ptvPoints->horizontalHeader();
    HorizontalHeader->setStretchLastSection(true);

    m_pFloatDelegate = new FloatEditDelegate(this);
    m_ptvPoints->setItemDelegate(m_pFloatDelegate);

    QVector<int>  m_precision(14, 3);
    m_precision.last() = 0;

    m_pFloatDelegate->setPrecision(m_precision);

    if(m_pXDirect && pPolar)        fillPolarData();
    else if(m_pMiarex && m_pWPolar) fillWPolarData();
}


void EditPlrDlg::fillPolarData()
{
    m_pPointModel->setColumnCount(14);
    m_pPointModel->setRowCount(m_pPolar->m_Alpha.size());
    QModelIndex index;
    for (int i=0; i<m_pPolar->m_Alpha.size(); i++)
    {
        index = m_pPointModel->index(i, 0, QModelIndex());
        m_pPointModel->setData(index, m_pPolar->m_Alpha.at(i));

        index = m_pPointModel->index(i, 1, QModelIndex());
        m_pPointModel->setData(index, m_pPolar->m_Cl.at(i));

        index = m_pPointModel->index(i, 2, QModelIndex());
        m_pPointModel->setData(index, m_pPolar->m_Cd.at(i));

        index = m_pPointModel->index(i, 3, QModelIndex());
        m_pPointModel->setData(index, m_pPolar->m_Cm.at(i));

        index = m_pPointModel->index(i, 4, QModelIndex());
        m_pPointModel->setData(index, m_pPolar->m_XTr1.at(i));

        index = m_pPointModel->index(i, 5, QModelIndex());
        m_pPointModel->setData(index, m_pPolar->m_XTr2.at(i));

        index = m_pPointModel->index(i, 6, QModelIndex());
        m_pPointModel->setData(index, m_pPolar->m_ClCd.at(i));

        index = m_pPointModel->index(i, 7, QModelIndex());
        m_pPointModel->setData(index, m_pPolar->m_Cl32Cd.at(i));

        index = m_pPointModel->index(i, 8, QModelIndex());
        m_pPointModel->setData(index, m_pPolar->m_RtCl.at(i));

        index = m_pPointModel->index(i, 9, QModelIndex());
        m_pPointModel->setData(index, m_pPolar->m_XCp.at(i));

        index = m_pPointModel->index(i, 10, QModelIndex());
        m_pPointModel->setData(index, m_pPolar->m_HMom.at(i));

        index = m_pPointModel->index(i, 11, QModelIndex());
        m_pPointModel->setData(index, m_pPolar->m_Cdp.at(i));

        index = m_pPointModel->index(i, 12, QModelIndex());
        m_pPointModel->setData(index, m_pPolar->m_Cpmn.at(i));

        index = m_pPointModel->index(i, 13, QModelIndex());
        m_pPointModel->setData(index, m_pPolar->m_Re.at(i));
    }
    m_ptvPoints->resizeRowsToContents();
}



void EditPlrDlg::fillWPolarData()
{
    m_pPointModel->setColumnCount(4);
    m_pPointModel->setRowCount(m_pWPolar->m_Alpha.size());
    QModelIndex index;
    for (int i=0; i<m_pWPolar->dataSize(); i++)
    {
        index = m_pPointModel->index(i, 0, QModelIndex());
        m_pPointModel->setData(index, m_pWPolar->m_Alpha.at(i));

        index = m_pPointModel->index(i, 1, QModelIndex());
        m_pPointModel->setData(index, m_pWPolar->m_CL.at(i));

        index = m_pPointModel->index(i, 2, QModelIndex());
        m_pPointModel->setData(index, m_pWPolar->m_TCd.at(i));

        index = m_pPointModel->index(i, 3, QModelIndex());
        m_pPointModel->setData(index, m_pWPolar->m_GCm.at(i));
    }
    m_ptvPoints->resizeRowsToContents();
}


void EditPlrDlg::keyPressEvent(QKeyEvent *event)
{
    // Prevent Return Key from closing App
    switch (event->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
                m_pButtonBox->setFocus();
            }
            else
            {
                QDialog::accept();
            }
            break;
        }
        case Qt::Key_Escape:
        {
            QDialog::reject();
            return;
        }
        default:
            event->ignore();
    }
}


void EditPlrDlg::deletePoint()
{
    QModelIndex index = m_ptvPoints->currentIndex();

    if(m_pXDirect)
    {
        m_pPolar->removePoint(index.row());
        fillPolarData();
        m_pXDirect->createPolarCurves();
        m_pXDirect->updateView();
    }
    else if(m_pMiarex)
    {
        m_pWPolar->remove(index.row());
        fillWPolarData();
        Miarex::s_bResetCurves = true;
//        pMiarex->createWPolarCurves();
        m_pMiarex->updateView();
    }

    if(index.row()>=m_pPointModel->rowCount()-1)
    {
        index = m_pPointModel->index(m_pPointModel->rowCount()-1,0);
    }
    if(m_pPointModel->rowCount()) m_ptvPoints->setCurrentIndex(index);
}



void EditPlrDlg::deleteAllPoints()
{
    if(m_pXDirect)
    {
        m_pPolar->resetPolar();
        fillPolarData();
        m_pXDirect->createPolarCurves();
        m_pXDirect->updateView();
    }
    else if(m_pMiarex)
    {
        m_pWPolar->clearData();
        fillWPolarData();
        Miarex::s_bResetCurves = true;
        m_pMiarex->updateView();
    }
}


void EditPlrDlg::setupLayout()
{
    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Discard);
    {
        m_ppbDeleteAllPoints = new QPushButton(tr("Delete All Points"));
        m_ppbDeletePoint       = new QPushButton(tr("Delete Point"));
        m_pButtonBox->addButton(m_ppbDeleteAllPoints, QDialogButtonBox::ActionRole);
        m_pButtonBox->addButton(m_ppbDeletePoint, QDialogButtonBox::ActionRole);
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }

    m_ptvPoints = new QTableView(this);
    m_ptvPoints->setFont(DisplayOptions::tableFont());
    m_ptvPoints->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_ptvPoints->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_ptvPoints->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout * pMainLayout = new QVBoxLayout(this);
    {
        pMainLayout->addWidget(m_ptvPoints);
        pMainLayout->addWidget(m_pButtonBox);
    }

    setLayout(pMainLayout);
}


void EditPlrDlg::onButton(QAbstractButton *pButton)
{
    if      (pButton==m_pButtonBox->button(QDialogButtonBox::Save))     accept();
    else if (pButton== m_pButtonBox->button(QDialogButtonBox::Discard)) reject();
    else if (pButton==m_ppbDeletePoint)                               deletePoint();
    else if (pButton==m_ppbDeleteAllPoints)                           deleteAllPoints();
}


void EditPlrDlg::showEvent(QShowEvent *)
{
    restoreGeometry(s_Geometry);
}


void EditPlrDlg::hideEvent(QHideEvent*)
{
    s_Geometry = saveGeometry();
}


void EditPlrDlg::resizeEvent(QResizeEvent*event)
{
    if(!m_pPointModel || !m_ptvPoints) return;
    int n = m_pPointModel->columnCount();
    int w = m_ptvPoints->width();
    int w14 = int(double(w)*0.9/double(n));

    for(int i=0; i<m_pPointModel->columnCount(); i++)
        m_ptvPoints->setColumnWidth(i,w14);

    event->accept();
}
