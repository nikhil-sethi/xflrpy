/****************************************************************************

    AboutQ5 Class
    Copyright (C) 2008-2008 André Deperrois 

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

#include <QBitmap>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QPushButton>

#include <xflcore/gui_params.h>
#include "aboutq5.h"


AboutQ5::AboutQ5(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("About XFLR5"));
    setupLayout();
}


void AboutQ5::setupLayout()
{
    QGridLayout *pLogoLayout = new QGridLayout;
    {
        QLabel *LabIconQ5 = new QLabel;
        LabIconQ5->setObjectName("iconXFLR5");
        LabIconQ5->setPixmap(QPixmap(QString::fromUtf8(":/images/xflr5_64.png")));
        QLabel *lab1  = new QLabel(VERSIONNAME);
        lab1->setAlignment(Qt::AlignVCenter| Qt::AlignLeft);
        QLabel *XFLR5Link = new QLabel;
        XFLR5Link->setText("<a href=http://www.xflr5.tech>http://www.xflr5.tech</a>");
        XFLR5Link->setOpenExternalLinks(true);
        XFLR5Link->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
        XFLR5Link->setAlignment(Qt::AlignVCenter| Qt::AlignLeft);

        pLogoLayout->setColumnStretch(1,1);
        pLogoLayout->setColumnStretch(2,2);
        pLogoLayout->addWidget(LabIconQ5,1,1,2,1);
        pLogoLayout->addWidget(lab1,1,2);
        pLogoLayout->addWidget(XFLR5Link,2,2);
    }

    QLabel *pLab2  = new QLabel(tr("Copyright (C) M. Drela and H. Youngren 2000 - XFoil v6.94"));
    QLabel *pLab3  = new QLabel(tr("Copyright (C) Matthieu Scherrer 2004 - Miarex v1.00"));
    QLabel *pLab4  = new QLabel(tr("Copyright (C) André Deperrois 2003-2019"));
    QLabel *pLab5  = new QLabel(tr("This program is distributed in the hope that it will be useful,"));
    QLabel *pLab6  = new QLabel(tr("but WITHOUT ANY WARRANTY; without even the implied warranty of"));
    QLabel *pLab7  = new QLabel(tr("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."));
//    QLabel *pLab8  = new QLabel(tr("This program has been developed exclusively for the analysis of model aircraft"));
//    QLabel *pLab9  = new QLabel(tr("Any other usage is expressly prohibited"));
    QLabel *plab10 = new QLabel(tr("Program distributed  under the terms of the GNU General Public License"));
    QLabel *pLab11 = new QLabel(tr("German translation by Christall, Jochen Günzel and Martin Willner"));
    QLabel *pLab12 = new QLabel("Traducció al català per David Cànovas");
    QLabel *pLab13 = new QLabel(tr("Japanese translation by IKUSU, Koichi Akabe, Misatus, dynamicsoar, hide253"));
    QLabel *pLab14 = new QLabel(tr("icchy_07, ina111, ohayo_cycling, ohisa_64, ozawa64."));
    QLabel *pLab15 = new QLabel(tr("French translation by Jean-Luc Coulon"));


    QDialogButtonBox *pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);


    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addLayout(pLogoLayout);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(pLab2);
        pMainLayout->addWidget(pLab3);
        pMainLayout->addWidget(pLab4);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(pLab11);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(pLab12);
        pMainLayout->addWidget(pLab13);
        pMainLayout->addWidget(pLab14);
        pMainLayout->addWidget(pLab15);
        pMainLayout->addSpacing(20);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(pLab5);
        pMainLayout->addWidget(pLab6);
        pMainLayout->addWidget(pLab7);
        pMainLayout->addStretch(1);
//        pMainLayout->addWidget(lab8);
//        pMainLayout->addWidget(lab9);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(plab10);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(pButtonBox);
    }
    setLayout(pMainLayout);
    setMinimumHeight(400);
}



