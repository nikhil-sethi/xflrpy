/****************************************************************************

    LECircleDlg Class
    Copyright (C) 2009-2016 Andre Deperrois

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

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

#include "lecircledlg.h"

#include <misc/text/doubleedit.h>


LECircleDlg::LECircleDlg(QWidget *pParent): QDialog(pParent)
{
    setWindowTitle(tr("L.E. Circle"));
    setupLayout();
}


void LECircleDlg::setupLayout()
{
    QHBoxLayout *pLERadius = new QHBoxLayout;
    {
        m_pctrlRadius = new DoubleEdit(0.0,3);
        QLabel *lab0 = new QLabel(tr("r="));
        QLabel *lab1 = new QLabel(tr("% Chord"));
        lab0->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        pLERadius->addStretch(1);
        pLERadius->addWidget(lab0);
        pLERadius->addWidget(m_pctrlRadius);
        pLERadius->addWidget(lab1);
    }

    m_pctrlShow = new QCheckBox(tr("Show"));

    QHBoxLayout *pCommandButtons = new QHBoxLayout;
    {
        OKButton     = new QPushButton(tr("OK"));
        CancelButton = new QPushButton(tr("Cancel"));

        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(OKButton);
        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(CancelButton);
        pCommandButtons->addStretch(1);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    pMainLayout->addWidget(m_pctrlShow);
    pMainLayout->addStretch(1);
    pMainLayout->addLayout(pLERadius);
    pMainLayout->addStretch(1);
    pMainLayout->addLayout(pCommandButtons);
    pMainLayout->addStretch(1);

    setLayout(pMainLayout);

    connect(OKButton, SIGNAL(clicked()),this, SLOT(OnOK()));
    connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}


void LECircleDlg::InitDialog()
{
    m_pctrlRadius->setValue(m_Radius);
    m_pctrlShow->setChecked(m_bShowRadius);
}


void LECircleDlg::keyPressEvent(QKeyEvent *event)
{
    // Prevent Return Key from closing App
    switch (event->key())
    {
        case Qt::Key_Escape:
        {
            reject();
            return;
        }
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!OKButton->hasFocus() && !CancelButton->hasFocus())
            {
                OKButton->setFocus();
            }
            else
            {
                OnOK();
            }
            break;
        }
        default:
            event->ignore();
            break;
    }
}


void LECircleDlg::OnOK()
{
    m_Radius = m_pctrlRadius->value();
    m_bShowRadius = m_pctrlShow->isChecked();
    accept();
}



