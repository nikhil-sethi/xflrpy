/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/


#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStringList>
#include <QShowEvent>
#include <QHideEvent>

#include "logmessagedlg.h"

#include <xflwidgets/customwts/plaintextoutput.h>


LogMessageDlg::LogMessageDlg(QWidget *pParent) : QWidget(pParent)
{
    setWindowTitle("All-purpose log message window");
    setupLayout();
}


void LogMessageDlg::setOnTop()
{
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
}


void LogMessageDlg::keyPressEvent(QKeyEvent *pEvent)
{
    bool bCtrl = pEvent->modifiers() & Qt::ControlModifier;

    switch (pEvent->key())
    {
        case Qt::Key_Escape:
        {
            onClose();
            return;
        }
        case Qt::Key_W:
        {
            if(bCtrl)
            {
                hide();
            }
            break;
        }
        default:
            pEvent->ignore();
            break;
    }
}


void LogMessageDlg::onClose()
{
    hide();
    emit closeLog();
}


void LogMessageDlg::clearText()
{
    m_pptoOutputLog->clear();
}


void LogMessageDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Close) == pButton)  onClose();
    else if (m_ppbClearButton == pButton)  clearText();
}



void LogMessageDlg::setupLayout()
{
    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    {
        m_ppbClearButton= new QPushButton(tr("Clear text"));
        m_pButtonBox->addButton(m_ppbClearButton, QDialogButtonBox::ActionRole);

        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout * pMainLayout = new QVBoxLayout(this);
    {
        m_pptoOutputLog = new PlainTextOutput;
        m_pptoOutputLog->setReadOnly(true);
        m_pptoOutputLog->setLineWrapMode(QPlainTextEdit::NoWrap);
        m_pptoOutputLog->setWordWrapMode(QTextOption::NoWrap);
        pMainLayout->addWidget(m_pptoOutputLog);
        pMainLayout->addSpacing(20);
        pMainLayout->addWidget(m_pButtonBox);

    }

    setLayout(pMainLayout);
}


void LogMessageDlg::initDialog(QString title, QString props)
{
    setWindowTitle(title);
    m_pptoOutputLog->insertPlainText(props);
}


void LogMessageDlg::setOutputFont(QFont const &fnt)
{
    m_pptoOutputLog->setFont(fnt);
}


void LogMessageDlg::onAppendMessage(QString const &msg)
{
    m_pptoOutputLog->onAppendThisPlainText(msg);
    update();
}


QString const LogMessageDlg::outputText() const
{
    return m_pptoOutputLog->toPlainText();
}

