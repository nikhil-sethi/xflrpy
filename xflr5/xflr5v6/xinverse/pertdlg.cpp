/****************************************************************************

    PertDlg class
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

#include <QVBoxLayout>
#include <QHBoxLayout>

#include "pertdlg.h"


#include <xflcore/displayoptions.h>
#include <xflobjects/objects2d/foil.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/customwts/floateditdelegate.h>


PertDlg::PertDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Pertubation Dialog"));
    m_pXInverse = pParent;
    memset(m_cnr,   0, sizeof(m_cnr));
    memset(m_cni,   0, sizeof(m_cni));
    memset(m_backr, 0, sizeof(m_backr));
    memset(m_backi, 0, sizeof(m_backi));

//    m_pCnModel = nullptr;
     m_pFloatDelegate = nullptr;

    setupLayout();
}

PertDlg::~PertDlg()
{
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
    QVector<int> precision = {0,5,5};
    m_pFloatDelegate->setPrecision(precision);

    m_ptvCn = new QTableView(this);
    m_ptvCn->setFont(DisplayOptions::tableFont());
    m_ptvCn->setWindowTitle(tr("Cn List"));
    m_ptvCn->setMinimumHeight(500);
    m_ptvCn->setMinimumWidth(350);
    m_ptvCn->setColumnWidth(0,30);
    m_ptvCn->setColumnWidth(1,100);
    m_ptvCn->setColumnWidth(2,100);

    m_ptvCn->setModel(&m_CnModel);
    m_ptvCn->setItemDelegate(m_pFloatDelegate);


    connect(m_pFloatDelegate, SIGNAL(closeEditor(QWidget*)), this, SLOT(onCellChanged(QWidget*)));

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply | QDialogButtonBox::RestoreDefaults);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout * pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_ptvCn);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}



void PertDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Escape:
        {
            reject();
            return;
        }
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
                m_pButtonBox->setFocus();
            }
            else
            {
                onOK();
                return;

            }
            break;
        }
        default:
            break;
    }
}


void PertDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok)              == pButton)  onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Apply)           == pButton)  onApply();
    else if (m_pButtonBox->button(QDialogButtonBox::RestoreDefaults) == pButton)  onRestore();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard)         == pButton)  reject();
}


void PertDlg::onApply()
{
    readData();
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

    m_ptvCn->setFocus();
}


void PertDlg::onCellChanged(QWidget *)
{
    QModelIndex index = m_ptvCn->currentIndex();
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

















