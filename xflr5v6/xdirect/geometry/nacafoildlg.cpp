/****************************************************************************

    Naca Foil Dlg
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

#include <xfoil.h>
#include "nacafoildlg.h"
#include <QGridLayout>
#include <QFormLayout>
#include <xflobjects/objects2d/foil.h>
#include <xflwidgets/customwts/intedit.h>

XFoil *NacaFoilDlg::s_pXFoil;
int NacaFoilDlg::s_Digits = 0;
int NacaFoilDlg::s_Panels = 100;



NacaFoilDlg::NacaFoilDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("NACA Foils"));

    m_pParent = pParent;

    m_bGenerated = false;
    m_pBufferFoil = nullptr;

    setupLayout();

    m_pleNumber->setText(QString("%1").arg(s_Digits,4,10,QChar('0')));
    m_piePanels->setValue(s_Panels);
}


void NacaFoilDlg::setupLayout()
{
    QFormLayout *pFormLayout = new QFormLayout;
    {
        m_pleNumber = new QLineEdit(this);
        m_pleNumber->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        m_piePanels = new IntEdit(100, this);
        m_piePanels->setMax(IQX);

        pFormLayout->addRow(tr("4 or 5 digits:"), m_pleNumber);
        pFormLayout->addRow(tr("Number of Panels:"), m_piePanels);
    }

    m_plabMessage = new QLabel();
    m_plabMessage->setMinimumWidth(120);

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addLayout(pFormLayout);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_plabMessage);
        pMainLayout->addSpacing(30);
        pMainLayout->addWidget(m_pButtonBox);
    }

    setLayout(pMainLayout);

    connect(m_pleNumber, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(m_piePanels, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
}


void NacaFoilDlg::onButton(QAbstractButton *pButton)
{
    if (     m_pButtonBox->button(QDialogButtonBox::Ok)     == pButton)  onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)  reject();
}



void NacaFoilDlg::onEditingFinished()
{
    bool bOK(false);
    int d = m_pleNumber->text().toInt(&bOK);

    if(bOK) s_Digits = d;

    s_Panels = m_piePanels->value();

    generateFoil();

    m_pleNumber->setText(QString("%1").arg(s_Digits,4,10,QChar('0')));
    m_pButtonBox->setFocus();
}


void NacaFoilDlg::generateFoil()
{
    int itype = 0;

    s_pXFoil->lflap = false;
    s_pXFoil->lbflap = false;

    if(s_Digits<=25099) itype = 5;
    if(s_Digits<=9999 ) itype = 4;

    if(itype==4) s_pXFoil->naca4(s_Digits, (int)(s_Panels/2));
    else if(itype==5)
    {
        int three  = s_Digits/100;
        if(three!=210 && three !=220 && three !=230 && three !=240 && three !=250)
        {
            m_pleNumber->selectAll();
            m_plabMessage->setText(tr("Illegal NACA Number"));
            m_bGenerated = false;
            return;
        }
        if(!s_pXFoil->naca5(s_Digits, s_Panels))
        {
            m_bGenerated = false;
            m_plabMessage->setText(tr("Illegal NACA Number"));
            return;
        }
    }
    else
    {
        m_pleNumber->selectAll();
        m_plabMessage->setText(tr("Illegal NACA Number"));
        m_bGenerated = false;
        return;
    }
    m_plabMessage->setText(" ");

    for (int j=0; j< s_pXFoil->nb; j++)
    {
        m_pBufferFoil->m_xb[j] = s_pXFoil->xb[j+1];
        m_pBufferFoil->m_yb[j] = s_pXFoil->yb[j+1];
        m_pBufferFoil->m_x[j]  = s_pXFoil->xb[j+1];
        m_pBufferFoil->m_y[j]  = s_pXFoil->yb[j+1];
    }
    m_pBufferFoil->m_nb = s_pXFoil->nb;
    m_pBufferFoil->m_n = s_pXFoil->nb;
    m_pBufferFoil->initFoil();

    m_pParent->update();
    m_bGenerated = true;
}


void NacaFoilDlg::keyPressEvent(QKeyEvent *event)
{
    // Prevent Return Key from closing App
    // Generate the foil instead
    switch (event->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
                generateFoil();
                if(m_bGenerated) m_pButtonBox->setFocus();
                else
                {
                    m_pleNumber->selectAll();
                }
            }
            return;
        }
        case Qt::Key_Escape:
        {
            reject();
            return;
        }
    }
}


void NacaFoilDlg::onOK()
{
    generateFoil();
    accept();
}
