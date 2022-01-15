/****************************************************************************

    Corner Add class
    Copyright (C) 2004-2016 André Deperrois

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

#include "cadddlg.h"
#include <QGridLayout>

#include <xfoil.h>
#include <xflobjects/objects2d/foil.h>
#include <xflwidgets/customwts/doubleedit.h>



XFoil *CAddDlg::s_pXFoil;

CAddDlg::CAddDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Local Panel Refinement"));
    m_pParent = pParent;
    m_pBufferFoil = nullptr;
    m_pMemFoil    = nullptr;

    m_iSplineType = 0;

    setupLayout();

    connect(m_pdeTo,         SIGNAL(valueChanged()),   SLOT(onApply()));
    connect(m_pdeFrom,       SIGNAL(valueChanged()),   SLOT(onApply()));
    connect(m_pdeAngTol,     SIGNAL(valueChanged()),   SLOT(onApply()));
    connect(m_prbUniform,    SIGNAL(toggled(bool)),    SLOT(onApply()));
    connect(m_prbArcLength,  SIGNAL(toggled(bool)),    SLOT(onApply()));
}


void CAddDlg::setupLayout()
{
    QGridLayout *pRefineGridLayout =new QGridLayout;
    {
        QLabel *pLab1 = new QLabel(tr("Angle Criterion ")+QString::fromUtf8("(°)"));
        QLabel *pLab2 = new QLabel(tr("Type of Spline"));
        QLabel *pLab3 = new QLabel(tr("Refinement X Limits"));
        QLabel *pLab4 = new QLabel(tr("From"));
        QLabel *pLab5 = new QLabel(tr("To"));
        pLab4->setAlignment(Qt::AlignCenter);
        pLab5->setAlignment(Qt::AlignCenter);
        m_pdeAngTol = new DoubleEdit;
        m_pdeFrom   = new DoubleEdit;
        m_pdeTo     = new DoubleEdit;

        m_prbUniform   = new QRadioButton(tr("Uniform"));
        m_prbArcLength = new QRadioButton(tr("Arc Length"));

        pRefineGridLayout->addWidget(pLab1,          1, 1);
        pRefineGridLayout->addWidget(m_pdeAngTol,    1, 2);
        pRefineGridLayout->addWidget(pLab2,          2, 1);
        pRefineGridLayout->addWidget(m_prbUniform,   2, 2);
        pRefineGridLayout->addWidget(m_prbArcLength, 2, 3);
        pRefineGridLayout->addWidget(pLab4,          4, 2);
        pRefineGridLayout->addWidget(pLab5,          4, 3);
        pRefineGridLayout->addWidget(m_pdeFrom,      5, 2);
        pRefineGridLayout->addWidget(m_pdeTo,        5, 3);
        pRefineGridLayout->addWidget(pLab3,          5, 1);
    }

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_plabTotal    = new QLabel(tr("Total"));
        m_plabAdded    = new QLabel(tr("Added"));
        m_plabMaxAngle = new QLabel(tr("MaxAngle"));
        m_plabAtPanel  = new QLabel(tr("At Panel"));
        pMainLayout->addLayout(pRefineGridLayout);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_plabTotal);
        pMainLayout->addWidget(m_plabAdded);
        pMainLayout->addWidget(m_plabMaxAngle);
        pMainLayout->addWidget(m_plabAtPanel);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void CAddDlg::onButton(QAbstractButton *pButton)
{
    if (     m_pButtonBox->button(QDialogButtonBox::Ok)     == pButton)  accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)  reject();
    else if (m_pButtonBox->button(QDialogButtonBox::Reset)  == pButton)  onApply();
}


void CAddDlg::onApply()
{
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
        /*        for (int k=0; k<pXFoil->n;k++){
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

    if (m_prbUniform->isChecked())
        m_iSplineType = 1;
    else
        m_iSplineType = 2;

    int added = s_pXFoil->cadd(m_iSplineType, m_pdeAngTol->value(),
                               m_pdeFrom->value(), m_pdeTo->value());
    s_pXFoil->abcopy();

    QString strong;
    strong  =QString(tr("Total number of points is %1")).arg(s_pXFoil->n);
    m_plabTotal->setText(strong);
    strong = QString(tr("(added %1 points to original foil)")).arg(added);
    m_plabAdded->setText(strong);

    for (int i=0; i<s_pXFoil->n; i++)
    {
        m_pBufferFoil->m_xb[i] = s_pXFoil->x[i+1];
        m_pBufferFoil->m_yb[i] = s_pXFoil->y[i+1];
    }
    m_pBufferFoil->m_nb = s_pXFoil->n;
    m_pBufferFoil->initFoil();
    m_pBufferFoil->setFlap();

    s_pXFoil->CheckAngles();
    strong = QString(tr("Maximum panel angle is %1")).arg( s_pXFoil->amax,0,'f',1);
    m_plabMaxAngle->setText(strong);
    strong = QString(tr("at panel position %1")).arg(s_pXFoil->imax);
    m_plabAtPanel->setText(strong);

    m_pParent->update();
}


void CAddDlg::onUniform()
{
    if(m_prbUniform->isChecked()) m_iSplineType = 1;
    else                        m_iSplineType = 2;
}


void CAddDlg::initDialog()
{
    double xbmin = s_pXFoil->xb[1];
    double xbmax = s_pXFoil->xb[1];

    for( int i=1; i<= s_pXFoil->nb; i++)
    {
        xbmin = qMin(xbmin, s_pXFoil->xb[i]);
        xbmax = qMax(xbmax, s_pXFoil->xb[i]);
    }

    //----- default inputs
    atol = 0.5 * s_pXFoil->amax;
    double xrf1 = xbmin - 0.1*(xbmax-xbmin);
    double xrf2 = xbmax + 0.1*(xbmax-xbmin);

    m_prbUniform->setChecked(1);
    m_iSplineType = 1;
    m_pdeFrom->setValue(xrf1);
    m_pdeTo->setValue(xrf2);
    m_pdeAngTol->setValue(atol);

    QString strong;
    s_pXFoil->CheckAngles();
    strong = QString(tr("Maximum panel angle is %1 deg")).arg(s_pXFoil->amax,0,'f',1);
    m_plabMaxAngle->setText(strong);
    strong = QString(tr("at panel position %1")).arg(s_pXFoil->imax);
    m_plabAtPanel->setText(strong);
    m_plabAdded->setText("  ");
    strong = QString(tr("Total number of points is %1")).arg(s_pXFoil->n);
    m_plabTotal->setText(strong);

}


void CAddDlg::keyPressEvent(QKeyEvent *pEvent)
{
    // Prevent Return Key from closing dialog
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
                m_pButtonBox->setFocus();
                return;
            }
            else
            {
                accept();
                return;
            }
        }
        default:
            pEvent->ignore();
    }
}

