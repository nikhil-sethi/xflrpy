/****************************************************************************

	gl3dBodyView Class
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
#include "gl3dbodyview.h"
#include <misc/Settings.h>
#include <miarex/view/W3dPrefsDlg.h>
#include <miarex/design/GL3dBodyDlg.h>
#include <mainframe.h>


gl3dBodyView::gl3dBodyView(QWidget *pParent) : gl3dView(pParent)
{
	m_pParent = pParent;
	m_pBody = NULL;
	m_bResetglFrameHighlight   = true;
	m_bResetglBody        = true;//otherwise endless repaint if no body present
}


void gl3dBodyView::glRenderView()
{
	if(m_pBody)
	{
		if(m_bVLMPanels) paintBodyMesh(m_pBody);
		paintBody(m_pBody);

		if(m_pBody->activeFrame()) paintSectionHighlight();
		if(m_bShowMasses) paintMasses(m_pBody->volumeMass(), Vector3d(0.0,0.0,0.0), "Structural mass", m_pBody->m_PointMass);
	}
}


/**
* Overrides the contextMenuEvent method of the base class.
* Dispatches the handling to the active child application.
*/
void gl3dBodyView::contextMenuEvent (QContextMenuEvent * event)
{
	Q_UNUSED(event);
	m_bArcball = false;
	update();

//	GL3dBodyDlg *pDlg = (GL3dBodyDlg*)m_pParent;
//	pDlg->showContextMenu(event);
}






void gl3dBodyView::on3DReset()
{
	startResetTimer(m_pBody->length());
}


void gl3dBodyView::paintGL()
{
	glMake3DObjects();

	paintGL3();
	paintOverlay();
}



/**
* Creates the VertexBufferObjects for OpenGL 3.0
*/
void gl3dBodyView::glMake3DObjects()
{
	if(m_bResetglFrameHighlight || m_bResetglBody)
	{
		if(m_pBody->activeFrame())
		{
			glMakeBodyFrameHighlight(m_pBody,Vector3d(0.0,0.0,0.0), m_pBody->m_iActiveFrame);
			m_bResetglFrameHighlight = false;
		}
	}

	if(m_bResetglBody)
	{
		m_bResetglBody = false;
		if(m_pBody->isSplineType())         glMakeBodySplines(m_pBody);
		else if(m_pBody->isFlatPanelType()) glMakeBody3DFlatPanels(m_pBody);
		glMakeBodyMesh(m_pBody);
	}
}




void gl3dBodyView::set3DRotationCenter(QPoint point)
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


	if(m_pBody->intersectFlatPanels(AA, AA+m_transIncrement*10, I))
	{
		bIntersect = true;
		PP.set(I);
	}


	if(bIntersect)
	{
		startTranslationTimer(PP);
	}
}


void gl3dBodyView::paintOverlay()
{
	QOpenGLPaintDevice device(size() * devicePixelRatio());
	QPainter painter(&device);

	/*	EditBodyDlg *pDlg = (EditBodyDlg*)m_pParent;
		painter.drawPixmap(0,0, pDlg->m_PixText);
		painter.drawPixmap(0,0, m_PixTextOverlay);
		m_PixTextOverlay.fill(Qt::transparent);
	*/
}



