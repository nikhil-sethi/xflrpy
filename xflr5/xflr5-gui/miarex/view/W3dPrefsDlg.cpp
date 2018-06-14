/****************************************************************************

	W3dPrefsDlg Class
    Copyright (C) 2009-2016 Andre Deperrois XFLR5@yahoo.com

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
#include <QLabel>
#include <QCheckBox>
#include <QGridLayout>
#include <QColorDialog>
#include <QPushButton>
#include <globals/gui_params.h>
#include "W3dPrefsDlg.h"
#include <misc/line/LinePickerDlg.h>


bool W3dPrefsDlg::s_bAutoAdjustScale = true;
bool W3dPrefsDlg::s_bWakePanels = false;

double W3dPrefsDlg::s_MassRadius = .017;
QColor W3dPrefsDlg::s_MassColor = QColor(67, 151, 169);

int W3dPrefsDlg::s_VLMStyle       = 0;
int W3dPrefsDlg::s_VLMWidth       = 1;
QColor W3dPrefsDlg::s_VLMColor       = QColor(180,180,180);
int W3dPrefsDlg::s_3DAxisStyle    = 3;
int W3dPrefsDlg::s_3DAxisWidth    = 1;
QColor W3dPrefsDlg::s_3DAxisColor    = QColor(150,150,150);
int W3dPrefsDlg::s_OutlineStyle   = 0;
int W3dPrefsDlg::s_OutlineWidth   = 1;
QColor W3dPrefsDlg::s_OutlineColor   = QColor(50,50,50);
int W3dPrefsDlg::s_XCPStyle       = 0;
int W3dPrefsDlg::s_XCPWidth       = 1;
QColor W3dPrefsDlg::s_XCPColor       = QColor(50, 150, 50);
int W3dPrefsDlg::s_MomentStyle    = 0;
int W3dPrefsDlg::s_MomentWidth    = 1;
QColor W3dPrefsDlg::s_MomentColor    = QColor(200, 100, 100);
int W3dPrefsDlg::s_IDragStyle     = 0;
int W3dPrefsDlg::s_IDragWidth     = 1;
QColor W3dPrefsDlg::s_IDragColor     = QColor(255,200,0);

int W3dPrefsDlg::s_DownwashStyle  = 0;
int W3dPrefsDlg::s_DownwashWidth  = 1;
QColor W3dPrefsDlg::s_DownwashColor  = QColor(255, 100, 100);
int W3dPrefsDlg::s_WakeStyle      = 0;
int W3dPrefsDlg::s_WakeWidth      = 1;
QColor W3dPrefsDlg::s_WakeColor      = QColor(0, 150, 200);
int W3dPrefsDlg::s_CpStyle        = 0;
int W3dPrefsDlg::s_CpWidth        = 1;
QColor W3dPrefsDlg::s_CpColor        = QColor(255,0,0);
int W3dPrefsDlg::s_StreamLinesStyle  = 0;
int W3dPrefsDlg::s_StreamLinesWidth  = 1;
QColor W3dPrefsDlg::s_StreamLinesColor  = QColor(200, 150, 255);

int W3dPrefsDlg::s_VDragStyle     = 0;
int W3dPrefsDlg::s_VDragWidth     = 1;
QColor W3dPrefsDlg::s_VDragColor  = QColor(200,100,220);
int W3dPrefsDlg::s_TopStyle = 0;
int W3dPrefsDlg::s_TopWidth = 1;
QColor W3dPrefsDlg::s_TopColor = QColor(171, 103, 220);
int W3dPrefsDlg::s_BotStyle = 1;
int W3dPrefsDlg::s_BotWidth = 1;
QColor W3dPrefsDlg::s_BotColor = QColor(171, 103, 220);

int W3dPrefsDlg::s_iChordwiseRes=29;
int W3dPrefsDlg::s_iBodyAxialRes=23;
int W3dPrefsDlg::s_iBodyHoopRes = 17;
bool W3dPrefsDlg::s_bAnimateTransitions = true;
bool W3dPrefsDlg::s_bEnableClipPlane = false;


W3dPrefsDlg::W3dPrefsDlg(QWidget *pParent) : QDialog(pParent)
{
	setWindowTitle(tr("3D Styles"));
//	s_pSail7  = NULL;

	setupLayout();

	connect(m_pctrlAxis,               SIGNAL(clickedLB()),   SLOT(on3DAxis()));
	connect(m_pctrlOutline,            SIGNAL(clickedLB()),   SLOT(onOutline()));
	connect(m_pctrlVLMMesh,            SIGNAL(clickedLB()),   SLOT(onVLMMesh()));
	connect(m_pctrlTopTrans,           SIGNAL(clickedLB()),   SLOT(onTopTrans()));
	connect(m_pctrlBotTrans,           SIGNAL(clickedLB()),   SLOT(onBotTrans()));
	connect(m_pctrlLift,               SIGNAL(clickedLB()),   SLOT(onXCP()));
	connect(m_pctrlMoments,            SIGNAL(clickedLB()),   SLOT(onMoments()));
	connect(m_pctrlInducedDrag,        SIGNAL(clickedLB()),   SLOT(onIDrag()));
	connect(m_pctrlViscousDrag,        SIGNAL(clickedLB()),   SLOT(onVDrag()));
	connect(m_pctrlDownwash,           SIGNAL(clickedLB()),   SLOT(onDownwash()));
	connect(m_pctrlStreamLines,        SIGNAL(clickedLB()),   SLOT(onStreamLines()));
	connect(m_pctrlWakePanels,         SIGNAL(clickedLB()),   SLOT(onWakePanels()));
	connect(m_pctrlMassColor,          SIGNAL(clicked()),     SLOT(onMasses()));
}


void W3dPrefsDlg::initDialog()
{
	m_pctrlAxis->setStyle(s_3DAxisStyle,s_3DAxisWidth, s_3DAxisColor, 0);
	m_pctrlOutline->setStyle(s_OutlineStyle, s_OutlineWidth, s_OutlineColor, 0);
	m_pctrlVLMMesh->setStyle(s_VLMStyle, s_VLMWidth, s_VLMColor, 0);
	m_pctrlLift->setStyle(s_XCPStyle, s_XCPWidth, s_XCPColor, 0);
	m_pctrlMoments->setStyle(s_MomentStyle, s_MomentWidth, s_MomentColor, 0);
	m_pctrlInducedDrag->setStyle(s_IDragStyle, s_IDragWidth, s_IDragColor, 0);
	m_pctrlViscousDrag->setStyle(s_VDragStyle, s_VDragWidth, s_VDragColor, 0);
	m_pctrlDownwash->setStyle(s_DownwashStyle, s_DownwashWidth, s_DownwashColor, 0);
	m_pctrlWakePanels->setStyle(s_WakeStyle, s_WakeWidth, s_WakeColor, 0);
	m_pctrlStreamLines->setStyle(s_StreamLinesStyle, s_StreamLinesWidth, s_StreamLinesColor, 0);
	m_pctrlTopTrans->setStyle(s_TopStyle, s_TopWidth, s_TopColor, 0);
	m_pctrlBotTrans->setStyle(s_BotStyle, s_BotWidth, s_BotColor, 0);

	m_pctrlMassColor->setColor(s_MassColor);
	m_pctrlChordwiseRes->setValue(s_iChordwiseRes);
	m_pctrlBodyAxialRes->setValue(s_iBodyAxialRes);
	m_pctrlBodyHoopRes->setValue(s_iBodyHoopRes);

	m_pctrlAnimateTransitions->setChecked(s_bAnimateTransitions);
	m_pctrlAutoAdjustScale->setChecked(s_bAutoAdjustScale);
	m_pctrlEnableClipPlane->setChecked(s_bEnableClipPlane);
}



void W3dPrefsDlg::setupLayout()
{
	QLabel *lab1 = new QLabel(tr("Axis"));
	QLabel *lab2 = new QLabel(tr("Outline"));
	QLabel *lab3 = new QLabel(tr("VLM Mesh"));
	QLabel *lab4 = new QLabel(tr("Top transition"));
	QLabel *lab5 = new QLabel(tr("Bottom transition"));
	QLabel *lab6 = new QLabel(tr("Lift"));
	QLabel *lab7 = new QLabel(tr("Moments"));
	QLabel *lab8 = new QLabel(tr("Induced Drag"));
	QLabel *lab9 = new QLabel(tr("Viscous Drag"));
	QLabel *lab10 = new QLabel(tr("Downwash"));
	QLabel *lab11 = new QLabel(tr("WakePanels"));
	QLabel *lab12 = new QLabel(tr("Streamlines"));
	QLabel *lab13 = new QLabel(tr("Masses"));

	lab1->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
	lab2->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
	lab3->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
	lab4->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
	lab5->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
	lab6->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
	lab7->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
	lab8->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
	lab9->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
	lab10->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
	lab11->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
	lab12->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
	lab13->setAlignment(Qt::AlignVCenter|Qt::AlignRight);

	m_pctrlAxis     = new LineBtn(this);
	m_pctrlOutline  = new LineBtn(this);
	m_pctrlVLMMesh  = new LineBtn(this);
	m_pctrlTopTrans = new LineBtn(this);
	m_pctrlBotTrans = new LineBtn(this);
	m_pctrlLift     = new LineBtn(this);
	m_pctrlMoments  = new LineBtn(this);
	m_pctrlInducedDrag = new LineBtn(this);
	m_pctrlViscousDrag = new LineBtn(this);
	m_pctrlDownwash    = new LineBtn(this);
	m_pctrlWakePanels  = new LineBtn(this);
	m_pctrlStreamLines  = new LineBtn(this);
	m_pctrlMassColor = new ColorButton;

	QGroupBox *pColorPrefsBox = new QGroupBox(tr("Color settings"));
	{
		QGridLayout *pColorPrefsLayout = new QGridLayout;
		{
			pColorPrefsLayout->setColumnStretch(1,1);
			pColorPrefsLayout->setColumnStretch(2,2);
			pColorPrefsLayout->setColumnStretch(3,1);
			pColorPrefsLayout->setColumnStretch(4,2);
			pColorPrefsLayout->addWidget(lab1,1,1);
			pColorPrefsLayout->addWidget(lab2,2,1);
			pColorPrefsLayout->addWidget(lab3,3,1);
			pColorPrefsLayout->addWidget(lab4,4,1);
			pColorPrefsLayout->addWidget(lab5,5,1);
			pColorPrefsLayout->addWidget(lab6,6,1);
			pColorPrefsLayout->addWidget(lab7,1,3);
			pColorPrefsLayout->addWidget(lab8,2,3);
			pColorPrefsLayout->addWidget(lab9,3,3);
			pColorPrefsLayout->addWidget(lab10,4,3);
			pColorPrefsLayout->addWidget(lab11,5,3);
			pColorPrefsLayout->addWidget(lab12,6,3);
			pColorPrefsLayout->addWidget(lab13,7,3);

			pColorPrefsLayout->addWidget(m_pctrlAxis,1,2);
			pColorPrefsLayout->addWidget(m_pctrlOutline,2,2);
			pColorPrefsLayout->addWidget(m_pctrlVLMMesh,3,2);
			pColorPrefsLayout->addWidget(m_pctrlTopTrans,4,2);
			pColorPrefsLayout->addWidget(m_pctrlBotTrans,5,2);
			pColorPrefsLayout->addWidget(m_pctrlLift,6,2);
			pColorPrefsLayout->addWidget(m_pctrlMoments,1,4);
			pColorPrefsLayout->addWidget(m_pctrlInducedDrag,2,4);
			pColorPrefsLayout->addWidget(m_pctrlViscousDrag,3,4);
			pColorPrefsLayout->addWidget(m_pctrlDownwash,4,4);
			pColorPrefsLayout->addWidget(m_pctrlWakePanels,5,4);
			pColorPrefsLayout->addWidget(m_pctrlStreamLines,6,4);
			pColorPrefsLayout->addWidget(m_pctrlMassColor,7,4);
		}
		pColorPrefsBox->setLayout(pColorPrefsLayout);
	}

	QGroupBox *pTessBox = new QGroupBox(tr("Tessellation"));
	{
/*		QGridLayout *pTessLayout = new QGridLayout;
		{
			pTessLayout->addWidget(pTessBodyAxial,3,1);
			pTessLayout->addWidget(m_pctrlBodyAxialRes,3,2);
			pTessLayout->addWidget(pTessBodyHoop,4,1);
			pTessLayout->addWidget(m_pctrlBodyHoopRes,4,2);
		}*/

		QVBoxLayout *pTessLayout = new QVBoxLayout;
		{
			QLabel *pTessLabel = new QLabel(tr("Increase the number of points to improve the resolution\n"
											   "of the surfaces.This may reduce the display speed.\n"));
			QLabel *pTessChords = new QLabel(tr("Wing chordwise direction"));
			m_pctrlChordwiseRes = new IntEdit(37,this);
			QLabel *pTessBodyAxial = new QLabel(tr("Body axial direction"));
			m_pctrlBodyAxialRes = new IntEdit(29, this);
			QLabel *pTessBodyHoop = new QLabel(tr("Body hoop direction"));
			m_pctrlBodyHoopRes = new IntEdit(17,this);

			QHBoxLayout *pChordResLayout = new QHBoxLayout;
			{
				pChordResLayout->addStretch();
				pChordResLayout->addWidget(pTessChords);
				pChordResLayout->addWidget(m_pctrlChordwiseRes);
			}
			QHBoxLayout *pAxialResLayout = new QHBoxLayout;
			{
				pAxialResLayout->addStretch();
				pAxialResLayout->addWidget(pTessBodyAxial);
				pAxialResLayout->addWidget(m_pctrlBodyAxialRes);
			}
			QHBoxLayout *pHoopResLayout = new QHBoxLayout;
			{
				pHoopResLayout->addStretch();
				pHoopResLayout->addWidget(pTessBodyHoop);
				pHoopResLayout->addWidget(m_pctrlBodyHoopRes);
			}
			pTessLayout->addWidget(pTessLabel);
			pTessLayout->addLayout(pChordResLayout);
			pTessLayout->addLayout(pAxialResLayout);
			pTessLayout->addLayout(pHoopResLayout);
		}
		pTessBox->setLayout(pTessLayout);
	}

	m_pctrlAnimateTransitions = new QCheckBox(tr("Animate view transitions"));
	m_pctrlAutoAdjustScale = new QCheckBox(tr("Auto Adjust 3D scale"));
	m_pctrlAutoAdjustScale->setToolTip(tr("Automatically adjust the 3D scale to fit the plane in the display when switching between planes"));
	m_pctrlEnableClipPlane = new QCheckBox(tr("Enable clip plane"));

	QHBoxLayout *pCommandButtons = new QHBoxLayout;
    {
		QPushButton *pOKButton = new QPushButton(tr("Close"));
		pOKButton->setDefault(true);
		QPushButton *pResetButton = new QPushButton(tr("Reset Defaults"));
		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(pResetButton);
		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(pOKButton);
		pCommandButtons->addStretch(1);
		connect(pResetButton, SIGNAL(clicked()),this, SLOT(onResetDefaults()));
		connect(pOKButton, SIGNAL(clicked()),this, SLOT(onOK()));
    }

	QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
		pMainLayout->addStretch(1);
		pMainLayout->addWidget(pColorPrefsBox);
		pMainLayout->addStretch(1);
		pMainLayout->addWidget(pTessBox);
		pMainLayout->addStretch(1);
		pMainLayout->addWidget(m_pctrlAnimateTransitions);
		pMainLayout->addStretch(1);
		pMainLayout->addWidget(m_pctrlAutoAdjustScale);
		pMainLayout->addStretch(1);
		pMainLayout->addWidget(m_pctrlEnableClipPlane);
		pMainLayout->addSpacing(20);
		pMainLayout->addLayout(pCommandButtons);
		pMainLayout->addStretch(1);
    }
	setLayout(pMainLayout);
}


