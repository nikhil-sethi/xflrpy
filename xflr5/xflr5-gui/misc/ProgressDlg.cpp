/****************************************************************************

	ProgressDlg Class
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

#include <QVBoxLayout>
#include "ProgressDlg.h"

ProgressDlg::ProgressDlg(QWidget *pParent) : QDialog(pParent)
{
	m_Min = 0;
	m_Max = 100;
	m_bCancel = false;
	SetupLayout();
}


void ProgressDlg::SetValue(int value)
{
//	value = qMax(value, m_Min);
//	value = qMin(value, m_Max);
	m_pctrlProgress->setValue(value);
}


void ProgressDlg::InitDialog(int min, int max)
{
	m_Min = min;
	m_Max = max;
	m_pctrlProgress->setMinimum(m_Min);
	m_pctrlProgress->setMaximum(m_Max);
	m_pctrlProgress->setValue(0);
}


void ProgressDlg::SetupLayout()
{
	setWindowTitle(tr("Progress"));
	m_pctrlProgress = new QProgressBar;
	m_pctrlProgress->setOrientation(Qt::Horizontal);
	m_pctrlProgress->setMinimum(0);
	m_pctrlProgress->setMaximum(100);
	m_pctrlProgress->setValue(0);

	QHBoxLayout *CancelLayout = new QHBoxLayout;
	CancelButton = new QPushButton(tr("Cancel"));
	CancelButton->setDefault(true);
	connect(CancelButton, SIGNAL(clicked()), this, SLOT(OnCancel()));
	CancelLayout->addStretch(1);
	CancelLayout->addWidget(CancelButton);
	CancelLayout->addStretch(1);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(m_pctrlProgress);
	mainLayout->addStretch(1);
	mainLayout->addLayout(CancelLayout);

	setLayout(mainLayout);
}


void ProgressDlg::OnCancel()
{
	m_bCancel = true;
	reject();
}

bool ProgressDlg::IsCanceled()
{
	return m_bCancel;
}
