/****************************************************************************

    LineBtn Class
    Copyright (C) 2014 Andre Deperrois adeperrois@xflr5.com

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


#include <QStyleOption>
#include <QMouseEvent>
#include <QPen>
#include <QPainter>

#include "./Settings.h"
#include "LineBtn.h"
#include <globals.h>


LineBtn::LineBtn(QWidget *parent)
    : QAbstractButton(parent)
{
	QSizePolicy szPolicyExpanding;
	szPolicyExpanding.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
	szPolicyExpanding.setVerticalPolicy(QSizePolicy::Minimum);
//	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	setSizePolicy(szPolicyExpanding);

	m_LineStyle.m_Color = Qt::darkGray;
	m_LineStyle.m_Style = 0;
	m_LineStyle.m_Width = 1;
	m_LineStyle.m_PointStyle = 0;
}



void LineBtn::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		emit clickedLB();
	}
	else
		QWidget::mouseReleaseEvent(event);
}


QSize LineBtn::sizeHint() const
{
	QFontMetrics fm(Settings::s_TextFont);
	int w = 7 * fm.averageCharWidth();
	int h = fm.height();
	return QSize(w, h);
}


void LineBtn::setColor(QColor const & color)
{
	m_LineStyle.m_Color = color;
	update();
}


void LineBtn::setStyle(int const & style)
{
	m_LineStyle.m_Style = style;
	update();
}


void LineBtn::setWidth(int const & width)
{
	m_LineStyle.m_Width = width;
	update();
}


void LineBtn::setPointStyle(int const & pointStyle)
{
	m_LineStyle.m_PointStyle = pointStyle;
	update();
}

void LineBtn::setStyle(int const &style, int const &width, QColor const & color, int const & pointStyle)
{
	m_LineStyle.m_Style = style;
	m_LineStyle.m_Width = width;
	m_LineStyle.m_Color = color;
	m_LineStyle.m_PointStyle = pointStyle;
	update();
}



void LineBtn::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.save();

	QRect r = rect();

//	QStyleOption opt;
//	opt.initFrom(this);
//	style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);


//	painter.setBrush(Qt::DiagCrossPattern);
	painter.setBackgroundMode(Qt::TransparentMode);

	QPen LinePen(m_LineStyle.m_Color);
	LinePen.setStyle(::getStyle(m_LineStyle.m_Style));
	LinePen.setWidth(m_LineStyle.m_Width);
	painter.setPen(LinePen);
	painter.drawLine(r.left()+5, r.height()/2, r.width()-5, r.height()/2);

/*
	switch(m_LineStyle.m_PointStyle)
	{
		case 0: break;
		case 1:
		{
			int ptSide = 2;
			painter.drawEllipse(r.center().x()-ptSide, r.center().y()-ptSide, 2*ptSide, 2*ptSide );
			break;
		}
		case 2:
		{
			int ptSide = 4;
			painter.drawEllipse(r.center().x()-ptSide, r.center().y()-ptSide, 2*ptSide, 2*ptSide );
			break;
		}
		case 3:
		{
			int ptSide = 2;
			painter.drawRect(r.center().x()-ptSide, r.center().y()-ptSide, 2*ptSide, 2*ptSide );
			break;
		}
		case 4:
		{
			int ptSide = 4;
			painter.drawRect(r.center().x()-ptSide, r.center().y()-ptSide, 2*ptSide, 2*ptSide );
			break;
		}
		default: break;
	}*/

	painter.restore();
	event->accept();
}
