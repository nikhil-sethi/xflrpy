/****************************************************************************

	BodyGridDlg Class
	Copyright (C) 2009 Andre Deperrois 

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
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QLabel>
#include <QPushButton>

#include "BodyGridDlg.h"
#include <globals/globals.h>
#include <misc/line/LineBtn.h>
#include <misc/line/LinePickerDlg.h>
#include <misc/options/units.h>
#include <misc/text/DoubleEdit.h>


bool BodyGridDlg::s_bScale = false;

bool BodyGridDlg::s_bGrid = false;
int BodyGridDlg::s_Style = 1;
int BodyGridDlg::s_Width =1;
QColor BodyGridDlg::s_Color = QColor(150,150,150);
double BodyGridDlg::s_Unit = 0.2;

bool BodyGridDlg::s_bMinGrid = false;
int BodyGridDlg::s_MinStyle = 2;
int BodyGridDlg::s_MinWidth = 1;
QColor BodyGridDlg::s_MinColor=QColor(75,75,75);
double BodyGridDlg::s_MinorUnit=0.05;


bool BodyGridDlg::s_bGrid2 = false;
int BodyGridDlg::s_Style2 = 1;
int BodyGridDlg::s_Width2 = 1;
QColor BodyGridDlg::s_Color2 = QColor(150,150,150);
double BodyGridDlg::s_Unit2 = 0.01;

bool BodyGridDlg::s_bMinGrid2 = false;
int BodyGridDlg::s_MinStyle2 = 2;
int BodyGridDlg::s_MinWidth2 = 1;
QColor BodyGridDlg::s_MinColor2=QColor(75,75,75);
double BodyGridDlg::s_MinorUnit2=0.002;


BodyGridDlg::BodyGridDlg(QWidget *pParent):QDialog(pParent)
{
	setWindowTitle(tr("Body Grid Dialog"));

	setupLayout();
}



void BodyGridDlg::initDialog()
{
	QString length;
	Units::getLengthUnitLabel(length);
	m_pctrlLength1->setText(length);
	m_pctrlLength2->setText(length);
	m_pctrlLength3->setText(length);
	m_pctrlLength4->setText(length);

	m_pctrlScales->setChecked(s_bScale);
	m_pctrlGrid->setChecked(s_bGrid);
	m_pctrlLine->setStyle(s_Style);
	m_pctrlLine->setWidth(s_Width);
	m_pctrlLine->setColor(s_Color);
	m_pctrlMinGrid->setChecked(s_bMinGrid);
	m_pctrlMinLine->setStyle(s_MinStyle);
	m_pctrlMinLine->setWidth(s_MinWidth);
	m_pctrlMinLine->setColor(s_MinColor);
	m_pctrlUnit->setValue(s_Unit*Units::mtoUnit());
	m_pctrlMinUnit->setValue(s_MinorUnit*Units::mtoUnit());

	m_pctrlGrid2->setChecked(s_bGrid2);
	m_pctrlLine2->setStyle(s_Style2);
	m_pctrlLine2->setWidth(s_Width2);
	m_pctrlLine2->setColor(s_Color2);
	m_pctrlMinGrid2->setChecked(s_bMinGrid2);
	m_pctrlMinLine2->setStyle(s_MinStyle2);
	m_pctrlMinLine2->setWidth(s_MinWidth2);
	m_pctrlMinLine2->setColor(s_MinColor2);
	m_pctrlUnit2->setValue(s_Unit2*Units::mtoUnit());
	m_pctrlMinUnit2->setValue(s_MinorUnit2*Units::mtoUnit());

	enableControls();
}



void BodyGridDlg::setupLayout()
{
	setWindowTitle(tr("Grid Parameters"));

	m_pctrlScales   = new QCheckBox(tr("Show Scales"));
	m_pctrlGrid     = new QCheckBox(tr("Main Grid"));
	m_pctrlGrid2    = new QCheckBox(tr("Main Grid"));
	m_pctrlMinGrid  = new QCheckBox(tr("Minor Grid"));
	m_pctrlMinGrid2 = new QCheckBox(tr("Minor Grid"));

	m_pctrlLine  = new LineBtn(this);
	m_pctrlLine2 = new LineBtn(this);
	m_pctrlMinLine  = new LineBtn(this);
	m_pctrlMinLine2 = new LineBtn(this);

	m_pctrlUnit  = new DoubleEdit(100.00);
	m_pctrlUnit2 = new DoubleEdit(101.00);
	m_pctrlMinUnit  = new DoubleEdit(102.00);
	m_pctrlMinUnit2 = new DoubleEdit(103.00);
	m_pctrlUnit->setPrecision(3);
	m_pctrlUnit2->setPrecision(3);
	m_pctrlMinUnit->setPrecision(3);
	m_pctrlMinUnit2->setPrecision(3);

	m_pctrlLength1 = new QLabel("mm");
	m_pctrlLength2 = new QLabel("mm");
	m_pctrlLength3 = new QLabel("mm");
	m_pctrlLength4 = new QLabel("mm");

	QGroupBox *pBodyBox = new QGroupBox(tr("Body Grid"));
	{
		QGridLayout *pBodyLayout = new QGridLayout;
		{
			pBodyLayout->addWidget(m_pctrlGrid,1,1);
			pBodyLayout->addWidget(m_pctrlLine,1,2);
			pBodyLayout->addWidget(m_pctrlUnit,1,3);
			pBodyLayout->addWidget(m_pctrlLength1, 1,4);
			pBodyLayout->addWidget(m_pctrlMinGrid,2,1);
			pBodyLayout->addWidget(m_pctrlMinLine,2,2);
			pBodyLayout->addWidget(m_pctrlMinUnit,2,3);
			pBodyLayout->addWidget(m_pctrlLength2, 2,4);
		}
		pBodyBox->setLayout(pBodyLayout);
	}


	QGroupBox *pFrameBox = new QGroupBox(tr("Frame Grid"));
	{
		QGridLayout *pFrameLayout = new QGridLayout;
		{
			pFrameLayout->addWidget(m_pctrlGrid2,1,1);
			pFrameLayout->addWidget(m_pctrlLine2,1,2);
			pFrameLayout->addWidget(m_pctrlUnit2,1,3);
			pFrameLayout->addWidget(m_pctrlLength3, 1,4);
			pFrameLayout->addWidget(m_pctrlMinGrid2,2,1);
			pFrameLayout->addWidget(m_pctrlMinLine2,2,2);
			pFrameLayout->addWidget(m_pctrlMinUnit2,2,3);
			pFrameLayout->addWidget(m_pctrlLength4, 2,4);
		}
		pFrameBox->setLayout(pFrameLayout);
	}

	QHBoxLayout *pCommandButtons = new QHBoxLayout;
	{
		QPushButton *OKButton = new QPushButton(tr("OK"));
		QPushButton *Cancel = new QPushButton(tr("Cancel"));
		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(OKButton);
		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(Cancel);
		pCommandButtons->addStretch(1);
		connect(OKButton, SIGNAL(clicked()),this, SLOT(onOK()));
		connect(Cancel, SIGNAL(clicked()), this, SLOT(reject()));
	}

	QVBoxLayout *pMainLayout = new QVBoxLayout;
	{
		pMainLayout->addWidget(m_pctrlScales);
		pMainLayout->addStretch(1);
		pMainLayout->addWidget(pBodyBox);
		pMainLayout->addStretch(1);
		pMainLayout->addWidget(pFrameBox);
		pMainLayout->addStretch(1);
		pMainLayout->addLayout(pCommandButtons);
		pMainLayout->addStretch(1);
	}

	setLayout(pMainLayout);


	connect(m_pctrlGrid, SIGNAL(clicked()), this, SLOT(onGrid()));
	connect(m_pctrlMinGrid, SIGNAL(clicked()), this, SLOT(onMinGrid()));
	connect(m_pctrlGrid2, SIGNAL(clicked()), this, SLOT(onGrid2()));
	connect(m_pctrlMinGrid2, SIGNAL(clicked()), this, SLOT(onMinGrid2()));

	connect(m_pctrlLine, SIGNAL(clickedLB()), this, SLOT(onLineStyle()));
	connect(m_pctrlLine2, SIGNAL(clickedLB()), this, SLOT(onLine2Style()));
	connect(m_pctrlMinLine, SIGNAL(clickedLB()), this, SLOT(onMinLineStyle()));
	connect(m_pctrlMinLine2, SIGNAL(clickedLB()), this, SLOT(onMinLine2Style()));
}


void BodyGridDlg::enableControls()
{
	m_pctrlLine->setEnabled(s_bGrid);
	m_pctrlUnit->setEnabled(s_bGrid);
	m_pctrlMinLine->setEnabled(s_bMinGrid);
	m_pctrlMinUnit->setEnabled(s_bMinGrid);

	m_pctrlLine2->setEnabled(s_bGrid2);
	m_pctrlUnit2->setEnabled(s_bGrid2);
	m_pctrlMinLine2->setEnabled(s_bMinGrid2);
	m_pctrlMinUnit2->setEnabled(s_bMinGrid2);
}



void BodyGridDlg::onOK()
{
	s_bScale = m_pctrlScales->isChecked();
	accept();
}



void BodyGridDlg::onGrid()
{
	s_bGrid = m_pctrlGrid->isChecked();
	enableControls();
}


void BodyGridDlg::onGrid2()
{
	s_bGrid2 = m_pctrlGrid2->isChecked();
	enableControls();
}

void BodyGridDlg::onMinGrid()
{
	s_bMinGrid = m_pctrlMinGrid->isChecked();
	enableControls();
}


void BodyGridDlg::onMinGrid2()
{
	s_bMinGrid2 = m_pctrlMinGrid2->isChecked();
	enableControls();
}


void BodyGridDlg::onLineStyle()
{
	LinePickerDlg dlg(this);
	dlg.initDialog(0, s_Style, s_Width, s_Color);

	if(QDialog::Accepted==dlg.exec())
	{
		s_Style = dlg.lineStyle();
		s_Width = dlg.lineWidth();
		s_Color = dlg.lineColor();
		m_pctrlLine->setStyle(s_Style);
		m_pctrlLine->setWidth(s_Width);
		m_pctrlLine->setColor(s_Color);
	}
}


void BodyGridDlg::onLine2Style()
{
	LinePickerDlg dlg(this);
	dlg.initDialog(0, s_Style2, s_Width2, s_Color2);

	if(QDialog::Accepted==dlg.exec())
	{
		s_Style2 = dlg.lineStyle();
		s_Width2 = dlg.lineWidth();
		s_Color2 = dlg.lineColor();
		m_pctrlLine2->setStyle(s_Style2);
		m_pctrlLine2->setWidth(s_Width2);
		m_pctrlLine2->setColor(s_Color2);
	}
}


void BodyGridDlg::onMinLineStyle()
{
	LinePickerDlg dlg(this);
	dlg.initDialog(0, s_MinStyle, s_MinWidth, s_MinColor);

	if(QDialog::Accepted==dlg.exec())
	{
		s_MinStyle = dlg.lineStyle();
		s_MinWidth = dlg.lineWidth();
		s_MinColor = dlg.lineColor();
		m_pctrlMinLine->setStyle(s_MinStyle);
		m_pctrlMinLine->setWidth(s_MinWidth);
		m_pctrlMinLine->setColor(s_MinColor);
	}
}


void BodyGridDlg::onMinLine2Style()
{
	LinePickerDlg dlg(this);
	dlg.initDialog(0, s_MinStyle2, s_MinWidth2, s_MinColor2);

	if(QDialog::Accepted==dlg.exec())
	{
		s_MinStyle2 = dlg.lineStyle();
		s_MinWidth2 = dlg.lineWidth();
		s_MinColor2 = dlg.lineColor();
		m_pctrlMinLine2->setStyle(s_MinStyle2);
		m_pctrlMinLine2->setWidth(s_MinWidth2);
		m_pctrlMinLine2->setColor(s_MinColor2);
	}
}



void BodyGridDlg::loadSettings (QSettings &settings)
{
    settings.beginGroup("GL3dBodyGrid");
	{
        s_bGrid      = settings.value("Grid").toBool();
        s_bMinGrid   = settings.value("MinGrid").toBool();
        s_bGrid2     = settings.value("Grid2").toBool();
        s_bMinGrid2  = settings.value("MinGrid2").toBool();
        s_Style      = settings.value("Style").toInt();
        s_MinStyle   = settings.value("MinStyle").toInt();
        s_Style2     = settings.value("Style2").toInt();
        s_MinStyle2  = settings.value("MinStyle2").toInt();
        s_Width      = settings.value("Width").toInt();
        s_MinWidth   = settings.value("MinWidth").toInt();
        s_Width2     = settings.value("Width2").toInt();
        s_MinWidth2  = settings.value("MinWidth2").toInt();
        s_Color      = settings.value("Color").value<QColor>();
        s_MinColor   = settings.value("MinColor").value<QColor>();
        s_Color2     = settings.value("Color2").value<QColor>();
        s_MinColor2  = settings.value("MinColor2").value<QColor>();
        s_Unit       = settings.value("Unit").toDouble();
        s_MinorUnit  = settings.value("MinorUnit").toDouble();
        s_Unit2      = settings.value("Unit2").toDouble();
        s_MinorUnit2 = settings.value("MinorUnit2").toDouble();
        s_bScale     = settings.value("bScale").toBool();
	}
    settings.endGroup();
}


void BodyGridDlg::saveSettings (QSettings &settings)
{
    settings.beginGroup("GL3dBodyGrid");
	{
        settings.setValue("Grid", s_bGrid);
        settings.setValue("MinGrid", s_bMinGrid);
        settings.setValue("Grid2", s_bGrid2);
        settings.setValue("MinGrid2", s_bMinGrid2);
        settings.setValue("Style", s_Style);
        settings.setValue("MinStyle", s_MinStyle);
        settings.setValue("Style2", s_Style2);
        settings.setValue("MinStyle2", s_MinStyle2);
        settings.setValue("Width", s_Width);
        settings.setValue("MinWidth", s_MinWidth);
        settings.setValue("Width2", s_Width2);
        settings.setValue("MinWidth2", s_MinWidth2);
        settings.setValue("Color", s_Color);
        settings.setValue("MinColor", s_MinColor);
        settings.setValue("Color2", s_Color2);
        settings.setValue("MinColor2", s_MinColor2);
        settings.setValue("Unit", s_Unit);
        settings.setValue("MinorUnit", s_MinorUnit);
        settings.setValue("Unit2", s_Unit2);
        settings.setValue("MinorUnit2", s_MinorUnit2);
        settings.setValue("bScale", s_bScale);
	}
    settings.endGroup();
}






