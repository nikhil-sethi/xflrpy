/****************************************************************************

    PertDlg class
    Copyright (C) 2004-2016 Andre Deperrois 

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

#include "pertdlg.h"
#include <misc/options/settings.h>

#include <misc/text/floatrditdelegate.h>
#include <objects/objects2d/foil.h>
#include <misc/text/doubleedit.h>


PertDlg::PertDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Pertubation Dialog"));
    m_pXInverse = pParent;
    memset(m_cnr,   0, sizeof(m_cnr));
    memset(m_cni,   0, sizeof(m_cni));
    memset(m_backr, 0, sizeof(m_backr));
    memset(m_backi, 0, sizeof(m_backi));

//    m_pCnModel = nullptr;
    m_precision = nullptr;
    m_pFloatDelegate = nullptr;

    setupLayout();

    connect(pRestoreButton, SIGNAL(clicked()),this, SLOT(onRestore()));
    connect(pApplyButton, SIGNAL(clicked()),this, SLOT(onApply()));
    connect(pOKButton, SIGNAL(clicked()),this, SLOT(accept()));
    connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

PertDlg::~PertDlg()
{
    if(m_precision)       delete [] m_precision;
//    if(m_pCnModel)        delete m_pCnModel;
    if (m_pFloatDelegate) delete m_pFloatDelegate;
}


void PertDlg::setupLayout()
{
    m_CnModel.setRowCount(10);//temporary
    m_CnModel.setColumnCount(3);
    m_CnModel.setHeaderData(0, Qt::Horizontal, "i");
    m_CnModel.setHeaderData(1, Qt::Horizontal, "Cn");
    m_CnModel.setHeaderData(2, Qt::Horizontal, "Ci");

    m_pFloatDelegate = new FloatEditDelegate(this);
    m_precision = new int[3];
    m_precision[0] = 0;
    m_precision[1] = 5;
    m_precision[2] = 5;
    m_pFloatDelegate->setPrecision(m_precision);

    m_pctrlCnTable = new QTableView(this);
    m_pctrlCnTable->setFont(Settings::s_TableFont);
    m_pctrlCnTable->setWindowTitle(tr("Cn List"));
    m_pctrlCnTable->setMinimumHeight(500);
    m_pctrlCnTable->setMinimumWidth(350);
    m_pctrlCnTable->setColumnWidth(0,30);
    m_pctrlCnTable->setColumnWidth(1,100);
    m_pctrlCnTable->setColumnWidth(2,100);

    m_pctrlCnTable->setModel(&m_CnModel);
    m_pctrlCnTable->setItemDelegate(m_pFloatDelegate);


    connect(m_pFloatDelegate, SIGNAL(closeEditor(QWidget *)), this, SLOT(onCellChanged(QWidget *)));

    QVBoxLayout *pCommandButtons = new QVBoxLayout;
    {
        pRestoreButton    = new QPushButton(tr("Restore"));
        pApplyButton    = new QPushButton(tr("Apply"));
        pOKButton       = new QPushButton(tr("OK"));
        pCancelButton   = new QPushButton(tr("Cancel"));
        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(pRestoreButton);
        pCommandButtons->addWidget(pApplyButton);
        pCommandButtons->addStretch(2);
        pCommandButtons->addWidget(pOKButton);
        pCommandButtons->addWidget(pCancelButton);
        pCommandButtons->addStretch(1);
    }

    QHBoxLayout * pMainLayout = new QHBoxLayout(this);
    {
        pMainLayout->addWidget(m_pctrlCnTable);
        pMainLayout->addLayout(pCommandButtons);
    }
    setLayout(pMainLayout);
}



void PertDlg::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Escape:
        {
            reject();
            return;
        }
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!pOKButton->hasFocus() && !pCancelButton->hasFocus())
            {
                pOKButton->setFocus();
            }
            else if(pCancelButton->hasFocus())
            {
                reject();
                return;
            }
            else
            {
                accept();
                return;

            }
            break;
        }
        default:
            break;
    }
}

void PertDlg::onApply()
{
    readData();
    pOKButton->setFocus();

}


void PertDlg::onOK()
{
    readData();
    accept();
}


void PertDlg::readData()
{
    int pos;
    bool bOK;
    QString strong;
    QStandardItem *pItem;

    for (pos=0; pos<m_CnModel.rowCount(); pos++)
    {
        pItem = m_CnModel.item(pos,1);
        strong =pItem->text();
        m_cnr[pos]=strong.toDouble(&bOK);

        pItem = m_CnModel.item(pos,2);
        strong =pItem->text();
        m_cni[pos]=strong.toDouble(&bOK);
    }
}


void PertDlg::initDialog()
{
    memcpy(m_backr, m_cnr, sizeof(m_cnr));
    memcpy(m_backi, m_cni, sizeof(m_cni));

    fillCnModel();

    m_pctrlCnTable->setFocus();
}


void PertDlg::onCellChanged(QWidget *)
{
    QModelIndex index = m_pctrlCnTable->currentIndex();
    int pos = index.row();
    if(pos<0)  return;
    
    bool bOK;
    QString strong;
    QStandardItem *pItem;
    pItem = m_CnModel.item(pos,1);
    strong =pItem->text();
    m_cnr[pos]=strong.toDouble(&bOK);

    pItem = m_CnModel.item(pos,2);
    strong =pItem->text();
    m_cni[pos]=strong.toDouble(&bOK);

    fillCnModel();
}


void PertDlg::fillCnModel()
{
    m_CnModel.setRowCount(m_nc);

    for (int i=0; i<m_nc; i++)
    {
        QModelIndex Xindex = m_CnModel.index(i, 0, QModelIndex());
        m_CnModel.setData(Xindex, i);

        QModelIndex Yindex =m_CnModel.index(i, 1, QModelIndex());
        m_CnModel.setData(Yindex, m_cnr[i]);

        QModelIndex Zindex =m_CnModel.index(i, 2, QModelIndex());
        m_CnModel.setData(Zindex, m_cni[i]);
    }
}


void PertDlg::onRestore()
{
    for (int i=0; i<m_nc; i++)
    {
        m_cnr[i]   = m_backr[i];
        m_cni[i]   = m_backi[i];
//        m_backi[i] = m_cni[i];
    }

    fillCnModel();
}

















