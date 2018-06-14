/****************************************************************************

	UnitsDlg Class
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

#include <globals/globals.h>
#include <misc/options/displayoptions.h>
#include "LengthUnitDlg.h"
#include <QGridLayout>
#include <QVBoxLayout>




LengthUnitDlg::LengthUnitDlg(QWidget *parent): QDialog(parent)
{
	m_Question = tr("Select units for this project :");
	setWindowTitle(tr("Units Dialog"));
	s_mtoUnit  = 1.0;
	m_LengthUnitIndex = 0;
	SetupLayout();
}


void LengthUnitDlg::SetupLayout()
{
	QGridLayout *UnitsLayout = new QGridLayout;
	{
		QLabel *lab1 = new QLabel(tr("Length"));

		UnitsLayout->addWidget(lab1, 1,1);


		m_pctrlQuestion = new QLabel(tr("Define the project units"));

		m_pctrlLengthFactor = new QLabel(" ");
		m_pctrlLengthFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);


		UnitsLayout->addWidget(m_pctrlLengthFactor, 1,2);

		m_pctrlLength  = new QComboBox;
		QFontMetrics fm(Settings::s_TextFont);
		m_pctrlLength->setMinimumWidth(fm.averageCharWidth() * 10);

		UnitsLayout->addWidget(m_pctrlLength,  1,3);

		m_pctrlLengthInvFactor = new QLabel(" ");

		m_pctrlLengthInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);

		UnitsLayout->addWidget(m_pctrlLengthInvFactor, 1,4);

		UnitsLayout->setColumnStretch(4,2);
		UnitsLayout->setColumnMinimumWidth(4,220);
	}

	QHBoxLayout *CommandButtons = new QHBoxLayout;
	{
		OKButton      = new QPushButton(tr("OK"));
		CancelButton  = new QPushButton(tr("Cancel"));
		CommandButtons->addStretch(1);
		CommandButtons->addWidget(OKButton);
		CommandButtons->addStretch(1);
		CommandButtons->addWidget(CancelButton);
		CommandButtons->addStretch(1);
	}

	QVBoxLayout *MainLayout = new QVBoxLayout;
	{
		MainLayout->addWidget(m_pctrlQuestion);
		MainLayout->addLayout(UnitsLayout);
		MainLayout->addStretch(1);
		MainLayout->addSpacing(20);
		MainLayout->addLayout(CommandButtons);
		MainLayout->addStretch(1);
	}

	setLayout(MainLayout);

	connect(OKButton, SIGNAL(clicked()),this, SLOT(accept()));
	connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	connect(m_pctrlLength, SIGNAL(activated(const QString &)),this, SLOT(OnSelChanged(const QString &)));
}


void LengthUnitDlg::InitDialog(int lengthUnitInd)
{
	QStringList list;
	list <<"mm" << "cm"<<"dm"<<"m"<<"in"<<"ft";
	m_pctrlLength->clear();
	m_pctrlLength->addItems(list);		//5


	m_LengthUnitIndex = lengthUnitInd;

	m_pctrlLength->setCurrentIndex(m_LengthUnitIndex);

	m_pctrlLength->setFocus();
	OnSelChanged(" ");


	m_pctrlQuestion->setText(m_Question);
}


void LengthUnitDlg::OnSelChanged(const QString &)
{
	m_LengthUnitIndex  = m_pctrlLength->currentIndex();


	SetUnits();

	QString str, strange;

	getLengthUnitLabel(str);
	strange= QString("     1 m = %1").arg(s_mtoUnit,15,'f',5);
	m_pctrlLengthFactor->setText(strange);
	strange= "1 "+str+" = " +QString("%1 m").arg(1./s_mtoUnit,15,'f',5);
	m_pctrlLengthInvFactor->setText(strange);


}


/**
 * Returns the name of the user-selected length unit, based on its index in the array.
 *@param str the reference of the QString to be filled with the name of the length unit
 *@param unit the index of the length unit
 */
void LengthUnitDlg::getLengthUnitLabel(QString &str)
{
	switch(m_LengthUnitIndex)
	{
		case 0:
		{
			str="mm";
			break;
		}
		case 1:
		{
			str="cm";
			break;
		}
		case 2:
		{
			str="dm";
			break;
		}
		case 3:
		{
			str="m";
			break;
		}
		case 4:
		{
			str="in";
			break;
		}
		case 5:
		{
			str="ft";
			break;
		}
		default:
		{
			str=" ";
			break;
		}
	}
}




/**
* Initializes the conversion factors for all user-defined units
*/
//void UnitsDlg::SetUnits(int s_LengthUnit, int s_AreaUnit, int s_SpeedUnit, int s_WeightUnit, int s_ForceUnit, int s_MomentMUnit,
//			         double &s_mtoUnit, double &s_m2toUnit, double &s_mstoUnit,  double &s_kgtoUnit, double &s_NtoUnit, double &s_NmtoUnit)
void LengthUnitDlg::SetUnits()
{
	switch(m_LengthUnitIndex)
	{
		case 0:
		{//mdm
			s_mtoUnit  = 1000.0;
			break;
		}
		case 1:{//cm
			s_mtoUnit  = 100.0;
			break;
		}
		case 2:{//dm
			s_mtoUnit  = 10.0;
			break;
		}
		case 3:{//m
			s_mtoUnit  = 1.0;
			break;
		}
		case 4:{//in
			s_mtoUnit  = 1000.0/25.4;
			break;
		}
		case 5:{///ft
			s_mtoUnit  = 1000.0/25.4/12.0;
			break;
		}
		default:{//m
			s_mtoUnit  = 1.0;
			break;
		}
	}

}



QString LengthUnitDlg::lengthUnitLabel()
{
	QString str;
	getLengthUnitLabel(str);
	return str;
}



