/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#include <QVBoxLayout>

#include "stringvaluedlg.h"


StringValueDlg::StringValueDlg(QWidget *pParent, const QStringList &list) : QDialog(pParent)
{
    setupLayout();
    m_plwStrings->insertItems(0, list);
}


void StringValueDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_plwStrings = new QListWidget;
        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        {
            connect(m_pButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
            connect(m_pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        }
        pMainLayout->addWidget(m_plwStrings);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);

    connect(m_plwStrings, SIGNAL(itemDoubleClicked(QListWidgetItem *)), SLOT(onItemDoubleClicked(QListWidgetItem *)));
}


void StringValueDlg::onItemDoubleClicked(QListWidgetItem *pItem)
{
    if(pItem) accept();
}


QString StringValueDlg::selectedString() const
{
    QListWidgetItem const *pItem = m_plwStrings->currentItem();
    if(pItem) return pItem->text();
    return QString();
}
