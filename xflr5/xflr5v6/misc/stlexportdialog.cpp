/****************************************************************************

    STLExportDialog
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


#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "stlexportdialog.h"
#include <xflwidgets/customwts/intedit.h>
#include <xflobjects/objects3d/plane.h>

bool STLExportDlg::s_bBinary = true;
int STLExportDlg::s_iObject = 0;
int STLExportDlg::s_NChordPanels = 13;
int STLExportDlg::s_NSpanPanels = 17;

STLExportDlg::STLExportDlg()
{
    setWindowTitle(tr("STL exporter"));
    setupLayout();
    connectSignals();

    setLabels();
    m_prbBinary->setEnabled(false);
    m_prbASCII->setEnabled(false);
}


void STLExportDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QGroupBox *pExportFormat = new QGroupBox(tr("File format"));
        {
            QHBoxLayout *pFormatLayout = new QHBoxLayout;
            {
                m_prbBinary = new QRadioButton(tr("Binary"));
                m_prbASCII  = new QRadioButton(tr("ASCII"));
                pFormatLayout->addWidget(m_prbBinary);
                pFormatLayout->addWidget(m_prbASCII);
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
                    m_plabChordLabel = new QLabel("Chordwise panels");
                    m_plabChordLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    m_pieChordPanels = new IntEdit(17);
                    m_pieChordPanels->setAlignment(Qt::AlignRight);
                    pChordLayout->addStretch();
                    pChordLayout->addWidget(m_plabChordLabel);
                    pChordLayout->addWidget(m_pieChordPanels);
                }

                QHBoxLayout *pSpanLayout = new QHBoxLayout;
                {
                    m_plabSpanLabel = new QLabel("Spanwise panels");
                    m_plabSpanLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    m_pieSpanPanels = new IntEdit(17);
                    m_pieSpanPanels->setAlignment(Qt::AlignRight);
                    pSpanLayout->addStretch();
                    pSpanLayout->addWidget(m_plabSpanLabel);
                    pSpanLayout->addWidget(m_pieSpanPanels);
                }
                pResolutionLayout->addLayout(pChordLayout);
                pResolutionLayout->addLayout(pSpanLayout);
            }
            pResolutionBox->setLayout(pResolutionLayout);
        }

        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        {
            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
        }

        pMainLayout->addWidget(pExportFormat);
        pMainLayout->addWidget(pObjectBox);
        pMainLayout->addWidget(pResolutionBox);
        pMainLayout->addStretch();
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void STLExportDlg::connectSignals()
{
    connect(m_pieChordPanels, SIGNAL(editingFinished()), this, SLOT(onReadParams()));
    connect(m_pieSpanPanels,  SIGNAL(editingFinished()), this, SLOT(onReadParams()));
}


void STLExportDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok)      == pButton)  accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)  reject();
}


void STLExportDlg::accept()
{
    onReadParams();
    done(QDialog::Accepted);
}


void STLExportDlg::initDialog(Plane *pPlane)
{

    m_prbBinary->setChecked(s_bBinary);
    m_prbASCII->setChecked(!s_bBinary);
    m_pieChordPanels->setValue(s_NChordPanels);
    m_pieSpanPanels->setValue(s_NSpanPanels);

    m_prb[0]->setEnabled(pPlane->hasBody());
    m_prb[2]->setEnabled(pPlane->hasSecondWing());
    m_prb[3]->setEnabled(pPlane->hasElevator());
    m_prb[4]->setEnabled(pPlane->hasFin());

    if(s_iObject==0 && !pPlane->hasBody())       s_iObject=1;
    if(s_iObject==2 && !pPlane->hasSecondWing()) s_iObject=1;
    if(s_iObject==3 && !pPlane->hasElevator())   s_iObject=1;
    if(s_iObject==4 && !pPlane->hasFin())        s_iObject=1;

    for(int i=0; i<5; i++)
        m_prb[i]->setChecked(s_iObject==i);
}


void STLExportDlg::onReadParams()
{
    s_bBinary = m_prbBinary->isChecked();
    s_NChordPanels = m_pieChordPanels->value();
    s_NSpanPanels  = m_pieSpanPanels->value();
    for(int i=0; i<5; i++)
    {
        if(m_prb[i]->isChecked())
        {
            s_iObject=i;
            break;
        }
    }
}


void STLExportDlg::onObjectSelection()
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


void STLExportDlg::setLabels()
{
    if(s_iObject==0)
    {
        m_plabChordLabel->setText(tr("Number of x-panels"));
        m_plabSpanLabel->setText(tr("Number of hoop/y panels"));
    }
    else
    {
        m_plabChordLabel->setText(tr("Number of chordwise panels"));
        m_plabSpanLabel->setText(tr("Number of span panels per surface"));
    }
}


bool STLExportDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("STLExportDlg");
    {
        s_NChordPanels = settings.value("NChordPanels", 13).toInt();
        s_NSpanPanels  = settings.value("NSpanPanels", 17).toInt();
    }
    settings.endGroup();
    return true;
}



bool STLExportDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("STLExportDlg");
    {
        settings.setValue("NChordPanels", s_NChordPanels);
        settings.setValue("NSpanPanels", s_NSpanPanels);
    }
    settings.endGroup();
    return true;
}
