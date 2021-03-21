/****************************************************************************

    Preferences Class
    Copyright (C) 2018 Andre Deperrois

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


#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

#include "preferencesdlg.h"
#include <globals/mainframe.h>
#include <misc/color/colorbutton.h>
#include <misc/options/units.h>
#include <misc/options/settings.h>
#include <misc/options/languagewt.h>
#include <misc/options/saveoptions.h>
#include <misc/text/intedit.h>
#include <misc/text/textclrbtn.h>
#include <misc/updater.h>

PreferencesDlg::PreferencesDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Preferences"));
    setupLayout();
}



void PreferencesDlg::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            m_pButtonBox->button(QDialogButtonBox::Ok)->setFocus();
            break;
        }
        case Qt::Key_Escape:
        {
            onClose();
            return;
        }
        default:
            event->ignore();
    }
}


void PreferencesDlg::setupLayout()
{
    QWidget *pUpdateFrame = new QWidget;
    {
        QVBoxLayout *pUpdateLayout = new QVBoxLayout;
        m_pctrlUpdateCheck = new QCheckBox("Check for updates on startup");
//        m_pctrlUpdateCheck->setChecked(Updater::bAutoCheck());
        m_pctrlUpdateCheck->setChecked(false);
        m_pctrlUpdateCheck->setEnabled(false);
        pUpdateLayout->addWidget(m_pctrlUpdateCheck);
        pUpdateLayout->addStretch();
        pUpdateFrame->setLayout(pUpdateLayout);
    }

    m_pDisplayOptionsWt = new Settings(this);
    m_pSaveOptionsWt = new SaveOptions(this);
    m_pLanguageWt = new LanguageWt(this);
    m_pUnitsWt = new Units(this);

    QHBoxLayout *pOptionsLayout = new QHBoxLayout;
    {
        m_pTabWidget = new QListWidget;
        m_pTabWidget->addItem(tr("Updates"));
        m_pTabWidget->addItem(tr("Save options"));
        m_pTabWidget->addItem(tr("Display options"));
        m_pTabWidget->addItem(tr("Language"));
        m_pTabWidget->addItem(tr("Units"));
        m_pPageStack = new QStackedWidget;
        m_pPageStack->addWidget(pUpdateFrame);
        m_pPageStack->addWidget(m_pSaveOptionsWt);
        m_pPageStack->addWidget(m_pDisplayOptionsWt);
        m_pPageStack->addWidget(m_pLanguageWt);
        m_pPageStack->addWidget(m_pUnitsWt);
        pOptionsLayout->addWidget(m_pTabWidget);
        pOptionsLayout->addWidget(m_pPageStack);
    }

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addLayout(pOptionsLayout);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);

    connect(m_pTabWidget, SIGNAL(currentRowChanged(int)), this, SLOT(onPage(int)));
}


void PreferencesDlg::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Close) == pButton)  onClose();
}


void PreferencesDlg::onPage(int iRow)
{
    m_pPageStack->setCurrentIndex(iRow);
}


void PreferencesDlg::onClose()
{
    Updater::setAutoCheck(m_pctrlUpdateCheck->isChecked());
    m_pSaveOptionsWt->onOK();
//    m_pLanguageWt->readLanguage();
    accept();
}
