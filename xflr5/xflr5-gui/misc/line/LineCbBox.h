/****************************************************************************

	LineCbBox Class
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


#ifndef LINECBBOX_H
#define LINECBBOX_H

#include <QComboBox>
#include <linestyle.h>


class LineCbBox : public QComboBox
{

public:
	LineCbBox(QWidget *pParent=NULL);

	QSize sizeHint() const;
	void paintEvent (QPaintEvent *event);

	void setLine(int const &style, int const &width, QColor const &color, const int &pointStyle);
	void setLine(LineStyle lineStyle);
	void showPoints(bool bShow){m_bShowPoints=bShow;}

	bool points() {return m_bShowPoints; }

private:
	LineStyle m_LineStyle;
	bool m_bShowPoints;
};

#endif // LINECBBOX_H
