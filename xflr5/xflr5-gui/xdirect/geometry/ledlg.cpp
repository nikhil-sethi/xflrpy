/****************************************************************************

    LEDlg Class
    Copyright (C) 2008-2019 Andre Deperrois

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

#include "ledlg.h"
#include <xfoil.h>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QLabel>


#include <misc/text/doubleedit.h>
#include <objects/objects2d/foil.h>

XFoil *LEDlg::s_pXFoil;

LEDlg::LEDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Leading Edge"));

    m_pParent = pParent;

    m_LErfac    = 1.0;
    m_Blend     = 0.1;
    m_bModified = false;
    m_bApplied  = true;

    setupLayout();

    connect(m_pctrlLE, SIGNAL(editingFinished()), this, SLOT(onChanged()));
    connect(m_pctrlBlend, SIGNAL(editingFinished()), this, SLOT(onChanged()));
    connect(ApplyButton, SIGNAL(clicked()),this, SLOT(onApply()));
    connect(OKButton, SIGNAL(clicked()),this, SLOT(onOK()));
    connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));

}

void LEDlg::setupLayout()
{
    QHBoxLayout *pLEValue = new QHBoxLayout;
    {
        QLabel *lab1 = new QLabel(tr("Approximate new/old ratio for L.E. radius"));
        lab1->setMinimumWidth(200);
        lab1->setAlignment(Qt::AlignRight);
        QLabel *lab2 = new QLabel(tr("ratio"));
        lab2->setMinimumWidth(80);
        m_pctrlLE = new DoubleEdit;
        pLEValue->addWidget(lab1);
        pLEValue->addWidget(m_pctrlLE);
        pLEValue->addWidget(lab2);
    }

    QHBoxLayout *pBlendValue = new QHBoxLayout;
    {
        QLabel *lab3 = new QLabel(tr("Blending Distance from L.E."));
        lab3->setMinimumWidth(200);
        lab3->setAlignment(Qt::AlignRight);
        QLabel *lab4 = new QLabel(tr("% chord"));
        lab4->setMinimumWidth(80);
        m_pctrlBlend = new DoubleEdit;
        pBlendValue->addWidget(lab3);
        pBlendValue->addWidget(m_pctrlBlend);
        pBlendValue->addWidget(lab4);
    }

    QHBoxLayout *pCommandButtons = new QHBoxLayout;
    {
        OKButton     = new QPushButton(tr("OK"));
        CancelButton = new QPushButton(tr("Cancel"));
        ApplyButton  = new QPushButton(tr("Apply"));
        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(ApplyButton);
        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(OKButton);
        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(CancelButton);
        pCommandButtons->addStretch(1);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addStretch(1);
        pMainLayout->addLayout(pLEValue);
        pMainLayout->addStretch(1);
        pMainLayout->addLayout(pBlendValue);
        pMainLayout->addStretch(1);
        pMainLayout->addLayout(pCommandButtons);
        pMainLayout->addStretch(1);
    }

    setLayout(pMainLayout);
}


void LEDlg::keyPressEvent(QKeyEvent *event)
{
    // Prevent Return Key from closing App
    switch (event->key())
    {
        case Qt::Key_Escape:
        {
            done(0);
            return;
        }
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!OKButton->hasFocus() && !CancelButton->hasFocus())
            {
                onApply();
                OKButton->setFocus();
                m_bApplied  = true;
            }
            else
            {
                QDialog::accept();
            }
            break;
        }
        default:
            event->ignore();
            break;
    }
}


void LEDlg::initDialog()
{
    m_pctrlLE->setMin(  0.0);
    m_pctrlLE->setMax(100.0);

    m_pctrlBlend->setMin(  0.001);
    m_pctrlBlend->setMax(100.0);

    m_pctrlLE->setValue(m_LErfac);
    m_pctrlBlend->setValue(m_Blend*100.0);
}


void LEDlg::onChanged()
{
    m_bApplied = false;
    onApply();
}


void LEDlg::onApply()
{
    if(m_bApplied) return;

    //reset everything and retry

    for (int i=0; i< m_pMemFoil->nb; i++)
    {
        s_pXFoil->xb[i+1] = m_pMemFoil->xb[i] ;
        s_pXFoil->yb[i+1] = m_pMemFoil->yb[i];
    }
    s_pXFoil->nb = m_pMemFoil->nb;

    s_pXFoil->lflap = false;
    s_pXFoil->lbflap = false;

    if(s_pXFoil->Preprocess())
    {
        s_pXFoil->CheckAngles();
/*        for (int k=0; k<pXFoil->n;k++)
        {
            m_pMemFoil->nx[k] = pXFoil->nx[k+1];
            m_pMemFoil->ny[k] = pXFoil->ny[k+1];
        }
        m_pMemFoil->n = pXFoil->n;*/
    }
    else
    {
        QMessageBox::information(window(), tr("Warning"), tr("Unrecognized foil format"));
        return;
    }

    m_LErfac = m_pctrlLE->value();
    m_Blend = m_pctrlBlend->value()/100.0;

    s_pXFoil->lerad(m_LErfac,m_Blend);

    if(s_pXFoil->n>IQX)
    {
        QMessageBox::information(window(), tr("Warning"), tr("Panel number cannot exceed 300"));
        //reset everything and retry
        for (int i=0; i< m_pMemFoil->nb; i++)
        {
            s_pXFoil->x[i+1] = m_pMemFoil->xb[i] ;
            s_pXFoil->y[i+1] = m_pMemFoil->yb[i];
        }
        s_pXFoil->n = m_pMemFoil->nb;
    }
    else
    {
        for (int j=0; j< s_pXFoil->n; j++)
        {
            m_pBufferFoil->xb[j] = s_pXFoil->xb[j+1];
            m_pBufferFoil->yb[j] = s_pXFoil->yb[j+1];
        }
        m_pBufferFoil->nb = s_pXFoil->nb;
        m_pBufferFoil->initFoil();
        m_pBufferFoil->setFlap();
    }
    m_bApplied = true;
    m_bModified = true;

    m_pParent->update();
}


void LEDlg::onOK()
{
    if(!m_bApplied)    onApply();
    if(!m_bModified) done(0);
    else done(1);
}
