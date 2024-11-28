/****************************************************************************

    RenameDlg Class
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


#include "renamedlg.h"
#include <QMessageBox>

QByteArray RenameDlg::s_Geometry;

RenameDlg::RenameDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Rename"));
    m_bEnableOverwrite = true;
    m_bExists = true;
    m_strArray.clear();
    setupLayout();
}

void RenameDlg::showEvent(QShowEvent *)
{
    restoreGeometry(s_Geometry);
}


void RenameDlg::hideEvent(QHideEvent *)
{
    s_Geometry = saveGeometry();
}


void RenameDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok) == pButton)       onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)  reject();
    else if (m_ppbOverwrite==pButton)                                     onOverwrite();
}


void RenameDlg::setupLayout()
{
    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Discard);
    {
        m_ppbOverwrite = new QPushButton(tr("Overwrite"));
        m_ppbOverwrite->setAutoDefault(false);
        m_pButtonBox->addButton(m_ppbOverwrite, QDialogButtonBox::ActionRole);
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_plabMessage = new QLabel("A Message here");

        m_pleName = new QLineEdit("");
        QLabel* plabNameList = new QLabel(tr("Existing Names:"));
        m_plwNameList = new QListWidget;

        pMainLayout->addWidget(m_plabMessage);
        pMainLayout->addWidget(m_pleName);

        pMainLayout->addWidget(plabNameList);
        pMainLayout->addWidget(m_plwNameList);

        pMainLayout->addWidget(m_pButtonBox);

        pMainLayout->setStretchFactor(m_plwNameList, 1);
    }

    setLayout(pMainLayout);

    connect(m_plwNameList, SIGNAL(currentRowChanged(int)), SLOT(onSelChangeList(int)));
    connect(m_plwNameList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(onDoubleClickList(QListWidgetItem*)));
//    connect(m_pOverwriteButton, SIGNAL(clicked()), this, SLOT(onOverwrite()));
}


void RenameDlg::initDialog(QStringList *pStrList, const QString &startName, const QString &question)
{
    m_plwNameList->clear();

    m_strQuestion = question;

    if(!m_bEnableOverwrite) m_ppbOverwrite->setEnabled(false);

    if(m_strQuestion.length())
    {
        m_plabMessage->setText(m_strQuestion);
    }
    else
    {
        m_plabMessage->setText(tr("Enter a name"));
    }

    m_startName = startName;
    m_pleName->setText(startName);
    m_pleName->setFocus();
    m_pleName->selectAll();

    m_strArray.clear();

    if(pStrList)
    {
        for (int i=0; i<pStrList->size(); i++)
        {
            m_strArray.append(pStrList->at(i));
            m_plwNameList->addItem(pStrList->at(i));
        }
    }
    else
    {
        m_plwNameList->setEnabled(false);
        m_ppbOverwrite->setEnabled(false);
    }
}


void RenameDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            m_pButtonBox->button(QDialogButtonBox::Ok)->setFocus();
            break;
        }
        case Qt::Key_Escape:
        {
            reject();
            return;
        }
        default:
            pEvent->ignore();
    }
}


void RenameDlg::onOverwrite()
{
    m_bExists = true;
    m_pleName->text();
    done(10);
}


void RenameDlg::onOK()
{
    if (!m_pleName->text().length())
    {
        QMessageBox::warning(this, tr("Warning"), tr("Must enter a name"));
        m_pleName->setFocus();
        return;
    }
    QString newname = m_pleName->text();

    if(m_startName==newname)
    {
        // nothing to do
        QDialog::accept();
        return;
    }

    //exists ?
    m_bExists = false;
    for (int l=0; l<m_strArray.size(); l++)
    {
        QString oldName = m_strArray.at(l);
        if(oldName == newname)
        {
            QString str = tr("Do you wish to overwrite ")+oldName + "?";
            if (QMessageBox::Yes == QMessageBox::question(window(), tr("Question"), str,
                                                          QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel))
            {
                m_bExists = true;
                done(10);
                return;
            }
            else return;
        }
    }

    QDialog::accept();
}


void RenameDlg::onSelChangeList(int)
{
    QListWidgetItem *pItem =  m_plwNameList->currentItem();

    if(pItem)
    {
        QString str = pItem->text();
        m_pleName->setText(str);
        m_pleName->selectAll();
    }
}


void RenameDlg::onDoubleClickList(QListWidgetItem *pItem)
{
    if(pItem)
    {
        QString str = pItem->text();
        m_pleName->setText(str);
        onOK();
    }
}









