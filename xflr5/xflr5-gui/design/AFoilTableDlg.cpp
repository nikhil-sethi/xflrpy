/****************************************************************************

    AFoilTableDlg Class
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

#include "AFoil.h"
#include "AFoilTableDlg.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <misc/line/LinePickerDlg.h>


AFoilTableDlg::AFoilTableDlg(QWidget *pParent): QDialog(pParent)
{
    setWindowTitle(tr("Foil Table Columns"));

    setupLayout();

    m_bFoilName = m_bPoints = true;
    m_bThickness = m_bThicknessAt = m_bCamber = m_bCamberAt = true;
    m_bTEFlapAngle = m_bTEXHinge = m_bTEYHinge = m_bLEFlapAngle = m_bLEXHinge = m_bLEYHinge = true;
}


void AFoilTableDlg::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Escape:
        {
            done(0);
            break;
        }
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pOKButton->hasFocus() && !m_pCancelButton->hasFocus())
            {
                m_pOKButton->setFocus();
                //				m_bApplied  = true;
            }
            else
            {
                QDialog::accept();
            }
            break;
        }
        default:
            event->ignore();
    }
}


void AFoilTableDlg::initDialog()
{
    m_pctrlFoilName->setChecked(m_bFoilName);
    m_pctrlThickness->setChecked(m_bThickness);
    m_pctrlThicknessAt->setChecked(m_bThicknessAt);
    m_pctrlCamber->setChecked(m_bCamber);
    m_pctrlCamberAt->setChecked(m_bCamberAt);
    m_pctrlPoints->setChecked(m_bPoints);
    m_pctrlTEFlapAngle->setChecked(m_bTEFlapAngle);
    m_pctrlTEXHinge->setChecked(m_bTEXHinge);
    m_pctrlTEYHinge->setChecked(m_bTEYHinge);
    m_pctrlLEFlapAngle->setChecked(m_bLEFlapAngle);
    m_pctrlLEXHinge->setChecked(m_bLEXHinge);
    m_pctrlLEYHinge->setChecked(m_bLEYHinge);
}


void AFoilTableDlg::setupLayout()
{
    QVBoxLayout *pColumnsLayout = new QVBoxLayout;
    {
        m_pctrlFoilName    = new QCheckBox(tr("Foil Name"));
        m_pctrlThickness   = new QCheckBox(tr("Thickness"));
        m_pctrlThicknessAt = new QCheckBox(tr("Thickness max. position"));
        m_pctrlCamber      = new QCheckBox(tr("Camber"));
        m_pctrlCamberAt    = new QCheckBox(tr("Camber max. position"));
        m_pctrlPoints      = new QCheckBox(tr("Number of points"));
        m_pctrlTEFlapAngle = new QCheckBox(tr("Trailing edge flap angle"));
        m_pctrlTEXHinge    = new QCheckBox(tr("Trailing edge hinge x-position"));
        m_pctrlTEYHinge    = new QCheckBox(tr("Trailing edge hinge y-position"));
        m_pctrlLEFlapAngle = new QCheckBox(tr("Leading edge flap angle"));
        m_pctrlLEXHinge    = new QCheckBox(tr("Leading edge hinge x-position"));
        m_pctrlLEYHinge    = new QCheckBox(tr("Leading edge hinge y-position"));

        pColumnsLayout->addWidget(m_pctrlFoilName);
        pColumnsLayout->addWidget(m_pctrlThickness);
        pColumnsLayout->addWidget(m_pctrlThicknessAt);
        pColumnsLayout->addWidget(m_pctrlCamber);
        pColumnsLayout->addWidget(m_pctrlCamberAt);
        pColumnsLayout->addWidget(m_pctrlPoints);
        pColumnsLayout->addWidget(m_pctrlTEFlapAngle);
        pColumnsLayout->addWidget(m_pctrlTEXHinge);
        pColumnsLayout->addWidget(m_pctrlTEYHinge);
        pColumnsLayout->addWidget(m_pctrlLEFlapAngle);
        pColumnsLayout->addWidget(m_pctrlLEXHinge);
        pColumnsLayout->addWidget(m_pctrlLEYHinge);
    }

    QHBoxLayout *pCommandButtons = new QHBoxLayout;
    {
        m_pOKButton      = new QPushButton(tr("OK"));
        m_pCancelButton  = new QPushButton(tr("Cancel"));
        pCommandButtons->addWidget(m_pOKButton);
        pCommandButtons->addWidget(m_pCancelButton);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addLayout(pColumnsLayout);
        pMainLayout->addStretch(1);
        pMainLayout->addLayout(pCommandButtons);
    }
    setLayout(pMainLayout);

    connect(m_pOKButton, SIGNAL(clicked()),this, SLOT(onOK()));
    connect(m_pCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}


void AFoilTableDlg::onOK()
{
    m_bFoilName    = m_pctrlFoilName->isChecked();
    m_bThickness   = m_pctrlThickness->isChecked();
    m_bThicknessAt = m_pctrlThicknessAt->isChecked();
    m_bCamber      = m_pctrlCamber->isChecked();
    m_bCamberAt    = m_pctrlCamberAt->isChecked();
    m_bPoints      = m_pctrlPoints->isChecked();
    m_bTEFlapAngle = m_pctrlTEFlapAngle->isChecked();
    m_bTEXHinge    = m_pctrlTEXHinge->isChecked();
    m_bTEYHinge    = m_pctrlTEYHinge->isChecked();
    m_bLEFlapAngle = m_pctrlLEFlapAngle->isChecked();
    m_bLEXHinge    = m_pctrlLEXHinge->isChecked();
    m_bLEYHinge    = m_pctrlLEYHinge->isChecked();

    accept();
}
