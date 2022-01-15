/****************************************************************************

    ManagePlanesDlg Class
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

#include <QHeaderView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStringList>
#include <QMessageBox>
#include <QKeyEvent>

#include "manageplanesdlg.h"
#include <xflobjects/objects3d/objects3d.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflcore/displayoptions.h>

#include "planetabledelegate.h"
#include <xflobjects/objects3d/plane.h>



ManagePlanesDlg::ManagePlanesDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Plane Object Management"));

    m_pPlane     = nullptr;
    //    m_pSelectionModel = nullptr;
    m_pPlaneDelegate = nullptr;

    m_bChanged = false;

    setupLayout();

    connect(m_ppbDelete, SIGNAL(clicked()),this, SLOT(onDelete()));
    connect(m_ppbRename, SIGNAL(clicked()),this, SLOT(onRename()));
    connect(m_ptvPlanes, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(onDoubleClickTable(const QModelIndex &)));
    connect(m_pteDescription, SIGNAL(textChanged()), this, SLOT(onDescriptionChanged()));
}


void ManagePlanesDlg::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Close) == pButton)         accept();
}



ManagePlanesDlg::~ManagePlanesDlg()
{
    //    if(m_pSelectionModel)   delete m_pSelectionModel;
    if(m_pPlaneDelegate)    delete m_pPlaneDelegate;
    if(m_pPlaneModel)       delete m_pPlaneModel;
}


void ManagePlanesDlg::initDialog(QString &PlaneName)
{
    fillPlaneTable();

    QString strong;
    QString strArea, strLength;
    Units::getLengthUnitLabel(strLength);
    Units::getAreaUnitLabel(strArea);
    m_pPlaneModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
    m_pPlaneModel->setHeaderData(1, Qt::Horizontal, tr("Span")+" ("+strLength+")");
    m_pPlaneModel->setHeaderData(2, Qt::Horizontal, tr("Area")+" ("+strArea+")");
    m_pPlaneModel->setHeaderData(3, Qt::Horizontal, tr("M.A.C.")+" ("+strLength+")");
    m_pPlaneModel->setHeaderData(4, Qt::Horizontal, tr("AR"));
    m_pPlaneModel->setHeaderData(5, Qt::Horizontal, tr("TR"));
    QString str = tr("Rt-Tip Sweep") +QString::fromUtf8(" (°)");
    m_pPlaneModel->setHeaderData(6, Qt::Horizontal, str);
    m_pPlaneModel->setHeaderData(7, Qt::Horizontal, tr("Tail Volume"));


    if(m_pPlaneModel->rowCount())
    {
        if(PlaneName.length())
        {
            QModelIndex ind;
            for(int i=0; i< m_pPlaneModel->rowCount(); i++)
            {
                ind = m_pPlaneModel->index(i, 0, QModelIndex());
                strong = ind.model()->data(ind, Qt::EditRole).toString();

                if(strong == PlaneName)
                {
                    m_ptvPlanes->selectRow(i);
                    break;
                }
            }
        }
        else
        {
            m_ptvPlanes->selectRow(0);
            QStandardItem *pItem = m_pPlaneModel->item(0,0);
            if(pItem) PlaneName = pItem->text();
            else      PlaneName.clear();
        }
        m_pPlane = Objects3d::getPlane(PlaneName);
    }
    else
    {
        m_pPlane = nullptr;
    }
}


void ManagePlanesDlg::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus()) m_pButtonBox->setFocus();
            else                          accept();

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
    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
    }

    QHBoxLayout *pPlaneLayout = new QHBoxLayout;
    {
        QVBoxLayout *pLeftLayout = new QVBoxLayout;
        {
            m_ptvPlanes = new QTableView(this);
            m_ptvPlanes->setFont(DisplayOptions::tableFont());

            m_ptvPlanes->setSelectionMode(QAbstractItemView::SingleSelection);
            m_ptvPlanes->setSelectionBehavior(QAbstractItemView::SelectRows);
            m_ptvPlanes->horizontalHeader()->setStretchLastSection(true);

            QSizePolicy szPolicyExpanding;
            szPolicyExpanding.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
            szPolicyExpanding.setVerticalPolicy(QSizePolicy::Expanding);
            m_ptvPlanes->setSizePolicy(szPolicyExpanding);
            m_ptvPlanes->setMinimumWidth(800);

            m_pteDescription = new QTextEdit;
            QLabel *Description = new QLabel(tr("Description:"));
            pLeftLayout->addWidget(m_ptvPlanes);
            pLeftLayout->addWidget(Description);
            pLeftLayout->addWidget(m_pteDescription);
            pLeftLayout->setStretchFactor(m_ptvPlanes, 5);
            pLeftLayout->setStretchFactor(m_pteDescription, 1);
        }
        QVBoxLayout *pCmdLayout = new QVBoxLayout;
        {
            m_ppbDelete = new QPushButton(tr("Delete"));
            m_ppbRename = new QPushButton(tr("Rename"));
            pCmdLayout->addWidget(m_ppbRename);
            pCmdLayout->addWidget(m_ppbDelete);
            pCmdLayout->addStretch();
        }
        pPlaneLayout->addLayout(pLeftLayout);
        pPlaneLayout->addLayout(pCmdLayout);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addLayout(pPlaneLayout);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);

    m_pPlaneModel = new QStandardItemModel;
    m_pPlaneModel->setRowCount(10);//temporary
    m_pPlaneModel->setColumnCount(8);

    m_ptvPlanes->setModel(m_pPlaneModel);
    m_ptvPlanes->setWindowTitle(tr("Planes"));

    connect(m_ptvPlanes->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onTableRowChanged(QModelIndex)));

    m_pPlaneDelegate = new PlaneTableDelegate;

    m_ptvPlanes->setItemDelegate(m_pPlaneDelegate);
    m_pPlaneDelegate->m_pPlaneModel = m_pPlaneModel;

    QVector<int> m_pPrecision(12);
    m_pPrecision[0]  = 2;
    m_pPrecision[1]  = 3;
    m_pPrecision[2]  = 3;
    m_pPrecision[3]  = 3;
    m_pPrecision[4]  = 2;
    m_pPrecision[5]  = 2;
    m_pPrecision[6]  = 1;
    m_pPrecision[7]  = 2;
    m_pPrecision[8]  = 3;

    m_pPlaneDelegate->m_Precision = m_pPrecision;
}



void ManagePlanesDlg::fillPlaneTable()
{
    m_pPlaneModel->setRowCount(Objects3d::planeCount());

    for(int i=0; i<Objects3d::planeCount(); i++)
    {
        fillPlaneRow(i);
    }
}




void ManagePlanesDlg::fillPlaneRow(int row)
{
    QModelIndex ind;

    if(row>=Objects3d::s_oaPlane.size()) return;

    Plane *pPlane = Objects3d::s_oaPlane.at(row);
    if(!pPlane) return;
    Wing *pWing = pPlane->wing();

    ind = m_pPlaneModel->index(row, 0, QModelIndex());
    m_pPlaneModel->setData(ind,pPlane->name());
    if(pPlane->description().length()) m_pPlaneModel->setData(ind, pPlane->description(), Qt::ToolTipRole);

    ind = m_pPlaneModel->index(row, 1, QModelIndex());
    m_pPlaneModel->setData(ind, pWing->m_PlanformSpan*Units::mtoUnit());

    ind = m_pPlaneModel->index(row, 2, QModelIndex());
    m_pPlaneModel->setData(ind, pWing->m_PlanformArea*Units::m2toUnit());

    ind = m_pPlaneModel->index(row, 3, QModelIndex());
    m_pPlaneModel->setData(ind, pWing->m_MAChord*Units::mtoUnit());

    ind = m_pPlaneModel->index(row, 4, QModelIndex());
    m_pPlaneModel->setData(ind, pWing->m_AR);

    ind = m_pPlaneModel->index(row, 5, QModelIndex());
    m_pPlaneModel->setData(ind, pWing->m_TR);

    ind = m_pPlaneModel->index(row, 6, QModelIndex());
    m_pPlaneModel->setData(ind,pWing->averageSweep());

    ind = m_pPlaneModel->index(row, 7, QModelIndex());
    m_pPlaneModel->setData(ind,pPlane->tailVolume());
}


void ManagePlanesDlg::onRename()
{
    if(m_pPlane)      Objects3d::renamePlane(m_pPlane->name());

    fillPlaneTable();

    selectPlane();
    m_bChanged = true;
}


void ManagePlanesDlg::onDelete()
{
    if(!m_pPlane) return;

    QString strong;
    if(m_pPlane) strong = tr("Are you sure you want to delete the plane :\n") +  m_pPlane->name() +"?\n";

    if (QMessageBox::Yes != QMessageBox::question(window(), tr("Question"), strong,
                                                  QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
                                                  QMessageBox::Yes)) return;

    if(m_pPlane)
    {
        Objects3d::deletePlaneResults(m_pPlane, true);
        Objects3d::deletePlane(m_pPlane);
    }
    QModelIndex index = m_ptvPlanes->currentIndex();
    int sel = std::max(index.row()-1,0);

    fillPlaneTable();

    if(m_pPlaneModel->rowCount()>0)
    {
        m_ptvPlanes->selectRow(sel);
        QString PlaneName;

        QStandardItem *pItem = m_pPlaneModel->item(sel,0);
        if(pItem) PlaneName = pItem->text();
        else      PlaneName.clear();

        m_pPlane = Objects3d::getPlane(PlaneName);
    }
    else
    {
        m_pPlane = nullptr;
    }

    m_bChanged = true;
}


void ManagePlanesDlg::selectPlane()
{
    if(m_pPlane)
    {
        for(int irow=0; irow<m_pPlaneModel->rowCount(); irow++)
        {
            QStandardItem *pItem = m_pPlaneModel->item(irow,0);
            if(pItem && pItem->text().compare(m_pPlane->name())==0)
            {
                m_ptvPlanes->selectRow(irow);
                return;
            }
        }
    }
    if(m_pPlaneModel->rowCount()>0)  m_ptvPlanes->selectRow(0);
}


void ManagePlanesDlg::onDoubleClickTable(const QModelIndex &index)
{
    if(index.row()>=0) accept();
}


void ManagePlanesDlg::onDescriptionChanged()
{
    if(m_pPlane)
    {
        m_pPlane->setDescription(m_pteDescription->toPlainText());
    }
    m_bChanged = true;
}


void ManagePlanesDlg::onTableRowChanged(QModelIndex index)
{
    QStandardItem *pItem = m_pPlaneModel->item(index.row(),0);
    QString PlaneName;
    if(pItem) PlaneName = pItem->text();
    else      PlaneName.clear();

    m_pPlane = Objects3d::getPlane(PlaneName);
    if(m_pPlane) m_pteDescription->setText(m_pPlane->description());
}


void ManagePlanesDlg::resizeEvent(QResizeEvent *event)
{
    int w = m_ptvPlanes->width();
    int w8 = (int)((double)w/12.0);

    m_ptvPlanes->setColumnWidth(1,w8);
    m_ptvPlanes->setColumnWidth(3,w8);
    m_ptvPlanes->setColumnWidth(2,w8);
    m_ptvPlanes->setColumnWidth(4,w8);
    m_ptvPlanes->setColumnWidth(5,w8);
    m_ptvPlanes->setColumnWidth(6,w8);
    m_ptvPlanes->setColumnWidth(7,w8);

    m_ptvPlanes->setColumnWidth(0,w-7*w8-40);
    event->accept();
}


