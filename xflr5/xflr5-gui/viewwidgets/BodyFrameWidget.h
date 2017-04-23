/****************************************************************************

	BodyFrameWidget Class
	Copyright (C) 2015 Andre Deperrois adeperrois@xflr5.com

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

#ifndef BODYFRAMEWIDGET_H
#define BODYFRAMEWIDGET_H


#include "section2dwidget.h"
#include <objects3d/Body.h>


class BodyFrameWidget : public Section2dWidget
{
	Q_OBJECT

	friend class GL3dBodyDlg;
//	friend class Body;

public:
	BodyFrameWidget(QWidget *pParent=NULL, Body *pBody=NULL);

	void setBody(Body *pBody);
	void drawFrameLines();
	void drawFramePoints();
	void drawScaleLegend(QPainter &painter);

	void setScale();
	void paintEvent(QPaintEvent *event);
	void resizeEvent (QResizeEvent *event);

	int highlightPoint(Vector3d real);
	int selectPoint(Vector3d real);
	void dragSelectedPoint(double x, double y);
	void createActions();

signals:
	void pointSelChanged();

private slots:
	void onInsertPt();
	void onRemovePt();
	void onScaleFrame();
	void onShowCurFrameOnly();

private:
	Body *m_pBody;
	QAction *m_pShowCurFrameOnly;
	static bool s_bCurFrameOnly;

};

#endif // BODYFRAMEWIDGET_H
