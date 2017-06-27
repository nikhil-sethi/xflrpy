/****************************************************************************

	LineDelegate Class
	Copyright (C) 2009 Andre Deperrois adeperrois@xflr5.com

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


#ifndef LINEDELEGATE_H
#define LINEDELEGATE_H

#include <QAbstractItemDelegate>
#include <QModelIndex>
#include <QSize>
#include <QColor>
#include <QAbstractItemModel>


class LineDelegate : public QAbstractItemDelegate
{
	Q_OBJECT

public:
	LineDelegate (QObject *parent = NULL);

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const;

	void setLineColor(QColor color);
	void setLineStyle(int *style);
	void setLineWidth(int *width);
	void setPointStyle(int *pointStyle);


private:
	void * m_pCbBox; //pointer to the parent QLineComboBox
	QSize m_Size;

	int m_LineStyle[5]; // values depend on whether we have a line or width CbBox....
	int m_LineWidth[5]; // values depend on whether we have a line or width CbBox....
	QColor m_LineColor; // the same for all CbBox items
	int m_PointStyle[5];
};


#endif //LINEDELEGATE_H
