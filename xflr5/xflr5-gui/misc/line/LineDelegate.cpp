/****************************************************************************

	LineDelegate Class
	Copyright (C) 2009 Andre Deperrois 

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

#include <QtDebug>
#include "LineDelegate.h"
#include "LineCbBox.h"
#include <globals/globals.h>
#include <graph_globals.h>


LineDelegate::LineDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{
	//initialize with something, just in case
	for (int i=0; i<5; i++)
	{
		m_LineWidth[i]  = i+1;
		m_LineStyle[i]  = i;
		m_PointStyle[i] = 0;
	}

	m_LineColor = QColor(0,255,0);
	m_Size.setHeight(15);
	m_Size.setWidth(50);

	m_pCbBox = parent;
}



void LineDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (option.state & QStyle::State_Selected)    painter->fillRect(option.rect, option.palette.highlight());

	painter->save();

	QPen LinePen(m_LineColor);
	LinePen.setStyle(getStyle(m_LineStyle[index.row()]));
	LinePen.setWidth(m_LineWidth[index.row()]);
	painter->setPen(LinePen);

//	if (option.state & QStyle::State_Selected)  painter->setBrush(option.palette.highlightedText());
//	else                                        painter->setBrush(QBrush(Qt::black));

	painter->drawLine(option.rect.x()+3,
					  option.rect.center().y(),
					  option.rect.width()-6,
					  option.rect.center().y());


	if(m_pCbBox && ((LineCbBox*)m_pCbBox)->points())
	{
		LinePen.setStyle(Qt::SolidLine);
		painter->setPen(LinePen);
		drawPoint(*painter, m_PointStyle[index.row()], QColor(255,255,255), option.rect.center());
	}
	painter->restore();
}


QSize LineDelegate::sizeHint(const QStyleOptionViewItem & /* option */, const QModelIndex & /* index */) const
{
	return m_Size;
}


void LineDelegate::setLineColor(QColor color)
{
	m_LineColor = color;
}


void LineDelegate::setLineStyle(int *style)
{
	for (int i=0; i<5; i++)	m_LineStyle[i] = style[i];
}


void LineDelegate::setLineWidth(int *width)
{
	for (int i=0; i<5; i++)	m_LineWidth[i] = width[i];
}


void LineDelegate::setPointStyle(int *pointStyle)
{
	for (int i=0; i<5; i++)	m_PointStyle[i] = pointStyle[i];
}














