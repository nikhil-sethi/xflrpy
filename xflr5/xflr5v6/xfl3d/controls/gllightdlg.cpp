/****************************************************************************

    GLLightDlg class
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

#include <QGroupBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QShowEvent>

#include "gllightdlg.h"
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/customwts/exponentialslider.h>
#include <xflcore/units.h>
#include <xfl3d/views/gl3dview.h>


QByteArray GLLightDlg::s_Geometry;


GLLightDlg::GLLightDlg(QWidget *pParent) : QDialog(pParent)
{
    m_pglView = nullptr;

    setWindowTitle(tr("OpenGL Light Options"));
    setModal(false);
    setWindowFlag(Qt::WindowStaysOnTopHint);
//    setAttribute(Qt::WA_DeleteOnClose);

    setupLayout();
    connectSignals();
}


void GLLightDlg::connectSignals()
{
    connect(m_plabLight,               SIGNAL(clicked()),         SLOT(onLight()));
    connect(m_ppbClose,                SIGNAL(clicked()),         SLOT(accept()));
    connect(m_ppbDefaults,             SIGNAL(clicked()),         SLOT(onDefaults()));

    connect(m_pslRed,                  SIGNAL(sliderMoved(int)),  SLOT(onChanged()));
    connect(m_pslGreen,                SIGNAL(sliderMoved(int)),  SLOT(onChanged()));
    connect(m_pslBlue,                 SIGNAL(sliderMoved(int)),  SLOT(onChanged()));
    connect(m_peslLightAmbient,        SIGNAL(sliderMoved(int)),  SLOT(onChanged()));
    connect(m_peslLightDiffuse,        SIGNAL(sliderMoved(int)),  SLOT(onChanged()));
    connect(m_peslLightSpecular,       SIGNAL(sliderMoved(int)),  SLOT(onChanged()));
    connect(m_peslXLight,              SIGNAL(sliderMoved(int)),  SLOT(onChanged()));
    connect(m_peslYLight,              SIGNAL(sliderMoved(int)),  SLOT(onChanged()));
    connect(m_peslZLight,              SIGNAL(sliderMoved(int)),  SLOT(onChanged()));

    connect(m_pslMatShininess,         SIGNAL(sliderMoved(int)),  SLOT(onChanged()));

    connect(m_pdeConstantAttenuation,  SIGNAL(editingFinished()), SLOT(onChanged()));
    connect(m_pdeLinearAttenuation,    SIGNAL(editingFinished()), SLOT(onChanged()));
    connect(m_pdeQuadAttenuation,      SIGNAL(editingFinished()), SLOT(onChanged()));

}


void GLLightDlg::setupLayout()
{
    QGroupBox *pLightIntensityBox = new QGroupBox(tr("Light Intensity"));
    {
        QVBoxLayout *pLightIntensities = new QVBoxLayout;
        QGridLayout *pLightIntensityLayout = new QGridLayout;
        {
            QLabel *pLab1 = new QLabel(tr("Diffuse"));
            QLabel *pLab2 = new QLabel(tr("Ambient"));
            QLabel *pLab3 = new QLabel(tr("Specular"));

            m_peslLightAmbient      = new ExponentialSlider(false, 2.0, Qt::Horizontal);
            m_peslLightAmbient->setToolTip("Ambient:\n"
                                            "Bounced light which has been scattered so much that it\n"
                                            "is impossible to tell the direction to its source.\n"
                                            "It is not attenuated by distance, and disappears if\n"
                                            "the light is turned off.");
            m_peslLightDiffuse      = new ExponentialSlider(false, 2.0, Qt::Horizontal);
            m_peslLightDiffuse->setToolTip("Diffuse:\n"
                                            "Directional light which is brighter on perpendicular\n"
                                            "surfaces. Its reflection is scattered evenly.");
            m_peslLightSpecular     = new ExponentialSlider(false, 2.0, Qt::Horizontal);
            m_peslLightSpecular->setToolTip("Specular:\n"
                                             "Directional light which tends to reflect in a preferred\n"
                                             "direction. It is associated with shininess.");

            m_peslLightAmbient->setMinimum(0);
            m_peslLightAmbient->setMaximum(100);
            m_peslLightAmbient->setTickInterval(10);
            m_peslLightDiffuse->setMinimum(0);
            m_peslLightDiffuse->setMaximum(100);
            m_peslLightDiffuse->setTickInterval(10);
            m_peslLightSpecular->setMinimum(0);
            m_peslLightSpecular->setMaximum(100);
            m_peslLightSpecular->setTickInterval(10);
            m_peslLightDiffuse->setTickPosition(QSlider::TicksBelow);
            m_peslLightAmbient->setTickPosition(QSlider::TicksBelow);
            m_peslLightSpecular->setTickPosition(QSlider::TicksBelow);

            m_plabLightAmbient = new QLabel;
            m_plabLightDiffuse = new QLabel;
            m_plabLightSpecular = new QLabel;
            pLightIntensityLayout->addWidget(pLab2,1,1);
            pLightIntensityLayout->addWidget(pLab1,2,1);
            pLightIntensityLayout->addWidget(pLab3,3,1);
            pLightIntensityLayout->addWidget(m_peslLightAmbient,1,2);
            pLightIntensityLayout->addWidget(m_peslLightDiffuse,2,2);
            pLightIntensityLayout->addWidget(m_peslLightSpecular,3,2);
            pLightIntensityLayout->addWidget(m_plabLightAmbient,1,3);
            pLightIntensityLayout->addWidget(m_plabLightDiffuse,2,3);
            pLightIntensityLayout->addWidget(m_plabLightSpecular,3,3);
        }

        QHBoxLayout *pAttenuationLayout = new QHBoxLayout;
        {
            QLabel *pAtt = new QLabel(tr("Attenuation factor=1/("));
            QLabel *pConstant = new QLabel("+");
            QLabel *pLinear = new QLabel(".d +");
            QLabel *pQuadratic = new QLabel(QString::fromUtf8(".d²)"));
            pConstant->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            pLinear->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            pQuadratic->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            m_pdeConstantAttenuation = new DoubleEdit(0.0);
            m_pdeLinearAttenuation = new DoubleEdit(0.0);
            m_pdeQuadAttenuation = new DoubleEdit(0.0);

            pAttenuationLayout->addWidget(pAtt);
            pAttenuationLayout->addWidget(m_pdeConstantAttenuation);
            pAttenuationLayout->addWidget(pConstant);
            pAttenuationLayout->addWidget(m_pdeLinearAttenuation);
            pAttenuationLayout->addWidget(pLinear);
            pAttenuationLayout->addWidget(m_pdeQuadAttenuation);
            pAttenuationLayout->addWidget(pQuadratic);
        }
        pLightIntensities->addLayout(pLightIntensityLayout);
        pLightIntensities->addLayout(pAttenuationLayout);
        pLightIntensityBox->setLayout(pLightIntensities);
    }

    QGroupBox *pLightColorBox = new QGroupBox(tr("Light Color"));
    {
        QGridLayout *pLightColor = new QGridLayout;
        {
            QLabel *pLab11 = new QLabel(tr("Red"));
            QLabel *pLab12 = new QLabel(tr("Green"));
            QLabel *pLab13 = new QLabel(tr("Blue"));
            m_pslRed    = new QSlider(Qt::Horizontal);
            m_pslGreen  = new QSlider(Qt::Horizontal);
            m_pslBlue   = new QSlider(Qt::Horizontal);
            m_pslRed->setMinimum(0);
            m_pslRed->setMaximum(100);
            m_pslRed->setTickInterval(10);
            m_pslGreen->setMinimum(0);
            m_pslGreen->setMaximum(100);
            m_pslGreen->setTickInterval(10);
            m_pslBlue->setMinimum(0);
            m_pslBlue->setMaximum(100);
            m_pslBlue->setTickInterval(10);
            m_pslRed->setTickPosition(QSlider::TicksBelow);
            m_pslGreen->setTickPosition(QSlider::TicksBelow);
            m_pslBlue->setTickPosition(QSlider::TicksBelow);

            m_plabLightRed   = new QLabel;
            m_plabLightGreen = new QLabel;
            m_plabLightBlue  = new QLabel;

            pLightColor->addWidget(pLab11,1,1);
            pLightColor->addWidget(pLab12,2,1);
            pLightColor->addWidget(pLab13,3,1);
            pLightColor->addWidget(m_pslRed,1,2);
            pLightColor->addWidget(m_pslGreen,2,2);
            pLightColor->addWidget(m_pslBlue,3,2);
            pLightColor->addWidget(m_plabLightRed,1,3);
            pLightColor->addWidget(m_plabLightGreen,2,3);
            pLightColor->addWidget(m_plabLightBlue,3,3);
            pLightColorBox->setLayout(pLightColor);
        }
    }

    QGroupBox *pLightPositionBox = new QGroupBox(tr("Light Position"));
    {
        QGridLayout *pLightPosition = new QGridLayout;
        {
            QLabel *plab21 = new QLabel(tr("x"));
            QLabel *plab22 = new QLabel(tr("y"));
            QLabel *plab23 = new QLabel(tr("z"));

            m_peslXLight = new ExponentialSlider(true, 2.0, Qt::Horizontal);
            m_peslYLight = new ExponentialSlider(true, 2.0, Qt::Horizontal);
            m_peslZLight = new ExponentialSlider(true, 2.0, Qt::Horizontal);
            m_peslXLight->setTickPosition(QSlider::TicksBelow);
            m_peslYLight->setTickPosition(QSlider::TicksBelow);
            m_peslZLight->setTickPosition(QSlider::TicksBelow);
            m_plabPosXValue = new QLabel(Units::lengthUnitLabel());
            m_plabPosYValue = new QLabel(Units::lengthUnitLabel());
            m_plabPosZValue = new QLabel(Units::lengthUnitLabel());

            pLightPosition->addWidget(plab21,1,1);
            pLightPosition->addWidget(plab22,2,1);
            pLightPosition->addWidget(plab23,3,1);
            pLightPosition->addWidget(m_peslXLight,1,2);
            pLightPosition->addWidget(m_peslYLight,2,2);
            pLightPosition->addWidget(m_peslZLight,3,2);
            pLightPosition->addWidget(m_plabPosXValue,1,3);
            pLightPosition->addWidget(m_plabPosYValue,2,3);
            pLightPosition->addWidget(m_plabPosZValue,3,3);
            pLightPositionBox->setLayout(pLightPosition);
        }
    }

    QHBoxLayout *pMaterialDataLayout = new QHBoxLayout;
    {
        m_pslMatShininess = new QSlider(Qt::Horizontal);
        m_pslMatShininess->setRange(4, 64);
        m_pslMatShininess->setTickInterval(2);
        m_pslMatShininess->setTickPosition(QSlider::TicksBelow);

        QLabel *plab35 = new QLabel(tr("Material Shininess"));
        m_plabMatShininess = new QLabel("1");

        pMaterialDataLayout->addWidget(plab35);
        pMaterialDataLayout->addWidget(m_pslMatShininess);
        pMaterialDataLayout->addWidget(m_plabMatShininess);
    }

    QHBoxLayout *pCommandButtons = new QHBoxLayout;
    {
        m_ppbClose = new QPushButton(tr("Close"));
        m_ppbDefaults = new QPushButton(tr("Reset Defaults"));
        m_ppbDefaults->setDefault(false);
        m_ppbDefaults->setAutoDefault(false);
        m_ppbClose->setDefault(false);
        m_ppbClose->setAutoDefault(false);
        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(m_ppbDefaults);
        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(m_ppbClose);
        pCommandButtons->addStretch(1);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_plabLight = new QCheckBox(tr("Light"));
        pMainLayout->addWidget(m_plabLight);
        pMainLayout->addStretch();
        pMainLayout->addWidget(pLightIntensityBox);
        pMainLayout->addStretch();
        pMainLayout->addWidget(pLightPositionBox);
        pMainLayout->addStretch();
        pMainLayout->addWidget(pLightColorBox);
        pMainLayout->addSpacing(11);
        pMainLayout->addStretch();
        pMainLayout->addLayout(pMaterialDataLayout);
        pMainLayout->addSpacing(11);
        pMainLayout->addStretch();
        pMainLayout->addLayout(pCommandButtons);
    }

    setLayout(pMainLayout);
}



void GLLightDlg::apply()
{
    readParams();
    setLabels();

    if(m_pglView)
    {
        m_pglView->glSetupLight();
        m_pglView->update();
    }
}



void GLLightDlg::onChanged()
{
    apply();
}


void GLLightDlg::onDefaults()
{
    setDefaults();
    setParams();
    setEnabled();

    if(m_pglView)
    {
        m_pglView->glSetupLight();
        m_pglView->update();
    }
}


void GLLightDlg::readParams(void)
{
    Light &light = gl3dView::s_Light;

    light.m_bIsLightOn = m_plabLight->isChecked();

    light.m_Red     = float(m_pslRed->value())    /100.0f;
    light.m_Green   = float(m_pslGreen->value())  /100.0f;
    light.m_Blue    = float(m_pslBlue->value())   /100.0f;

    light.m_X  = float(m_peslXLight->expValue())/100.0f;
    light.m_Y  = float(m_peslYLight->expValue())/100.0f;
    light.m_Z  = float(m_peslZLight->expValue())/100.0f;

    light.m_Ambient     = float(m_peslLightAmbient->expValue())  / 20.0f;
    light.m_Diffuse     = float(m_peslLightDiffuse->expValue())  / 20.0f;
    light.m_Specular    = float(m_peslLightSpecular->expValue()) / 20.0f;

    light.m_iShininess   = m_pslMatShininess->value();

    light.m_Attenuation.m_Constant  = float(m_pdeConstantAttenuation->value());
    light.m_Attenuation.m_Linear    = float(m_pdeLinearAttenuation->value());
    light.m_Attenuation.m_Quadratic = float(m_pdeQuadAttenuation->value());
}


void GLLightDlg::setParams(void)
{
    Light &light = gl3dView::s_Light;

    m_plabLight->setChecked(light.m_bIsLightOn);

    m_peslLightAmbient->setExpValue( double(light.m_Ambient)  *20.0);
    m_peslLightDiffuse->setExpValue( double(light.m_Diffuse)  *20.0);
    m_peslLightSpecular->setExpValue(double(light.m_Specular) *20.0);

    m_peslXLight->setRange(-int(LIGHTREFLENGTH*100), int(LIGHTREFLENGTH*100));
    m_peslYLight->setRange(-int(LIGHTREFLENGTH*100), int(LIGHTREFLENGTH*100));
    m_peslZLight->setRange(-int(LIGHTREFLENGTH*100), int(LIGHTREFLENGTH*100));
    m_peslXLight->setTickInterval(int(LIGHTREFLENGTH*10.0));
    m_peslYLight->setTickInterval(int(LIGHTREFLENGTH*10.0));
    m_peslZLight->setTickInterval(int(LIGHTREFLENGTH*10.0));
    m_peslXLight->setExpValue(double(light.m_X)*100.0);
    m_peslYLight->setExpValue(double(light.m_Y)*100.0);
    m_peslZLight->setExpValue(double(light.m_Z)*100.0);

    m_pslRed->setValue(  int(light.m_Red  *100.0f));
    m_pslGreen->setValue(int(light.m_Green*100.0f));
    m_pslBlue->setValue( int(light.m_Blue *100.0f));

    m_pslMatShininess->setValue(light.m_iShininess);

    m_pdeConstantAttenuation->setValue(double(light.m_Attenuation.m_Constant));
    m_pdeLinearAttenuation->setValue(  double(light.m_Attenuation.m_Linear));
    m_pdeQuadAttenuation->setValue(    double(light.m_Attenuation.m_Quadratic));

    setLabels();
}


void GLLightDlg::setLabels()
{
    Light &light = gl3dView::s_Light;

    QString strong;

    strong = QString::asprintf("%7.1f", double(light.m_Ambient));
    m_plabLightAmbient->setText(strong);
    strong = QString::asprintf("%7.1f", double(light.m_Diffuse));
    m_plabLightDiffuse->setText(strong);
    strong = QString::asprintf("%7.1f", double(light.m_Specular));
    m_plabLightSpecular->setText(strong);    strong = QString::asprintf("%7.1f", double(light.m_X)*Units::mtoUnit());
    m_plabPosXValue->setText(strong + Units::lengthUnitLabel());
    strong = QString::asprintf("%7.1f", double(light.m_Y)*Units::mtoUnit());
    m_plabPosYValue->setText(strong + Units::lengthUnitLabel());
    strong = QString::asprintf("%7.1f", double(light.m_Z)*Units::mtoUnit());
    m_plabPosZValue->setText(strong + Units::lengthUnitLabel());    strong = QString::asprintf("%7.1f", double(light.m_Red));
    m_plabLightRed->setText(strong);
    strong = QString::asprintf("%7.1f", double(light.m_Green));
    m_plabLightGreen->setText(strong);
    strong = QString::asprintf("%7.1f", double(light.m_Blue));
    m_plabLightBlue->setText(strong);

    strong = QString::asprintf("%d", light.m_iShininess);
    m_plabMatShininess->setText(strong);
}



bool GLLightDlg::loadSettings(QSettings &settings)
{
    Light &light = gl3dView::s_Light;
    settings.beginGroup("GLLightDlg");
    {
        s_Geometry = settings.value("WindowGeom", QByteArray()).toByteArray();

        light.m_Ambient           = settings.value("Ambient",   light.m_Ambient).toFloat();
        light.m_Diffuse           = settings.value("Diffuse",   light.m_Diffuse).toFloat();
        light.m_Specular          = settings.value("Specular",  light.m_Specular).toFloat();

        light.m_X                 = settings.value("XLight",    light.m_X).toFloat();
        light.m_Y                 = settings.value("YLight",    light.m_Y).toFloat();
        light.m_Z                 = settings.value("ZLight",    light.m_Z).toFloat();

        light.m_Red               = settings.value("RedLight",  light.m_Red).toFloat();
        light.m_Green             = settings.value("GreenLight",light.m_Green).toFloat();
        light.m_Blue              = settings.value("BlueLight", light.m_Blue).toFloat();

        light.m_iShininess        = settings.value("MatShininess", 5).toInt();

        light.m_Attenuation.m_Constant    = settings.value("ConstantAtt",  light.m_Attenuation.m_Constant).toFloat();
        light.m_Attenuation.m_Linear      = settings.value("LinearAtt",    light.m_Attenuation.m_Linear).toFloat();
        light.m_Attenuation.m_Quadratic   = settings.value("QuadraticAtt", light.m_Attenuation.m_Quadratic).toFloat();

        light.m_bIsLightOn        = settings.value("bLight", true).toBool();
    }
    settings.endGroup();
    return true;
}


void GLLightDlg::setDefaults()
{
    Light &light = gl3dView::s_Light;
    light.setDefaults(LIGHTREFLENGTH);
}


bool GLLightDlg::saveSettings(QSettings &settings)
{
    Light &light = gl3dView::s_Light;
    settings.beginGroup("GLLightDlg");
    {
        settings.setValue("WindowGeom",   s_Geometry);

        settings.setValue("Ambient",      light.m_Ambient);
        settings.setValue("Diffuse",      light.m_Diffuse);
        settings.setValue("Specular",     light.m_Specular);

        settings.setValue("XLight",       light.m_X);
        settings.setValue("YLight",       light.m_Y);
        settings.setValue("ZLight",       light.m_Z);
        settings.setValue("RedLight",     light.m_Red);
        settings.setValue("GreenLight",   light.m_Green);
        settings.setValue("BlueLight",    light.m_Blue);
        settings.setValue("bLight",       light.m_bIsLightOn);

        settings.setValue("MatShininess", light.m_iShininess);

        settings.setValue("ConstantAtt",  light.m_Attenuation.m_Constant);
        settings.setValue("LinearAtt",    light.m_Attenuation.m_Linear);
        settings.setValue("QuadraticAtt", light.m_Attenuation.m_Quadratic);

    }
    settings.endGroup();

    return true;
}


void GLLightDlg::showEvent(QShowEvent *)
{
    setParams();
    setEnabled();
    restoreGeometry(s_Geometry);
}


void GLLightDlg::hideEvent(QHideEvent *)
{
    s_Geometry = saveGeometry();
    if(m_pglView) m_pglView->setLightVisible(false);
}


void GLLightDlg::onLight()
{
    Light &light = gl3dView::s_Light;
    light.m_bIsLightOn = m_plabLight->isChecked();
    setEnabled();
    apply();
}


void GLLightDlg::setEnabled()
{
    Light &light = gl3dView::s_Light;
    m_pslRed->setEnabled(light.m_bIsLightOn);
    m_pslGreen->setEnabled(light.m_bIsLightOn);
    m_pslBlue->setEnabled(light.m_bIsLightOn);

    m_peslLightAmbient->setEnabled(light.m_bIsLightOn);
    m_peslLightDiffuse->setEnabled(light.m_bIsLightOn);
    m_peslLightSpecular->setEnabled(light.m_bIsLightOn);

    m_peslXLight->setEnabled(light.m_bIsLightOn);
    m_peslYLight->setEnabled(light.m_bIsLightOn);
    m_peslZLight->setEnabled(light.m_bIsLightOn);

    m_pslMatShininess->setEnabled(light.m_bIsLightOn);
}









