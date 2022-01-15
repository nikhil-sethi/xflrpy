/****************************************************************************

    TEGapDlg Class
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

#include <QLabel>
#include <QMessageBox>
#include <QHBoxLayout>

#include "tegapdlg.h"
#include <xfoil.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflobjects/objects2d/foil.h>


XFoil *TEGapDlg::s_pXFoil;

TEGapDlg::TEGapDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("T.E. Gap"));

    m_pParent = pParent;

    m_Gap   = 0.0;
    m_Blend = 0.8;
    m_bModified = false;
    m_bApplied  = true;

    setupLayout();

    connect(m_pdeGap, SIGNAL(editingFinished()), this, SLOT(onChanged()));
    connect(m_pdeBlend, SIGNAL(editingFinished()), this, SLOT(onChanged()));
}


void TEGapDlg::setupLayout()
{
    QHBoxLayout *pGapValueLayout = new QHBoxLayout;
    {
        QLabel *lab1 = new QLabel(tr("T.E. Gap Value"));
        lab1->setAlignment(Qt::AlignRight);
        lab1->setMinimumWidth(150);
        QLabel *lab2 = new QLabel(tr("% chord"));
        m_pdeGap = new DoubleEdit;
        pGapValueLayout->addWidget(lab1);
        pGapValueLayout->addWidget(m_pdeGap);
        pGapValueLayout->addWidget(lab2);
    }

    QHBoxLayout *pBlendValueLayout = new QHBoxLayout;
    {
        QLabel *lab3 = new QLabel(tr("Blending Distance from T.E."));
        lab3->setAlignment(Qt::AlignRight);
        lab3->setMinimumWidth(150);
        QLabel *lab4 = new QLabel(tr("% chord"));
        m_pdeBlend = new DoubleEdit;
        pBlendValueLayout->addWidget(lab3);
        pBlendValueLayout->addWidget(m_pdeBlend);
        pBlendValueLayout->addWidget(lab4);
    }


    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addLayout(pGapValueLayout);
        pMainLayout->addLayout(pBlendValueLayout);
        pMainLayout->addWidget(m_pButtonBox);
    }

    setLayout(pMainLayout);
}


void TEGapDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok) == pButton)      onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)  reject();
    else if (m_pButtonBox->button(QDialogButtonBox::Apply) == pButton)   onApply();
}


void TEGapDlg::keyPressEvent(QKeyEvent *event)
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
                onApply();
                m_pButtonBox->setFocus();
                m_bApplied  = true;
            }
            break;
        }
        default:
            event->ignore();
            break;
    }
}


void TEGapDlg::initDialog()
{
    m_pdeGap->setMin(  0.0);
    m_pdeGap->setMax(100.0);

    m_pdeBlend->setMin(  0.0);
    m_pdeBlend->setMax(100.0);

    m_pdeGap->setValue(m_pMemFoil->TEGap()*100.0);
    m_pdeBlend->setValue(m_Blend*100.0);

}


void TEGapDlg::onChanged()
{
    m_bApplied = false;
    onApply();
}


void TEGapDlg::onOK()
{
    if(!m_bApplied)   onApply();
    if(!m_bModified) done(0);
    else done(1);
}


void TEGapDlg::onApply()
{
    if(m_bApplied) return;
    //reset everything and retry

    int i, j;

    for (i=0; i< m_pMemFoil->m_nb; i++)
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

    m_Gap = m_pdeGap->value();
    m_Blend = m_pdeBlend->value();
    s_pXFoil->tgap(m_Gap/100.0,m_Blend/100.0);
    if(s_pXFoil->n>IQX)
    {
        QMessageBox::information(window(), tr("Warning"), tr("Panel number cannot exceed 300"));
        //reset everything and retry
        for (i=0; i< m_pMemFoil->m_nb; i++)
        {
            s_pXFoil->x[i+1] = m_pMemFoil->m_xb[i] ;
            s_pXFoil->y[i+1] = m_pMemFoil->m_yb[i];
        }
        s_pXFoil->n = m_pMemFoil->m_nb;
    }
    else
    {
        for (j=0; j< s_pXFoil->n; j++)
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





