/****************************************************************************

    ReListDlg Class
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
#include <QHeaderView>
#include <QStringList>

#include "ReListDlg.h"
#include <misc/options/displayoptions.h>
#include <misc/text/DoubleEdit.h>
#include <misc/text/FloatEditDelegate.h>

ReListDlg::ReListDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Reynolds Number List"));

    m_iSelection = 0;

    m_pFloatDelegate = nullptr;
    m_Precision = nullptr;

    setupLayout();

    connect(m_pctrlDelete, SIGNAL(clicked()),this, SLOT(onDelete()));
    connect(m_pctrlInsert, SIGNAL(clicked()),this, SLOT(onInsert()));

    connect(OKButton, SIGNAL(clicked()),this, SLOT(onOK()));
    connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}


ReListDlg::~ReListDlg()
{
    if(m_Precision)      delete [] m_Precision;
    if(m_pFloatDelegate) delete m_pFloatDelegate;
}


void ReListDlg::initDialog(QList<double> ReList, QList<double> MachList, QList<double> NCritList)
{
    m_ReList.clear();
    m_MachList.clear();
    m_NCritList.clear();

    m_ReList.append(ReList);
    m_MachList.append(MachList);
    m_NCritList.append(NCritList);

    m_pReModel = new QStandardItemModel(this);
    m_pReModel->setRowCount(5);//temporary
    m_pReModel->setColumnCount(3);

    m_pReModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Re"));
    m_pReModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Mach"));
    m_pReModel->setHeaderData(2, Qt::Horizontal, QObject::tr("NCrit"));

    m_pctrlReTable->setModel(m_pReModel);

    int w = m_pctrlReTable->width();
    m_pctrlReTable->setColumnWidth(0,(int)(w/3));
    m_pctrlReTable->setColumnWidth(1,(int)(w/3));
    m_pctrlReTable->setColumnWidth(2,(int)(w/3));
    QHeaderView *HorizontalHeader = m_pctrlReTable->horizontalHeader();
    HorizontalHeader->setStretchLastSection(true);

    m_pFloatDelegate = new FloatEditDelegate(this);
    m_pctrlReTable->setItemDelegate(m_pFloatDelegate);

    m_Precision = new int[3];
    m_Precision[0] = 0;//no digits for Re Number
    m_Precision[1] = 2;//two digits for Mach and NCrit
    m_Precision[2] = 2;//two digits for Mach and NCrit
    m_pFloatDelegate->setPrecision(m_Precision);

    //	connect(m_pFloatDelegate, SIGNAL(closeEditor(QWidget *)), this, SLOT(onCellChanged(QWidget *)));
    connect(m_pReModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(onCellChanged(QModelIndex ,QModelIndex)));

    fillReModel();
}


void ReListDlg::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!OKButton->hasFocus()) OKButton->setFocus();
            else                      accept();

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


void ReListDlg::onCellChanged(QModelIndex topLeft, QModelIndex botRight)
{
    Q_UNUSED(botRight);
    if(topLeft.column()==0)
        sortData();
}


void ReListDlg::setupLayout()
{
    QVBoxLayout *pCommandButtons = new QVBoxLayout;
    {
        m_pctrlInsert	= new QPushButton(tr("Insert"));
        m_pctrlDelete	= new QPushButton(tr("Delete"));

        OKButton        = new QPushButton(tr("OK"));
        CancelButton    = new QPushButton(tr("Cancel"));
        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(m_pctrlInsert);
        pCommandButtons->addWidget(m_pctrlDelete);
        pCommandButtons->addStretch(2);
        pCommandButtons->addWidget(OKButton);
        pCommandButtons->addWidget(CancelButton);
        pCommandButtons->addStretch(1);
    }

    m_pctrlReTable = new QTableView(this);
    m_pctrlReTable->setFont(Settings::s_TableFont);

    m_pctrlReTable->setMinimumHeight(500);
    m_pctrlReTable->setMinimumWidth(250);
    m_pctrlReTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pctrlReTable->setEditTriggers(QAbstractItemView::AllEditTriggers);
    m_pctrlReTable->setWindowTitle(QObject::tr("Re List"));

    QHBoxLayout * pMainLayout = new QHBoxLayout(this);
    {
        pMainLayout->addWidget(m_pctrlReTable);
        pMainLayout->addLayout(pCommandButtons);
    }
    setLayout(pMainLayout);
}


void ReListDlg::fillReModel()
{
    m_pReModel->setColumnCount(3);
    m_pReModel->setRowCount(m_ReList.count());

    m_pReModel->blockSignals(true);
    for (int i=0; i<m_ReList.count(); i++)
    {
        QModelIndex Xindex = m_pReModel->index(i, 0, QModelIndex());
        m_pReModel->setData(Xindex, m_ReList[i]);

        QModelIndex Yindex =m_pReModel->index(i, 1, QModelIndex());
        m_pReModel->setData(Yindex, m_MachList[i]);

        QModelIndex Zindex =m_pReModel->index(i, 2, QModelIndex());
        m_pReModel->setData(Zindex, m_NCritList[i]);
    }
    m_pReModel->blockSignals(false);
    m_pctrlReTable->resizeRowsToContents();
}


void ReListDlg::onDelete()
{
    QModelIndex index = m_pctrlReTable->currentIndex();
    int sel = index.row();

    if(sel<0 || sel>=m_ReList.count()) return;

    m_ReList.removeAt(sel);
    m_MachList.removeAt(sel);
    m_NCritList.removeAt(sel);


    fillReModel();
}


void ReListDlg::onInsert()
{
    int sel = m_pctrlReTable->currentIndex().row();
    //	if(sel<0) return;

    m_ReList.insert(sel, 0.0);
    m_MachList.insert(sel, 0.0);
    m_NCritList.insert(sel, 0.0);

    if(sel>0)        m_ReList[sel]    = (m_ReList[sel-1]+m_ReList[sel+1]) /2.0;
    else if(sel==0)  m_ReList[sel]    = m_ReList[sel+1]                   /2.0;
    else             m_ReList[0]      = 100000.0;

    if(sel>=0)
    {
        m_MachList[sel]  = m_MachList[sel+1];
        m_NCritList[sel] = m_NCritList[sel+1];
    }
    else
    {
        sel = 0;
        m_MachList[sel]  = 0.0;
        m_NCritList[sel] = 0.0;
    }

    fillReModel();

    QModelIndex index = m_pReModel->index(sel, 0, QModelIndex());
    m_pctrlReTable->setCurrentIndex(index);
    m_pctrlReTable->openPersistentEditor(index);
}


void ReListDlg::onOK()
{
    for (int i=0; i<m_ReList.count(); i++)
    {
        QModelIndex index = m_pReModel->index(i, 0, QModelIndex());
        m_ReList[i] = index.data().toDouble();

        index = m_pReModel->index(i, 1, QModelIndex());
        m_MachList[i] = index.data().toDouble();

        index = m_pReModel->index(i, 2, QModelIndex());
        m_NCritList[i] = index.data().toDouble();

    }
    done(1);
}



void ReListDlg::sortData()
{
    int i;
    m_ReList.clear();
    m_MachList.clear();
    m_NCritList.clear();

    for (i=0; i<m_pReModel->rowCount(); i++)
    {
        m_ReList.append(m_pReModel->index(i, 0, QModelIndex()).data().toDouble());
        m_MachList.append(m_pReModel->index(i, 1, QModelIndex()).data().toDouble());
        m_NCritList.append(m_pReModel->index(i, 2, QModelIndex()).data().toDouble());
    }
    sortRe();

    //and fill back the model
    fillReModel();
}





/**
* Bubble sort algorithm for the arrays of Reynolds, Mach and NCrit numbers.
* The arrays are sorted by crescending Re numbers.
*/
void ReListDlg::sortRe()
{
    int indx, indx2;
    double Retemp, Retemp2;
    double Matemp, Matemp2;
    double NCtemp, NCtemp2;
    int flipped;

    if (m_ReList.size() <= 1) return;

    indx = 1;
    do
    {
        flipped = 0;
        for (indx2 = m_ReList.size() - 1; indx2 >= indx; --indx2)
        {
            Retemp  = m_ReList[indx2];
            Retemp2 = m_ReList[indx2 - 1];
            Matemp  = m_MachList[indx2];
            Matemp2 = m_MachList[indx2 - 1];
            NCtemp  = m_NCritList[indx2];
            NCtemp2 = m_NCritList[indx2 - 1];
            if (Retemp2> Retemp)
            {
                m_ReList[indx2 - 1]    = Retemp;
                m_ReList[indx2]        = Retemp2;
                m_MachList[indx2 - 1]  = Matemp;
                m_MachList[indx2]      = Matemp2;
                m_NCritList[indx2 - 1] = NCtemp;
                m_NCritList[indx2]     = NCtemp2;
                flipped = 1;
            }
        }
    } while ((++indx < m_ReList.size()) && flipped);
}












