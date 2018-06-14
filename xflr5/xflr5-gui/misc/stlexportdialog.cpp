/****************************************************************************

	STLExportDialog
	Copyright (C) 2016 Andre Deperrois 

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
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "stlexportdialog.h"


bool STLExportDialog::s_bBinary = true;
int STLExportDialog::s_iObject = 0;
int STLExportDialog::s_iChordPanels = 13;
int STLExportDialog::s_iSpanPanels = 17;

STLExportDialog::STLExportDialog()
{
	setupLayout();
	initDialog();
	setLabels();
	m_pctrlBinary->setEnabled(false);
	m_pctrlASCII->setEnabled(false);
}


void STLExportDialog::setupLayout()
{
	QVBoxLayout *pMainLayout = new QVBoxLayout;
	{
		QGroupBox *pExportFormat = new QGroupBox(tr("File format"));
		{
			QHBoxLayout *pFormatLayout = new QHBoxLayout;
			{
				m_pctrlBinary = new QRadioButton(tr("Binary"));
				m_pctrlASCII  = new QRadioButton(tr("ASCII"));
				pFormatLayout->addWidget(m_pctrlBinary);
				pFormatLayout->addWidget(m_pctrlASCII);
			}
			pExportFormat->setLayout(pFormatLayout);
		}


		QGroupBox *pObjectBox = new QGroupBox(tr("Object to export"));
		{
			QVBoxLayout *pObjectLayout = new QVBoxLayout;
			{
				m_prb[0] = new QRadioButton(tr("Body"));
				m_prb[1] = new QRadioButton(tr("Main Wing"));
				m_prb[2] = new QRadioButton(tr("Second Wing"));
				m_prb[3] = new QRadioButton(tr("Elevator"));
				m_prb[4] = new QRadioButton(tr("Fin"));
				for(int i=0; i<5; i++)
				{
					connect(m_prb[i], SIGNAL(clicked()), this, SLOT(onObjectSelection()));
					pObjectLayout->addWidget(m_prb[i]);
				}
			}
			pObjectBox->setLayout(pObjectLayout);
		}

		QGroupBox *pResolutionBox = new QGroupBox(tr("Output Resolution"));
		{
			QVBoxLayout *pResolutionLayout = new QVBoxLayout;
			{
				QHBoxLayout *pChordLayout = new QHBoxLayout;
				{
					m_pctrlChordLabel = new QLabel("Chordwise panels");
					m_pctrlChordLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
					m_pctrlChordPanels = new IntEdit(17);
					m_pctrlChordPanels->setAlignment(Qt::AlignRight);
					pChordLayout->addStretch();
					pChordLayout->addWidget(m_pctrlChordLabel);
					pChordLayout->addWidget(m_pctrlChordPanels);
				}

				QHBoxLayout *pSpanLayout = new QHBoxLayout;
				{
					m_pctrlSpanLabel = new QLabel("Spanwise panels");
					m_pctrlSpanLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
					m_pctrlSpanPanels = new IntEdit(17);
					m_pctrlSpanPanels->setAlignment(Qt::AlignRight);
					pSpanLayout->addStretch();
					pSpanLayout->addWidget(m_pctrlSpanLabel);
					pSpanLayout->addWidget(m_pctrlSpanPanels);
				}
				pResolutionLayout->addLayout(pChordLayout);
				pResolutionLayout->addLayout(pSpanLayout);
			}
			pResolutionBox->setLayout(pResolutionLayout);
		}

		QHBoxLayout *pButtonsLayout = new QHBoxLayout;
		{
			QPushButton *pOKButton = new QPushButton(tr("OK"));
			pOKButton->setDefault(true);
			pOKButton->setAutoDefault(true);
			QPushButton *pCancelButton = new QPushButton(tr("Cancel"));
			pCancelButton->setAutoDefault(false);

			pButtonsLayout->addStretch(1);
			pButtonsLayout->addWidget(pOKButton);
			pButtonsLayout->addStretch(1);
			pButtonsLayout->addWidget(pCancelButton);
			pButtonsLayout->addStretch(1);
			connect(pOKButton, SIGNAL(clicked()),this, SLOT(accept()));
			connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
		}

		pMainLayout->addWidget(pExportFormat);
		pMainLayout->addWidget(pObjectBox);
		pMainLayout->addWidget(pResolutionBox);
		pMainLayout->addStretch();
		pMainLayout->addLayout(pButtonsLayout);
	}
	setLayout(pMainLayout);
}


void STLExportDialog::accept()
{
	readParams();
	done(QDialog::Accepted);
}


void STLExportDialog::initDialog()
{
	m_pctrlBinary->setChecked(s_bBinary);
	m_pctrlASCII->setChecked(!s_bBinary);
	m_pctrlChordPanels->setValue(s_iChordPanels);
	m_pctrlSpanPanels->setValue(s_iSpanPanels);
	for(int i=0; i<5; i++)	 m_prb[i]->setChecked(s_iObject==i);
}


void STLExportDialog::readParams()
{
	s_bBinary = m_pctrlBinary->isChecked();
	s_iChordPanels = m_pctrlChordPanels->value();
	s_iSpanPanels  = m_pctrlSpanPanels->value();
	for(int i=0; i<5; i++)
	{
		if(m_prb[i]->isChecked())
		{
			s_iObject=i;
			break;
		}
	}
}


void STLExportDialog::onObjectSelection()
{
	for(int i=0; i<5; i++)
	{
		if(m_prb[i]->isChecked())
		{
			s_iObject=i;
			break;
		}
	}
	setLabels();
}


void STLExportDialog::setLabels()
{
	if(s_iObject==0)
	{
		m_pctrlChordLabel->setText(tr("Number of x-panels"));
		m_pctrlSpanLabel->setText(tr("Number of hoop panels"));
	}
	else
	{
		m_pctrlChordLabel->setText(tr("Number of chordwise panels"));
		m_pctrlSpanLabel->setText(tr("Number of span panels per surface"));
	}
}

