/****************************************************************************

    ModDlg class
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

#include "moddlg.h"
#include <QPushButton>
#include <QHBoxLayout>


ModDlg::ModDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Modification"));
    m_Question = "";
    setupLayout();
}


void ModDlg::initDialog()
{
    m_pctrlQuestion->setText(m_Question);
}


void ModDlg::setupLayout()
{
    m_pctrlQuestion = new QLabel("Question here");

    QHBoxLayout *CommandButtons = new QHBoxLayout;
    QPushButton *OKButton = new QPushButton(tr("OK"));
    QPushButton *CancelButton = new QPushButton(tr("Cancel"));
    QPushButton *SaveNewButton = new QPushButton(tr("Save as new"));
    CommandButtons->addStretch(1);
    CommandButtons->addWidget(OKButton);
    CommandButtons->addStretch(1);
    CommandButtons->addWidget(CancelButton);
    CommandButtons->addStretch(1);
    CommandButtons->addWidget(SaveNewButton);
    CommandButtons->addStretch(1);

    connect(OKButton, SIGNAL(clicked()),this, SLOT(accept()));
    connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(SaveNewButton, SIGNAL(clicked()), this, SLOT(onSaveAsNew()));

    QVBoxLayout *MainLayout = new QVBoxLayout;
    MainLayout->addWidget(m_pctrlQuestion);
    MainLayout->addStretch(1);
    MainLayout->addLayout(CommandButtons);

    setLayout(MainLayout);
}


void ModDlg::onSaveAsNew()
{
    done(20);
}



