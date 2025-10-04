/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/


#include "lineaction.h"


LineAction::LineAction(QObject *pParent) : QWidgetAction(pParent)
{
    setDefaultWidget(&m_LineBtn);
    connect(&m_LineBtn, SIGNAL(clickedLB(LineStyle)), this, SLOT(onClickedLB(LineStyle)));
}


void LineAction::onClickedLB(LineStyle )
{
    QAction::trigger();
}
