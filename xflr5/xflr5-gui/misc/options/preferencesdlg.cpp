/****************************************************************************

	Techwing Application

	Copyright (C) Andre Deperrois techwinder@gmail.com

	All rights reserved.

*****************************************************************************/


#include "preferencesdlg.h"
#include <globals/mainframe.h>
#include <misc/options/saveoptions.h>
#include <misc/options/language.h>
#include <misc/options/Units.h>
#include <misc/options/displayoptions.h>

#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

PreferencesDlg::PreferencesDlg(QWidget *pParent) : QDialog(pParent)
{
	setupLayout();
}



void PreferencesDlg::setupLayout()
{
	m_pDisplayOptionsWidget = new Settings(this);
	m_pSaveOptionsWidget = new SaveOptions(this);
	m_pLanguageOptionsWidget = new TranslatorDlg(this);
	m_pUnitsWidget = new Units(this);

	QHBoxLayout *pOptionsLayout = new QHBoxLayout;
	{
		m_pTabWidget = new QListWidget;
		m_pTabWidget->addItem(tr("Save options"));
		m_pTabWidget->addItem(tr("Display options"));
		m_pTabWidget->addItem(tr("Language"));
		m_pTabWidget->addItem(tr("Units"));
		m_pPageStack = new QStackedWidget;
		m_pPageStack->addWidget(m_pSaveOptionsWidget);
		m_pPageStack->addWidget(m_pDisplayOptionsWidget);
		m_pPageStack->addWidget(m_pLanguageOptionsWidget);
		m_pPageStack->addWidget(m_pUnitsWidget);
		pOptionsLayout->addWidget(m_pTabWidget);
		pOptionsLayout->addWidget(m_pPageStack);
	}

	QHBoxLayout *pCommandButtonsLayout = new QHBoxLayout;
	{
		QPushButton *pOKButton = new QPushButton(tr("OK"));
		pOKButton->setAutoDefault(true);
		QPushButton *pCancelButton = new QPushButton(tr("Cancel"));
		connect(pOKButton, SIGNAL(clicked(bool)), this, SLOT(onOK()));
		connect(pCancelButton, SIGNAL(clicked(bool)), this, SLOT(reject()));
		pCancelButton->setAutoDefault(false);
		pCommandButtonsLayout->addStretch(1);
		pCommandButtonsLayout->addWidget(pOKButton);
		pCommandButtonsLayout->addWidget(pCancelButton);
	}
	QVBoxLayout *pMainLayout = new QVBoxLayout;
	{
		pMainLayout->addLayout(pOptionsLayout);
		pMainLayout->addLayout(pCommandButtonsLayout);
	}
	setLayout(pMainLayout);

	connect(m_pTabWidget, SIGNAL(currentRowChanged(int)), this, SLOT(onPage(int)));
}



void PreferencesDlg::onPage(int iRow)
{
	m_pPageStack->setCurrentIndex(iRow);
}


void PreferencesDlg::onOK()
{
	m_pSaveOptionsWidget->onOK();
	m_pLanguageOptionsWidget->onOK();
	accept();
}
