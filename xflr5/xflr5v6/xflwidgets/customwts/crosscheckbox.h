/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QCheckBox>

class CrossCheckBox : public QCheckBox
{
    public:
        CrossCheckBox(QWidget *pParent = nullptr);


    public:
        void paintEvent(QPaintEvent *) override;
};

