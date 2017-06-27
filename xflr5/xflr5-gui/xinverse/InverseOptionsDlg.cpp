/****************************************************************************

	InverseOptionsDlg  Classes
	Copyright (C) 2009-2016 Andre Deperrois adeperrois@xflr5.com

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
#include "globals.h"
#include "XInverse.h"
#include "InverseOptionsDlg.h"
#include <misc/LinePickerDlg.h>

InverseOptionsDlg::InverseOptionsDlg(QWidget *pParent) : QDialog(pParent)
{
	setWindowTitle(tr("XInverse Style"));
	m_pXInverse = NULL;
	setupLayout();
}


void InverseOptionsDlg::setupLayout()
{
	QGridLayout *pStyleLayout = new QGridLayout;
	{
		QLabel * lab1 = new QLabel(tr("Reference Foil"));
		QLabel * lab2 = new QLabel(tr("Modified Foil"));
		QLabel * lab3 = new QLabel(tr("Spline"));
		QLabel * lab4 = new QLabel(tr("Reflected Curve"));
		lab1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		lab2->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		lab3->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		lab4->setAlignment(Qt::AlignRight | Qt::AlignVCenter);


		m_pctrlRefFoil   = new LineBtn(this);
		m_pctrlModFoil   = new LineBtn(this);
		m_pctrlSpline    = new LineBtn(this);
		m_pctrlReflected = new LineBtn(this);

		pStyleLayout->addWidget(lab1,1,1);
		pStyleLayout->addWidget(lab2,2,1);
		pStyleLayout->addWidget(lab3,3,1);
		pStyleLayout->addWidget(lab4,4,1);
		pStyleLayout->addWidget(m_pctrlRefFoil,1,2);
		pStyleLayout->addWidget(m_pctrlModFoil,2,2);
		pStyleLayout->addWidget(m_pctrlSpline,3,2);
		pStyleLayout->addWidget(m_pctrlReflected,4,2);
	}

	QHBoxLayout *pCommandButtons = new QHBoxLayout;
	{
		QPushButton *pOKButton      = new QPushButton(tr("OK"));
		QPushButton *pCancelButton  = new QPushButton(tr("Cancel"));
		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(pOKButton);
		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(pCancelButton);
		pCommandButtons->addStretch(1);

		connect(pOKButton, SIGNAL(clicked()),this, SLOT(accept()));
		connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	}

	QVBoxLayout *pMainLayout = new QVBoxLayout;
	{
		pMainLayout->addLayout(pStyleLayout);
		pMainLayout->addLayout(pCommandButtons);
	}

	setLayout(pMainLayout);

	connect(m_pctrlRefFoil,   SIGNAL(clickedLB()), this, SLOT(onRefStyle()));
	connect(m_pctrlModFoil,   SIGNAL(clickedLB()), this, SLOT(onModStyle()));
	connect(m_pctrlSpline,    SIGNAL(clickedLB()), this, SLOT(onSplineStyle()));
	connect(m_pctrlReflected, SIGNAL(clickedLB()), this, SLOT(onReflectedStyle()));
}


void InverseOptionsDlg::initDialog()
{
	QXInverse *pXInverse = (QXInverse*)m_pXInverse;
	m_pctrlRefFoil->setStyle(pXInverse->m_pRefFoil->foilLineStyle(), pXInverse->m_pRefFoil->foilLineWidth(), colour(pXInverse->m_pRefFoil),0);
	m_pctrlModFoil->setStyle(pXInverse->m_pModFoil->foilLineStyle(), pXInverse->m_pModFoil->foilLineWidth(), colour(pXInverse->m_pModFoil),0);
	m_pctrlSpline->setStyle(pXInverse->m_Spline.style(), pXInverse->m_Spline.width(), pXInverse->m_Spline.color(),0);
	m_pctrlReflected->setStyle(pXInverse->m_ReflectedStyle, pXInverse->m_ReflectedWidth, pXInverse->m_ReflectedClr,0);
}


void InverseOptionsDlg::onRefStyle()
{
	QXInverse *pXInverse = (QXInverse*)m_pXInverse;
    LinePickerDlg dlg(this);
	dlg.initDialog(pXInverse->m_pRefFoil->foilPointStyle(), pXInverse->m_pRefFoil->foilLineStyle(), pXInverse->m_pRefFoil->foilLineWidth(), colour(pXInverse->m_pRefFoil));

	if(QDialog::Accepted==dlg.exec())
	{
		m_pctrlRefFoil->setStyle(dlg.lineStyle(),dlg.lineWidth(),dlg.lineColor(),0);
		pXInverse->m_pRefFoil->foilLineStyle() = dlg.lineStyle();
		pXInverse->m_pRefFoil->foilLineWidth() = dlg.lineWidth();
		QColor clr = dlg.lineColor();
		pXInverse->m_pRefFoil->setColor(clr.red(), clr.green(), clr.blue(), clr.alpha());
	}
}


void InverseOptionsDlg::onModStyle()
{
	QXInverse *pXInverse = (QXInverse*)m_pXInverse;
    LinePickerDlg dlg(this);
	dlg.initDialog(pXInverse->m_pModFoil->foilPointStyle(),pXInverse->m_pModFoil->foilLineStyle(), pXInverse->m_pModFoil->foilLineWidth(), colour(pXInverse->m_pModFoil));

	if(QDialog::Accepted==dlg.exec())
	{
		m_pctrlModFoil->setStyle(dlg.lineStyle(),dlg.lineWidth(),dlg.lineColor(),0);
		pXInverse->m_pModFoil->foilLineStyle() = dlg.lineStyle();
		pXInverse->m_pModFoil->foilLineWidth() = dlg.lineWidth();
		QColor clr = dlg.lineColor();
		pXInverse->m_pModFoil->setColor(clr.red(), clr.green(), clr.blue(), clr.alpha());
	}
}


void InverseOptionsDlg::onSplineStyle()
{
	QXInverse *pXInverse = (QXInverse*)m_pXInverse;
    LinePickerDlg dlg(this);
	dlg.initDialog(0, pXInverse->m_Spline.style(), pXInverse->m_Spline.width(), pXInverse->m_Spline.color());

	if(QDialog::Accepted==dlg.exec())
	{
		m_pctrlSpline->setStyle(dlg.lineStyle(),dlg.lineWidth(),dlg.lineColor(),0);
		pXInverse->m_Spline.setStyle(dlg.lineStyle());
		pXInverse->m_Spline.setWidth(dlg.lineWidth());
		pXInverse->m_Spline.setColor(dlg.lineColor());
	}
}


void InverseOptionsDlg::onReflectedStyle()
{
	QXInverse *pXInverse = (QXInverse*)m_pXInverse;
    LinePickerDlg dlg(this);
	dlg.initDialog(0, pXInverse->m_ReflectedStyle, pXInverse->m_ReflectedWidth, pXInverse->m_ReflectedClr);

	if(QDialog::Accepted==dlg.exec())
	{
		m_pctrlReflected->setStyle(dlg.lineStyle(),dlg.lineWidth(),dlg.lineColor(),0);
		pXInverse->m_ReflectedStyle = dlg.lineStyle();
		pXInverse->m_ReflectedWidth = dlg.lineWidth();
		pXInverse->m_ReflectedClr   = dlg.lineColor();
	}
}


