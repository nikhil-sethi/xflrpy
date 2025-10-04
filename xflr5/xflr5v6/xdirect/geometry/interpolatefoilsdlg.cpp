/****************************************************************************

    InterpolateFoilsDlg Class
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

#include <QDebug>
#include <QGroupBox>
#include <QVBoxLayout>
#include "interpolatefoilsdlg.h"
#include <xflobjects/objects2d/objects2d.h>

#include <xfoil.h>
#include <xflobjects/objects2d/foil.h>
#include <xflwidgets/customwts/doubleedit.h>


XFoil *InterpolateFoilsDlg::s_pXFoil;

#define SLIDERSCALE 10000

InterpolateFoilsDlg::InterpolateFoilsDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Interpolate Foils"));

    m_pParent = pParent;
    m_pBufferFoil = nullptr;

    setupLayout();

    connect(m_pcbFoil1,  SIGNAL(activated(int)),    this, SLOT(onSelChangeFoil1(int)));
    connect(m_pcbFoil2,  SIGNAL(activated(int)),    this, SLOT(onSelChangeFoil2(int)));
    connect(m_pdeFrac,   SIGNAL(editingFinished()), this, SLOT(onFrac()));
    connect(m_pslMix, SIGNAL(sliderMoved(int)),  this, SLOT(onVScroll(int)));
}


void InterpolateFoilsDlg::setupLayout()
{
    QVBoxLayout *pLeftSide = new QVBoxLayout;
    {
        m_pcbFoil1 = new QComboBox;
        m_pcbFoil1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_pcbFoil2 = new QComboBox;
        m_plabCamb1 = new QLabel(tr("Camb1"));
        m_plabCamb2 = new QLabel(tr("Camb2"));
        m_plabThick1 = new QLabel(tr("Thick1"));
        m_plabThick2 = new QLabel(tr("Thick2"));
        m_plabCamb1->setMinimumWidth(250);
        m_plabCamb2->setMinimumWidth(250);
        m_plabThick1->setMinimumWidth(250);
        m_plabThick2->setMinimumWidth(250);
        pLeftSide->addWidget(m_pcbFoil1);
        pLeftSide->addWidget(m_plabCamb1);
        pLeftSide->addWidget(m_plabThick1);
        pLeftSide->addStretch(1);
        pLeftSide->addWidget(m_pcbFoil2);
        pLeftSide->addWidget(m_plabCamb2);
        pLeftSide->addWidget(m_plabThick2);
    }


    m_pslMix = new QSlider;
    m_pslMix->setMinimumHeight(300);
    m_pslMix->setMinimum(0);
    m_pslMix->setMaximum(SLIDERSCALE);
    m_pslMix->setTickInterval(SLIDERSCALE/10);
    m_pslMix->setTickPosition(QSlider::TicksLeft);


    QVBoxLayout *pFoil3Layout = new QVBoxLayout;
    {
        m_pdeFrac = new DoubleEdit;
        m_plabCamb3 = new QLabel(tr("Camb3"));
        m_plabCamb3->setMinimumWidth(250);
        m_plabThick3 = new QLabel(tr("Thick3"));
        m_plabThick3->setMinimumWidth(250);
        m_pdeFrac->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        m_plabCamb3->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        m_plabThick3->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        pFoil3Layout->addStretch();
        pFoil3Layout->addWidget(m_pdeFrac);
        pFoil3Layout->addWidget(m_plabCamb3);
        pFoil3Layout->addWidget(m_plabThick3);
        pFoil3Layout->addStretch();
    }

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }


    QHBoxLayout *pColumnLayout = new QHBoxLayout;
    {
        pColumnLayout->addLayout(pLeftSide);
        pColumnLayout->addStretch(1);
        pColumnLayout->addWidget(m_pslMix);
        pColumnLayout->addLayout(pFoil3Layout);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addLayout(pColumnLayout);
        pMainLayout->addWidget(m_pButtonBox);
    }

    setLayout(pMainLayout);
}


void InterpolateFoilsDlg::initDialog()
{
    Foil const* pFoil;
    m_pcbFoil1->clear();
    m_pcbFoil2->clear();
    for (int i=0; i<Objects2d::foilCount(); i++)
    {
        pFoil = Objects2d::foilAt(i);
        if(pFoil)
        {
            m_pcbFoil1->addItem(pFoil->name());
            m_pcbFoil2->addItem(pFoil->name());
        }
    }
    m_pcbFoil1->setCurrentIndex(0);
    m_pcbFoil2->setCurrentIndex(1);

    m_Frac = 0.0;
    m_pdeFrac->setValue(100);
    m_pslMix->setSliderPosition(SLIDERSCALE);

    onSelChangeFoil1(0);
    onSelChangeFoil2(1);
}


void InterpolateFoilsDlg::keyPressEvent(QKeyEvent *event)
{
    // Prevent Return Key from closing App
    // Generate the foil instead
    switch (event->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus() )
            {
                update();
                m_pButtonBox->setFocus();
            }
            break;
        }
        case Qt::Key_Escape:
        {
            reject();
            return;
        }
        default:
            event->ignore();
    }
}


void InterpolateFoilsDlg::onSelChangeFoil1(int)
{
    QString strong  = m_pcbFoil1->currentText();

    Foil* pFoil = Objects2d::foil(strong);

    if(pFoil)
    {
        QString str;
        str = QString(tr("Camb.=%1")).arg(pFoil->camber()*100,5,'f',2);
        str += "%";
        strong = QString(tr(" at x=%1")).arg(pFoil->xCamber()*100,5,'f',1);
        strong += "%";
        str+=strong;
        m_plabCamb1->setText(str);

        str = QString(tr("Thick.=%1")).arg(pFoil->thickness()*100,5,'f',2);
        str += "%";
        strong = QString(tr(" at x=%1")).arg(pFoil->xThickness()*100,5,'f',1);
        strong += "%";
        str+=strong;
        m_plabThick1->setText(str);

        m_NewFoilName = pFoil->name();
    }
    update();
}


void InterpolateFoilsDlg::onSelChangeFoil2(int)
{
    QString strong  = m_pcbFoil2->currentText();

    Foil* pFoil = Objects2d::foil(strong);

    if(pFoil)
    {
        QString str;
        str = QString(tr("Camb.=%1")).arg(pFoil->camber()*100,5,'f',2);
        str += "%";
        strong = QString(tr(" at x=%1")).arg(pFoil->xCamber()*100,5,'f',1);
        strong += "%";
        str+=strong;
        m_plabCamb2->setText(str);

        str = QString(tr("Thick.=%1")).arg(pFoil->thickness()*100,5,'f',2);
        str += "%";
        strong = QString(tr(" at x=%1")).arg(pFoil->xThickness()*100,5,'f',1);
        strong += "%";
        str+=strong;
        m_plabThick2->setText(str);
    }
    update();
}


void InterpolateFoilsDlg::update()
{
    QString strong;

    strong = m_pcbFoil1->currentText();
    Foil* pFoil1 = Objects2d::foil(strong);

    strong = m_pcbFoil2->currentText();
    Foil* pFoil2 = Objects2d::foil(strong);

    if(!pFoil1 || !pFoil2) return;

    s_pXFoil->interpolate(pFoil1->m_x, pFoil1->m_y, pFoil1->m_n,
                        pFoil2->m_x, pFoil2->m_y, pFoil2->m_n,
                        m_Frac/100.0);
/*
qDebug()<<pFoil1->foilName();
pFoil1->displayCoords();
qDebug()<<"________";
qDebug()<<pFoil2->foilName();
pFoil2->displayCoords(false);
qDebug()<<"________";*/


    for (int j=0; j< pFoil1->m_n; j++)
    {
        m_pBufferFoil->m_x[j]  = s_pXFoil->xb[j+1];
        m_pBufferFoil->m_y[j]  = s_pXFoil->yb[j+1];
        m_pBufferFoil->m_xb[j] = s_pXFoil->xb[j+1];
        m_pBufferFoil->m_yb[j] = s_pXFoil->yb[j+1];
    }

    m_pBufferFoil->m_n  = pFoil1->m_n;
    m_pBufferFoil->m_nb = pFoil1->m_n;

    m_pBufferFoil->initFoil();

    QString str;
    str = QString(tr("Camb.=%1")).arg(m_pBufferFoil->camber()*100,5,'f',2);
    str += "%";
    strong = QString(tr(" at x=%1")).arg(m_pBufferFoil->xCamber()*100,5,'f',1);
    strong += "%";
    str+=strong;
    m_plabCamb3->setText(str);

    str = QString(tr("Thick.=%1")).arg(m_pBufferFoil->thickness()*100,5,'f',2);
    str += "%";
    strong = QString(tr(" at x=%1")).arg(m_pBufferFoil->xThickness()*100,5,'f',1);
    strong += "%";
    str+=strong;
    m_plabThick3->setText(str);
    m_pParent->update();
}


void InterpolateFoilsDlg::onButton(QAbstractButton *pButton)
{
    if (     m_pButtonBox->button(QDialogButtonBox::Ok)     == pButton)  onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)  reject();
}


void InterpolateFoilsDlg::onFrac()
{
    m_Frac = m_pdeFrac->value();
    m_pslMix->setSliderPosition(int(m_Frac/100.0*SLIDERSCALE));
    m_Frac = 100.0 - m_Frac;

    update();
}


void InterpolateFoilsDlg::onOK()
{
    m_pBufferFoil->setName(m_NewFoilName);

    QDialog::accept();
}


void InterpolateFoilsDlg::onVScroll(int val)
{
    val = m_pslMix->sliderPosition();
    m_Frac = (SLIDERSCALE - double(val))/SLIDERSCALE*100.0;
    m_pdeFrac->setValue(100.0-m_Frac);
    update();
}










