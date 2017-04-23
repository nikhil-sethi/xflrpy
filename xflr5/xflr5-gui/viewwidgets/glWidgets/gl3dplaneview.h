/****************************************************************************

	gl3dPlaneView Class
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

#ifndef GL3DPLANEVIEW_H
#define GL3DPLANEVIEW_H
#include <gl3dview.h>
#include <objects3d/Plane.h>

class gl3dPlaneView : public gl3dView
{
public:
	gl3dPlaneView(QWidget *pParent = NULL);
	void setPlane(Plane*pPlane){m_pPlane = pPlane;}

private:
	void glRenderView();
	void paintGL();
	void paintOverlay();
	void resizeGL(int width, int height);
	void set3DRotationCenter(QPoint point);

public slots:
	void on3DReset();

private:
	Plane* m_pPlane;
};

#endif // GL3DPLANEVIEW_H
