/****************************************************************************

	gl3dWingView Class
	Copyright (C) 2016 Andre Deperrois adeperrois@xflr5.com

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

#ifndef GL3DWINGVIEW_H
#define GL3DWINGVIEW_H
#include <objects3d/Wing.h>
#include <gl3dview.h>

class gl3dWingView : public gl3dView
{
public:
	gl3dWingView(QWidget *pParent = NULL);
	void setWing(Wing *pWing){m_pWing = pWing;}
private:
	void glRenderView();
	void paintGL();
	void paintOverlay();
	void set3DRotationCenter(QPoint point);

public slots:
	void on3DReset();

private:
	Wing *m_pWing;
};

#endif // GL3DWINGVIEW_H
