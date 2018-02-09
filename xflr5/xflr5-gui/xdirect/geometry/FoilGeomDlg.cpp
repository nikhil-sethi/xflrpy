/****************************************************************************

	FoilGeomDlg Class
	Copyright (C) 2008-2016 Andre Deperrois adeperrois@xflr5.com

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

#include <XFoil.h>
#include "FoilGeomDlg.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>


void *FoilGeomDlg::s_pXFoil;


FoilGeomDlg::FoilGeomDlg(QWidget *pParent) : QDialog(pParent)
{
	setWindowTitle(tr("Foil Geometry"));

	m_pParent = pParent;

	setupLayout();

	connect(RestoreButton, SIGNAL(clicked()),this, SLOT(onRestore()));
	connect(OKButton, SIGNAL(clicked()),this, SLOT(onOK()));
	connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	connect(m_pctrlCamber, SIGNAL(editingFinished()), this, SLOT(onCamber()));
	connect(m_pctrlXCamber, SIGNAL(editingFinished()), this, SLOT(onXCamber()));
	connect(m_pctrlThickness, SIGNAL(editingFinished()), this, SLOT(onThickness()));
	connect(m_pctrlXThickness, SIGNAL(editingFinished()), this, SLOT(onXThickness()));

	connect(m_pctrlCamberSlide, SIGNAL(sliderMoved(int)), this, SLOT(onCamberSlide(int)));
	connect(m_pctrlXCamberSlide, SIGNAL(sliderMoved(int)), this, SLOT(onXCamberSlide(int)));
	connect(m_pctrlThickSlide, SIGNAL(sliderMoved(int)), this, SLOT(onThickSlide(int)));
	connect(m_pctrlXThickSlide, SIGNAL(sliderMoved(int)), this, SLOT(onXThickSlide(int)));
}


void FoilGeomDlg::setupLayout()
{
	QGroupBox *pCamberGroup = new QGroupBox(tr("Camber"));
	{
		QVBoxLayout *pCamberData = new QVBoxLayout;
		{
			m_pctrlCamberSlide = new QSlider;
			m_pctrlCamberSlide->setOrientation(Qt::Horizontal);
			m_pctrlCamberSlide->setTickPosition(QSlider::TicksBelow);
			m_pctrlCamberSlide->setMinimumWidth(200);
			m_pctrlCamber =new DoubleEdit;
			m_pctrlXCamberSlide = new QSlider;
			m_pctrlXCamberSlide->setOrientation(Qt::Horizontal);
			m_pctrlXCamberSlide->setTickPosition(QSlider::TicksBelow);
			m_pctrlXCamberSlide->setMinimumWidth(200);
			m_pctrlXCamber = new DoubleEdit;
			QLabel *lab1 = new QLabel(tr("Value"));
			QLabel *lab2 = new QLabel(tr("%Chord"));
			QLabel *lab3 = new QLabel(tr("0%"));
			QLabel *lab4 = new QLabel(tr("10%"));
			QLabel *lab5 = new QLabel(tr("Max x-pos"));
			QLabel *lab6 = new QLabel(tr("%Chord"));
			QLabel *lab7 = new QLabel(tr("0%"));
			QLabel *lab8 = new QLabel(tr("100%"));
			lab1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
			lab5->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
			lab3->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
			lab7->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
			lab1->setMinimumWidth(70);
			lab2->setMinimumWidth(70);
			lab3->setMinimumWidth(50);
			lab4->setMinimumWidth(50);
			lab5->setMinimumWidth(70);
			lab6->setMinimumWidth(70);
			lab7->setMinimumWidth(50);
			lab8->setMinimumWidth(50);

			QHBoxLayout *CambVal = new QHBoxLayout;
			{
				CambVal->addWidget(lab1);
				CambVal->addWidget(m_pctrlCamber);
				CambVal->addWidget(lab2);
				CambVal->addStretch(1);
				CambVal->addWidget(lab3);
				CambVal->addWidget(m_pctrlCamberSlide);
				CambVal->addWidget(lab4);
			}

			QHBoxLayout *XCambVal = new QHBoxLayout;
			{
				XCambVal->addWidget(lab5);
				XCambVal->addWidget(m_pctrlXCamber);
				XCambVal->addWidget(lab6);
				XCambVal->addStretch(1);
				XCambVal->addWidget(lab7);
				XCambVal->addWidget(m_pctrlXCamberSlide);
				XCambVal->addWidget(lab8);
			}
			pCamberData->addLayout(CambVal);
			pCamberData->addLayout(XCambVal);
		}
		pCamberGroup->setLayout(pCamberData);
	}


	QGroupBox *pThicknessGroup = new QGroupBox(tr("Thickness"));
	{
		QVBoxLayout *pThicknessData = new QVBoxLayout;
		{
			m_pctrlThickSlide = new QSlider;
			m_pctrlThickSlide->setOrientation(Qt::Horizontal);
			m_pctrlThickSlide->setTickPosition(QSlider::TicksBelow);
			m_pctrlThickSlide->setMinimumWidth(200);
			m_pctrlThickness =new DoubleEdit;
			m_pctrlXThickSlide = new QSlider;
			m_pctrlXThickSlide->setOrientation(Qt::Horizontal);
			m_pctrlXThickSlide->setTickPosition(QSlider::TicksBelow);
			m_pctrlXThickSlide->setMinimumWidth(200);
			m_pctrlXThickness = new DoubleEdit;
			QLabel *lab11 = new QLabel(tr("Value"));
			QLabel *lab12 = new QLabel(tr("%Chord"));
			QLabel *lab13 = new QLabel("0%");
			QLabel *lab14 = new QLabel("20%");
			QLabel *lab15 = new QLabel(tr("Max x-pos"));
			QLabel *lab16 = new QLabel(tr("%Chord"));
			QLabel *lab17 = new QLabel("0%");
			QLabel *lab18 = new QLabel("100%");

			lab11->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
			lab15->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
			lab13->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
			lab17->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
			lab11->setMinimumWidth(70);
			lab12->setMinimumWidth(70);
			lab13->setMinimumWidth(50);
			lab14->setMinimumWidth(50);
			lab15->setMinimumWidth(70);
			lab16->setMinimumWidth(70);
			lab17->setMinimumWidth(50);
			lab18->setMinimumWidth(50);

			QHBoxLayout *ThickVal = new QHBoxLayout;
			{
				ThickVal->addWidget(lab11);
				ThickVal->addWidget(m_pctrlThickness);
				ThickVal->addWidget(lab12);
				ThickVal->addStretch(1);
				ThickVal->addWidget(lab13);
				ThickVal->addWidget(m_pctrlThickSlide);
				ThickVal->addWidget(lab14);
			}

			QHBoxLayout *XThickVal = new QHBoxLayout;
			{
				XThickVal->addWidget(lab15);
				XThickVal->addWidget(m_pctrlXThickness);
				XThickVal->addWidget(lab16);
				XThickVal->addStretch(1);
				XThickVal->addWidget(lab17);
				XThickVal->addWidget(m_pctrlXThickSlide);
				XThickVal->addWidget(lab18);
			}

			pThicknessData->addLayout(ThickVal);
			pThicknessData->addLayout(XThickVal);
		}
		pThicknessGroup->setLayout(pThicknessData);
	}

	QHBoxLayout *pCommandButtons = new QHBoxLayout;
	{
		OKButton      = new QPushButton(tr("OK"));
		CancelButton  = new QPushButton(tr("Cancel"));
		RestoreButton  = new QPushButton(tr("Restore"));
		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(RestoreButton);
		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(OKButton);
		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(CancelButton);
		pCommandButtons->addStretch(1);
	}


	QVBoxLayout *pMainLayout = new QVBoxLayout;
	{
		pMainLayout->addWidget(pCamberGroup);
		pMainLayout->addStretch(1);
		pMainLayout->addWidget(pThicknessGroup);
		pMainLayout->addStretch(1);
		pMainLayout->addLayout(pCommandButtons);
		setLayout(pMainLayout);
	}

	setMinimumWidth(500);
	setMinimumHeight(300);

	m_pctrlCamber->setPrecision(2);
	m_pctrlXCamber->setPrecision(2);
	m_pctrlThickness->setPrecision(2);
	m_pctrlXThickness->setPrecision(2);

	m_pctrlCamberSlide->setRange(0,100);
	m_pctrlCamberSlide->setTickInterval(5);
	m_pctrlXCamberSlide->setRange(0,1000);
	m_pctrlXCamberSlide->setTickInterval(100);
	m_pctrlThickSlide->setRange(0,200);
	m_pctrlThickSlide->setTickInterval(5);
	m_pctrlXThickSlide->setRange(0,1000);
	m_pctrlXThickSlide->setTickInterval(100);
}



void FoilGeomDlg::apply()
{
	XFoil *pXFoil = (XFoil*)s_pXFoil;

	//reset everything and retry
	int i,j;

	m_pBufferFoil->copyFoil(m_pMemFoil);
	pXFoil->initXFoilGeometry(m_pBufferFoil->n, m_pBufferFoil->x, m_pBufferFoil->y, m_pBufferFoil->nx, m_pBufferFoil->ny);
/*	for (i=0; i< m_pMemFoil->nb; i++)
	{
		pXFoil->xb[i+1] = m_pMemFoil->xb[i];
		pXFoil->yb[i+1] = m_pMemFoil->yb[i];
	}
	pXFoil->nb = m_pMemFoil->nb;
	pXFoil->lflap = false;
	pXFoil->lbflap = false;

	if(pXFoil->Preprocess())
	{
		pXFoil->CheckAngles();
	}
	else
	{
		QMessageBox::information(window(), tr("Warning"), "Unrecognized foil format");
		return;
	}*/

