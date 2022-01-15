/****************************************************************************

    LEDlg Class
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

#include <QMessageBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>


#include "ledlg.h"
#include <xfoil.h>

#include <xflwidgets/customwts/doubleedit.h>
#include <xflobjects/objects2d/foil.h>

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

    connect(m_pdeLE, SIGNAL(editingFinished()), this, SLOT(onChanged()));
    connect(m_pdeBlend, SIGNAL(editingFinished()), this, SLOT(onChanged()));

}

void LEDlg::setupLayout()
{
    QHBoxLayout *pLEValue = new QHBoxLayout;
    {
        QLabel *plab1 = new QLabel(tr("Approximate new/old ratio for L.E. radius"));
        plab1->setAlignment(Qt::AlignRight);
        QLabel *plab2 = new QLabel(tr("ratio"));
        m_pdeLE = new DoubleEdit;
        pLEValue->addWidget(plab1);
        pLEValue->addWidget(m_pdeLE);
        pLEValue->addWidget(plab2);
    }

    QHBoxLayout *pBlendValue = new QHBoxLayout;
    {
        QLabel *plab3 = new QLabel(tr("Blending Distance from L.E."));

        plab3->setAlignment(Qt::AlignRight);
        QLabel *plab4 = new QLabel(tr("% chord"));

        m_pdeBlend = new DoubleEdit;
        pBlendValue->addWidget(plab3);
        pBlendValue->addWidget(m_pdeBlend);
        pBlendValue->addWidget(plab4);
    }

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
    }


    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addStretch(1);
        pMainLayout->addLayout(pLEValue);
        pMainLayout->addStretch(1);
        pMainLayout->addLayout(pBlendValue);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_pButtonBox);
        pMainLayout->addStretch(1);
    }

    setLayout(pMainLayout);
}


void LEDlg::onButton(QAbstractButton *pButton)
{
    if (     m_pButtonBox->button(QDialogButtonBox::Ok)     == pButton)  onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)  reject();
    else if (m_pButtonBox->button(QDialogButtonBox::Apply)  == pButton)  onApply();
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
            if(!m_pButtonBox->hasFocus())
            {
                m_pButtonBox->setFocus();
                return;
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
    m_pdeLE->setMin(  0.0);
    m_pdeLE->setMax(100.0);

    m_pdeBlend->setMin(  0.001);
    m_pdeBlend->setMax(100.0);

    m_pdeLE->setValue(m_LErfac);
    m_pdeBlend->setValue(m_Blend*100.0);
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

    for (int i=0; i< m_pMemFoil->m_nb; i++)
    {
        s_pXFoil->xb[i+1] = m_pMemFoil->m_xb[i] ;
        s_pXFoil->yb[i+1] = m_pMemFoil->m_yb[i];
    }
    s_pXFoil->nb = m_pMemFoil->m_nb;

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

    m_LErfac = m_pdeLE->value();
    m_Blend = m_pdeBlend->value()/100.0;

    s_pXFoil->lerad(m_LErfac,m_Blend);

    if(s_pXFoil->n>IQX)
    {
        QMessageBox::information(window(), tr("Warning"), tr("Panel number cannot exceed 300"));
        //reset everything and retry
        for (int i=0; i< m_pMemFoil->m_nb; i++)
        {
            s_pXFoil->x[i+1] = m_pMemFoil->m_xb[i] ;
            s_pXFoil->y[i+1] = m_pMemFoil->m_yb[i];
        }
        s_pXFoil->n = m_pMemFoil->m_nb;
    }
    else
    {
        for (int j=0; j< s_pXFoil->n; j++)
        {
            m_pBufferFoil->m_xb[j] = s_pXFoil->xb[j+1];
            m_pBufferFoil->m_yb[j] = s_pXFoil->yb[j+1];
        }
        m_pBufferFoil->m_nb = s_pXFoil->nb;
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
