/****************************************************************************

    LECircleDlg Class
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

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

#include "lecircledlg.h"

#include <xflwidgets/customwts/doubleedit.h>


LECircleDlg::LECircleDlg(QWidget *pParent): QDialog(pParent)
{
    setWindowTitle(tr("L.E. Circle"));
    setupLayout();
}


void LECircleDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QHBoxLayout *pLERadius = new QHBoxLayout;
        {
            m_pdeRadius = new DoubleEdit(0.0,3);
            QLabel *lab0 = new QLabel(tr("r="));
            QLabel *lab1 = new QLabel(tr("% Chord"));
            lab0->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            pLERadius->addStretch(1);
            pLERadius->addWidget(lab0);
            pLERadius->addWidget(m_pdeRadius);
            pLERadius->addWidget(lab1);
        }

        m_pchShow = new QCheckBox(tr("Show"));

        QDialogButtonBox *m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
        connect(m_pButtonBox, &QDialogButtonBox::rejected, this, &LECircleDlg::onOK);

        pMainLayout->addWidget(m_pchShow);
        pMainLayout->addStretch(1);
        pMainLayout->addLayout(pLERadius);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_pButtonBox);
        pMainLayout->addStretch(1);
    }

    setLayout(pMainLayout);
}


void LECircleDlg::initDialog()
{
    m_pdeRadius->setValue(m_Radius);
    m_pchShow->setChecked(m_bShowRadius);
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
            if(!m_pButtonBox->hasFocus())
            {
                m_pButtonBox->setFocus();
            }
            break;
        }
        default:
            event->ignore();
            break;
    }
}


void LECircleDlg::onOK()
{
    m_Radius = m_pdeRadius->value();
    m_bShowRadius = m_pchShow->isChecked();
    accept();
}



