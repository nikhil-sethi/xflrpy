/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/


#include "crosscheckbox.h"

#include <xflwidgets/wt_globals.h>

CrossCheckBox::CrossCheckBox(QWidget *pParent) : QCheckBox(pParent)
{

}


void CrossCheckBox::paintEvent(QPaintEvent *)
{
    QRect r(rect());
    r.adjust(1,1,-1,-1);
//    int side = (r.height()-2)/2;
    QFont fnt;
    QFontMetrics fm(fnt);

    QPainter painter(this);

    QColor backcolor;
    QColor crosscolor;

    backcolor = palette().window().color();
    crosscolor = palette().windowText().color();

    drawCheckBox(&painter, checkState(), r, fm.height(), false, true, crosscolor, backcolor, Qt::black);

}
