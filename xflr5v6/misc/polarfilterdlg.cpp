/****************************************************************************

    PolarFilterDlg Class
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

#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>

#include "polarfilterdlg.h"


PolarFilterDlg::PolarFilterDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Polar Filter"));

    m_bType1 = m_bType2 = m_bType3 = m_bType4 = m_bType7 = true;

    m_bMiarex = false;
    setupLayout();
}


void PolarFilterDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QLabel *Label = new QLabel(tr("Show polar types"));

        m_pchType1 = new QCheckBox(tr("Type 1")+" - "+tr("fixed speed polars"));
        m_pchType2 = new QCheckBox(tr("Type 2")+" - "+tr("fixed lift polars"));
        m_pchType3 = new QCheckBox(tr("Type 3")+" - "+tr("XFoil rubber chord polars"));
        m_pchType4 = new QCheckBox(tr("Type 4")+" - "+tr("fixed a.o.a. polars"));
        m_pchType7 = new QCheckBox(tr("Type 7")+" - "+tr("stability polars"));


        QDialogButtonBox *pButtonBox =  new QDialogButtonBox(QDialogButtonBox::Close);
        connect(pButtonBox, &QDialogButtonBox::rejected, this, &PolarFilterDlg::onClose);

        pMainLayout->addWidget(Label);
        pMainLayout->addWidget(m_pchType1);
        pMainLayout->addWidget(m_pchType2);
        pMainLayout->addWidget(m_pchType3);
        pMainLayout->addWidget(m_pchType4);
        pMainLayout->addWidget(m_pchType7);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(pButtonBox);
        pMainLayout->addStretch(1);
    }
    setLayout(pMainLayout);
}


void PolarFilterDlg::initDialog()
{
    m_pchType1->setChecked(m_bType1);
    m_pchType2->setChecked(m_bType2);
    m_pchType3->setChecked(m_bType3);
    m_pchType4->setChecked(m_bType4);
    m_pchType7->setChecked(m_bType7);

    if(m_bMiarex)
    {
        m_pchType3->setEnabled(false);
        m_pchType3->setChecked(false);
    }
    else
    {
        m_pchType7->setChecked(false);
        m_pchType7->setEnabled(false);
    }
}


void PolarFilterDlg::onClose()
{
    m_bType1 = m_pchType1->isChecked();
    m_bType2 = m_pchType2->isChecked();
    m_bType3 = m_pchType3->isChecked();
    m_bType4 = m_pchType4->isChecked();
    m_bType7 = m_pchType7->isChecked();

    QDialog::accept();
}




