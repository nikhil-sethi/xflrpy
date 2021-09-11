/****************************************************************************

    LogWt Class

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

#include <QVBoxLayout>
#include <QKeyEvent>


#include "logwt.h"

QByteArray LogWt::s_Geometry;


LogWt::LogWt(QWidget *pParent) : QWidget(pParent)
{
    setWindowTitle(tr("Analysis log report"));
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlag(Qt::WindowStaysOnTopHint);

    m_bFinished = false;
    setupLayout();
}


void LogWt::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout();
    {
        m_ppteLogView = new QPlainTextEdit;
        QHBoxLayout *pCtrlLayout = new QHBoxLayout;
        {
            m_ppbButton = new QPushButton("Cancel/Close");
            connect(m_ppbButton, SIGNAL(clicked(bool)), this, SLOT(onCancelClose()));
            pCtrlLayout->addStretch();
            pCtrlLayout->addWidget(m_ppbButton);
            pCtrlLayout->addStretch();
        }
        pMainLayout->addWidget(m_ppteLogView);
        pMainLayout->addLayout(pCtrlLayout);
    }
    setLayout(pMainLayout);
}


void LogWt::onUpdate(QString msg)
{
    m_ppteLogView->insertPlainText(msg);
    m_ppteLogView->textCursor().movePosition(QTextCursor::End);
    m_ppteLogView->ensureCursorVisible();
}


void LogWt::onCancelClose()
{
    if(m_bFinished)
    {
        close();
    }
    else
    {
        m_ppbButton->setText(tr("Cancelling..."));
        m_ppbButton->setEnabled(false);
        onUpdate("\n_____________Cancel request emitted_____________\n\n");
    }
}


void LogWt::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
    case Qt::Key_Escape:
        m_ppbButton->animateClick();
        pEvent->accept();
        break;
    default:
        pEvent->ignore();
        break;
    }
}


void LogWt::showEvent(QShowEvent *)
{
    restoreGeometry(s_Geometry);
}


void LogWt::hideEvent(QHideEvent*)
{
    s_Geometry = saveGeometry();
}


void LogWt::setCancelButton(bool bCancel)
{
    if(bCancel) m_ppbButton->setText(tr("Cancel"));
    else        m_ppbButton->setText(tr("Close"));
}


void LogWt::setFinished(bool bFinished)
{
    m_bFinished = bFinished;
    if(bFinished)
    {
        m_ppbButton->setText(tr("Close"));
        m_ppbButton->setEnabled(true);
        m_ppbButton->setFocus();
    }
}


void LogWt::loadSettings(QSettings &settings)
{
    settings.beginGroup("LogWt");
    {
        s_Geometry = settings.value("WindowGeometry").toByteArray();
    }
    settings.endGroup();
}


void LogWt::saveSettings(QSettings &settings)
{
    settings.beginGroup("LogWt");
    {
        settings.setValue("WindowGeometry", s_Geometry);
    }
    settings.endGroup();
}





