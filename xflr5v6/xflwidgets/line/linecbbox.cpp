/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/


#include <QPainter>
#include <QPaintEvent>

#include "linecbbox.h"
#include <xflcore/xflcore.h>
#include <xflwidgets/line/linedelegate.h>


LineCbBox::LineCbBox(int nItem, QWidget *pParent) : QComboBox(pParent)
{
    setParent(pParent);

    for (int i=0; i<nItem; i++) addItem("item");

    m_LineStyle.m_Stipple = Line::SOLID;
    m_LineStyle.m_Width = 1;
    m_LineStyle.m_Color = Qt::lightGray;
    m_LineStyle.m_Symbol = Line::NOSYMBOL;

    QSizePolicy szPolicyExpanding;
    szPolicyExpanding.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
    szPolicyExpanding.setVerticalPolicy(  QSizePolicy::Minimum);
    setSizePolicy(szPolicyExpanding);
}


QSize LineCbBox::sizeHint() const
{
    QFont fnt;
    QFontMetrics fm(fnt);
    int w = 7 * fm.averageCharWidth();
    int h = fm.height();
    return QSize(w, h);
}


void LineCbBox::setLine(Line::enumLineStipple style, int width, QColor color, Line::enumPointStyle pointStyle)
{
    m_LineStyle.m_Stipple = style;
    m_LineStyle.m_Width = width;
    m_LineStyle.m_Color = color;
    m_LineStyle.m_Symbol = pointStyle;
    setLine(m_LineStyle);
}


void LineCbBox::setLine(LineStyle const &ls)
{
    m_LineStyle = ls;

    LineDelegate *pLineDelegate = dynamic_cast<LineDelegate*>(itemDelegate());
    if(pLineDelegate) pLineDelegate->setLineColor(ls.m_Color);
}


void LineCbBox::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
//    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    painter.save();

    //	painter.setRenderHint(QPainter::Antialiasing);
    //	QColor ContourColor = Qt::gray;
    //	if(!isEnabled()) ContourColor = Qt::lightGray;

//    QRect r = pEvent->rect();
    QRect r = opt.rect;
    painter.setBrush(Qt::NoBrush);
    painter.setBackgroundMode(Qt::TransparentMode);

    QPen LinePen(m_LineStyle.m_Color);
    LinePen.setStyle(xfl::getStyle(m_LineStyle.m_Stipple));
    LinePen.setWidth(m_LineStyle.m_Width);
    painter.setPen(LinePen);
    painter.drawLine(r.left()+5, r.center().y(), r.width()-30, r.center().y());

    if(m_LineStyle.m_Symbol>0)
    {
        LinePen.setStyle(Qt::SolidLine);
        painter.setPen(LinePen);

        QPalette palette;
        xfl::drawSymbol(painter, m_LineStyle.m_Symbol, palette.window().color(), m_LineStyle.m_Color, r.center());
    }

//    painter.setPen(Qt::blue);
//    painter.drawRect(r);

    painter.restore();
}