//	if(!m_bApplied)
	{
		double thickness = m_pctrlThickness->value()/100.0;
		double camber    = m_pctrlCamber->value()/100.0;
		pXFoil->tcset(camber, thickness);
		m_pctrlCamberSlide->setSliderPosition((int)(camber*100*10));
		m_pctrlThickSlide->setSliderPosition((int)(thickness*100*10));
		m_bApplied = true;
	}

//	if(!m_bAppliedX)
	{
		double Xthickness = m_pctrlXThickness->value()/100.0;
		double Xcamber    = m_pctrlXCamber->value()/100.0;
		pXFoil->hipnt(Xcamber, Xthickness);
		m_pctrlXCamberSlide->setSliderPosition((int)(Xcamber*100*10));
		m_pctrlXThickSlide->setSliderPosition((int)(Xthickness*100*10));
		m_bAppliedX = true;
	}

	if(pXFoil->nb>IQX)
	{
		QMessageBox::information(window(), tr("Warning"), tr("Panel number cannot exceed 300"));
		//reset everything and retry
		for (i=0; i< m_pMemFoil->nb; i++)
		{
			pXFoil->x[i+1] = m_pMemFoil->xb[i];
			pXFoil->y[i+1] = m_pMemFoil->yb[i];
		}
		pXFoil->n = m_pMemFoil->nb;
	}
	else
	{
		for (j=0; j< pXFoil->nb; j++)
		{
			m_pBufferFoil->xb[j] = pXFoil->xb[j+1];
			m_pBufferFoil->yb[j] = pXFoil->yb[j+1];
		}
		m_pBufferFoil->nb = pXFoil->nb;
		m_pBufferFoil->initFoil();
		m_pBufferFoil->setFlap();
	}
	m_bModified = true;

	m_pParent->update();
}


