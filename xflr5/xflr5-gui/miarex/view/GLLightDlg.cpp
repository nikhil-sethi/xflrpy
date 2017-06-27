/****************************************************************************

	GLLightDlg class
	Copyright (C) 2009 Andre Deperrois xflr5@yahoo.com

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

#include "GLLightDlg.h"
#include <Units.h>
#include <gl3dview.h>
#include <QGroupBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QShowEvent>


Light GLLightDlg::s_Light;
Attenuation GLLightDlg::s_Attenuation;
int GLLightDlg::s_iShininess = 3;

GLLightDlg::GLLightDlg(QWidget *pParent) : QDialog(pParent)
{
	m_ModelSize = 3.0; //meters

	setWindowTitle(tr("OpenGL Light Options"));
    setModal(false);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

	setupLayout();
	setDefaults();

	connect(m_pctrlLight,    SIGNAL(clicked()), this, SLOT(onLight()));
	connect(m_pctrlClose,    SIGNAL(clicked()), this, SLOT(accept()));
	connect(m_pctrlDefaults, SIGNAL(clicked()), this, SLOT(onDefaults()));

	connect(m_pctrlRed,           SIGNAL(sliderMoved(int)), this, SLOT(onChanged()));
	connect(m_pctrlGreen,         SIGNAL(sliderMoved(int)), this, SLOT(onChanged()));
	connect(m_pctrlBlue,          SIGNAL(sliderMoved(int)), this, SLOT(onChanged()));
	connect(m_pctrlLightAmbient,  SIGNAL(sliderMoved(int)), this, SLOT(onChanged()));
	connect(m_pctrlLightDiffuse,  SIGNAL(sliderMoved(int)), this, SLOT(onChanged()));
	connect(m_pctrlLightSpecular, SIGNAL(sliderMoved(int)), this, SLOT(onChanged()));
	connect(m_pctrlXLight,        SIGNAL(sliderMoved(int)), this, SLOT(onChanged()));
	connect(m_pctrlYLight,        SIGNAL(sliderMoved(int)), this, SLOT(onChanged()));
	connect(m_pctrlZLight,        SIGNAL(sliderMoved(int)), this, SLOT(onChanged()));

	connect(m_pctrlMatShininess,  SIGNAL(sliderMoved(int)), this, SLOT(onChanged()));

	connect(m_pctrlConstantAttenuation,  SIGNAL(editingFinished()), this, SLOT(onChanged()));
	connect(m_pctrlLinearAttenuation,    SIGNAL(editingFinished()), this, SLOT(onChanged()));
	connect(m_pctrlQuadAttenuation,      SIGNAL(editingFinished()), this, SLOT(onChanged()));

	m_pglView = NULL;
}



void GLLightDlg::setupLayout()
{
	QGroupBox *pLightIntensityBox = new QGroupBox(tr("Light Intensity"));
	{
		QVBoxLayout *pLightIntensities = new QVBoxLayout;
		QGridLayout *pLightIntensityLayout = new QGridLayout;
		{
			QLabel *lab1 = new QLabel(tr("Diffuse"));
			QLabel *lab2 = new QLabel(tr("Ambient"));
			QLabel *lab3 = new QLabel(tr("Specular"));

			m_pctrlLightAmbient      = new ExponentialSlider(false, 2.0, Qt::Horizontal);
			m_pctrlLightAmbient->setToolTip("Ambient:\n"
									   "Bounced light which has been scattered so much that it\n"
									   "is impossible to tell the direction to its source.\n"
									   "It is not attenuated by distance, and disappears if\n"
									   "the light is turned off.");
			m_pctrlLightDiffuse      = new ExponentialSlider(false, 2.0, Qt::Horizontal);
			m_pctrlLightDiffuse->setToolTip("Diffuse:\n"
									   "Directional light which is brighter on perpendicular\n"
									   "surfaces. Its reflection is scattered evenly.");
			m_pctrlLightSpecular     = new ExponentialSlider(false, 2.0, Qt::Horizontal);
			m_pctrlLightSpecular->setToolTip("Specular:\n"
										"Directional light which tends to reflect in a preferred\n"
										"direction. It is associated with shininess.");

			m_pctrlLightAmbient->setMinimum(0);
			m_pctrlLightAmbient->setMaximum(100);
			m_pctrlLightAmbient->setTickInterval(10);
			m_pctrlLightDiffuse->setMinimum(0);
			m_pctrlLightDiffuse->setMaximum(100);
			m_pctrlLightDiffuse->setTickInterval(10);
			m_pctrlLightSpecular->setMinimum(0);
			m_pctrlLightSpecular->setMaximum(100);
			m_pctrlLightSpecular->setTickInterval(10);
			m_pctrlLightDiffuse->setTickPosition(QSlider::TicksBelow);
			m_pctrlLightAmbient->setTickPosition(QSlider::TicksBelow);
			m_pctrlLightSpecular->setTickPosition(QSlider::TicksBelow);

			m_pctrlLightAmbientLabel = new QLabel;
			m_pctrlLightDiffuseLabel = new QLabel;
			m_pctrlLightSpecularLabel = new QLabel;
			pLightIntensityLayout->addWidget(lab2,1,1);
			pLightIntensityLayout->addWidget(lab1,2,1);
			pLightIntensityLayout->addWidget(lab3,3,1);
			pLightIntensityLayout->addWidget(m_pctrlLightAmbient,1,2);
			pLightIntensityLayout->addWidget(m_pctrlLightDiffuse,2,2);
			pLightIntensityLayout->addWidget(m_pctrlLightSpecular,3,2);
			pLightIntensityLayout->addWidget(m_pctrlLightAmbientLabel,1,3);
			pLightIntensityLayout->addWidget(m_pctrlLightDiffuseLabel,2,3);
			pLightIntensityLayout->addWidget(m_pctrlLightSpecularLabel,3,3);
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
			m_pctrlConstantAttenuation = new DoubleEdit(0.0);
			m_pctrlLinearAttenuation = new DoubleEdit(0.0);
			m_pctrlQuadAttenuation = new DoubleEdit(0.0);

			pAttenuationLayout->addWidget(pAtt);
			pAttenuationLayout->addWidget(m_pctrlConstantAttenuation);
			pAttenuationLayout->addWidget(pConstant);
			pAttenuationLayout->addWidget(m_pctrlLinearAttenuation);
			pAttenuationLayout->addWidget(pLinear);
			pAttenuationLayout->addWidget(m_pctrlQuadAttenuation);
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
			QLabel *lab11 = new QLabel(tr("Red"));
			QLabel *lab12 = new QLabel(tr("Green"));
			QLabel *lab13 = new QLabel(tr("Blue"));
			m_pctrlRed    = new QSlider(Qt::Horizontal);
			m_pctrlGreen  = new QSlider(Qt::Horizontal);
			m_pctrlBlue   = new QSlider(Qt::Horizontal);
			m_pctrlRed->setMinimum(0);
			m_pctrlRed->setMaximum(100);
			m_pctrlRed->setTickInterval(10);
			m_pctrlGreen->setMinimum(0);
			m_pctrlGreen->setMaximum(100);
			m_pctrlGreen->setTickInterval(10);
			m_pctrlBlue->setMinimum(0);
			m_pctrlBlue->setMaximum(100);
			m_pctrlBlue->setTickInterval(10);
			m_pctrlRed->setTickPosition(QSlider::TicksBelow);
			m_pctrlGreen->setTickPosition(QSlider::TicksBelow);
			m_pctrlBlue->setTickPosition(QSlider::TicksBelow);

			m_pctrlLightRed   = new QLabel;
			m_pctrlLightGreen = new QLabel;
			m_pctrlLightBlue  = new QLabel;

			pLightColor->addWidget(lab11,1,1);
			pLightColor->addWidget(lab12,2,1);
			pLightColor->addWidget(lab13,3,1);
			pLightColor->addWidget(m_pctrlRed,1,2);
			pLightColor->addWidget(m_pctrlGreen,2,2);
			pLightColor->addWidget(m_pctrlBlue,3,2);
			pLightColor->addWidget(m_pctrlLightRed,1,3);
			pLightColor->addWidget(m_pctrlLightGreen,2,3);
			pLightColor->addWidget(m_pctrlLightBlue,3,3);
			pLightColorBox->setLayout(pLightColor);
		}
	}

	QGroupBox *pLightPositionBox = new QGroupBox(tr("Light Position"));
	{
		QGridLayout *pLightPosition = new QGridLayout;
		{
			QLabel *lab21 = new QLabel(tr("x"));
			QLabel *lab22 = new QLabel(tr("y"));
			QLabel *lab23 = new QLabel(tr("z"));

			m_pctrlXLight = new ExponentialSlider(true, 2.0, Qt::Horizontal);
			m_pctrlYLight = new ExponentialSlider(true, 2.0, Qt::Horizontal);
			m_pctrlZLight = new ExponentialSlider(true, 2.0, Qt::Horizontal);
			m_pctrlXLight->setTickPosition(QSlider::TicksBelow);
			m_pctrlYLight->setTickPosition(QSlider::TicksBelow);
			m_pctrlZLight->setTickPosition(QSlider::TicksBelow);
			m_pctrlposXValue = new QLabel(Units::lengthUnitLabel());
			m_pctrlposYValue = new QLabel(Units::lengthUnitLabel());
			m_pctrlposZValue = new QLabel(Units::lengthUnitLabel());

			pLightPosition->addWidget(lab21,1,1);
			pLightPosition->addWidget(lab22,2,1);
			pLightPosition->addWidget(lab23,3,1);
			pLightPosition->addWidget(m_pctrlXLight,1,2);
			pLightPosition->addWidget(m_pctrlYLight,2,2);
			pLightPosition->addWidget(m_pctrlZLight,3,2);
			pLightPosition->addWidget(m_pctrlposXValue,1,3);
			pLightPosition->addWidget(m_pctrlposYValue,2,3);
			pLightPosition->addWidget(m_pctrlposZValue,3,3);
			pLightPositionBox->setLayout(pLightPosition);
		}
	}

	QHBoxLayout *pMaterialDataLayout = new QHBoxLayout;
	{
		m_pctrlMatShininess = new QSlider(Qt::Horizontal);
		m_pctrlMatShininess->setRange(4, 64);
		m_pctrlMatShininess->setTickInterval(2);
		m_pctrlMatShininess->setTickPosition(QSlider::TicksBelow);

		QLabel *lab35 = new QLabel(tr("Material Shininess"));
		m_pctrlMatShininessLabel = new QLabel("1");

		pMaterialDataLayout->addWidget(lab35);
		pMaterialDataLayout->addWidget(m_pctrlMatShininess);
		pMaterialDataLayout->addWidget(m_pctrlMatShininessLabel);
	}

	QHBoxLayout *pCommandButtons = new QHBoxLayout;
	{
		m_pctrlClose = new QPushButton(tr("Close"));
		m_pctrlDefaults = new QPushButton(tr("Reset Defaults"));
		m_pctrlDefaults->setDefault(false);
		m_pctrlDefaults->setAutoDefault(false);
		m_pctrlClose->setDefault(false);
		m_pctrlClose->setAutoDefault(false);
		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(m_pctrlDefaults);
		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(m_pctrlClose);
		pCommandButtons->addStretch(1);
	}

	QVBoxLayout *pMainLayout = new QVBoxLayout;
	{
		m_pctrlLight = new QCheckBox(tr("Light"));
		pMainLayout->addWidget(m_pctrlLight);
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
		gl3dView *pgl3dView =(gl3dView*)m_pglView;
		pgl3dView->glSetupLight();
		pgl3dView->update();
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
		gl3dView *pglView =(gl3dView*)m_pglView;
		pglView->glSetupLight();
		pglView->update();
	}
}


void GLLightDlg::readParams(void)
{
	s_Light.m_bIsLightOn = m_pctrlLight->isChecked();

	s_Light.m_Red     = (float)m_pctrlRed->value()    /100.0f;
	s_Light.m_Green   = (float)m_pctrlGreen->value()  /100.0f;
	s_Light.m_Blue    = (float)m_pctrlBlue->value()   /100.0f;

	s_Light.m_X  = m_pctrlXLight->expValue()/100.0;
	s_Light.m_Y  = m_pctrlYLight->expValue()/100.0;
	s_Light.m_Z  = m_pctrlZLight->expValue()/100.0;

	s_Light.m_Ambient     = m_pctrlLightAmbient->expValue()  / 20.0f;
	s_Light.m_Diffuse     = m_pctrlLightDiffuse->expValue()  / 20.0f;
	s_Light.m_Specular    = m_pctrlLightSpecular->expValue() / 20.0f;

	s_iShininess   = m_pctrlMatShininess->value();

	s_Attenuation.m_Constant  = m_pctrlConstantAttenuation->value();
	s_Attenuation.m_Linear    = m_pctrlLinearAttenuation->value();
	s_Attenuation.m_Quadratic = m_pctrlQuadAttenuation->value();
}


void GLLightDlg::setParams(void)
{
	m_pctrlLight->setChecked(s_Light.m_bIsLightOn);

	m_pctrlLightAmbient->setExpValue( s_Light.m_Ambient  *20.0);
	m_pctrlLightDiffuse->setExpValue( s_Light.m_Diffuse  *20.0);
	m_pctrlLightSpecular->setExpValue(s_Light.m_Specular *20.0);

	m_pctrlXLight->setExpValue(s_Light.m_X*100.0);
	m_pctrlYLight->setExpValue(s_Light.m_Y*100.0);
	m_pctrlZLight->setExpValue(s_Light.m_Z*100.0);

	m_pctrlRed->setValue(  (int)(s_Light.m_Red  *100.0));
	m_pctrlGreen->setValue((int)(s_Light.m_Green*100.0));
	m_pctrlBlue->setValue( (int)(s_Light.m_Blue *100.0));

	m_pctrlMatShininess->setValue(s_iShininess);

	m_pctrlConstantAttenuation->setValue(s_Attenuation.m_Constant);
	m_pctrlLinearAttenuation->setValue(s_Attenuation.m_Linear);
	m_pctrlQuadAttenuation->setValue(s_Attenuation.m_Quadratic);

	setLabels();
}


void GLLightDlg::setModelSize(double span)
{
	m_ModelSize = span; //meters
}


void GLLightDlg::setLabels()
{
	QString strong;

	strong.sprintf("%7.1f", s_Light.m_Ambient);
	m_pctrlLightAmbientLabel->setText(strong);
	strong.sprintf("%7.1f", s_Light.m_Diffuse);
	m_pctrlLightDiffuseLabel->setText(strong);
	strong.sprintf("%7.1f", s_Light.m_Specular);
	m_pctrlLightSpecularLabel->setText(strong);	strong.sprintf("%7.1f", s_Light.m_X*Units::mtoUnit());
	m_pctrlposXValue->setText(strong + Units::lengthUnitLabel());
	strong.sprintf("%7.1f", s_Light.m_Y*Units::mtoUnit());
	m_pctrlposYValue->setText(strong + Units::lengthUnitLabel());
	strong.sprintf("%7.1f", s_Light.m_Z*Units::mtoUnit());
	m_pctrlposZValue->setText(strong + Units::lengthUnitLabel());	strong.sprintf("%7.1f", s_Light.m_Red);
	m_pctrlLightRed->setText(strong);
	strong.sprintf("%7.1f", s_Light.m_Green);
	m_pctrlLightGreen->setText(strong);
	strong.sprintf("%7.1f", s_Light.m_Blue);
	m_pctrlLightBlue->setText(strong);

	strong.sprintf("%d", s_iShininess);
	m_pctrlMatShininessLabel->setText(strong);

}



bool GLLightDlg::loadSettings(QSettings *pSettings)
{
	pSettings->beginGroup("GLLight3");
	{
	//  we're reading/loading
		s_Light.m_Ambient           = pSettings->value("Ambient",0.3).toDouble();
		s_Light.m_Diffuse           = pSettings->value("Diffuse",1.2).toDouble();
		s_Light.m_Specular          = pSettings->value("Specular",0.50).toDouble();

		s_Light.m_X                 = pSettings->value("XLight", 0.300).toDouble();
		s_Light.m_Y                 = pSettings->value("YLight", 0.300).toDouble();
		s_Light.m_Z                 = pSettings->value("ZLight", 3.000).toDouble();

		s_Light.m_Red               = pSettings->value("RedLight",1.0).toDouble();
		s_Light.m_Green             = pSettings->value("GreenLight",1.0).toDouble();
		s_Light.m_Blue              = pSettings->value("BlueLight",1.0).toDouble();

		s_iShininess     = pSettings->value("MatShininess", 5).toInt();

		s_Attenuation.m_Constant    = pSettings->value("ConstantAtt",2.0).toDouble();
		s_Attenuation.m_Linear      = pSettings->value("LinearAtt",1.0).toDouble();
		s_Attenuation.m_Quadratic   = pSettings->value("QuadraticAtt",.5).toDouble();

		s_Light.m_bIsLightOn        = pSettings->value("bLight", true).toBool();
	}
	pSettings->endGroup();
	return true;
}


void GLLightDlg::setDefaults()
{
	s_Light.m_Red   = 1.0f;
	s_Light.m_Green = 1.0f;
	s_Light.m_Blue  = 1.0f;

	s_Light.m_Ambient      = 0.3f;
	s_Light.m_Diffuse      = 1.20f;
	s_Light.m_Specular     = 0.50f;

	s_Light.m_X   =  0.1f * m_ModelSize;
	s_Light.m_Y   =  0.3f * m_ModelSize;
	s_Light.m_Z   =  0.5f * m_ModelSize;
	m_pctrlXLight->setRange(-(int)m_ModelSize*100, (int)m_ModelSize*100);
	m_pctrlYLight->setRange(-(int)m_ModelSize*100, (int)m_ModelSize*100);
	m_pctrlZLight->setRange(-(int)m_ModelSize*100, (int)m_ModelSize*100);
	m_pctrlXLight->setTickInterval((int)((double)m_ModelSize*10.0));
	m_pctrlYLight->setTickInterval((int)((double)m_ModelSize*10.0));
	m_pctrlZLight->setTickInterval((int)((double)m_ModelSize*10.0));

	s_iShininess = 5;

	s_Attenuation.m_Constant  = 1.0;
	s_Attenuation.m_Linear    = 0.5;
	s_Attenuation.m_Quadratic = 0.0;

	s_Light.m_bIsLightOn = true;
}



bool GLLightDlg::saveSettings(QSettings *pSettings)
{
	pSettings->beginGroup("GLLight3");
	{
		pSettings->setValue("Ambient",      s_Light.m_Ambient);
		pSettings->setValue("Diffuse",      s_Light.m_Diffuse);
		pSettings->setValue("Specular",     s_Light.m_Specular);

		pSettings->setValue("XLight",       s_Light.m_X);
		pSettings->setValue("YLight",       s_Light.m_Y);
		pSettings->setValue("ZLight",       s_Light.m_Z);
		pSettings->setValue("RedLight",     s_Light.m_Red);
		pSettings->setValue("GreenLight",   s_Light.m_Green);
		pSettings->setValue("BlueLight",    s_Light.m_Blue);
		pSettings->setValue("bLight",       s_Light.m_bIsLightOn);

		pSettings->setValue("MatShininess", s_iShininess);

		pSettings->setValue("ConstantAtt",  s_Attenuation.m_Constant);
		pSettings->setValue("LinearAtt",    s_Attenuation.m_Linear);
		pSettings->setValue("QuadraticAtt", s_Attenuation.m_Quadratic);

	}
	pSettings->endGroup();

	return true;
}


void GLLightDlg::showEvent(QShowEvent *event)
{
	setParams();
	setEnabled();

	event->accept();
}



QSize GLLightDlg::minimumSizeHint() const
{
	return QSize(300, 350);
}


QSize GLLightDlg::sizeHint() const
{
	return QSize(400, 400);
}


void GLLightDlg::onLight()
{
	s_Light.m_bIsLightOn = m_pctrlLight->isChecked();
	setEnabled();
	apply();
}


void GLLightDlg::setEnabled()
{
	m_pctrlRed->setEnabled(s_Light.m_bIsLightOn);
	m_pctrlGreen->setEnabled(s_Light.m_bIsLightOn);
	m_pctrlBlue->setEnabled(s_Light.m_bIsLightOn);

	m_pctrlLightAmbient->setEnabled(s_Light.m_bIsLightOn);
	m_pctrlLightDiffuse->setEnabled(s_Light.m_bIsLightOn);
	m_pctrlLightSpecular->setEnabled(s_Light.m_bIsLightOn);

	m_pctrlXLight->setEnabled(s_Light.m_bIsLightOn);
	m_pctrlYLight->setEnabled(s_Light.m_bIsLightOn);
	m_pctrlZLight->setEnabled(s_Light.m_bIsLightOn);

	m_pctrlMatShininess->setEnabled(s_Light.m_bIsLightOn);
}









