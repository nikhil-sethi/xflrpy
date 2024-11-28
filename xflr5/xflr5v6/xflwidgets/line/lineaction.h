/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/



#pragma once

#include <xflcore/linestyle.h>

#include <xflwidgets/line/linebtn.h>
#include <QWidgetAction>

class LineAction : public QWidgetAction
{
    Q_OBJECT

    public:
        LineAction(QObject *pParent=nullptr);
        LineBtn &lineBtn() {return m_LineBtn;}


    private slots:
        void onClickedLB(LineStyle ls);

    private:
        LineBtn m_LineBtn;
};

