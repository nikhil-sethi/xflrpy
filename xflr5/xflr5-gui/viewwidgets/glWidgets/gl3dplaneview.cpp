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


#include <QOpenGLPaintDevice>
#include <QPainter>
#include "gl3dplaneview.h"
#include <miarex/design/EditPlaneDlg.h>


gl3dPlaneView::gl3dPlaneView(QWidget *pParent) : gl3dView(pParent)
{
	m_pParent = pParent;
	m_pPlane = NULL;
}



void gl3dPlaneView::glRenderView()
{
	EditPlaneDlg *pEPdlg = (EditPlaneDlg*)m_pParent;

	if(pEPdlg->m_pPlane)
	{
		paintBody(pEPdlg->m_pPlane->body());
		for(int iw=0; iw<MAXWINGS; iw++)
		{
			Wing * pWing = pEPdlg->m_pPlane->wing(iw);
			if(pWing)
			{
				paintWing(iw, pWing);
				if(m_bFoilNames) paintFoilNames(pWing);
			}
		}
		if(m_bShowMasses) glDrawMasses(pEPdlg->m_pPlane);
	}
}



void gl3dPlaneView::paintGL()
{
	EditPlaneDlg *pDlg = (EditPlaneDlg*)m_pParent;
	pDlg->glMake3DObjects();

	paintGL3();
	paintOverlay();
}



void gl3dPlaneView::on3DReset()
{
	startResetTimer(m_pPlane->span());
}


void gl3dPlaneView::set3DRotationCenter(QPoint point)
{
	//adjusts the new rotation center after the user has picked a point on the screen
	//finds the closest panel under the point,
	//and changes the rotation vector and viewport translation
	Vector3d I, A, B, AA, BB, PP;

	screenToViewport(point, B);
	B.z = -1.0;
	A.set(B.x, B.y, +1.0);

	viewportToWorld(A, AA);
	viewportToWorld(B, BB);

	m_transIncrement.set(BB.x-AA.x, BB.y-AA.y, BB.z-AA.z);
	m_transIncrement.normalize();

	bool bIntersect = false;


	EditPlaneDlg *pDlg = (EditPlaneDlg*)m_pParent;
	if(pDlg->intersectObject(AA, m_transIncrement, I))
	{
		bIntersect = true;
		PP.set(I);
	}

	if(bIntersect)
	{
		startTranslationTimer(PP);
	}
}



/**
*Overrides the resizeGL method of the base class.
* Sets the GL viewport to fit in the client area.
* Sets the scaling factors for the objects to be drawn in the viewport.
*@param width the width in pixels of the client area
*@param height the height in pixels of the client area
*/
void gl3dPlaneView::resizeGL(int width, int height)
{
	int side = qMin(width, height);
	glViewport((width - side) / 2, (height - side) / 2, side, side);

	double w, h, s;
	w = (double)width;
	h = (double)height;
	s = 1.0;

	if(w>h)	m_GLViewRect.setRect(-s, s*h/w, s, -s*h/w);
	else    m_GLViewRect.setRect(-s*w/h, s, s*w/h, -s);

	if(!m_PixTextOverlay.isNull())	m_PixTextOverlay = m_PixTextOverlay.scaled(rect().size()*devicePixelRatio());
	if(!m_PixTextOverlay.isNull())	m_PixTextOverlay.fill(Qt::transparent);

	EditPlaneDlg *pDlg = (EditPlaneDlg*)m_pParent;
	pDlg->resize3DView();
}


void gl3dPlaneView::paintOverlay()
{
	QOpenGLPaintDevice device(size() * devicePixelRatio());
	QPainter painter(&device);

	EditPlaneDlg *pDlg = (EditPlaneDlg*)m_pParent;
	if(!pDlg->m_PixText.isNull())   painter.drawPixmap(0,0, pDlg->m_PixText);
	if(!m_PixTextOverlay.isNull())  painter.drawPixmap(0,0, m_PixTextOverlay);
	m_PixTextOverlay.fill(Qt::transparent);
}