void FoilGeomDlg::keyPressEvent(QKeyEvent *event)
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
			if(!OKButton->hasFocus() && !CancelButton->hasFocus())
			{
				apply();
				OKButton->setFocus();
				m_bApplied  = true;
				m_bAppliedX = true;
			}
			else
			{
				QDialog::accept();
			}
			break;
		}
		default:
			event->ignore();
			break;
	}
}


void FoilGeomDlg::initDialog()
{

	m_fCamber     = m_pMemFoil->camber();
	m_fThickness  = m_pMemFoil->thickness();
	m_fXCamber    = m_pMemFoil->xCamber();
	m_fXThickness = m_pMemFoil->xThickness();

	if(qAbs(m_fCamber) <0.0001)
	{
//		m_pctrlCamb->SetWindowText("The foil's camber is too small to be scaled"); //TODO
		m_pctrlCamberSlide->setEnabled(false);
		m_pctrlCamber->setEnabled(false);
	}

	m_pctrlCamber->setValue(m_fCamber*100.0);
	m_pctrlThickness->setValue(m_fThickness*100.0);
	m_pctrlXCamber->setValue(m_fXCamber*100.0);
	m_pctrlXThickness->setValue(m_fXThickness*100.0);


	m_pctrlCamberSlide->setSliderPosition((int)(m_fCamber*1000.0));
	m_pctrlThickSlide->setSliderPosition((int)(m_fThickness*1000.0));

	m_pctrlXCamberSlide->setSliderPosition((int)(m_fXCamber*1000.0));
	m_pctrlXThickSlide->setSliderPosition((int)(m_fXThickness*1000.0));

	m_bApplied  = true;
	m_bAppliedX = true;

}



