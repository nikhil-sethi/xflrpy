/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/



#include <QPainter>

#include <xflcore/xflcore.h>
#include "linedelegate.h"
#include "linecbbox.h"

LineDelegate::LineDelegate(LineCbBox *pCbBox) : QAbstractItemDelegate(pCbBox)
{
    //initialize with something, just in case
    m_Size.setHeight(15);
    m_Size.setWidth(50);

    m_pCbBox = pCbBox;
}


void LineDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (option.state & QStyle::State_Selected) painter->fillRect(option.rect, option.palette.highlight());

    painter->save();

    LineStyle ls;
    if(index.row()<m_LineStyle.size()) ls = m_LineStyle.at(index.row());
    else                               ls = {true, Line::SOLID, 1, Qt::lightGray,  Line::NOSYMBOL, QString(), false};


    QPen LinePen(ls.m_Color);
    LinePen.setStyle(xfl::getStyle(ls.m_Stipple));
    LinePen.setWidth(ls.m_Width);
    painter->setPen(LinePen);

    //	if (option.state & QStyle::State_Selected)  painter->setBrush(option.palette.highlightedText());
    //	else                                        painter->setBrush(QBrush(Qt::black));

    painter->drawLine(option.rect.x()+3,
                      option.rect.center().y(),
                      option.rect.width()-6,
                      option.rect.center().y());



    LinePen.setStyle(Qt::SolidLine);
    painter->setPen(LinePen);
    xfl::drawSymbol(*painter, ls.m_Symbol, Qt::transparent, ls.m_Color, option.rect.center());

    painter->restore();
}


QSize LineDelegate::sizeHint(const QStyleOptionViewItem & /* option */, const QModelIndex & /* index */) const
{
    return m_Size;
}


void LineDelegate::setLineColor(QColor const &clr)
{
    for(int i=0; i<m_LineStyle.size(); i++) m_LineStyle[i].m_Color = clr;
}