void W3dPrefsDlg::onOutline()
{
    LinePickerDlg LPdlg(this);
	LPdlg.setLineColor(s_OutlineColor);
	LPdlg.setLineStyle(s_OutlineStyle);
	LPdlg.setLineWidth(s_OutlineWidth);
	LPdlg.initDialog();
	if (QDialog::Accepted == LPdlg.exec())
	{
		s_OutlineColor = LPdlg.lineColor();
		s_OutlineStyle = LPdlg.lineStyle();
		s_OutlineWidth = LPdlg.lineWidth();
		m_pctrlOutline->setStyle(s_OutlineStyle, s_OutlineWidth, s_OutlineColor, 0);
	}
}

void W3dPrefsDlg::on3DAxis()
{
    LinePickerDlg LPdlg(this);
	LPdlg.setLineColor(s_3DAxisColor);
	LPdlg.setLineStyle(s_3DAxisStyle);
	LPdlg.setLineWidth(s_3DAxisWidth);
	LPdlg.initDialog();

	if (QDialog::Accepted == LPdlg.exec())
	{
		s_3DAxisColor = LPdlg.lineColor();
		s_3DAxisStyle = LPdlg.lineStyle();
		s_3DAxisWidth = LPdlg.lineWidth();
		m_pctrlAxis->setStyle(s_3DAxisStyle, s_3DAxisWidth, s_3DAxisColor, 0);
	}
}

