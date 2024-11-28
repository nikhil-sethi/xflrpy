/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#include <QFormLayout>

#include "separatorsdlg.h"

bool SeparatorsDlg::s_bWhiteSpace = true;
bool SeparatorsDlg::s_bTab        = true;
bool SeparatorsDlg::s_bComma      = false;
bool SeparatorsDlg::s_bSemiColon  = false;

SeparatorsDlg::SeparatorsDlg(QWidget *pParent) : QDialog(pParent)
{
    setupLayout();
}


void SeparatorsDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QLabel *pLabTitle = new QLabel("Select the text separators to use");
        m_pchWhiteSpace = new QCheckBox("WhiteSpace");
        m_pchWhiteSpace->setChecked(s_bWhiteSpace);
        m_pchTab        = new QCheckBox("tab");
        m_pchTab->setChecked(s_bTab);
        m_pchComma      = new QCheckBox("comma ',' ");
        m_pchComma->setChecked(s_bComma);
        m_pchSemiColon  = new QCheckBox("semi-colon ';'");
        m_pchSemiColon->setChecked(s_bSemiColon);
        QDialogButtonBox *pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
        connect(pButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

        pMainLayout->addWidget(pLabTitle);
        pMainLayout->addWidget(m_pchWhiteSpace);
        pMainLayout->addWidget(m_pchTab);
        pMainLayout->addWidget(m_pchComma);
        pMainLayout->addWidget(m_pchSemiColon);
        pMainLayout->addWidget(pButtonBox);
    }
    setLayout(pMainLayout);
}


void SeparatorsDlg::hideEvent(QHideEvent *)
{
    s_bWhiteSpace = m_pchWhiteSpace->isChecked();
    s_bTab        = m_pchTab->isChecked();
    s_bComma      = m_pchComma->isChecked();
    s_bSemiColon  = m_pchSemiColon->isChecked();
}


