/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>

#include "doublevaluedlg.h"



DoubleValueDlg::DoubleValueDlg(QWidget *pParent, QVector<double> values, QStringList const &leftlabels, QStringList const &rightlabels) : QDialog(pParent)
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QGridLayout *pIntLayout = new QGridLayout;
        {
            m_pDoubleEdit.clear();
            for(int i=0; i<values.size(); i++)
            {
                QLabel *pLeftLabel = new QLabel;
                if(i<leftlabels.size()) pLeftLabel->setText(leftlabels.at(i));
                m_pDoubleEdit.push_back(new DoubleEdit);
                QLabel *pRightLabel = new QLabel;
                if(i<rightlabels.size()) pRightLabel->setText(rightlabels.at(i));

                pIntLayout->addWidget(pLeftLabel,           i, 1);
                pIntLayout->addWidget(m_pDoubleEdit.last(), i, 2);
                pIntLayout->addWidget(pRightLabel,          i, 3);
            }
        }

        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        {
            connect(m_pButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
            connect(m_pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        }

        pMainLayout->addLayout(pIntLayout);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
    for(int i=0; i<values.size(); i++)
        m_pDoubleEdit[i]->setValue(values.at(i));
}


void DoubleValueDlg::showEvent(QShowEvent *)
{
    if(m_pDoubleEdit.size())
    {
        m_pDoubleEdit.first()->selectAll();
        m_pDoubleEdit.first()->setFocus();
    }
}


void DoubleValueDlg::keyPressEvent(QKeyEvent *pEvent)
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