void W3dPrefsDlg::onTopTrans()
{
    LinePickerDlg LPdlg(this);
	LPdlg.setLineColor(s_TopColor);
	LPdlg.setLineStyle(s_TopStyle);
	LPdlg.setLineWidth(s_TopWidth);
	LPdlg.initDialog();

	if (QDialog::Accepted == LPdlg.exec())
	{
		s_TopColor = LPdlg.lineColor();
		s_TopStyle = LPdlg.lineStyle();
		s_TopWidth = LPdlg.lineWidth();
		m_pctrlTopTrans->setStyle(s_TopStyle, s_TopWidth, s_TopColor, 0);
	}
}

void W3dPrefsDlg::onBotTrans()
{
    LinePickerDlg LPdlg(this);
	LPdlg.setLineColor(s_BotColor);
	LPdlg.setLineStyle(s_BotStyle);
	LPdlg.setLineWidth(s_BotWidth);
	LPdlg.initDialog();

	if (QDialog::Accepted == LPdlg.exec())
	{
		s_BotColor = LPdlg.lineColor();
		s_BotStyle = LPdlg.lineStyle();
		s_BotWidth = LPdlg.lineWidth();
		m_pctrlBotTrans->setStyle(s_BotStyle, s_BotWidth, s_BotColor, 0);
	}
}

