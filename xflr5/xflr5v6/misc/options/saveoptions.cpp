/****************************************************************************

    SaveOptions Class
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

#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>


#include "saveoptions.h"
#include <xflwidgets/customwts/intedit.h>
#include "saveoptions.h"

bool SaveOptions::s_bOpps(false);
bool SaveOptions::s_bPOpps(true);
bool SaveOptions::s_bAutoSave(true);
bool SaveOptions::s_bAutoLoadLast(false);
int SaveOptions::s_SaveInterval(17);

SaveOptions::SaveOptions(QWidget *parent) : QWidget(parent)
{
    setupLayout();
}



void SaveOptions::setupLayout()
{
    QGroupBox *pLoadBox = new QGroupBox(tr("Load options"));
    {
        QVBoxLayout *pLoadLayout = new QVBoxLayout;
        {
            m_pchAutoLoadLast = new QCheckBox(tr("Load last project on startup"));
            pLoadLayout->addWidget(m_pchAutoLoadLast);
        }
        pLoadBox->setLayout(pLoadLayout);
    }
    QGroupBox *pSaveOppBox = new QGroupBox(tr("Operating point save"));
    {
        QVBoxLayout *pSaveOppLayout = new QVBoxLayout;
        {
            QLabel *label = new QLabel(tr("Save:"));
            m_pchOpps  = new QCheckBox(tr("Foil Operating Points"));
            m_pchWOpps = new QCheckBox(tr("Wing and Plane Operating Points"));
            pSaveOppLayout->addWidget(label);
            pSaveOppLayout->addWidget(m_pchOpps);
            pSaveOppLayout->addWidget(m_pchWOpps);
        }
        pSaveOppBox->setLayout(pSaveOppLayout);
    }

    QGroupBox *pSaveTimerBox = new QGroupBox(tr("Autosave setting"));
    {
        QHBoxLayout *pSaveTimerLayout = new QHBoxLayout;
        {
            m_pchAutoSave = new QCheckBox("Autosave");
            QLabel *pctrlIntervalLabel = new QLabel(tr("Every"));
            m_pieInterval = new IntEdit(s_SaveInterval);
            QLabel *pctrlMinutes = new QLabel("mn");
            pSaveTimerLayout->addWidget(m_pchAutoSave);
            pSaveTimerLayout->addWidget(pctrlIntervalLabel);
            pSaveTimerLayout->addWidget(m_pieInterval);
            pSaveTimerLayout->addWidget(pctrlMinutes);

            connect(m_pchAutoSave, SIGNAL(clicked(bool)), m_pieInterval, SLOT(setEnabled(bool)));

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


void SaveOptions::initWidget()
{
    m_pchOpps->setChecked(s_bOpps);
    m_pchWOpps->setChecked(s_bPOpps);

    m_pchAutoLoadLast->setChecked(s_bAutoLoadLast);
    m_pchAutoSave->setChecked(s_bAutoSave);
    m_pieInterval->setValue(s_SaveInterval);
    m_pieInterval->setEnabled(s_bAutoSave);
}


void SaveOptions::onOK()
{
    s_bAutoLoadLast = m_pchAutoLoadLast->isChecked();
    s_bOpps = m_pchOpps->isChecked();
    s_bPOpps = m_pchWOpps->isChecked();
    s_bAutoSave = m_pchAutoSave->isChecked();
    s_SaveInterval = m_pieInterval->value();
}


void SaveOptions::loadSettings(QSettings &settings)
{
    settings.beginGroup("SaveOptions");
    {
        s_bAutoLoadLast = settings.value("AutoLoadLastProject", s_bAutoLoadLast).toBool();
        s_bOpps         = settings.value("SaveOpps",            s_bOpps).toBool();
        s_bPOpps        = settings.value("SaveWOpps",           s_bPOpps).toBool();
        s_bAutoSave     = settings.value("AutoSaveProject",     s_bAutoSave).toBool();
        s_SaveInterval  = settings.value("AutoSaveInterval",    s_SaveInterval).toInt();
    }
    settings.endGroup();
}


void SaveOptions::saveSettings(QSettings &settings)
{
    settings.beginGroup("SaveOptions");
    {
        settings.setValue("AutoSaveProject",     s_bAutoSave);
        settings.setValue("AutoSaveInterval",    s_SaveInterval);
        settings.setValue("AutoLoadLastProject", s_bAutoLoadLast);
        settings.setValue("SaveOpps",            s_bOpps);
        settings.setValue("SaveWOpps",           s_bPOpps);
    }
    settings.endGroup();
}






