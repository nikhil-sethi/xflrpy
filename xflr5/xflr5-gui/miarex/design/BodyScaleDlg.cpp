/****************************************************************************

	BodyScaleDlg Class
	Copyright (C) 2009 Andre Deperrois adeperrois@xflr5.com

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

#include <globals.h>
#include <misc/LinePickerDlg.h>
#include "BodyScaleDlg.h"
#include "GL3dBodyDlg.h"


BodyScaleDlg::BodyScaleDlg(QWidget *pParent ): QDialog(pParent)
{
	setWindowTitle(tr("Body Scale Dialog"));
	m_pBodyDlg = pParent;
	m_XFactor = 1.0;
	m_YFactor = 1.0;
	m_ZFactor = 1.0;
	m_bFrameOnly = false;
	m_FrameID = 0;
	setupLayout();
}



void BodyScaleDlg::initDialog(bool bFrameOnly)
{
	m_pctrlXScaleFactor->setValue(m_XFactor);
	m_pctrlYScaleFactor->setValue(m_YFactor);
	m_pctrlZScaleFactor->setValue(m_ZFactor);

	m_pctrlXScaleFactor->setFocus();


	m_pctrlFrameID->setEnabled(false);
	m_pctrlFrameID->setValue(m_FrameID+1);

	m_bFrameOnly = bFrameOnly;
	if(!m_bFrameOnly)
	{
		m_pctrlBody->setChecked(true);
		m_pctrlFrame->setChecked(false);
	}
	else
	{
		m_pctrlBody->setChecked(false);
		m_pctrlFrame->setChecked(true);
		m_pctrlXScaleFactor->setEnabled(false);
	}

	enableControls();
}



void BodyScaleDlg::setupLayout()
{
	setWindowTitle("Scale Body");
//	QDesktopWidget desktop;
//	QRect r = desktop.geometry();
//	setMinimumHeight(r.height()/3);
//	move(r.width()/3, r.height()/6);

	QGridLayout *pTopLayout = new QGridLayout;
	{
		m_pctrlBody  = new QRadioButton(tr("Whole Body"));
		m_pctrlFrame = new QRadioButton(tr("Frame Only"));
		m_pctrlFrameID = new DoubleEdit(10);
		m_pctrlFrameID->setPrecision(0);
		pTopLayout->addWidget(m_pctrlBody,1,1);
		pTopLayout->addWidget(m_pctrlFrame,2,1);
		pTopLayout->addWidget(m_pctrlFrameID,2,2);
	}

	QGridLayout *pScaleLayout = new QGridLayout;
	{
		m_pctrlXScaleFactor = new DoubleEdit(1.0);
		m_pctrlYScaleFactor = new DoubleEdit(2.000);
		m_pctrlZScaleFactor = new DoubleEdit(3.);
		m_pctrlXScaleFactor->setPrecision(3);
		m_pctrlYScaleFactor->setPrecision(3);
		m_pctrlZScaleFactor->setPrecision(3);
		QLabel *lab0 = new QLabel(tr("Scale Factor"));
		QLabel *lab1 = new QLabel(tr("X Scale"));
		QLabel *lab2 = new QLabel(tr("Y Scale"));
		QLabel *lab3 = new QLabel(tr("Z Scale"));
		pScaleLayout->addWidget(lab0,1,2);
		pScaleLayout->addWidget(lab1,2,1);
		pScaleLayout->addWidget(lab2,3,1);
		pScaleLayout->addWidget(lab3,4,1);
		pScaleLayout->addWidget(m_pctrlXScaleFactor,2,2);
		pScaleLayout->addWidget(m_pctrlYScaleFactor,3,2);
		pScaleLayout->addWidget(m_pctrlZScaleFactor,4,2);
	}

	QHBoxLayout *pCommandButtons = new QHBoxLayout;
	{
		m_pOKButton      = new QPushButton(tr("OK"));
		m_pCancelButton  = new QPushButton(tr("Cancel"));
		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(m_pOKButton);
		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(m_pCancelButton);
		pCommandButtons->addStretch(1);
	}

	QVBoxLayout *pMainLayout = new QVBoxLayout;
	{
		pMainLayout->addStretch(1);
		pMainLayout->addLayout(pTopLayout);
		pMainLayout->addStretch(1);
		pMainLayout->addLayout(pScaleLayout);
		pMainLayout->addStretch(1);
		pMainLayout->addLayout(pCommandButtons);
		pMainLayout->addStretch(1);
	}
	setLayout(pMainLayout);

	connect(m_pOKButton, SIGNAL(clicked()),this, SLOT(onOK()));
	connect(m_pCancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	connect(m_pctrlBody, SIGNAL(clicked()), this, SLOT(onRadio()));
	connect(m_pctrlFrame, SIGNAL(clicked()), this, SLOT(onRadio()));

//	connect(m_pctrlXScaleFactor, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
//	connect(m_pctrlYScaleFactor, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
//	connect(m_pctrlZScaleFactor, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
//	connect(m_pctrlFrameID, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
}


void BodyScaleDlg::enableControls()
{

}



void BodyScaleDlg::keyPressEvent(QKeyEvent *event)
{
	// Prevent Return Key from closing App
	switch (event->key())
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			if(!m_pOKButton->hasFocus() && !m_pCancelButton->hasFocus())
			{
				m_pOKButton->setFocus();
				return;
			}
			else
			{
				onOK();
				return;
			}
			break;
		}
		case Qt::Key_Escape:
		{
			reject();
			break;
		}
		default:
			event->ignore();
	}
}



void BodyScaleDlg::onRadio()
{
	if(m_pctrlBody->isChecked())
	{
		m_pctrlFrameID->setEnabled(false);
		m_pctrlXScaleFactor->setEnabled(true);
		m_bFrameOnly = false;
	}
	else
	{
		m_pctrlFrameID->setEnabled(true);
		m_pctrlXScaleFactor->setEnabled(false);
		m_bFrameOnly = true;
	}
}


void BodyScaleDlg::onOK()
{
	m_FrameID = m_pctrlFrameID->value()-1;

	m_XFactor = m_pctrlXScaleFactor->value();
	m_YFactor = m_pctrlYScaleFactor->value();
	m_ZFactor = m_pctrlZScaleFactor->value();

	QDialog::accept();
}


void BodyScaleDlg::onEditingFinished()
{
	GL3dBodyDlg *pBodyDlg = (GL3dBodyDlg*)m_pBodyDlg;
	m_FrameID = m_pctrlFrameID->value()-1;
	pBodyDlg->m_pBody->m_iActiveFrame = m_FrameID;
	pBodyDlg->m_pFrame = pBodyDlg->m_pBody->activeFrame();

	pBodyDlg->updateView();
}





