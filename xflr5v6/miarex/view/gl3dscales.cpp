/****************************************************************************

    GL3DScales Class
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

#include <QGridLayout>
#include <QGroupBox>
#include <QDockWidget>


#include "gl3dscales.h"
#include <miarex/miarex.h>
#include <miarex/view/gl3dmiarexview.h>
#include <xflcore/displayoptions.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflwidgets/customwts/exponentialslider.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/customwts/intedit.h>


Miarex *GL3DScales::s_pMiarex;

int GL3DScales::s_pos = 1;
int GL3DScales::s_NX = 30;
double GL3DScales::s_DeltaL = 0.01;
double GL3DScales::s_XFactor = 1.10;
double GL3DScales::s_XOffset = 0.0;
double GL3DScales::s_ZOffset = 0.0;


GL3DScales::GL3DScales(QWidget *pParent) : QWidget(pParent)
{
    setAttribute(Qt::WA_DeleteOnClose, false);
    setWindowTitle(tr("3D Scales Settings"));

    s_pos       = 1;
    s_NX        =   30;
    s_XFactor   = 1.10;
    s_DeltaL    =  0.01;
    s_XOffset   =  0.0;
    s_ZOffset   =  0.0;

    setupLayout();

    connect(m_ppbApply, SIGNAL(clicked()),this, SLOT(onApply()));

    connect(m_pchAutoCpScale, SIGNAL(clicked()), this, SLOT(onCpScale()));
    connect(m_pdeLegendMin, SIGNAL(editingFinished()), this, SLOT(onCpScale()));
    connect(m_pdeLegendMax, SIGNAL(editingFinished()), this, SLOT(onCpScale()));

    connect(m_peslLiftScaleSlider, SIGNAL(sliderMoved(int)), this, SLOT(onLiftScale()));
    connect(m_peslDragScaleSlider, SIGNAL(sliderMoved(int)), this, SLOT(onDragScale()));
    connect(m_peslVelocityScaleSlider, SIGNAL(sliderMoved(int)), this, SLOT(onVelocityScale()));

    connect(m_pdeLiftScale, SIGNAL(editingFinished()), this, SLOT(onLiftEdit()));
    connect(m_pdeDragScale, SIGNAL(editingFinished()), this, SLOT(onDragEdit()));
    connect(m_pdeVelocityScale, SIGNAL(editingFinished()), this, SLOT(onVelocityEdit()));

}


void GL3DScales::setupLayout()
{
    QSizePolicy szPolicyMinimum;
    szPolicyMinimum.setHorizontalPolicy(QSizePolicy::Minimum);
    szPolicyMinimum.setVerticalPolicy(QSizePolicy::Minimum);

    setSizePolicy(szPolicyMinimum);

    QSizePolicy szPolicyExpanding;
    szPolicyExpanding.setHorizontalPolicy(QSizePolicy::Expanding);
    szPolicyExpanding.setVerticalPolicy(QSizePolicy::Expanding);

    QGroupBox *pCpScaleBox = new QGroupBox(tr("Cp Scale"));
    {
        QGridLayout *pCpScaleLayout = new QGridLayout;
        {
            m_pchAutoCpScale = new QCheckBox(tr("Auto Scales"));
            m_pdeLegendMin = new DoubleEdit(-1.0);
            m_pdeLegendMax = new DoubleEdit(1.0);
            m_pdeLegendMin->setDigits(2);
            m_pdeLegendMax->setDigits(2);
            QLabel *lab0 = new QLabel(tr("Min"));
            QLabel *lab1 = new QLabel(tr("Max"));
            lab0->setAlignment(Qt::AlignVCenter |Qt::AlignRight);
            lab1->setAlignment(Qt::AlignVCenter |Qt::AlignRight);
            pCpScaleLayout->addWidget(m_pchAutoCpScale,1,2);
            pCpScaleLayout->addWidget(lab1,2,1);
            pCpScaleLayout->addWidget(m_pdeLegendMax,2,2);
            pCpScaleLayout->addWidget(lab0,3,1);
            pCpScaleLayout->addWidget(m_pdeLegendMin,3,2);
        }
        pCpScaleBox->setLayout(pCpScaleLayout);
    }

//_______________________3D Scales

    QGroupBox *pScaleBox = new QGroupBox(tr("Vector Scales"));
    {
        QHBoxLayout *pScaleLayout = new QHBoxLayout;
        {
            QVBoxLayout *pSliderLayout = new QVBoxLayout;
            {
                QFontMetrics fm(DisplayOptions::textFont());
                int w = 6 * fm.averageCharWidth();
                QHBoxLayout *pLiftScaleLayout = new QHBoxLayout;
                {
                    m_peslLiftScaleSlider  = new ExponentialSlider(false, 3.0, Qt::Horizontal);
                    m_peslLiftScaleSlider->setMinimum(0);
                    m_peslLiftScaleSlider->setMaximum(100);
                    m_peslLiftScaleSlider->setSliderPosition(50);
                    m_peslLiftScaleSlider->setTickInterval(5);
                    m_peslLiftScaleSlider->setTickPosition(QSlider::TicksBelow);
                    m_pdeLiftScale = new DoubleEdit(0,1);
                    m_pdeLiftScale->setMaximumWidth(w);
                    pLiftScaleLayout->addWidget(m_peslLiftScaleSlider);
                    pLiftScaleLayout->addWidget(m_pdeLiftScale);
                }
                QHBoxLayout *pDragScaleLayout = new QHBoxLayout;
                {
                    m_peslDragScaleSlider = new ExponentialSlider(false, 3.0, Qt::Horizontal);
                    m_peslDragScaleSlider->setMinimum(0);
                    m_peslDragScaleSlider->setMaximum(100);
                    m_peslDragScaleSlider->setSliderPosition(50);
                    m_peslDragScaleSlider->setTickInterval(5);
                    m_peslDragScaleSlider->setTickPosition(QSlider::TicksBelow);
                    m_pdeDragScale = new DoubleEdit(0,1);
                    m_pdeDragScale->setMaximumWidth(w);
                    pDragScaleLayout->addWidget(m_peslDragScaleSlider);
                    pDragScaleLayout->addWidget(m_pdeDragScale);
                }
                QHBoxLayout *pVelocityScaleLayout = new QHBoxLayout;
                {
                    m_peslVelocityScaleSlider  = new ExponentialSlider(false, 3.0, Qt::Horizontal);
                    m_peslVelocityScaleSlider->setMinimum(0);
                    m_peslVelocityScaleSlider->setMaximum(100);
                    m_peslVelocityScaleSlider->setSliderPosition(50);
                    m_peslVelocityScaleSlider->setTickInterval(5);
                    m_peslVelocityScaleSlider->setTickPosition(QSlider::TicksBelow);
                    m_pdeVelocityScale = new DoubleEdit(0,1);
                    m_pdeVelocityScale->setMaximumWidth(w);
                    pVelocityScaleLayout->addWidget(m_peslVelocityScaleSlider);
                    pVelocityScaleLayout->addWidget(m_pdeVelocityScale);
                }
                pSliderLayout->addLayout(pLiftScaleLayout);
                pSliderLayout->addLayout(pDragScaleLayout);
                pSliderLayout->addLayout(pVelocityScaleLayout);
            }

            QVBoxLayout *pLabelLayout = new QVBoxLayout;
            {
                QLabel *lab2 = new QLabel(tr("Lift "));
                QLabel *lab3 = new QLabel(tr("Drag "));
                QLabel *lab4 = new QLabel(tr("Velocity "));
                pLabelLayout->addWidget(lab2);
                pLabelLayout->addWidget(lab3);
                pLabelLayout->addWidget(lab4);
            }

            pScaleLayout->addLayout(pLabelLayout);
            pScaleLayout->addLayout(pSliderLayout);
        }
        pScaleBox->setLayout(pScaleLayout);
    }

//__________________________________    Streamlines

    QGroupBox *pLengthBox = new QGroupBox(tr("Streamline length"));
    {
        m_pieNXPoint = new IntEdit(0);
        m_pdeDeltaL = new DoubleEdit(12.34,2);
        m_pdeXFactor       = new DoubleEdit(1.23,2);
        m_plabLengthUnit1 = new QLabel("miles");
        QLabel *lab5 = new QLabel(tr("X-axis points"));
        QLabel *lab6 = new QLabel(tr("1st segment"));
        QLabel *lab7 = new QLabel(tr("X factor"));
        lab5->setAlignment(Qt::AlignVCenter |Qt::AlignRight);
        lab6->setAlignment(Qt::AlignVCenter |Qt::AlignRight);
        lab7->setAlignment(Qt::AlignVCenter |Qt::AlignRight);
        QGridLayout *pLengthLayout = new QGridLayout;
        {
            pLengthLayout->addWidget(lab5, 1, 1);
            pLengthLayout->addWidget(m_pieNXPoint , 1, 2);
            pLengthLayout->addWidget(lab6, 2, 1);
            pLengthLayout->addWidget(m_pdeDeltaL, 2, 2);
            pLengthLayout->addWidget(m_plabLengthUnit1, 2, 3);
            pLengthLayout->addWidget(lab7, 3, 1);
            pLengthLayout->addWidget(m_pdeXFactor, 3, 2);
        }
        pLengthBox->setLayout(pLengthLayout);
    }

    QGroupBox *pStartBox = new QGroupBox(tr("Start Streamline at"));
    {
        QVBoxLayout *pStartLayout = new QVBoxLayout;
        {
            m_pdeXOffset       = new DoubleEdit(4.56,3);
            m_pdeZOffset       = new DoubleEdit(7.89,3);
            m_plabLengthUnit2 = new QLabel("km");
            m_plabLengthUnit3 = new QLabel("m");
            m_prbLE = new QRadioButton(tr("L.E."));
            m_pebTE = new QRadioButton(tr("T.E."));
            m_prbLine = new QRadioButton(tr("Y-Line"));
            QLabel *lab8 = new QLabel(tr("X-Offset"));
            QLabel *lab9 = new QLabel(tr("Z-Offset"));
            lab8->setAlignment(Qt::AlignVCenter |Qt::AlignRight);
            lab9->setAlignment(Qt::AlignVCenter |Qt::AlignRight);
            QHBoxLayout *pLineLayout = new QHBoxLayout;
            {
                pLineLayout->addWidget(m_prbLE);
                pLineLayout->addWidget(m_pebTE);
                pLineLayout->addWidget(m_prbLine);
            }
            QGridLayout *pOffsetLayout = new QGridLayout;
            {
                pOffsetLayout->addWidget(lab8,1,1);
                pOffsetLayout->addWidget(m_pdeXOffset,1,2);
                pOffsetLayout->addWidget(m_plabLengthUnit2,1,3);
                pOffsetLayout->addWidget(lab9,2,1);
                pOffsetLayout->addWidget(m_pdeZOffset,2,2);
                pOffsetLayout->addWidget(m_plabLengthUnit3,2,3);
            }
            pStartLayout->addLayout(pLineLayout);
            pStartLayout->addLayout(pOffsetLayout);
        }
        pStartBox->setLayout(pStartLayout);
    }

    QGroupBox *pStreamBox = new QGroupBox(tr("Streamlines"));
    {
        m_ppbApply = new QPushButton(tr("Apply"));
        QVBoxLayout *StreamLayout = new QVBoxLayout;
        StreamLayout->addWidget(pLengthBox);
        StreamLayout->addWidget(pStartBox);
        StreamLayout->addStretch(1);
        StreamLayout->addWidget(m_ppbApply);
        StreamLayout->addStretch(1);
        pStreamBox->setLayout(StreamLayout);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(pScaleBox);
        pMainLayout->addSpacing(15);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(pCpScaleBox);
        pMainLayout->addSpacing(15);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(pStreamBox);
        pMainLayout->addStretch(1);
    }

    setLayout(pMainLayout);
}


void GL3DScales::initDialog()
{
    QString str;

    Units::getLengthUnitLabel(str);
    m_plabLengthUnit1->setText(str);
    m_plabLengthUnit2->setText(str);
    m_plabLengthUnit3->setText(str);

    m_pchAutoCpScale->setChecked(gl3dMiarexView::s_bAutoCpScale);
    m_pdeLegendMin->setValue(gl3dMiarexView::s_LegendMin);
    m_pdeLegendMax->setValue(gl3dMiarexView::s_LegendMax);
    m_pdeLegendMin->setEnabled(!gl3dMiarexView::s_bAutoCpScale);
    m_pdeLegendMax->setEnabled(!gl3dMiarexView::s_bAutoCpScale);

    m_peslLiftScaleSlider->setExpValue(gl3dMiarexView::s_LiftScale);
    m_peslDragScaleSlider->setExpValue(gl3dMiarexView::s_DragScale);
    m_peslVelocityScaleSlider->setExpValue(gl3dMiarexView::s_VelocityScale);

    m_pdeLiftScale->setValue(gl3dMiarexView::s_LiftScale);
    m_pdeDragScale->setValue(gl3dMiarexView::s_DragScale);
    m_pdeVelocityScale->setValue(gl3dMiarexView::s_VelocityScale);


    if(s_pos==0)        m_prbLE->setChecked(true);
    else if(s_pos==1)    m_pebTE->setChecked(true);
    else if(s_pos==2)    m_prbLine->setChecked(true);

    m_pdeDeltaL->setValue(s_DeltaL* Units::mtoUnit());
    m_pdeXOffset->setValue(s_XOffset* Units::mtoUnit());
    m_pdeZOffset->setValue(s_ZOffset* Units::mtoUnit());
    m_pdeXFactor->setValue(s_XFactor);
    m_pieNXPoint->setValue(s_NX);
}


void GL3DScales::onCpScale()
{
    gl3dMiarexView::s_bAutoCpScale = m_pchAutoCpScale->isChecked();
    if(!gl3dMiarexView::s_bAutoCpScale)
    {
        gl3dMiarexView::s_LegendMax = m_pdeLegendMax->value();
        gl3dMiarexView::s_LegendMin = m_pdeLegendMin->value();
    }
    m_pdeLegendMin->setEnabled(!gl3dMiarexView::s_bAutoCpScale);
    m_pdeLegendMax->setEnabled(!gl3dMiarexView::s_bAutoCpScale);

    gl3dMiarexView::s_bResetglPanelCp = true;
    gl3dMiarexView::s_bResetglLegend = true;
    s_pMiarex->m_bResetTextLegend = true;
    s_pMiarex->updateView();
}


void GL3DScales::onApply()
{
    gl3dMiarexView::s_LegendMax = m_pdeLegendMax->value();
    gl3dMiarexView::s_LegendMin = m_pdeLegendMin->value();
    gl3dMiarexView::s_bAutoCpScale = m_pchAutoCpScale->isChecked();
    readStreamParams();
    gl3dMiarexView::s_bResetglStream = true;
    s_pMiarex->updateView();
}



void GL3DScales::onLiftEdit()
{
    gl3dMiarexView::s_LiftScale = m_pdeLiftScale->value();
    m_peslLiftScaleSlider->setValue(int(gl3dMiarexView::s_LiftScale));
    gl3dMiarexView::s_bResetglLift = true;
    gl3dMiarexView::s_bResetglPanelForce = true;
    s_pMiarex->updateView();
}


void GL3DScales::onDragEdit()
{
    gl3dMiarexView::s_DragScale = m_pdeDragScale->value();
    m_peslDragScaleSlider->setValue(int(gl3dMiarexView::s_DragScale));
    gl3dMiarexView::s_bResetglDrag = true;
    s_pMiarex->updateView();
}


void GL3DScales::onVelocityEdit()
{
    gl3dMiarexView::s_VelocityScale = m_pdeVelocityScale->value();
    m_peslVelocityScaleSlider->setValue(int(gl3dMiarexView::s_VelocityScale));
    gl3dMiarexView::s_bResetglDownwash = true;
    gl3dMiarexView::s_bResetglSurfVelocities = true;
    s_pMiarex->updateView();

}


void GL3DScales::onLiftScale()
{
    gl3dMiarexView::s_LiftScale    = m_peslLiftScaleSlider->expValue();
    m_pdeLiftScale->setValue(gl3dMiarexView::s_LiftScale);
    gl3dMiarexView::s_bResetglLift = true;
    gl3dMiarexView::s_bResetglPanelForce = true;
    s_pMiarex->updateView();
}


void GL3DScales::onDragScale()
{
    gl3dMiarexView::s_DragScale    = m_peslDragScaleSlider->expValue();
    m_pdeDragScale->setValue(gl3dMiarexView::s_DragScale);
    gl3dMiarexView::s_bResetglDrag = true;
    s_pMiarex->updateView();
}


void GL3DScales::onVelocityScale()
{
    gl3dMiarexView::s_VelocityScale    = m_peslVelocityScaleSlider->expValue();
    m_pdeVelocityScale->setValue(gl3dMiarexView::s_VelocityScale);
    gl3dMiarexView::s_bResetglDownwash = true;
    gl3dMiarexView::s_bResetglSurfVelocities = true;
    s_pMiarex->updateView();
}


void GL3DScales::showEvent(QShowEvent *event)
{
    initDialog();
    event->accept();
}


void GL3DScales::hideEvent(QHideEvent *event)
{
    s_pMiarex->setControls();
    event->accept();
}


void GL3DScales::readStreamParams()
{
    s_NX = m_pieNXPoint->value();
    s_XOffset = m_pdeXOffset->value() / Units::mtoUnit();
    s_ZOffset = m_pdeZOffset->value() / Units::mtoUnit();
    s_DeltaL  = m_pdeDeltaL->value()  / Units::mtoUnit();
    s_XFactor = m_pdeXFactor->value();

    if(m_prbLE->isChecked())            s_pos=0;
    else if(m_pebTE->isChecked())     s_pos=1;
    else if(m_prbLine->isChecked())   s_pos=2;
}



bool GL3DScales::loadSettings(QSettings &settings)
{
    settings.beginGroup("GL3DScales");
    {
        gl3dMiarexView::s_bAutoCpScale = settings.value("AutoCpScale").toBool();
        gl3dMiarexView::s_LegendMin    = settings.value("LegendMin").toDouble();
        gl3dMiarexView::s_LegendMax    = settings.value("LegendMax").toDouble();
        s_pos     = settings.value("Position").toInt();
        s_NX      = settings.value("NX").toInt();
        s_DeltaL  = settings.value("DeltaL").toDouble();
        s_XFactor = settings.value("XFactor").toDouble();
        s_XOffset = settings.value("XOffset").toDouble();
        s_ZOffset = settings.value("ZOffset").toDouble();
    }
    settings.endGroup();
    return true;
}


bool GL3DScales::saveSettings(QSettings &settings)
{
    settings.beginGroup("GL3DScales");
    {
        settings.setValue("AutoCpScale", gl3dMiarexView::s_bAutoCpScale);
        settings.setValue("LegendMin", gl3dMiarexView::s_LegendMin);
        settings.setValue("LegendMax", gl3dMiarexView::s_LegendMax);
        settings.setValue("Position", s_pos);
        settings.setValue("NX", s_NX);
        settings.setValue("DeltaL", s_DeltaL);
        settings.setValue("XFactor", s_XFactor);
        settings.setValue("XOffset", s_XOffset);
        settings.setValue("ZOffset", s_ZOffset);
    }
    settings.endGroup();
    return true;
}