void W3dPrefsDlg::onIDrag()
{
    LinePickerDlg LPdlg(this);
	LPdlg.setLineColor(s_IDragColor);
	LPdlg.setLineStyle(s_IDragStyle);
	LPdlg.setLineWidth(s_IDragWidth);
	LPdlg.initDialog();

	if (QDialog::Accepted == LPdlg.exec())
	{
		s_IDragColor = LPdlg.lineColor();
		s_IDragStyle = LPdlg.lineStyle();
		s_IDragWidth = LPdlg.lineWidth();
		m_pctrlInducedDrag->setStyle(s_IDragStyle, s_IDragWidth, s_IDragColor, 0);
	}
}

void W3dPrefsDlg::onVDrag()
{
    LinePickerDlg LPdlg(this);
	LPdlg.setLineColor(s_VDragColor);
	LPdlg.setLineStyle(s_VDragStyle);
	LPdlg.setLineWidth(s_VDragWidth);
	LPdlg.initDialog();

	if (QDialog::Accepted == LPdlg.exec())
	{
		s_VDragColor = LPdlg.lineColor();
		s_VDragStyle = LPdlg.lineStyle();
		s_VDragWidth = LPdlg.lineWidth();
		m_pctrlViscousDrag->setStyle(s_VDragStyle, s_VDragWidth, s_VDragColor, 0);
	}
}

