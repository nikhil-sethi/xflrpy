/****************************************************************************

    FoilSelectionDlg Classe
    Copyright (C)  André Deperrois

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

#include <xflobjects/objects2d/foil.h>
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
        m_plwNameList = new QListWidget;
        m_plwNameList->setMinimumHeight(300);
        m_plwNameList->setSelectionMode(QAbstractItemView::MultiSelection);

        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Discard);
        {
            m_ppbSelectAll = new QPushButton(tr("Select all"));
            m_pButtonBox->addButton(m_ppbSelectAll, QDialogButtonBox::ActionRole);
            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
        }

        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_plwNameList);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_pButtonBox);
    }

    connect(m_plwNameList, SIGNAL(itemClicked(QListWidgetItem*)),       SLOT(onSelChangeList(QListWidgetItem*)));
    connect(m_plwNameList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(onDoubleClickList(QListWidgetItem*)));

    setLayout(pMainLayout);
}


void FoilSelectionDlg::onButton(QAbstractButton *pButton)
{
    if (     m_pButtonBox->button(QDialogButtonBox::Ok)      == pButton)  onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)  reject();
    else if (m_ppbSelectAll                                == pButton)  onSelectAll();
}


void FoilSelectionDlg::onSelectAll()
{
    m_plwNameList->selectAll();
}


void FoilSelectionDlg::onOK()
{
    QListWidgetItem *pItem =  m_plwNameList->currentItem();
    m_FoilName = pItem->text();

    m_FoilSelectionList.clear();
    for(int i=0; i<m_plwNameList->count();i++)
    {
        pItem = m_plwNameList->item(i);
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
        m_plwNameList->addItem(pFoil->name());
        QListWidgetItem *pItem =  m_plwNameList->item(i);
        pItem->setSelected(false);
        for(int j=0; j<FoilSelList.size(); j++)
        {
            if(FoilSelList.at(j)==pFoil->name())
            {
                QListWidgetItem *pItem =  m_plwNameList->item(i);
                pItem->setSelected(true);
                break;
            }
        }
    }
}





