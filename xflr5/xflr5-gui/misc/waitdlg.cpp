/****************************************************************************

    WaitDlg    Copyright (C) 2014-2018 Andre Deperrois

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

#include "waitdlg.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QDialogButtonBox>

#include <misc/text/intedit.h>

WaitDlg::WaitDlg() : QDialog()
{
    setAttribute(Qt::WA_DeleteOnClose);

    m_bCancel = false;

    m_pProgress= new IntEdit;
    QDialogButtonBox *pBtnBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
    {
        connect(pBtnBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pProgress);
        pMainLayout->addWidget(pBtnBox);
    }
    setLayout(pMainLayout);
}


void WaitDlg::onButton(QAbstractButton*)
{
    m_bCancel = true;
}


void WaitDlg::setProgress(int p)
{
    m_pProgress->setValue(p);
}
