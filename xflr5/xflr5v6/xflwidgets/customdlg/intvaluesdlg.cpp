/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#include <QFormLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>


#include "intvaluesdlg.h"


IntValuesDlg::IntValuesDlg(QWidget *pParent, QVector<int> const &values, QStringList const &labels) : QDialog(pParent)
{
    setupLayout(values.size());

    for(int i=0; i<values.size(); i++)
        m_pIntEdit[i]->setValue(values.at(i));

    QString strange;
    for(int i=0; i<values.size(); i++)
    {
        if(i<labels.size()) strange = labels.at(i);
        else strange = QString::asprintf("Label_%d", i+1);
        m_pLabel[i]->setText(strange);
    }
}


void IntValuesDlg::setupLayout(int nValues)
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QFormLayout *pIntLayout = new QFormLayout;
        {
            m_pLabel.clear();
            m_pIntEdit.clear();
            for(int i=0; i<nValues; i++)
            {
                m_pLabel.push_back(new QLabel);
                m_pIntEdit.push_back(new IntEdit);
                pIntLayout->addRow(m_pLabel.last(), m_pIntEdit.last());
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
}


void IntValuesDlg::showEvent(QShowEvent *)
{
    if(m_pIntEdit.size()>0)
    {
        m_pIntEdit.first()->selectAll();
        m_pIntEdit.first()->setFocus();
    }
}


void IntValuesDlg::keyPressEvent(QKeyEvent *pEvent)
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


void IntValuesDlg::setLabel(int iLabel, QString label)
{
    if(iLabel>=0 && iLabel<m_pLabel.size()) m_pLabel[iLabel]->setText(label);
}


