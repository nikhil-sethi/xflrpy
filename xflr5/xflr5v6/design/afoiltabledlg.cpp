/****************************************************************************

    AFoilTableDlg Class
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
#include <QLabel>
#include <QKeyEvent>


#include "afoil.h"
#include "afoiltabledlg.h"



AFoilTableDlg::AFoilTableDlg(QWidget *pParent): QDialog(pParent)
{
    setWindowTitle(tr("Foil Table Columns"));

    setupLayout();

    m_bFoilName = m_bPoints = true;
    m_bThickness = m_bThicknessAt = m_bCamber = m_bCamberAt = true;
    m_bTEFlapAngle = m_bTEXHinge = m_bTEYHinge = m_bLEFlapAngle = m_bLEXHinge = m_bLEYHinge = true;
}


void AFoilTableDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Escape:
        {
            done(0);
            break;
        }
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
                m_pButtonBox->setFocus();
                //                m_bApplied  = true;
            }
            break;
        }
        default:
            pEvent->ignore();
    }
}


void AFoilTableDlg::initDialog()
{
    m_pchFoilName->setChecked(m_bFoilName);
    m_pchThickness->setChecked(m_bThickness);
    m_pchThicknessAt->setChecked(m_bThicknessAt);
    m_pchCamber->setChecked(m_bCamber);
    m_pchCamberAt->setChecked(m_bCamberAt);
    m_pchPoints->setChecked(m_bPoints);
    m_pchTEFlapAngle->setChecked(m_bTEFlapAngle);
    m_pchTEXHinge->setChecked(m_bTEXHinge);
    m_pchTEYHinge->setChecked(m_bTEYHinge);
    m_pchLEFlapAngle->setChecked(m_bLEFlapAngle);
    m_pchLEXHinge->setChecked(m_bLEXHinge);
    m_pchLEYHinge->setChecked(m_bLEYHinge);
}


void AFoilTableDlg::setupLayout()
{
    QVBoxLayout *pColumnsLayout = new QVBoxLayout;
    {
        m_pchFoilName    = new QCheckBox(tr("Foil Name"));
        m_pchThickness   = new QCheckBox(tr("Thickness"));
        m_pchThicknessAt = new QCheckBox(tr("Thickness max. position"));
        m_pchCamber      = new QCheckBox(tr("Camber"));
        m_pchCamberAt    = new QCheckBox(tr("Camber max. position"));
        m_pchPoints      = new QCheckBox(tr("Number of points"));
        m_pchTEFlapAngle = new QCheckBox(tr("Trailing edge flap angle"));
        m_pchTEXHinge    = new QCheckBox(tr("Trailing edge hinge x-position"));
        m_pchTEYHinge    = new QCheckBox(tr("Trailing edge hinge y-position"));
        m_pchLEFlapAngle = new QCheckBox(tr("Leading edge flap angle"));
        m_pchLEXHinge    = new QCheckBox(tr("Leading edge hinge x-position"));
        m_pchLEYHinge    = new QCheckBox(tr("Leading edge hinge y-position"));

        pColumnsLayout->addWidget(m_pchFoilName);
        pColumnsLayout->addWidget(m_pchThickness);
        pColumnsLayout->addWidget(m_pchThicknessAt);
        pColumnsLayout->addWidget(m_pchCamber);
        pColumnsLayout->addWidget(m_pchCamberAt);
        pColumnsLayout->addWidget(m_pchPoints);
        pColumnsLayout->addWidget(m_pchTEFlapAngle);
        pColumnsLayout->addWidget(m_pchTEXHinge);
        pColumnsLayout->addWidget(m_pchTEYHinge);
        pColumnsLayout->addWidget(m_pchLEFlapAngle);
        pColumnsLayout->addWidget(m_pchLEXHinge);
        pColumnsLayout->addWidget(m_pchLEYHinge);
    }


    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(m_pButtonBox, &QDialogButtonBox::rejected, this, &AFoilTableDlg::onOK);

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addLayout(pColumnsLayout);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void AFoilTableDlg::onOK()
{
    m_bFoilName    = m_pchFoilName->isChecked();
    m_bThickness   = m_pchThickness->isChecked();
    m_bThicknessAt = m_pchThicknessAt->isChecked();
    m_bCamber      = m_pchCamber->isChecked();
    m_bCamberAt    = m_pchCamberAt->isChecked();
    m_bPoints      = m_pchPoints->isChecked();
    m_bTEFlapAngle = m_pchTEFlapAngle->isChecked();
    m_bTEXHinge    = m_pchTEXHinge->isChecked();
    m_bTEYHinge    = m_pchTEYHinge->isChecked();
    m_bLEFlapAngle = m_pchLEFlapAngle->isChecked();
    m_bLEXHinge    = m_pchLEXHinge->isChecked();
    m_bLEYHinge    = m_pchLEYHinge->isChecked();

    accept();
}