void W3dPrefsDlg::onXCP()
{
    LinePickerDlg LPdlg(this);
	LPdlg.setLineColor(s_XCPColor);
	LPdlg.setLineStyle(s_XCPStyle);
	LPdlg.setLineWidth(s_XCPWidth);
	LPdlg.initDialog();

	if (QDialog::Accepted == LPdlg.exec())
	{
		s_XCPColor = LPdlg.lineColor();
		s_XCPStyle = LPdlg.lineStyle();
		s_XCPWidth = LPdlg.lineWidth();
		m_pctrlLift->setStyle(s_XCPStyle, s_XCPWidth, s_XCPColor, 0);
    }
}

void W3dPrefsDlg::onMoments()
{
    LinePickerDlg LPdlg(this);
	LPdlg.setLineColor(s_MomentColor);
	LPdlg.setLineStyle(s_MomentStyle);
	LPdlg.setLineWidth(s_MomentWidth);
	LPdlg.initDialog();

	if (QDialog::Accepted == LPdlg.exec())
	{
		s_MomentColor = LPdlg.lineColor();
		s_MomentStyle = LPdlg.lineStyle();
		s_MomentWidth = LPdlg.lineWidth();
		m_pctrlMoments->setStyle(s_MomentStyle, s_MomentWidth, s_MomentColor, 0);
    }
}

void W3dPrefsDlg::onDownwash()
{
    LinePickerDlg LPdlg(this);
	LPdlg.setLineColor(s_DownwashColor);
	LPdlg.setLineStyle(s_DownwashStyle);
	LPdlg.setLineWidth(s_DownwashWidth);
	LPdlg.initDialog();

	if (QDialog::Accepted == LPdlg.exec())
	{
		s_DownwashColor = LPdlg.lineColor();
		s_DownwashStyle = LPdlg.lineStyle();
		s_DownwashWidth = LPdlg.lineWidth();
		m_pctrlDownwash->setStyle(s_DownwashStyle, s_DownwashWidth,s_DownwashColor, 0);
	}
}

