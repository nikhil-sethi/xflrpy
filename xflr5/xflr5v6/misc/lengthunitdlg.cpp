/****************************************************************************

    UnitsDlg Class
    Copyright (C) André Deperrois

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

#include <QGridLayout>
#include <QVBoxLayout>

#include "lengthunitdlg.h"

#include <xflcore/displayoptions.h>
#include <xflcore/xflcore.h>


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


        m_plabQuestion = new QLabel(tr("Define the project units"));

        m_plabLengthFactor = new QLabel(" ");
        m_plabLengthFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);


        UnitsLayout->addWidget(m_plabLengthFactor, 1,2);

        m_pcbLength  = new QComboBox;
        QFontMetrics fm(DisplayOptions::textFont());
        m_pcbLength->setMinimumWidth(fm.averageCharWidth() * 10);

        UnitsLayout->addWidget(m_pcbLength,  1,3);

        m_plabLengthInvFactor = new QLabel(" ");

        m_plabLengthInvFactor->setAlignment(Qt::AlignRight | Qt::AlignCenter);

        UnitsLayout->addWidget(m_plabLengthInvFactor, 1,4);

        UnitsLayout->setColumnStretch(4,2);
        UnitsLayout->setColumnMinimumWidth(4,220);
    }

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_plabQuestion);
        pMainLayout->addLayout(UnitsLayout);
        pMainLayout->addStretch(1);
        pMainLayout->addSpacing(20);
        pMainLayout->addWidget(m_pButtonBox);
        pMainLayout->addStretch(1);
    }

    setLayout(pMainLayout);

    connect(m_pcbLength, SIGNAL(activated(const QString &)),this, SLOT(onSelChanged(const QString &)));
}


void LengthUnitDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok) == pButton)      accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)  reject();
}


void LengthUnitDlg::initDialog(int lengthUnitInd)
{
    QStringList list;
    list <<"mm" << "cm"<<"dm"<<"m"<<"in"<<"ft";
    m_pcbLength->clear();
    m_pcbLength->addItems(list);        //5


    m_LengthUnitIndex = lengthUnitInd;

    m_pcbLength->setCurrentIndex(m_LengthUnitIndex);

    m_pcbLength->setFocus();
    onSelChanged(" ");


    m_plabQuestion->setText(m_Question);
}


void LengthUnitDlg::onSelChanged(const QString &)
{
    m_LengthUnitIndex  = m_pcbLength->currentIndex();


    setUnits();

    QString str, strange;

    getLengthUnitLabel(str);
    strange= QString("     1 m = %1").arg(s_mtoUnit,15,'f',5);
    m_plabLengthFactor->setText(strange);
    strange= "1 "+str+" = " +QString("%1 m").arg(1./s_mtoUnit,15,'f',5);
    m_plabLengthInvFactor->setText(strange);


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
//                     double &s_mtoUnit, double &s_m2toUnit, double &s_mstoUnit,  double &s_kgtoUnit, double &s_NtoUnit, double &s_NmtoUnit)
void LengthUnitDlg::setUnits()
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



