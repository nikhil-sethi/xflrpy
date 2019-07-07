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

#include <objects/objects2d/foil.h>
#include "foilselectiondlg.h"



FoilSelectionDlg::FoilSelectionDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Foil Selection"));

    m_FoilName.clear();
    setupLayout();
}


void FoilSelectionDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_pctrlNameList = new QListWidget;
        m_pctrlNameList->setMinimumHeight(300);
        m_pctrlNameList->setSelectionMode(QAbstractItemView::MultiSelection);

        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Discard);
        {
            m_pctrlSelectAll = new QPushButton(tr("Select all"));
            m_pButtonBox->addButton(m_pctrlSelectAll, QDialogButtonBox::ActionRole);
            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
        }

        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_pctrlNameList);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_pButtonBox);
    }

    connect(m_pctrlNameList, SIGNAL(itemClicked(QListWidgetItem *)),       SLOT(onSelChangeList(QListWidgetItem *)));
    connect(m_pctrlNameList, SIGNAL(itemDoubleClicked(QListWidgetItem *)), SLOT(onDoubleClickList(QListWidgetItem *)));

    setLayout(pMainLayout);
}


void FoilSelectionDlg::onButton(QAbstractButton *pButton)
{
    if (     m_pButtonBox->button(QDialogButtonBox::Ok)      == pButton)  onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)  reject();
    else if (m_pctrlSelectAll                                == pButton)  onSelectAll();
}


void FoilSelectionDlg::onSelectAll()
{
    m_pctrlNameList->selectAll();
}


void FoilSelectionDlg::onOK()
{
    QListWidgetItem *pItem =  m_pctrlNameList->currentItem();
    m_FoilName = pItem->text();

    m_FoilSelectionList.clear();
    for(int i=0; i<m_pctrlNameList->count();i++)
    {
        pItem = m_pctrlNameList->item(i);
        if(pItem->isSelected())
        {
            m_FoilSelectionList.append(pItem->text());
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


void FoilSelectionDlg::initDialog(QVector<Foil*> const *FoilList, QStringList const &FoilSelList)
{
    m_FoilList = *FoilList;

    for (int i=0; i<m_FoilList.size(); i++)
    {
        const Foil *pFoil = m_FoilList.at(i);
        m_pctrlNameList->addItem(pFoil->foilName());
        m_pctrlNameList->setItemSelected(m_pctrlNameList->item(i), false);
        for(int j=0; j<FoilSelList.size(); j++)
        {
            if(FoilSelList.at(j)==pFoil->foilName())
            {
                m_pctrlNameList->setItemSelected(m_pctrlNameList->item(i), true);
                break;
            }
        }
//        if(pFoil->foilName()==m_FoilName) m_pctrlNameList->setItemSelected(m_pctrlNameList->item(i), true);
//        else                              m_pctrlNameList->setItemSelected(m_pctrlNameList->item(i), false);
    }
}