void W3dPrefsDlg::onStreamLines()
{
    LinePickerDlg LPdlg(this);
	LPdlg.setLineColor(s_StreamLinesColor);
	LPdlg.setLineStyle(s_StreamLinesStyle);
	LPdlg.setLineWidth(s_StreamLinesWidth);
	LPdlg.initDialog();

	if (QDialog::Accepted == LPdlg.exec())
	{
		s_StreamLinesColor = LPdlg.lineColor();
		s_StreamLinesStyle = LPdlg.lineStyle();
		s_StreamLinesWidth = LPdlg.lineWidth();
		m_pctrlStreamLines->setStyle(s_StreamLinesStyle, s_StreamLinesWidth, s_StreamLinesColor, 0);
	}
}

void W3dPrefsDlg::onWakePanels()
{
    LinePickerDlg LPdlg(this);
	LPdlg.setLineColor(s_WakeColor);
	LPdlg.setLineStyle(s_WakeStyle);
	LPdlg.setLineWidth(s_WakeWidth);
	LPdlg.initDialog();

	if (QDialog::Accepted == LPdlg.exec())
	{
		s_WakeColor = LPdlg.lineColor();
		s_WakeStyle = LPdlg.lineStyle();
		s_WakeWidth = LPdlg.lineWidth();
		m_pctrlWakePanels->setStyle(s_WakeStyle, s_WakeWidth, s_WakeColor, 0);
	}
}

void W3dPrefsDlg::onVLMMesh()
{
    LinePickerDlg LPdlg(this);
	LPdlg.setLineColor(s_VLMColor);
	LPdlg.setLineStyle(s_VLMStyle);
	LPdlg.setLineWidth(s_VLMWidth);
	LPdlg.initDialog();

	if (QDialog::Accepted == LPdlg.exec())
	{
		s_VLMColor = LPdlg.lineColor();
		s_VLMStyle = LPdlg.lineStyle();
		s_VLMWidth = LPdlg.lineWidth();
		m_pctrlVLMMesh->setStyle(s_VLMStyle, s_VLMWidth, s_VLMColor, 0);
	}
	repaint();
}


void W3dPrefsDlg::onMasses()
{
    QColorDialog::ColorDialogOptions dialogOptions = QColorDialog::ShowAlphaChannel;
#ifdef Q_OS_MAC
#if QT_VERSION >= 0x040700
    dialogOptions |= QColorDialog::DontUseNativeDialog;
#endif
#endif
    QColor Color = QColorDialog::getColor(s_MassColor,
                                   this, "Select the color", dialogOptions);
	if(Color.isValid()) s_MassColor = Color;
	m_pctrlMassColor->setColor(s_MassColor);

	update();
}


void W3dPrefsDlg::onShowWake()
{
//	s_bWakePanels = m_pctrlShowWake->isChecked();
}


