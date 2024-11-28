/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "newnamedlg.h"


NewNameDlg::NewNameDlg(QString const &name, QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle("Name Dialog");

    setupLayout();

    m_pleName->setText(name);
    m_pleName->selectAll();
}


void NewNameDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_plabQuestion = new QLabel("Name:");
        m_pleName = new QLineEdit(this);

        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        {
            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
        }
        pMainLayout->addWidget(m_plabQuestion);
        pMainLayout->addWidget(m_pleName);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void NewNameDlg::onButton(QAbstractButton*pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok)     == pButton) accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton) reject();
}


void NewNameDlg::accept()
{
    m_NewName = m_pleName->text();
    QDialog::accept();
}


void NewNameDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus()) m_pButtonBox->setFocus();
            else accept();

            break;
        }
        case Qt::Key_Escape:
        {
            reject();
            break;
        }
        default:
            pEvent->ignore();
    }
}








