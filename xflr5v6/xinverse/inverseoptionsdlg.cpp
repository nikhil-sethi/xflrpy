/****************************************************************************

    InverseOptionsDlg  Classes
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
#include <QGridLayout>

#include <xflcore/xflcore.h>

#include "xinverse.h"
#include "inverseoptionsdlg.h"
#include <xflwidgets/line/linemenu.h>
#include <xflobjects/objects2d/foil.h>
#include <xflwidgets/line/linebtn.h>


InverseOptionsDlg::InverseOptionsDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("XInverse Style"));
    m_pXInverse = nullptr;
    setupLayout();
}


void InverseOptionsDlg::setupLayout()
{
    QGridLayout *pStyleLayout = new QGridLayout;
    {
        QLabel *plab1 = new QLabel(tr("Reference Foil"));
        QLabel *plab2 = new QLabel(tr("Modified Foil"));
        QLabel *plab3 = new QLabel(tr("Spline"));
        QLabel *plab4 = new QLabel(tr("Reflected Curve"));
        plab1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        plab2->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        plab3->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        plab4->setAlignment(Qt::AlignRight | Qt::AlignVCenter);


        m_plbRefFoil   = new LineBtn(this);
        m_plbModFoil   = new LineBtn(this);
        m_plbSpline    = new LineBtn(this);
        m_plbReflected = new LineBtn(this);

        pStyleLayout->addWidget(plab1,1,1);
        pStyleLayout->addWidget(plab2,2,1);
        pStyleLayout->addWidget(plab3,3,1);
        pStyleLayout->addWidget(plab4,4,1);
        pStyleLayout->addWidget(m_plbRefFoil,1,2);
        pStyleLayout->addWidget(m_plbModFoil,2,2);
        pStyleLayout->addWidget(m_plbSpline,3,2);
        pStyleLayout->addWidget(m_plbReflected,4,2);
    }

    QDialogButtonBox *pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    {
        connect(pButtonBox, &QDialogButtonBox::accepted, this, &InverseOptionsDlg::accept);
        connect(pButtonBox, &QDialogButtonBox::rejected, this, &InverseOptionsDlg::reject);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addLayout(pStyleLayout);
        pMainLayout->addWidget(pButtonBox);
    }

    setLayout(pMainLayout);

    connect(m_plbRefFoil,   SIGNAL(clickedLB(LineStyle)), SLOT(onRefStyle()));
    connect(m_plbModFoil,   SIGNAL(clickedLB(LineStyle)), SLOT(onModStyle()));
    connect(m_plbSpline,    SIGNAL(clickedLB(LineStyle)), SLOT(onSplineStyle()));
    connect(m_plbReflected, SIGNAL(clickedLB(LineStyle)), SLOT(onReflectedStyle()));
}


void InverseOptionsDlg::initDialog()
{
    m_plbRefFoil->setTheStyle(m_pXInverse->m_pRefFoil->theStyle());
    m_plbModFoil->setTheStyle(m_pXInverse->m_pModFoil->theStyle());
    m_plbSpline->setTheStyle(m_pXInverse->m_Spline.theStyle());
    m_plbReflected->setTheStyle(m_pXInverse->m_ReflectedStyle);
}


void InverseOptionsDlg::onRefStyle()
{
    LineMenu lm(nullptr, true);
    lm.initMenu(m_pXInverse->m_pRefFoil->theStyle());
    lm.exec(QCursor::pos());
    m_pXInverse->m_pRefFoil->setTheStyle(lm.theStyle());
    m_plbRefFoil->setTheStyle(lm.theStyle());
}


void InverseOptionsDlg::onModStyle()
{
    LineMenu lm(nullptr, true);
    lm.initMenu(m_pXInverse->m_pModFoil->theStyle());
    lm.exec(QCursor::pos());
    m_pXInverse->m_pModFoil->setTheStyle(lm.theStyle());
    m_plbModFoil->setTheStyle(lm.theStyle());
}


void InverseOptionsDlg::onSplineStyle()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(m_pXInverse->m_Spline.theStyle());
    lm.exec(QCursor::pos());

    m_pXInverse->m_Spline.setTheStyle(lm.theStyle());
    m_plbSpline->setTheStyle(lm.theStyle());
}


void InverseOptionsDlg::onReflectedStyle()
{

    LineMenu lm(nullptr, false);
    lm.initMenu(m_pXInverse->m_ReflectedStyle);
    lm.exec(QCursor::pos());

    m_pXInverse->m_ReflectedStyle = lm.theStyle();
    m_plbReflected->setTheStyle(lm.theStyle());
}


