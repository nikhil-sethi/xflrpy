/****************************************************************************

    InterpolateFoilsDlg Class
    Copyright (C) 2008-2017 Andre Deperrois 

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
#include <xdirect/objects2d.h>

#include <xfoil.h>
#include <objects/objects2d/foil.h>
#include <misc/text/doubleedit.h>


void *InterpolateFoilsDlg::s_pXFoil;

#define SLIDERSCALE 10000

InterpolateFoilsDlg::InterpolateFoilsDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Interpolate Foils"));

    m_pParent = pParent;
    m_pBufferFoil = nullptr;
    m_poaFoil = nullptr;


    setupLayout();

    connect(m_pctrlFoil1,  SIGNAL(activated(int)),    this, SLOT(onSelChangeFoil1(int)));
    connect(m_pctrlFoil2,  SIGNAL(activated(int)),    this, SLOT(onSelChangeFoil2(int)));
    connect(m_pctrlFrac,   SIGNAL(editingFinished()), this, SLOT(onFrac()));
    connect(m_pctrlSlider, SIGNAL(sliderMoved(int)),  this, SLOT(onVScroll(int)));
}


void InterpolateFoilsDlg::setupLayout()
{
    QVBoxLayout *pLeftSide = new QVBoxLayout;
    {
        m_pctrlFoil1 = new QComboBox;
        m_pctrlFoil1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_pctrlFoil2 = new QComboBox;
        m_pctrlCamb1 = new QLabel(tr("Camb1"));
        m_pctrlCamb2 = new QLabel(tr("Camb2"));
        m_pctrlThick1 = new QLabel(tr("Thick1"));
        m_pctrlThick2 = new QLabel(tr("Thick2"));
        m_pctrlCamb1->setMinimumWidth(250);
        m_pctrlCamb2->setMinimumWidth(250);
        m_pctrlThick1->setMinimumWidth(250);
        m_pctrlThick2->setMinimumWidth(250);
        pLeftSide->addWidget(m_pctrlFoil1);
        pLeftSide->addWidget(m_pctrlCamb1);
        pLeftSide->addWidget(m_pctrlThick1);
        pLeftSide->addStretch(1);
        pLeftSide->addWidget(m_pctrlFoil2);
        pLeftSide->addWidget(m_pctrlCamb2);
        pLeftSide->addWidget(m_pctrlThick2);
    }


    m_pctrlSlider = new QSlider;
    m_pctrlSlider->setMinimumHeight(300);
    m_pctrlSlider->setMinimum(0);
    m_pctrlSlider->setMaximum(SLIDERSCALE);
    m_pctrlSlider->setTickInterval(SLIDERSCALE/10);
    m_pctrlSlider->setTickPosition(QSlider::TicksLeft);


    QVBoxLayout *pFoil3Layout = new QVBoxLayout;
    {
        m_pctrlFrac = new DoubleEdit;
        m_pctrlCamb3 = new QLabel(tr("Camb3"));
        m_pctrlCamb3->setMinimumWidth(250);
        m_pctrlThick3 = new QLabel(tr("Thick3"));
        m_pctrlThick3->setMinimumWidth(250);
        m_pctrlFrac->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        m_pctrlCamb3->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        m_pctrlThick3->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        pFoil3Layout->addStretch();
        pFoil3Layout->addWidget(m_pctrlFrac);
        pFoil3Layout->addWidget(m_pctrlCamb3);
        pFoil3Layout->addWidget(m_pctrlThick3);
        pFoil3Layout->addStretch();
    }

    QHBoxLayout *pCommandButtons = new QHBoxLayout;
    {
        OKButton = new QPushButton(tr("OK"));
        CancelButton = new QPushButton(tr("Cancel"));
        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(OKButton);
        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(CancelButton);
        pCommandButtons->addStretch(1);
        connect(OKButton, SIGNAL(clicked()),this, SLOT(onOK()));
        connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    }


    QHBoxLayout *pColumnLayout = new QHBoxLayout;
    {
        pColumnLayout->addLayout(pLeftSide);
        pColumnLayout->addStretch(1);
        pColumnLayout->addWidget(m_pctrlSlider);
        pColumnLayout->addLayout(pFoil3Layout);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addLayout(pColumnLayout);
        pMainLayout->addLayout(pCommandButtons);
    }

    setLayout(pMainLayout);
    setMinimumWidth(400);
    setMinimumHeight(400);
}


void InterpolateFoilsDlg::initDialog()
{
    int i;
    Foil* pFoil;
    m_pctrlFoil1->clear();
    m_pctrlFoil2->clear();
    for (i=0; i<m_poaFoil->size(); i++)
    {
        pFoil = (Foil*)m_poaFoil->at(i);
        if(pFoil)
        {
            m_pctrlFoil1->addItem(pFoil->foilName());
            m_pctrlFoil2->addItem(pFoil->foilName());
        }
    }
    m_pctrlFoil1->setCurrentIndex(0);
    m_pctrlFoil2->setCurrentIndex(1);

    m_Frac = 0.0;
    m_pctrlFrac->setValue(100);
    m_pctrlSlider->setSliderPosition(SLIDERSCALE);

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
            if(!OKButton->hasFocus() && !CancelButton->hasFocus())
            {
                update();
                OKButton->setFocus();
            }
            else if (OKButton->hasFocus())
            {
                onOK();
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
    QString strong  = m_pctrlFoil1->currentText();

    Foil* pFoil = Objects2d::foil(strong);

    if(pFoil)
    {
        QString str;
        str = QString(tr("Camb.=%1")).arg(pFoil->camber()*100,5,'f',2);
        str += "%";
        strong = QString(tr(" at x=%1")).arg(pFoil->xCamber()*100,5,'f',1);
        strong += "%";
        str+=strong;
        m_pctrlCamb1->setText(str);

        str = QString(tr("Thick.=%1")).arg(pFoil->thickness()*100,5,'f',2);
        str += "%";
        strong = QString(tr(" at x=%1")).arg(pFoil->xThickness()*100,5,'f',1);
        strong += "%";
        str+=strong;
        m_pctrlThick1->setText(str);

        m_NewFoilName = pFoil->foilName();
    }
    update();
}


void InterpolateFoilsDlg::onSelChangeFoil2(int)
{
    QString strong  = m_pctrlFoil2->currentText();

    Foil* pFoil = Objects2d::foil(strong);

    if(pFoil)
    {
        QString str;
        str = QString(tr("Camb.=%1")).arg(pFoil->camber()*100,5,'f',2);
        str += "%";
        strong = QString(tr(" at x=%1")).arg(pFoil->xCamber()*100,5,'f',1);
        strong += "%";
        str+=strong;
        m_pctrlCamb2->setText(str);

        str = QString(tr("Thick.=%1")).arg(pFoil->thickness()*100,5,'f',2);
        str += "%";
        strong = QString(tr(" at x=%1")).arg(pFoil->xThickness()*100,5,'f',1);
        strong += "%";
        str+=strong;
        m_pctrlThick2->setText(str);
    }
    update();
}


void InterpolateFoilsDlg::update()
{
    XFoil *pXFoil = (XFoil*)s_pXFoil;
    QString strong;

    strong = m_pctrlFoil1->currentText();
    Foil* pFoil1 = Objects2d::foil(strong);

    strong = m_pctrlFoil2->currentText();
    Foil* pFoil2 = Objects2d::foil(strong);

    if(!pFoil1 || !pFoil2) return;

    pXFoil->interpolate(pFoil1->x, pFoil1->y, pFoil1->n,
                        pFoil2->x, pFoil2->y, pFoil2->n,
                        m_Frac/100.0);
/*
qDebug()<<pFoil1->foilName();
pFoil1->displayCoords();
qDebug()<<"________";
qDebug()<<pFoil2->foilName();
pFoil2->displayCoords(false);
qDebug()<<"________";*/


    for (int j=0; j< pFoil1->n; j++)
    {
        m_pBufferFoil->x[j]  = pXFoil->xb[j+1];
        m_pBufferFoil->y[j]  = pXFoil->yb[j+1];
        m_pBufferFoil->xb[j] = pXFoil->xb[j+1];
        m_pBufferFoil->yb[j] = pXFoil->yb[j+1];
    }

    m_pBufferFoil->n  = pFoil1->n;
    m_pBufferFoil->nb = pFoil1->n;

    m_pBufferFoil->initFoil();

    QString str;
    str = QString(tr("Camb.=%1")).arg(m_pBufferFoil->camber()*100,5,'f',2);
    str += "%";
    strong = QString(tr(" at x=%1")).arg(m_pBufferFoil->xCamber()*100,5,'f',1);
    strong += "%";
    str+=strong;
    m_pctrlCamb3->setText(str);

    str = QString(tr("Thick.=%1")).arg(m_pBufferFoil->thickness()*100,5,'f',2);
    str += "%";
    strong = QString(tr(" at x=%1")).arg(m_pBufferFoil->xThickness()*100,5,'f',1);
    strong += "%";
    str+=strong;
    m_pctrlThick3->setText(str);
    m_pParent->update();
}


void InterpolateFoilsDlg::onFrac()
{
    m_Frac = m_pctrlFrac->value();
    m_pctrlSlider->setSliderPosition((int)(m_Frac/100.0*SLIDERSCALE));
    m_Frac = 100.0 - m_Frac;

    update();
}


void InterpolateFoilsDlg::onOK()
{
    m_pBufferFoil->setFoilName(m_NewFoilName);

    QDialog::accept();
}


void InterpolateFoilsDlg::onVScroll(int val)
{
    val = m_pctrlSlider->sliderPosition();
    m_Frac = (SLIDERSCALE - (double)val)/SLIDERSCALE*100.0;
    m_pctrlFrac->setValue(100.0-m_Frac);
    update();
}










