/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#include <QMessageBox>
#include <QKeyEvent>
#include <QPushButton>

#include "xfldialog.h"

bool XflDialog::s_bConfirmDiscard = false;


XflDialog::XflDialog(QWidget *pParent) : QDialog(pParent)
{
    setWindowFlag(Qt::WindowMinMaxButtonsHint);
    m_bChanged = false;

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::NoButton, this);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }
}


void XflDialog::setButtons(QDialogButtonBox::StandardButtons buttons)
{
    m_pButtonBox->setStandardButtons(buttons);
}


void XflDialog::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
                m_pButtonBox->setFocus();
                return;
            }
            else
            {
                accept();
                return;
            }
            break;
        }
        default:
            pEvent->ignore();
    }
    QDialog::keyPressEvent(pEvent);
}


void XflDialog::onButton(QAbstractButton*pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok)      == pButton) accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Save)    == pButton) accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel)  == pButton) reject();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton) reject();
    else if (m_pButtonBox->button(QDialogButtonBox::Apply)   == pButton) onApply();
    else if (m_pButtonBox->button(QDialogButtonBox::Reset)   == pButton) onRestore();
    else if (m_pButtonBox->button(QDialogButtonBox::Close)   == pButton) close();
}


void XflDialog::reject()
{
    if(m_bChanged && s_bConfirmDiscard)
    {
        QString strong = tr("Discard the changes?");
        int Ans = QMessageBox::question(this, tr("Question"), strong,
                                        QMessageBox::Yes | QMessageBox::Cancel);
        if (QMessageBox::Yes == Ans)
        {
            done(QDialog::Rejected);
            return;
        }
        else return;
    }

    QDialog::reject();
}