/** used for body edition only */
void gl3dBodyView::glMakeBodyMesh(Body *pBody)
{
	if(!pBody) return;
	int NXXXX = W3dPrefsDlg::s_iBodyAxialRes;
	int NHOOOP = W3dPrefsDlg::s_iBodyAxialRes;
	int nx, nh;
	Vector3d Pt;
	Vector3d P1, P2, P3, P4, PStart, PEnd;
	float *meshVertexArray = NULL;
	int bufferSize = 0;
	m_iBodyMeshLines = 0;

	int iv=0;

	if(pBody->isFlatPanelType()) //LINES
	{
		bufferSize = 0;
		for (int j=0; j<pBody->frameCount()-1;j++)
		{
			for (int k=0; k<pBody->sideLineCount()-1;k++)
			{
				for(int jp=0; jp<pBody->m_xPanels[j]; jp++)
				{
					bufferSize += 6;
				}
				for(int kp=0; kp<pBody->m_hPanels[k]; kp++)
				{
					bufferSize += 6;
				}
			}
		}
		bufferSize *=2;

		meshVertexArray = new float[bufferSize];

		for (int j=0; j<pBody->frameCount()-1;j++)
		{
			for (int k=0; k<pBody->sideLineCount()-1;k++)
			{
				P1 = pBody->frame(j)->m_CtrlPoint[k];       P1.x = pBody->frame(j)->m_Position.x;
				P2 = pBody->frame(j+1)->m_CtrlPoint[k];     P2.x = pBody->frame(j+1)->m_Position.x;
				P3 = pBody->frame(j+1)->m_CtrlPoint[k+1];   P3.x = pBody->frame(j+1)->m_Position.x;
				P4 = pBody->frame(j)->m_CtrlPoint[k+1];     P4.x = pBody->frame(j)->m_Position.x;

				//left side panels
				for(int jp=0; jp<pBody->m_xPanels[j]; jp++)
				{
					PStart = P1 + (P2-P1) * (float)jp/(float)pBody->m_xPanels[j];
					PEnd   = P4 + (P3-P4) * (float)jp/(float)pBody->m_xPanels[j];
					meshVertexArray[iv++] = PStart.x;
					meshVertexArray[iv++] = PStart.y;
					meshVertexArray[iv++] = PStart.z;
					meshVertexArray[iv++] = PEnd.x;
					meshVertexArray[iv++] = PEnd.y;
					meshVertexArray[iv++] = PEnd.z;
					m_iBodyMeshLines++;
				}
				for(int kp=0; kp<pBody->m_hPanels[k]; kp++)
				{
					PStart = P1 + (P4-P1) * (float)kp/(float)pBody->m_hPanels[k];
					PEnd   = P2 + (P3-P2) * (float)kp/(float)pBody->m_hPanels[k];
					meshVertexArray[iv++] = PStart.x;
					meshVertexArray[iv++] = PStart.y;
					meshVertexArray[iv++] = PStart.z;
					meshVertexArray[iv++] = PEnd.x;
					meshVertexArray[iv++] = PEnd.y;
					meshVertexArray[iv++] = PEnd.z;
					m_iBodyMeshLines++;
				}

				//right side panels
				for(int jp=0; jp<pBody->m_xPanels[j]; jp++)
				{
					PStart = P1 + (P2-P1) * (float)jp/(float)pBody->m_xPanels[j];
					PEnd   = P4 + (P3-P4) * (float)jp/(float)pBody->m_xPanels[j];
					meshVertexArray[iv++] =  PStart.x;
					meshVertexArray[iv++] = -PStart.y;
					meshVertexArray[iv++] =  PStart.z;
					meshVertexArray[iv++] =  PEnd.x;
					meshVertexArray[iv++] = -PEnd.y;
					meshVertexArray[iv++] =  PEnd.z;
					m_iBodyMeshLines++;
				}
				for(int kp=0; kp<pBody->m_hPanels[k]; kp++)
				{
					PStart = P1 + (P4-P1) * (float)kp/(float)pBody->m_hPanels[k];
					PEnd   = P2 + (P3-P2) * (float)kp/(float)pBody->m_hPanels[k];
					meshVertexArray[iv++] =  PStart.x;
					meshVertexArray[iv++] = -PStart.y;
					meshVertexArray[iv++] =  PStart.z;
					meshVertexArray[iv++] =  PEnd.x;
					meshVertexArray[iv++] = -PEnd.y;
					meshVertexArray[iv++] =  PEnd.z;
					m_iBodyMeshLines++;
				}
			}
		}
		Q_ASSERT(m_iBodyMeshLines*6==bufferSize);
		Q_ASSERT(iv==bufferSize);
	}
	else if(pBody->isSplineType()) //NURBS
	{
		nx = pBody->m_nxPanels;
		nh = pBody->m_nhPanels;

		bufferSize = 0;
		bufferSize += nh * NXXXX; // nh longitudinal lines
		bufferSize += nx * NHOOOP; // nx hoop line
		bufferSize *= 2;       // two sides
		bufferSize *= 3;       // 3 components/vertex;

		meshVertexArray = new float[bufferSize];

		pBody->setPanelPos();
		//x-lines;
		for (int l=0; l<nh; l++)
		{
			double v = (double)l/(double)(nh-1);
			for (int k=0; k<NXXXX; k++)
			{
				double u = (double)k/(double)(NXXXX-1);
				pBody->getPoint(u,  v, true, Pt);
				meshVertexArray[iv++] = Pt.x;
				meshVertexArray[iv++] = Pt.y;
				meshVertexArray[iv++] = Pt.z;
			}
		}
		for (int l=0; l<nh; l++)
		{
			double v = (double)l/(double)(nh-1);
			for (int k=0; k<NXXXX; k++)
			{
				double u = (double)k/(double)(NXXXX-1);
				pBody->getPoint(u,  v, false, Pt);
				meshVertexArray[iv++] = Pt.x;
				meshVertexArray[iv++] = Pt.y;
				meshVertexArray[iv++] = Pt.z;
			}
		}

		//hoop lines;
		for (int k=0; k<nx; k++)
		{
			double uk = pBody->m_XPanelPos[k];
			for (int l=0; l<NHOOOP; l++)
			{
				double v = (double)l/(double)(NHOOOP-1);
				pBody->getPoint(uk,  v, true, Pt);
				meshVertexArray[iv++] = Pt.x;
				meshVertexArray[iv++] = Pt.y;
				meshVertexArray[iv++] = Pt.z;
			}
		}
		for (int k=0; k<nx; k++)
		{
			double uk = pBody->m_XPanelPos[k];
			for (int l=0; l<NHOOOP; l++)
			{
				double v = (double)l/(double)(NHOOOP-1);
				pBody->getPoint(uk,  v, false, Pt);
				meshVertexArray[iv++] = Pt.x;
				meshVertexArray[iv++] = Pt.y;
				meshVertexArray[iv++] = Pt.z;
			}
		}
	}
	Q_ASSERT(iv==bufferSize);

	m_vboEditMesh.destroy();
	m_vboEditMesh.create();
	m_vboEditMesh.bind();
	m_vboEditMesh.allocate(meshVertexArray, bufferSize * sizeof(GLfloat));
	m_vboEditMesh.release();

	delete[] meshVertexArray;
}




