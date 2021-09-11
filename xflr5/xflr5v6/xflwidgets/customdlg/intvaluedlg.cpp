/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#include <QVBoxLayout>

#include "intvaluedlg.h"
#include <xflwidgets/customwts/intedit.h>


IntValueDlg::IntValueDlg(QWidget *pParent) : QDialog (pParent)
{
    setupLayout();
}


void IntValueDlg::setValue(int intvalue)
{
    m_pieIntEdit->setValue(intvalue);
}


int IntValueDlg::value() const
{
    return m_pieIntEdit->value();
}


void IntValueDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QHBoxLayout *pValueLayout = new QHBoxLayout;
        {
            m_pLabel = new QLabel("Enter value:");
            m_pLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
            m_pieIntEdit = new IntEdit;
            pValueLayout->addWidget(m_pLabel);
            pValueLayout->addWidget(m_pieIntEdit);
        }
        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        {
            connect(m_pButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
            connect(m_pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        }

        pMainLayout->addLayout(pValueLayout);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void IntValueDlg::showEvent(QShowEvent *pEvent)
{
    m_pieIntEdit->selectAll();
    m_pieIntEdit->setFocus();
    QDialog::showEvent(pEvent);
}


void IntValueDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
                m_pButtonBox->setFocus();
            }
            else
            {
                QDialog::accept();
            }
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


