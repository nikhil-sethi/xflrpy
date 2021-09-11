/****************************************************************************

    LanguageWt Class
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



#include <QHBoxLayout>
#include <QLabel>
#include <QStringList>
#include <QListWidgetItem>
#include <QMutableStringListIterator>
#include <QDir>
#include <QMessageBox>
#include <QTranslator>
#include <QDebug>

#include "languagewt.h"
#include <globals/mainframe.h>

LanguageWt::LanguageWt(QWidget *pParent): QWidget(pParent)
{
    setWindowTitle(tr("Language settings"));
    QString LanguageName = tr("English");// will be translated in the ts & qm files and this will be used to fill the QListWidget
    m_bChanged = false;
    setupLayout();
}


void LanguageWt::setupLayout()
{
    QLabel *lab = new QLabel(tr("Select the application's default language:"));
    m_plwLanguageList = new QListWidget;
    connect(m_plwLanguageList, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(readLanguage()));

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(lab);
        pMainLayout->addWidget(m_plwLanguageList);
    }

    setLayout(pMainLayout);
}


void LanguageWt::initWidget()
{
    QStringList qmFiles = findQmFiles();
    qmFiles.sort();

    qmFileForLanguage.insert("English", "English");
    m_plwLanguageList->clear();

    for (int i=0; i<qmFiles.count(); ++i)
    {
        QString language = languageName(qmFiles[i]);
        qmFileForLanguage.insert(language, qmFiles[i]);
        m_plwLanguageList->addItem(language);
    }

    m_plwLanguageList->setCurrentRow(0);
    for (int i=0; i<qmFiles.count(); ++i)
    {
        if(qmFiles[i]==MainFrame::s_LanguageFilePath)
        {
            m_plwLanguageList->setCurrentRow(i);
            break;
        }
    }
}


QStringList LanguageWt::findQmFiles()
{
    if(!MainFrame::s_TranslationDir.exists())
    {
//        QMessageBox::warning(this, tr("Warning"), tr("The directory ")+MainFrame::s_TranslationDir.path()+tr(" does not exist"));
    }

    QStringList fileNames = MainFrame::s_TranslationDir.entryList(QStringList("*.qm"), QDir::Files, QDir::Name);
//    for(int i=0; i<fileNames.size(); i++)    qDebug()<<fileNames.at(i);

    QMutableStringListIterator i(fileNames);
    while (i.hasNext())
    {
        i.next();
        i.setValue(MainFrame::s_TranslationDir.filePath(i.value()));
    }

    return fileNames;
}


QString LanguageWt::languageName(const QString &qmFile)
{
    QTranslator translator;
    translator.load(qmFile);

    return translator.translate("Language", "English");
}


void LanguageWt::readLanguage()
{
    //read user language selection
    QListWidgetItem *pItem =  m_plwLanguageList->currentItem();
    if(pItem)
    {
        if(pItem->text()=="English") MainFrame::s_LanguageFilePath = "";
        else                         MainFrame::s_LanguageFilePath = qmFileForLanguage[pItem->text()];
    }
    else
    {
        MainFrame::s_LanguageFilePath = "";
    }
    m_bChanged = true;
}


void LanguageWt::hideEvent(QHideEvent *)
{
    if(m_bChanged)
        QMessageBox::information(this, tr("Warning"), tr("The language change will take effect at the next session"));
}