void W3dPrefsDlg::saveSettings(QSettings *pSettings)
{
	pSettings->beginGroup("3DPrefs");
	{
		pSettings->setValue("3DAxisStyle", s_3DAxisStyle );
		pSettings->setValue("3DAXisWidth", s_3DAxisWidth );
		pSettings->setValue("3DAxisColor", s_3DAxisColor);

		pSettings->setValue("VLMStyle", s_VLMStyle);
		pSettings->setValue("VLMWidth", s_VLMWidth);
		pSettings->setValue("VLMColor", s_VLMColor);

		pSettings->setValue("OutlineStyle", s_OutlineStyle);
		pSettings->setValue("OutlineWidth", s_OutlineWidth);
		pSettings->setValue("OutlineColor", s_OutlineColor);

		pSettings->setValue("XCPStyle", s_XCPStyle);
		pSettings->setValue("XCPWidth", s_XCPWidth);
		pSettings->setValue("XCPColor", s_XCPColor);

		pSettings->setValue("MomentStyle", s_MomentStyle);
		pSettings->setValue("MomentWidth", s_MomentWidth);
		pSettings->setValue("MomentColor", s_MomentColor);

		pSettings->setValue("IDragStyle", s_IDragStyle);
		pSettings->setValue("IDragWidth", s_IDragWidth);
		pSettings->setValue("IDragColor", s_IDragColor);

		pSettings->setValue("VDragStyle", s_VDragStyle);
		pSettings->setValue("VDragWidth", s_VDragWidth);
		pSettings->setValue("VDragColor", s_VDragColor);

		pSettings->setValue("DownwashStyle", s_DownwashStyle );
		pSettings->setValue("DownwashWidth", s_DownwashWidth );
		pSettings->setValue("DownwashColor", s_DownwashColor);

		pSettings->setValue("WakeStyle", s_WakeStyle );
		pSettings->setValue("WakeWidth", s_WakeWidth );
		pSettings->setValue("WakeColor", s_WakeColor);

		pSettings->setValue("CpStyle", s_CpStyle);
		pSettings->setValue("CpWidth", s_CpWidth);
		pSettings->setValue("CpColor", s_CpColor);

		pSettings->setValue("TopStyle", s_TopStyle);
		pSettings->setValue("TopWidth", s_TopWidth);
		pSettings->setValue("TopColor", s_TopColor);

		pSettings->setValue("BotStyle", s_BotStyle);
		pSettings->setValue("BotWidth", s_BotWidth);
		pSettings->setValue("BotColor", s_BotColor);

		pSettings->setValue("StreamLinesStyle", s_StreamLinesStyle);
		pSettings->setValue("StreamLinesWidth", s_StreamLinesWidth);
		pSettings->setValue("StreamLinesColor", s_StreamLinesColor);

		pSettings->setValue("showWakePanels", s_bWakePanels);

		pSettings->setValue("MassColor", s_MassColor);

		pSettings->setValue("AutoAdjustScale", s_bAutoAdjustScale);
		pSettings->setValue("AnimateTransitions", s_bAnimateTransitions);
		pSettings->setValue("EnableClipPlane", s_bEnableClipPlane);

		pSettings->setValue("ChordwiseRes", s_iChordwiseRes);
		pSettings->setValue("BodyAxialRes", s_iBodyAxialRes);
		pSettings->setValue("BodyHoopRes", s_iBodyHoopRes);

	}
	pSettings->endGroup();
}


void W3dPrefsDlg::loadSettings(QSettings *pSettings)
{
	resetDefaults();
	pSettings->beginGroup("3DPrefs");
	{
		s_3DAxisStyle   = pSettings->value("3DAxisStyle", 3).toInt();
		s_3DAxisWidth   = pSettings->value("3DAXisWidth",1).toInt();
		s_3DAxisColor = pSettings->value("3DAxisColor").value<QColor>();

		s_VLMStyle = pSettings->value("VLMStyle", 0).toInt();
		s_VLMWidth = pSettings->value("VLMWidth",1).toInt();
		s_VLMColor = pSettings->value("VLMColor").value<QColor>();

		s_OutlineStyle = pSettings->value("OutlineStyle",0).toInt();
		s_OutlineWidth = pSettings->value("OutlineWidth",1).toInt();
		s_OutlineColor = pSettings->value("OutlineColor").value<QColor>();

		s_XCPStyle = pSettings->value("XCPStyle",0).toInt();
		s_XCPWidth = pSettings->value("XCPWidth",2).toInt();
		s_XCPColor = pSettings->value("XCPColor").value<QColor>();

		s_MomentStyle = pSettings->value("MomentStyle",0).toInt();
		s_MomentWidth = pSettings->value("MomentWidth",3).toInt();
		s_MomentColor = pSettings->value("MomentColor").value<QColor>();

		s_IDragStyle = pSettings->value("IDragStyle",0).toInt();
		s_IDragWidth = pSettings->value("IDragWidth",1).toInt();
		s_IDragColor = pSettings->value("IDragColor").value<QColor>();

		s_VDragStyle = pSettings->value("VDragStyle",0).toInt();
		s_VDragWidth = pSettings->value("VDragWidth",1).toInt();
		s_VDragColor = pSettings->value("VDragColor").value<QColor>();

		s_DownwashStyle = pSettings->value("DownwashStyle",0).toInt();
		s_DownwashWidth = pSettings->value("DownwashWidth",1).toInt();
		s_DownwashColor = pSettings->value("DownwashColor").value<QColor>();

		s_WakeStyle = pSettings->value("WakeStyle",2).toInt();
		s_WakeWidth = pSettings->value("WakeWidth",1).toInt();
		s_WakeColor = pSettings->value("WakeColor").value<QColor>();

		s_CpStyle = pSettings->value("CpStyle",0).toInt();
		s_CpWidth = pSettings->value("CpWidth",1).toInt();
		s_CpColor = pSettings->value("CpColor").value<QColor>();

		s_TopStyle = pSettings->value("TopStyle",0).toInt();
		s_TopWidth = pSettings->value("TopWidth",1).toInt();
		s_TopColor = pSettings->value("TopColor",QColor(171, 103, 220)).value<QColor>();

		s_BotStyle = pSettings->value("BotStyle",1).toInt();
		s_BotWidth = pSettings->value("BotWidth",1).toInt();
		s_BotColor = pSettings->value("BotColor", QColor(171, 103, 220)).value<QColor>();

		s_StreamLinesStyle = pSettings->value("StreamLinesStyle", 0).toInt();
		s_StreamLinesWidth = pSettings->value("StreamLinesWidth", 1).toInt();
		s_StreamLinesColor = pSettings->value("StreamLinesColor", QColor(150, 140, 255)).value<QColor>();

		s_MassColor = pSettings->value("MassColor", QColor(67, 151, 169)).value<QColor>();
		s_bWakePanels = pSettings->value("showWakePanels", true).toBool();
		s_bEnableClipPlane = pSettings->value("EnableClipPlane", false).toBool();

		s_bAutoAdjustScale = pSettings->value("AutoAdjustScale", true).toBool();
		s_bAnimateTransitions = pSettings->value("AnimateTransitions", true).toBool();
		s_iChordwiseRes = pSettings->value("ChordwiseRes", 29).toInt();
		s_iBodyAxialRes = pSettings->value("BodyAxialRes", 23).toInt();
		s_iBodyHoopRes = pSettings->value("BodyHoopRes", 17).toInt();
	}
	pSettings->endGroup();
}


