/****************************************************************************

	Techwing Application

	Copyright (C) Andre Deperrois techwinder@gmail.com

	All rights reserved.

*****************************************************************************/

#include "saveoptions.h"
#include <misc/text/IntEdit.h>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "saveoptions.h"

SaveOptions::SaveOptions(QWidget *parent) : QWidget(parent)
{
	m_bOpps = false;
	m_bWOpps = true;
	m_bAutoSave = true;
	m_bAutoLoadLast = false;
	m_SaveInterval = 17;
	setupLayout();
}



void SaveOptions::setupLayout()
{
	QGroupBox *pLoadBox = new QGroupBox(tr("Load options"));
	{
		QVBoxLayout *pLoadLayout = new QVBoxLayout;
		{
			m_pctrlAutoLoadLast = new QCheckBox(tr("Load last project on startup"));
			pLoadLayout->addWidget(m_pctrlAutoLoadLast);
		}
		pLoadBox->setLayout(pLoadLayout);
	}
	QGroupBox *pSaveOppBox = new QGroupBox(tr("Operating point save"));
	{
		QVBoxLayout *pSaveOppLayout = new QVBoxLayout;
		{
			QLabel *label = new QLabel(tr("Save:"));
			m_pctrlOpps  = new QCheckBox(tr("Foil Operating Points"));
			m_pctrlWOpps = new QCheckBox(tr("Wing and Plane Operating Points"));
			pSaveOppLayout->addWidget(label);
			pSaveOppLayout->addWidget(m_pctrlOpps);
			pSaveOppLayout->addWidget(m_pctrlWOpps);
		}
		pSaveOppBox->setLayout(pSaveOppLayout);
	}

	QGroupBox *pSaveTimerBox = new QGroupBox(tr("Autosave setting"));
	{
		QHBoxLayout *pSaveTimerLayout = new QHBoxLayout;
		{
			m_pctrlAutoSave = new QCheckBox("Autosave");
			QLabel *pctrlIntervalLabel = new QLabel(tr("Every"));
			m_pctrlInterval = new IntEdit(m_SaveInterval);
			QLabel *pctrlMinutes = new QLabel("mn");
			pSaveTimerLayout->addWidget(m_pctrlAutoSave);
			pSaveTimerLayout->addWidget(pctrlIntervalLabel);
			pSaveTimerLayout->addWidget(m_pctrlInterval);
			pSaveTimerLayout->addWidget(pctrlMinutes);

			connect(m_pctrlAutoSave, SIGNAL(clicked(bool)), m_pctrlInterval, SLOT(setEnabled(bool)));

		}
		pSaveTimerBox->setLayout(pSaveTimerLayout);
	}




	QVBoxLayout *pMainLayout = new QVBoxLayout;
	{
		pMainLayout->addWidget(pLoadBox);
		pMainLayout->addWidget(pSaveTimerBox);
		pMainLayout->addStretch(1);
		pMainLayout->addWidget(pSaveOppBox);
	}
	setLayout(pMainLayout);
}


void SaveOptions::initWidget(bool bAutoLoadLast, bool bOpps, bool bWOpps, bool bAutoSave, int saveInterval)
{
	m_bAutoLoadLast = bAutoLoadLast;
	m_bAutoSave = bAutoSave;
	m_SaveInterval = saveInterval;
	m_bOpps  = bOpps;
	m_bWOpps = bWOpps;
	m_pctrlOpps->setChecked(m_bOpps);
	m_pctrlWOpps->setChecked(m_bWOpps);

	m_pctrlAutoLoadLast->setChecked(m_bAutoLoadLast);
	m_pctrlAutoSave->setChecked(m_bAutoSave);
	m_pctrlInterval->setValue(m_SaveInterval);
	m_pctrlInterval->setEnabled(m_bAutoSave);

}



void SaveOptions::onOK()
{
	m_bAutoLoadLast = m_pctrlAutoLoadLast->isChecked();
	m_bOpps = m_pctrlOpps->isChecked();
	m_bWOpps = m_pctrlWOpps->isChecked();
	m_bAutoSave = m_pctrlAutoSave->isChecked();
	m_SaveInterval = m_pctrlInterval->value();
}