/** Used only in ***BodyDlg, at a time when the mesh panels have not yet been built */
void gl3dBodyView::paintBodyMesh(Body *pBody)
{
	if(!pBody) return;

	//mesh background
	m_ShaderProgramSurface.bind();
	m_ShaderProgramSurface.setUniformValue(m_LightLocationSurface, 0); // no light for the background
	m_ShaderProgramSurface.enableAttributeArray(m_VertexLocationSurface);
	m_ShaderProgramSurface.enableAttributeArray(m_NormalLocationSurface);
	m_ShaderProgramSurface.setUniformValue(m_ColorLocationSurface, Settings::s_BackgroundColor);

	m_vboBody.bind();
	m_ShaderProgramSurface.setAttributeBuffer(m_VertexLocationSurface, GL_FLOAT, 0,                  3, 8 * sizeof(GLfloat));
	m_ShaderProgramSurface.setAttributeBuffer(m_NormalLocationSurface, GL_FLOAT, 3* sizeof(GLfloat), 3, 8 * sizeof(GLfloat));

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0, 1.0);
	glDrawElements(GL_TRIANGLES, m_iBodyElems/2, GL_UNSIGNED_SHORT, m_BodyIndicesArray);
	glDrawElements(GL_TRIANGLES, m_iBodyElems/2, GL_UNSIGNED_SHORT, m_BodyIndicesArray+m_iBodyElems/2);
	glDisable(GL_POLYGON_OFFSET_FILL);

	m_ShaderProgramSurface.disableAttributeArray(m_VertexLocationSurface);
	m_ShaderProgramSurface.disableAttributeArray(m_NormalLocationSurface);
	m_ShaderProgramSurface.release();

	m_vboEditMesh.release();
	m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.release();

	if(pBody->isFlatPanelType())
	{
		m_ShaderProgramLine.bind();
		m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
		m_vboEditMesh.bind();
		m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3);
		m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_VLMColor);

		glLineWidth(W3dPrefsDlg::s_VLMWidth);

		glPolygonOffset(1.0, 1.0);
		glDrawArrays(GL_LINES, 0, m_iBodyMeshLines*2);

		m_vboEditMesh.release();
		m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
		m_ShaderProgramLine.release();
	}
	else if(pBody->isSplineType())
	{
		int pos=0;
		int NXXXX = W3dPrefsDlg::s_iBodyAxialRes;
		int NHOOOP = W3dPrefsDlg::s_iBodyAxialRes;


		//panel lines
		m_ShaderProgramLine.bind();
		m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
		m_vboEditMesh.bind();
		m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3);
		m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_VLMColor);

		glEnable (GL_LINE_STIPPLE);
		switch(W3dPrefsDlg::s_VLMStyle)
		{
			case 1:  glLineStipple (1, 0xCFCF); break;
			case 2:  glLineStipple (1, 0x6666); break;
			case 3:  glLineStipple (1, 0xFF18); break;
			case 4:  glLineStipple (1, 0x7E66); break;
			default: glLineStipple (1, 0xFFFF); break;
		}
		glLineWidth(W3dPrefsDlg::s_VLMWidth);

		pos=0;
		//x-lines
		for (int l=0; l<2*pBody->m_nhPanels; l++)
		{
			glDrawArrays(GL_LINE_STRIP, pos, NXXXX);
			pos += NXXXX;
		}

		//hoop lines;
		for (int k=0; k<2*pBody->m_nxPanels; k++)
		{
			glDrawArrays(GL_LINE_STRIP, pos, NHOOOP);
			pos += NHOOOP;
		}
		glDisable(GL_LINE_STIPPLE);
	}
}







