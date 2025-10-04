/****************************************************************************

    FlapDlg class
    Copyright (C) 2004-2009 André Deperrois 

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

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>

#include "flapdlg.h"

#include <xflobjects/objects2d/foil.h>
#include <xflwidgets/customwts/doubleedit.h>


FlapDlg::FlapDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Flap Dlg"));

    m_pParent = pParent;

    m_pMemFoil    = nullptr;
    m_pBufferFoil = nullptr;
    m_bTEFlap     = false;
    m_TEFlapAngle = 0.0;
    m_TEXHinge    = 80.0;//%
    m_TEYHinge    = 50.0;//%
    m_bLEFlap     = false;
    m_LEFlapAngle = 0.0;
    m_LEXHinge    = 20.0;//%
    m_LEYHinge    = 50.0;//%

    m_bModified   = false;
    m_bApplied    = true;

    setupLayout();

    connect(m_pchLEFlapCheck, SIGNAL(stateChanged(int)), this, SLOT(onLEFlapCheck(int)));
    connect(m_pchTEFlapCheck, SIGNAL(stateChanged(int)), this, SLOT(onTEFlapCheck(int)));

    connect(m_pdeLEXHinge, SIGNAL(editingFinished()), this, SLOT(onChanged()));
    connect(m_pdeLEYHinge, SIGNAL(editingFinished()), this, SLOT(onChanged()));
    connect(m_pdeTEXHinge, SIGNAL(editingFinished()), this, SLOT(onChanged()));
    connect(m_pdeTEYHinge, SIGNAL(editingFinished()), this, SLOT(onChanged()));
    connect(m_pdeLEFlapAngle, SIGNAL(editingFinished()), this, SLOT(onChanged()));
    connect(m_pdeTEFlapAngle, SIGNAL(editingFinished()), this, SLOT(onChanged()));

}


void FlapDlg::setupLayout()
{
    QGridLayout *pFlapDataLayout = new QGridLayout;
    {
        m_pchLEFlapCheck = new QCheckBox(tr("L.E. Flap"));
        m_pchTEFlapCheck = new QCheckBox(tr("T.E. Flap"));
        m_pdeLEXHinge    = new DoubleEdit;
        m_pdeLEYHinge    = new DoubleEdit;
        m_pdeTEXHinge    = new DoubleEdit;
        m_pdeTEYHinge    = new DoubleEdit;
        m_pdeTEFlapAngle = new DoubleEdit;
        m_pdeLEFlapAngle = new DoubleEdit;

        QLabel *pLab1 = new QLabel(tr("Flap Angle"));
        QLabel *pLab2 = new QLabel(QString::fromUtf8("° (")+tr("+ is down") +")");
        QLabel *pLab3 = new QLabel(tr("Hinge X Position"));
        QLabel *pLab4 = new QLabel(tr("% Chord"));
        QLabel *pLab5 = new QLabel(tr("Hinge Y Position"));
        QLabel *pLab6 = new QLabel(tr("% Thickness"));

        pFlapDataLayout->addWidget(m_pchLEFlapCheck, 1, 2);
        pFlapDataLayout->addWidget(m_pchTEFlapCheck, 1, 3);
        pFlapDataLayout->addWidget(pLab1, 2, 1);
        pFlapDataLayout->addWidget(m_pdeLEFlapAngle, 2, 2);
        pFlapDataLayout->addWidget(m_pdeTEFlapAngle, 2, 3);
        pFlapDataLayout->addWidget(pLab2, 2, 4);
        pFlapDataLayout->addWidget(pLab3, 3, 1);
        pFlapDataLayout->addWidget(m_pdeLEXHinge, 3, 2);
        pFlapDataLayout->addWidget(m_pdeTEXHinge, 3, 3);
        pFlapDataLayout->addWidget(pLab4, 3, 4);
        pFlapDataLayout->addWidget(pLab5, 4, 1);
        pFlapDataLayout->addWidget(m_pdeLEYHinge, 4, 2);
        pFlapDataLayout->addWidget(m_pdeTEYHinge, 4, 3);
        pFlapDataLayout->addWidget(pLab6, 4, 4);
    }

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply, this);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }


    QVBoxLayout *MainLayout = new QVBoxLayout;
    MainLayout->addLayout(pFlapDataLayout);
    MainLayout->addWidget(m_pButtonBox);
    setLayout(MainLayout);
}


void FlapDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)    reject();
    else if (m_pButtonBox->button(QDialogButtonBox::Apply)  == pButton)    onApply();
    else if (m_pButtonBox->button(QDialogButtonBox::Ok)     == pButton)    accept();
}


void FlapDlg::initDialog()
{
    m_pchTEFlapCheck->setChecked(m_pMemFoil->m_bTEFlap);

    enableTEFlap(m_pMemFoil->m_bTEFlap);
    m_pdeTEFlapAngle->setValue(m_pMemFoil->m_TEFlapAngle);
    m_pdeTEXHinge->setValue(m_pMemFoil->m_TEXHinge);
    m_pdeTEYHinge->setValue(m_pMemFoil->m_TEYHinge);

    m_pchLEFlapCheck->setChecked(m_pMemFoil->m_bLEFlap);
    enableLEFlap(m_pMemFoil->m_bLEFlap);
    m_pdeLEFlapAngle->setValue(m_pMemFoil->m_LEFlapAngle);
    m_pdeLEXHinge->setValue(m_pMemFoil->m_LEXHinge);
    m_pdeLEYHinge->setValue(m_pMemFoil->m_LEYHinge);
}


void FlapDlg::readParams()
{
    m_bLEFlap = m_pchLEFlapCheck->isChecked();
    m_LEFlapAngle = m_pdeLEFlapAngle->value();
    m_LEXHinge    = m_pdeLEXHinge->value();
    m_LEYHinge    = m_pdeLEYHinge->value();

    m_bTEFlap = m_pchTEFlapCheck->isChecked();
    m_TEFlapAngle = m_pdeTEFlapAngle->value();
    m_TEXHinge    = m_pdeTEXHinge->value();
    m_TEYHinge    = m_pdeTEYHinge->value();

    if(m_LEXHinge>=m_TEXHinge && m_bLEFlap && m_bTEFlap)
    {
        QMessageBox::information(window(), tr("Warning"), tr("The trailing edge hinge must be downstream of the leading edge hinge"));
        m_pdeLEXHinge->setFocus();
        m_pdeLEXHinge->selectAll();
    }
}


void FlapDlg::onApply()
{
    if(m_bApplied) return;
    //reset everything and retry

    readParams();

    m_pBufferFoil->setTEFlapData(m_bTEFlap, m_TEXHinge, m_TEYHinge, m_TEFlapAngle);
    m_pBufferFoil->setLEFlapData(m_bLEFlap, m_LEXHinge, m_LEYHinge, m_LEFlapAngle);
    m_pBufferFoil->setFlap();

    m_bApplied = true;
    m_bModified = true;

    if(m_pParent) m_pParent->update();
}


void FlapDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Escape:
        {
            done(0);
            return;
        }
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
                onApply();
                m_pButtonBox->setFocus();
                m_bApplied  = true;
            }
            else
            {
                QDialog::accept();
            }
            break;
        }
        default:
            pEvent->ignore();
            break;
    }
}


void FlapDlg::enableLEFlap(bool bEnable)
{
    m_pdeLEFlapAngle->setEnabled(bEnable);
    m_pdeLEXHinge->setEnabled(bEnable);
    m_pdeLEYHinge->setEnabled(bEnable);
}

void FlapDlg::enableTEFlap(bool bEnable)
{
    m_pdeTEFlapAngle->setEnabled(bEnable);
    m_pdeTEXHinge->setEnabled(bEnable);
    m_pdeTEYHinge->setEnabled(bEnable);
}


void FlapDlg::onTEFlapCheck(int)
{
    if(m_pchTEFlapCheck->isChecked())
    {
        enableTEFlap(true);
        m_pdeTEFlapAngle->setFocus();
    }
    else
        enableTEFlap(false);
    m_bApplied = false;
    onApply();
}


void FlapDlg::onLEFlapCheck(int)
{
    if(m_pchLEFlapCheck->isChecked())
    {
        enableLEFlap(true);
        m_pdeLEFlapAngle->setFocus();
    }
    else
        enableLEFlap(false);
    m_bApplied = false;
    onApply();
}


void FlapDlg::onChanged()
{
    m_bApplied = false;
    onApply();
}


void FlapDlg::accept()
{
    if(!m_bApplied)
    {
        onApply();
        QDialog::accept();
    }
    else QDialog::accept();
}