void W3dPrefsDlg::onResetDefaults()
{
	resetDefaults();
	initDialog();
}


void W3dPrefsDlg::resetDefaults()
{
	s_bWakePanels = false;

	s_MassColor = QColor(67, 151, 169);

    s_VLMStyle       = 0;
    s_VLMWidth       = 1;
    s_VLMColor       = QColor(180,180,180);
    s_3DAxisStyle    = 3;
    s_3DAxisWidth    = 1;
    s_3DAxisColor    = QColor(150,150,150);
    s_OutlineStyle   = 0;
    s_OutlineWidth   = 1;
	s_OutlineColor   = QColor(50, 50, 50);
    s_XCPStyle       = 0;
    s_XCPWidth       = 1;
    s_XCPColor       = QColor(50, 150, 50);
    s_MomentStyle    = 0;
    s_MomentWidth    = 1;
    s_MomentColor    = QColor(200, 100, 100);

    s_IDragStyle     = 0;
    s_IDragWidth     = 1;
    s_IDragColor     = QColor(255,200,0);
	s_VDragStyle     = 0;
	s_VDragWidth     = 1;
	s_VDragColor     = QColor(200,100,220);

    s_DownwashStyle  = 0;
    s_DownwashWidth  = 1;
    s_DownwashColor  = QColor(255, 100, 100);
    s_WakeStyle      = 0;
    s_WakeWidth      = 1;
    s_WakeColor      = QColor(0, 150, 200);
    s_CpStyle        = 0;
    s_CpWidth        = 1;
    s_CpColor        = QColor(255,0,0);
    s_StreamLinesStyle  = 0;
    s_StreamLinesWidth  = 1;
    s_StreamLinesColor  = QColor(200, 150, 255);

    s_TopStyle = 0;
    s_TopWidth = 1;
	s_TopColor = QColor(171, 103, 220);
    s_BotStyle = 1;
    s_BotWidth = 1;
	s_BotColor = QColor(171, 103, 220);

	s_bAnimateTransitions = false;
	s_iChordwiseRes=29;
	s_iBodyAxialRes=23;
	s_iBodyHoopRes = 17;

}


void W3dPrefsDlg::onOK()
{
	readSettings();
	accept();
}

void W3dPrefsDlg::readSettings()
{
	s_bAnimateTransitions = m_pctrlAnimateTransitions->isChecked();
	s_bAutoAdjustScale = m_pctrlAutoAdjustScale->isChecked();
	s_bEnableClipPlane = m_pctrlEnableClipPlane->isChecked();

	s_iChordwiseRes = m_pctrlChordwiseRes->value();
	s_iBodyAxialRes = m_pctrlBodyAxialRes->value();
	s_iBodyHoopRes  = m_pctrlBodyHoopRes->value();

}