void FoilGeomDlg::onRestore()
{
	XFoil *pXFoil = (XFoil*)s_pXFoil;

	m_pBufferFoil->copyFoil(m_pMemFoil);

	m_fThickness   = m_pMemFoil->thickness();
	m_fCamber      = m_pMemFoil->camber();
	m_fXThickness  = m_pMemFoil->xThickness();
	m_fXCamber     = m_pMemFoil->xCamber();

	pXFoil->thickb = m_fThickness;
	pXFoil->cambrb = m_fCamber;

	m_pctrlThickness->setValue(m_fThickness*100.0);
	m_pctrlCamber->setValue(m_fCamber*100.0);
	m_pctrlThickSlide->setSliderPosition((int)(m_fThickness*1000.0));
	m_pctrlCamberSlide->setSliderPosition((int)(m_fCamber*1000.0));

	m_pctrlXThickness->setValue(m_fXThickness*100.0);
	m_pctrlXCamber->setValue(m_fXCamber*100.0);
	m_pctrlXThickSlide->setSliderPosition((int)(m_fXThickness*1000.0));
	m_pctrlXCamberSlide->setSliderPosition	((int)(m_fXCamber*1000.0));

	m_bApplied  = true;
	m_bAppliedX = true;
	m_bModified = false;

	m_pParent->update();
}



void FoilGeomDlg::onCamber()
{
	m_bApplied = false;
	m_fCamber = m_pctrlCamber->value();
	m_pctrlCamberSlide->setValue(m_fCamber*10.0);
	apply();
}


void FoilGeomDlg::onCamberSlide(int pos)
{
	m_fCamber = (double)pos/10.0;
	m_pctrlCamber->setValue(m_fCamber);
	m_bApplied = false;
	apply();
}


void FoilGeomDlg::onOK()
{
	if(!m_bApplied || !m_bAppliedX)	apply();
	if(!m_bModified) done(0);
	else done(1);
}


void FoilGeomDlg::onThickness()
{
	m_bApplied = false;
	m_fThickness = m_pctrlThickness->value();
	m_pctrlThickSlide->setValue(m_fThickness*10.0);
	apply();
}


void FoilGeomDlg::onThickSlide(int pos)
{
	m_fThickness = (double)pos/10.0;
	m_pctrlThickness->setValue(m_fThickness);
	m_bApplied = false;
	apply();
}

void FoilGeomDlg::onXCamberSlide(int pos)
{
	m_fXCamber = (double)pos/10.0;
	m_pctrlXCamber->setValue(m_fXCamber);
	m_bAppliedX = false;
	apply();
}


void FoilGeomDlg::onXCamber()
{
	m_bAppliedX = false;
	m_fXCamber = m_pctrlXCamber->value();
	m_pctrlXCamberSlide->setValue(m_fXCamber*10.0);
	apply();
}


void FoilGeomDlg::onXThickSlide(int pos)
{
	m_fXThickness = (double)pos/10.0;
	m_pctrlXThickness->setValue(m_fXThickness);
	m_bAppliedX = false;
	apply();
}



void FoilGeomDlg::onXThickness()
{
	m_bAppliedX = false;
	m_fXThickness = m_pctrlXThickness->value();
	m_pctrlXThickSlide->setValue(m_fXThickness*10.0);
	apply();
}











