/****************************************************************************

	RenameDlg Class
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



#include "RenameDlg.h"
#include <QMessageBox>


RenameDlg::RenameDlg(QWidget *pParent) : QDialog(pParent)
{
	setWindowTitle(tr("Rename"));
	m_bEnableOverwrite = true;
	m_bExists = true;
	m_strArray.clear();
	setupLayout();
}


void RenameDlg::setupLayout()
{

	QHBoxLayout *CommandButtons = new QHBoxLayout;
	{
		OKButton = new QPushButton(tr("OK"));
		OKButton->setAutoDefault(false);
		CancelButton = new QPushButton(tr("Cancel"));
		CancelButton->setAutoDefault(false);
		m_pctrlOverwriteButton = new QPushButton(tr("Overwrite"));
		m_pctrlOverwriteButton->setAutoDefault(false);
		CommandButtons->addStretch(1);
		CommandButtons->addWidget(OKButton);
		CommandButtons->addStretch(1);
		CommandButtons->addWidget(CancelButton);
		CommandButtons->addStretch(1);
		CommandButtons->addWidget(m_pctrlOverwriteButton);
		CommandButtons->addStretch(1);
	}

	QVBoxLayout *MainLayout = new QVBoxLayout;
	{
		QLabel *LabelNote = new QLabel;
		LabelNote->setText(tr("Note : Overwrite will delete Opps and reset polars"));
		m_pctrlMessage = new QLabel("A Message here");

		m_pctrlName = new QLineEdit("");
		QLabel* NameListLabel = new QLabel(tr("Existing Names:"));
		m_pctrlNameList = new QListWidget;

		MainLayout->setStretchFactor(m_pctrlMessage, 1);
		MainLayout->setStretchFactor(m_pctrlName, 1);
		MainLayout->setStretchFactor(NameListLabel, 1);
		MainLayout->setStretchFactor(m_pctrlNameList, 5);
		MainLayout->setStretchFactor(CommandButtons, 1);
		MainLayout->setStretchFactor(LabelNote, 1);

		MainLayout->addWidget(m_pctrlMessage);
		MainLayout->addWidget(m_pctrlName);
		MainLayout->addStretch(1);
		MainLayout->addWidget(NameListLabel);
		MainLayout->addWidget(m_pctrlNameList);
		MainLayout->addStretch(1);
		MainLayout->addLayout(CommandButtons);
		MainLayout->addWidget(LabelNote);

		MainLayout->setStretchFactor(m_pctrlMessage, 1);
		MainLayout->setStretchFactor(m_pctrlName, 1);
		MainLayout->setStretchFactor(NameListLabel, 1);
		MainLayout->setStretchFactor(m_pctrlNameList, 5);
		MainLayout->setStretchFactor(CommandButtons, 1);
		MainLayout->setStretchFactor(LabelNote, 1);
	}

	setLayout(MainLayout);

	connect(m_pctrlNameList, SIGNAL(currentRowChanged(int)), this, SLOT(onSelChangeList(int)));
	connect(m_pctrlNameList, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(onDoubleClickList(QListWidgetItem *)));
	connect(OKButton, SIGNAL(clicked()),this, SLOT(onOK()));
	connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	connect(m_pctrlOverwriteButton, SIGNAL(clicked()), this, SLOT(onOverwrite()));
}


void RenameDlg::initDialog(QStringList *pStrList, QString startName, QString question)
{
	m_pctrlNameList->clear();

	m_strQuestion = question;

	if(!m_bEnableOverwrite) m_pctrlOverwriteButton->setEnabled(false);

	if(m_strQuestion.length())
	{
		m_pctrlMessage->setText(m_strQuestion);
	}
	else
	{
		m_pctrlMessage->setText(tr("Enter a name"));
	}

	m_strName = startName;
	m_pctrlName->setText(startName);
	m_pctrlName->setFocus();
	m_pctrlName->selectAll();

	m_strArray.clear();

	if(pStrList)
	{
		for (int i=0; i<pStrList->size(); i++)
		{
			m_strArray.append(pStrList->at(i));
			m_pctrlNameList->addItem(pStrList->at(i));
		}
	}
	else
	{
		m_pctrlNameList->setEnabled(false);
		m_pctrlOverwriteButton->setEnabled(false);
	}
}



void RenameDlg::keyPressEvent(QKeyEvent *event)
{
	// Prevent Return Key from closing App
	switch (event->key())
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			if(!OKButton->hasFocus() && !CancelButton->hasFocus())
			{
				OKButton->setFocus();
			}
			else
			{
				onOK();
			}
			return;
		}
		case Qt::Key_Escape:
		{
			reject();
			return;
		}
		default:
			event->ignore();
	}

}



void RenameDlg::onOverwrite()
{
	m_bExists = true;
	m_strName = m_pctrlName->text();
	done(10);
}


void RenameDlg::onOK()
{
	m_strName = m_pctrlName->text();
	if (!m_strName.length())
	{
		QMessageBox::warning(this, tr("Warning"), tr("Must enter a name"));
		m_pctrlName->setFocus();
		return;
	}

	QString strong;

	//exists ?
	m_bExists = false;
	for (int l=0; l<m_strArray.size(); l++)
	{
		strong = m_strArray.at(l);
		if(strong == m_strName)
		{
			QString str = tr("Do you wish to overwrite ")+m_strName + " ?";
			if (QMessageBox::Yes == QMessageBox::question(window(), tr("Question"), str,
														  QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel))
			{
				m_bExists = true;
				done(10);
				return;
			}
			else return;
		}
	}

	QDialog::accept();
}


void RenameDlg::onSelChangeList(int)
{
	QListWidgetItem *pItem =  m_pctrlNameList->currentItem();

	if(pItem)
	{
		QString str = pItem->text();
		m_pctrlName->setText(str);
		m_pctrlName->selectAll();
	}
}




void RenameDlg::onDoubleClickList(QListWidgetItem * pItem)
{
//	QListWidgetItem *pItem =  m_pctrlNameList->currentItem();

	if(pItem)
	{
		QString str = pItem->text();
		m_pctrlName->setText(str);
		onOK();
	}
}












