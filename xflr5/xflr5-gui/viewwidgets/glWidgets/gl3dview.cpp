/****************************************************************************

	gl3dView Class
	Copyright (C) 2016 Andre Deperrois 

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

#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLPaintDevice>
#include <QMouseEvent>

#include "gl3dview.h"
#include <globals/globals.h>
#include <globals/mainframe.h>
#include <misc/options/displayoptions.h>
#include <misc/options/Units.h>
#include <globals/mainframe.h>
#include <objects/objects3d/vector3d.h>
#include <objects/objects3d/Body.h>
#include <objects/objects3d/Wing.h>
#include <objects/objects3d/Plane.h>
#include <objects/objects3d/WPolar.h>
#include <objects/objects3d/Surface.h>
#include <miarex/Miarex.h>
#include <miarex/objects3d.h>
#include <miarex/design/GL3dBodyDlg.h>
#include <miarex/design/GL3dWingDlg.h>
#include <miarex/design/EditBodyDlg.h>
#include <miarex/design/EditPlaneDlg.h>
#include <miarex/view/GL3DScales.h>
#include <miarex/view/W3dPrefsDlg.h>
#include <analysis3d/plane_analysis/LLTAnalysis.h>
#include <globals/gui_params.h>


Miarex *gl3dView::s_pMiarex;
MainFrame *gl3dView::s_pMainFrame;

GLLightDlg *gl3dView::s_pglLightDlg = NULL;


gl3dView::gl3dView(QWidget *pParent) : QOpenGLWidget(pParent)
{
	setAutoFillBackground(false);
	setMouseTracking(true);
	setCursor(Qt::CrossCursor);

	m_pParent = pParent;

	m_pTransitionTimer = NULL;
	memset(ab_new, 0, 16*sizeof(float));
	memset(ab_old, 0, 16*sizeof(float));
	m_iTransitionInc = 0;

	m_bArcball = m_bCrossPoint = false;

	m_bUse120StyleShaders = true;

	for(int iw=0; iw<MAXWINGS; iw++)
	{
		m_WingIndicesArray[iw] = NULL;
	}
	m_BodyIndicesArray = NULL;
	m_SphereIndicesArray = NULL;
	m_WingMeshIndicesArray = NULL;

	m_pLeftBodyTexture = m_pRightBodyTexture= NULL;
	for(int iw=0; iw<MAXWINGS; iw++)
	{
		m_pWingTopLeftTexture[iw] = m_pWingTopRightTexture[iw] = NULL;
		m_pWingBotLeftTexture[iw] = m_pWingBotRightTexture[iw] = NULL;
	}

	m_bOutline    = true;
	m_bSurfaces   = true;
	m_bVLMPanels  = false;
	m_bAxes       = true;
	m_bShowMasses = false;
	m_bFoilNames  = false;

	m_iBodyElems = 0;
	for(int iw=0; iw<MAXWINGS; iw++) m_iWingElems[iw]=0;
	m_iWingMeshElems = 0;

	m_NStreamLines = 0;
	m_nHighlightLines = m_HighlightLineSize = 0;

	m_glViewportTrans.x  = 0.0;
	m_glViewportTrans.y  = 0.0;
	m_glViewportTrans.z  = 0.0;

	m_glScaled = m_glScaledRef = 1.0;
	m_glScaleIncrement = 0.0;
	m_ClipPlanePos  = 5.0;

	m_bTrans = false;

	m_LastPoint.setX(0);
	m_LastPoint.setY(0);

	m_PixTextOverlay = QPixmap(107, 97);
	m_PixTextOverlay.fill(Qt::transparent);

	memset(m_Ny, 0, sizeof(m_Ny));
	memset(MatIn,  0, 16*sizeof(double));
	memset(MatOut, 0, 16*sizeof(double));
}


void gl3dView::printFormat(const QSurfaceFormat &format)
{
	Trace(QString("OpenGL version: %1.%2").arg(format.majorVersion()).arg(format.minorVersion()));

	switch (format.profile()) {
		case QSurfaceFormat::NoProfile:
				Trace("No Profile");
			break;
		case QSurfaceFormat::CoreProfile:
				Trace("Core Profile");
			break;
		case QSurfaceFormat::CompatibilityProfile:
				Trace("Compatibility Profile");
			break;
		default:
			break;
	}
	switch (format.renderableType())
	{
		case QSurfaceFormat::DefaultRenderableType:
				Trace("DefaultRenderableType: The default, unspecified rendering method");
			break;
		case QSurfaceFormat::OpenGL:
				Trace("OpenGL: Desktop OpenGL rendering");
			break;
		case QSurfaceFormat::OpenGLES:
				Trace("OpenGLES: OpenGL ES 2.0 rendering");
			break;
		case QSurfaceFormat::OpenVG:
				Trace("OpenVG: Open Vector Graphics rendering");
			break;
		default:
			break;
	}
}


gl3dView::~gl3dView()
{
	for(int iWing=0; iWing<MAXWINGS; iWing++)
	{
		if(m_WingIndicesArray[iWing]) delete [] m_WingIndicesArray[iWing];
		m_vboEditWingMesh[iWing].destroy();
	}

	if(m_BodyIndicesArray)     delete[] m_BodyIndicesArray;
	if(m_SphereIndicesArray)   delete[] m_SphereIndicesArray;
	if(m_WingMeshIndicesArray) delete[] m_WingMeshIndicesArray;

	// release all OpenGL resources.
	makeCurrent();
	m_vboArcBall.destroy();
	m_vboArcPoint.destroy();
	m_vboSphere.destroy();
	m_vboBody.destroy();
	m_vboEditBodyMesh.destroy();

	for(int iWing=0; iWing<MAXWINGS; iWing++)
	{
		m_vboWingSurface[iWing].destroy();
		m_vboWingOutline[iWing].destroy();
	}

	if(m_pLeftBodyTexture)     delete m_pLeftBodyTexture;
	if(m_pRightBodyTexture)    delete m_pRightBodyTexture;

	for(int iw=0; iw<MAXWINGS; iw++)
	{
		if(m_pWingBotLeftTexture[iw])  delete m_pWingBotLeftTexture[iw];
		if(m_pWingTopLeftTexture[iw])  delete m_pWingTopLeftTexture[iw];
		if(m_pWingBotRightTexture[iw]) delete m_pWingBotRightTexture[iw];
		if(m_pWingTopRightTexture[iw]) delete m_pWingTopRightTexture[iw];
		m_pWingBotLeftTexture[iw] = NULL;
		m_pWingTopLeftTexture[iw] = NULL;
		m_pWingBotRightTexture[iw] = NULL;
		m_pWingTopRightTexture[iw] = NULL;
	}

	doneCurrent();
}


QSize gl3dView::sizeHint() const
{
	return QSize(640, 480);
}


QSize gl3dView::minimumSizeHint() const
{
	return QSize(250, 200);
}


/**
* The user has modified the position of the clip plane in the 3D view
*@param pos the new z position in viewport coordinates of the clipping plane
*/
void gl3dView::onClipPlane(int pos)
{
	double coef = 4.0;
	double planepos =  (double)pos/100.0;
	m_ClipPlanePos = 5.0*sinh(planepos*coef)/sinh(coef);
	update();
}


void gl3dView::on3DIso()
{
	Quaternion qti;

	memcpy(ab_old, m_ArcBall.ab_quat, 16*sizeof(float));

	double yaw = -PI;
	double pitch = 0.0;
	double roll = -2.0*PI/3.0;
	m_ArcBall.quat(roll, pitch, yaw, qti);

	Quaternion qtyaw(-30.0, Vector3d(0.0,0.0,1.0));
	m_ArcBall.setQuat(qti*qtyaw);

	memcpy(ab_new, m_ArcBall.ab_quat, 16*sizeof(float));

	startRotationTimer();
	emit(viewModified());
}


void gl3dView::on3DFlip()
{
	memcpy(ab_old, m_ArcBall.ab_quat, 16*sizeof(float));

	Quaternion qtflip(180.0, Vector3d(0.0,1.0,0.0));
	float ab_flip[16];
	memset(ab_flip, 0, 16*sizeof(float));
	m_ArcBall.quatToMatrix(ab_flip, qtflip);
	m_ArcBall.quatNext(ab_new, m_ArcBall.ab_quat, ab_flip);
	memcpy(m_ArcBall.ab_quat, ab_new, 16*sizeof(float));

	startRotationTimer();
	emit(viewModified());
}



void gl3dView::on3DTop()
{
	memcpy(ab_old, m_ArcBall.ab_quat, 16*sizeof(float));
	m_ArcBall.setQuat(sqrt(2.0)/2.0, 0.0, 0.0, -sqrt(2.0)/2.0);
	memcpy(ab_new, m_ArcBall.ab_quat, 16*sizeof(float));

	startRotationTimer();
	emit(viewModified());
}


void gl3dView::on3DLeft()
{
	memcpy(ab_old, m_ArcBall.ab_quat, 16*sizeof(float));
	m_ArcBall.setQuat(sqrt(2.0)/2.0, -sqrt(2.0)/2.0, 0.0, 0.0);// rotate by 90° around x
	memcpy(ab_new, m_ArcBall.ab_quat, 16*sizeof(float));

	startRotationTimer();
	emit(viewModified());

}


void gl3dView::on3DFront()
{
	memcpy(ab_old, m_ArcBall.ab_quat, 16*sizeof(float));
	Quaternion Qt1(sqrt(2.0)/2.0, 0.0,           -sqrt(2.0)/2.0, 0.0);// rotate by 90° around y
	Quaternion Qt2(sqrt(2.0)/2.0, -sqrt(2.0)/2.0, 0.0,           0.0);// rotate by 90° around x

	m_ArcBall.setQuat(Qt1 * Qt2);
	memcpy(ab_new, m_ArcBall.ab_quat, 16*sizeof(float));

	startRotationTimer();
	emit(viewModified());

}



void gl3dView::onSurfaces(bool bChecked)
{
	m_bSurfaces = bChecked;
	update();
}


void gl3dView::onOutline(bool bChecked)
{
	m_bOutline = bChecked;
	update();
}


void gl3dView::onPanels(bool bChecked)
{
	m_bVLMPanels = bChecked;
	update();
}

void gl3dView::onAxes(bool bChecked)
{
	m_bAxes = bChecked;
	update();
}


void gl3dView::onFoilNames(bool bChecked)
{
	m_bFoilNames = bChecked;
	update();
}


void gl3dView::onShowMasses(bool bChecked)
{
	m_bShowMasses = bChecked;
	update();
}


void gl3dView::mousePressEvent(QMouseEvent *event)
{
	QPoint point(event->pos().x(), event->pos().y());

	bool bCtrl = false;
	if(event->modifiers() & Qt::ControlModifier) bCtrl =true;

//	setFocus();

	if (event->buttons() & Qt::MidButton)
	{
		m_bArcball = true;
		Vector3d real;
		QPoint pt(event->pos().x(), event->pos().y());
		screenToViewport(pt, real);
		m_ArcBall.start(real.x, real.y);
		m_bCrossPoint = true;

		reset3DRotationCenter();
		update();
	}
	else if (event->buttons() & Qt::LeftButton)
	{
		Vector3d real;
		QPoint pt(point.x(), point.y());
		screenToViewport(pt, real);
		m_ArcBall.start(real.x, real.y);
		m_bCrossPoint = true;
		reset3DRotationCenter();
		if (!bCtrl)
		{
			m_bTrans = true;
			setCursor(Qt::ClosedHandCursor);
		}
		else
		{
			m_bTrans=false;
			m_bArcball = true;
		}
		update();
	}

	m_LastPoint = point;
}


QPoint gl3dView::worldToScreen(Vector3d v)
{
	QVector4D v4(v.x, v.y, v.z, 1.0);
	QVector4D vS = m_pvmMatrix * v4;
	return QPoint((int)((vS.x()+1.0)*width()/2), (int)((1.0-vS.y())*height()/2));
}


QPoint gl3dView::worldToScreen(QVector4D v4)
{
	QVector4D vS = m_pvmMatrix * v4;
	return QPoint((int)((vS.x()+1.0)*width()/2), (int)((1.0-vS.y())*height()/2));
}



/**
*Overrides the mouseDoubleClickEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void gl3dView::mouseDoubleClickEvent(QMouseEvent *event)
{
	set3DRotationCenter(event->pos());
}



void gl3dView::mouseMoveEvent(QMouseEvent *event)
{
	QPoint point(event->pos().x(), event->pos().y());
	Vector3d Real;

	QPoint Delta(point.x()-m_LastPoint.x(), point.y()-m_LastPoint.y());
	screenToViewport(point, Real);

//	if(!hasFocus()) setFocus();

	bool bCtrl = false;

	if (event->modifiers() & Qt::ControlModifier) bCtrl =true;
	if (event->buttons()   & Qt::LeftButton)
	{
		if(bCtrl)
		{
			//rotate
			m_ArcBall.move(Real.x, Real.y);
			update();
		}
		else if(m_bTrans)
		{
			//translate

			int side = qMax(geometry().width(), geometry().height());

			m_glViewportTrans.x += (GLfloat)(Delta.x()*2.0/m_glScaled/side);
			m_glViewportTrans.y += (GLfloat)(Delta.y()*2.0/m_glScaled/side);

			m_glRotCenter.x = MatOut[0][0]*(m_glViewportTrans.x) + MatOut[0][1]*(-m_glViewportTrans.y) + MatOut[0][2]*m_glViewportTrans.z;
			m_glRotCenter.y = MatOut[1][0]*(m_glViewportTrans.x) + MatOut[1][1]*(-m_glViewportTrans.y) + MatOut[1][2]*m_glViewportTrans.z;
			m_glRotCenter.z = MatOut[2][0]*(m_glViewportTrans.x) + MatOut[2][1]*(-m_glViewportTrans.y) + MatOut[2][2]*m_glViewportTrans.z;

			update();

		}
	}
	else if (event->buttons() & Qt::MidButton)
	{
		m_ArcBall.move(Real.x, Real.y);
		update();
	}
	else if(event->modifiers().testFlag(Qt::AltModifier))
	{
		double zoomFactor=1.0;

		if(point.y()-m_LastPoint.y()<0) zoomFactor = 1./1.025;
		else                            zoomFactor = 1.025;

		m_glScaled *= zoomFactor;
		update();
	}
	m_LastPoint = point;
}



/**
*Overrides the wheelEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void gl3dView::wheelEvent(QWheelEvent *event)
{
	double zoomFactor=1.0;
	if(event->delta()>0)
	{
		if(!Settings::s_bReverseZoom) zoomFactor = 1./1.06;
		else                          zoomFactor = 1.06;
	}
	else
	{
		if(!Settings::s_bReverseZoom) zoomFactor = 1.06;
		else                          zoomFactor = 1./1.06;
	}
	m_glScaled *= zoomFactor;
	update();
}


void gl3dView::mouseReleaseEvent(QMouseEvent * event )
{
	setCursor(Qt::CrossCursor);

	m_bTrans      = false;
	m_bDragPoint  = false;
	m_bArcball    = false;
	m_bCrossPoint = false;

//	We need to re-calculate the translation vector
	int i,j;
	for(i=0; i<4; i++)
		for(j=0; j<4; j++)
			MatIn[i][j] =  m_ArcBall.ab_quat[i*4+j];

	glInverseMatrix();
	m_glViewportTrans.x =  (MatOut[0][0]*m_glRotCenter.x + MatOut[0][1]*m_glRotCenter.y + MatOut[0][2]*m_glRotCenter.z);
	m_glViewportTrans.y = -(MatOut[1][0]*m_glRotCenter.x + MatOut[1][1]*m_glRotCenter.y + MatOut[1][2]*m_glRotCenter.z);
	m_glViewportTrans.z =  (MatOut[2][0]*m_glRotCenter.x + MatOut[2][1]*m_glRotCenter.y + MatOut[2][2]*m_glRotCenter.z);

	update();
	event->accept();
}




/**
*Overrides the keyPressEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void gl3dView::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{

		case Qt::Key_Control:
		{
			m_bArcball = true;
			update();
			break;
		}
		case Qt::Key_R:
		{
			on3DReset();
			event->accept();
			break;
		}
		case Qt::Key_X:
		{
			break;
		}
		default:
			event->ignore();
	}
}


/**
*Overrides the keyReleaseEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void gl3dView::keyReleaseEvent(QKeyEvent *event)
{
	switch (event->key())
	{
		case Qt::Key_Control:
		{
			m_bArcball = false;
			update();
			break;
		}

		default:
			event->ignore();
	}
}



void gl3dView::reset3DRotationCenter()
{
	//adjust the new rotation center after a translation or a rotation

	int i,j;

	for(i=0; i<4; i++)
		for(j=0; j<4; j++)
			MatOut[i][j] =  m_ArcBall.ab_quat[i*4+j];

	m_glRotCenter.x = MatOut[0][0]*(m_glViewportTrans.x) + MatOut[0][1]*(-m_glViewportTrans.y) + MatOut[0][2]*m_glViewportTrans.z;
	m_glRotCenter.y = MatOut[1][0]*(m_glViewportTrans.x) + MatOut[1][1]*(-m_glViewportTrans.y) + MatOut[1][2]*m_glViewportTrans.z;
	m_glRotCenter.z = MatOut[2][0]*(m_glViewportTrans.x) + MatOut[2][1]*(-m_glViewportTrans.y) + MatOut[2][2]*m_glViewportTrans.z;
}


void gl3dView::glInverseMatrix()
{
	//Step 1. Transpose the 3x3 rotation portion of the 4x4 matrix to get the inverse rotation
	int i,j;

	for(i=0 ; i<3; i++)
	{
		for(j=0; j<3; j++)
		{
			MatOut[j][i] = MatIn[i][j];
		}
	}
}


/**
*Converts screen coordinates to OpenGL Viewport coordinates.
*@param point the screen coordinates.
*@param real the viewport coordinates.
*/
void gl3dView::screenToViewport(QPoint const &point, Vector3d &real)
{
	double h2, w2;
	h2 = (double)geometry().height() /2.0;
	w2 = (double)geometry().width()  /2.0;

	if(w2>h2)
	{
		real.x =  ((double)point.x() - w2) / w2;
		real.y = -((double)point.y() - h2) / w2;
	}
	else
	{
		real.x =  ((double)point.x() - w2) / h2;
		real.y = -((double)point.y() - h2) / h2;
	}
}



/**
*Converts OpenGL Viewport coordinates to screen coordinates
*@param real the viewport coordinates.
*@param point the screen coordinates.
*/
void gl3dView::viewportToScreen(Vector3d const &real, QPoint &point)
{
	double dx, dy, h2, w2;

	h2 = m_GLViewRect.height() /2.0;
	w2 = m_GLViewRect.width()  /2.0;

	dx = ( real.x + w2)/2.0;
	dy = (-real.y + h2)/2.0;

	if(w2>h2)
	{
		point.setX((int)(dx * (double)geometry().width()));
		point.setY((int)(dy * (double)geometry().width()));
	}
	else
	{
		point.setX((int)(dx * (double)geometry().height()));
		point.setY((int)(dy * (double)geometry().height()));
	}
}


QVector4D gl3dView::worldToViewport(Vector3d v)
{
	QVector4D v4(v.x, v.y, v.z, 1.0);
	return m_pvmMatrix * v4;
}


void gl3dView::viewportToWorld(Vector3d vp, Vector3d &w)
{
	//un-translate
	vp.x += - m_glViewportTrans.x*m_glScaled;
	vp.y += + m_glViewportTrans.y*m_glScaled;

	//un-scale
	vp.x *= 1.0/m_glScaled;
	vp.y *= 1.0/m_glScaled;
	vp.z *= 1.0/m_glScaled;


	//un-rotate
	w.x = m_ArcBall.ab_quat[0]*vp.x + m_ArcBall.ab_quat[1]*vp.y + m_ArcBall.ab_quat[2]*vp.z;
	w.y = m_ArcBall.ab_quat[4]*vp.x + m_ArcBall.ab_quat[5]*vp.y + m_ArcBall.ab_quat[6]*vp.z;
	w.z = m_ArcBall.ab_quat[8]*vp.x + m_ArcBall.ab_quat[9]*vp.y + m_ArcBall.ab_quat[10]*vp.z;
}



void gl3dView::glRenderText(double x, double y, double z, const QString & str, QColor textColor)
{
	QPoint point;

	if(z>m_ClipPlanePos) return;

	point = worldToScreen(Vector3d(x,y,z));
	point *= devicePixelRatio();
	if(!m_PixTextOverlay.isNull())
	{
		QPainter paint(&m_PixTextOverlay);
		paint.save();
		QPen textPen(textColor);
		paint.setPen(textPen);
		QFont font(paint.font());
		font.setPointSize(paint.font().pointSize()*devicePixelRatio());
		paint.setFont(font);
		paint.drawText(point, str);
		paint.restore();
	}
}




void gl3dView::glRenderText(int x, int y, const QString & str, QColor textColor)
{
	QPainter paint(&m_PixTextOverlay);
	paint.save();
	QPen textPen(textColor);
	paint.setPen(textPen);
	QFont font(paint.font());
	font.setPointSize(paint.font().pointSize()*devicePixelRatio());
	paint.setFont(font);
	paint.drawText(x*devicePixelRatio(),y*devicePixelRatio(), str);
	paint.restore();
}


/**
*Overrides the resizeGL method of the base class.
* Sets the GL viewport to fit in the client area.
* Sets the scaling factors for the objects to be drawn in the viewport.
*@param width the width in pixels of the client area
*@param height the height in pixels of the client area
*/
void gl3dView::resizeGL(int width, int height)
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
}



void gl3dView::getGLError()
{
	switch(glGetError())
	{
		case GL_NO_ERROR:
			Trace("No error has been recorded. The value of this symbolic constant is guaranteed to be 0.");
			break;

		case GL_INVALID_ENUM:
			Trace("An unacceptable value is specified for an enumerated argument. "
					  "The offending command is ignored and has no other side effect than to set the error flag.");
			break;

		case GL_INVALID_VALUE:
			Trace("A numeric argument is out of range. The offending command is ignored and has no other "
					  "side effect than to set the error flag.");
			break;

		case GL_INVALID_OPERATION:
			Trace("The specified operation is not allowed in the current state. The offending command is "
					  "ignored and has no other side effect than to set the error flag.");
			break;

		case GL_INVALID_FRAMEBUFFER_OPERATION:
			Trace("The command is trying to render to or read from the framebuffer while the currently "
					  "bound framebuffer is not framebuffer complete (i.e. the return value from glCheckFramebufferStatus "
					  "is not GL_FRAMEBUFFER_COMPLETE). The offending command is ignored and has no other side effect than "
					  "to set the error flag.");
			break;

		case GL_OUT_OF_MEMORY:
			Trace("There is not enough memory left to execute the command. The state of the GL is "
					  "undefined, except for the state of the error flags, after this error is recorded.");
			break;

		case GL_STACK_UNDERFLOW:
			Trace("An attempt has been made to perform an operation that would cause an internal stack to underflow.");
			break;

		case GL_STACK_OVERFLOW:
			Trace("An attempt has been made to perform an operation that would cause an internal stack to overflow.");
			break;
	}
}


static GLfloat const x_axis[] = {
	-1.0f, 0.0f, 0.0f,
	 1.0f, 0.0f, 0.0f,
	 1.0f,   0.0f,   0.0f,
	 0.95f,  0.015f, 0.015f,
	 1.0f,  0.0f,    0.0f,
	 0.95f,-0.015f,-0.015f
};

static GLfloat const y_axis[] = {
	  0.0f,    -1.0f,    0.0f,
	  0.0f,     1.0f,    0.0f,
	  0.f,      1.0f,    0.0f,
	  0.015f,   0.95f,   0.015f,
	  0.f,      1.0f,    0.0f,
	 -0.015f,   0.95f,  -0.015f
};

static GLfloat const z_axis[] = {
	 0.0f,    0.0f,   -1.0f,
	 0.0f,    0.0f,    1.0f,
	 0.0f,    0.0f,    1.0f,
	 0.015f,  0.015f,  0.95f,
	 0.0f,    0.0f,    1.0f,
	-0.015f, -0.015f,  0.95f
};



#define NUMANGLES     10
#define NUMCIRCLES     6
#define NUMPERIM      35
#define NUMARCPOINTS  10




/**
*Creates the OpenGL List for the ArcBall.
*@param ArcBall the ArcBall object associated to the view
*@param GLScale the overall scaling factor for the view
*/
void gl3dView::glMakeArcPoint()
{
	float GLScale = 1.0f;
	int row, col;
	double Radius=0.1, lat_incr, phi, theta;
	Vector3d eye(0.0, 0.0, 1.0);
	Vector3d up(0.0, 1.0, 0.0);
	m_ArcBall.setZoom(0.45, eye, up);

	Radius = m_ArcBall.ab_sphere;

	int iv=0;

	int bufferSize = NUMARCPOINTS*2*2*3;
	float *arcPointVertexArray = new float[bufferSize];

	//ARCPOINT
	lat_incr = 30.0 / NUMARCPOINTS;

	phi = 0.0* PI/180.0;//longitude
	for (row = -NUMARCPOINTS; row < NUMARCPOINTS; row++)
	{
		theta = (0.0+ row * lat_incr) * PI/180.0;
		arcPointVertexArray[iv++] = Radius*cos(phi)*cos(theta)*GLScale;
		arcPointVertexArray[iv++] = Radius*sin(theta)*GLScale;
		arcPointVertexArray[iv++] = Radius*sin(phi)*cos(theta)*GLScale;
	}

	theta = 0.0* PI/180.0;
	for(col=-NUMARCPOINTS; col<NUMARCPOINTS; col++)
	{
		phi = (0.0 + (double)col*30.0/NUMARCPOINTS) * PI/180.0;
		arcPointVertexArray[iv++] = Radius * cos(phi) * cos(theta)*GLScale;
		arcPointVertexArray[iv++] = Radius * sin(theta)*GLScale;
		arcPointVertexArray[iv++] = Radius * sin(phi) * cos(theta)*GLScale;
	}

	Q_ASSERT(iv==bufferSize);

	m_vboArcPoint.destroy();
	m_vboArcPoint.create();
	m_vboArcPoint.bind();
	m_vboArcPoint.allocate(arcPointVertexArray, bufferSize * sizeof(GLfloat));
	m_vboArcPoint.release();

	delete [] arcPointVertexArray;
}


void gl3dView::glMakeBody3DFlatPanels(Body *pBody)
{
	Vector3d P1, P2, P3, P4, N, P1P3, P2P4, Tj, Tjp1;

	if(m_pLeftBodyTexture)  delete m_pLeftBodyTexture;
	if(m_pRightBodyTexture) delete m_pRightBodyTexture;

	QString projectPath = Settings::s_LastDirName + QDir::separator() + MainFrame::s_ProjectName+ "_textures";
	QString planeName;
	Miarex *pMiarex = (Miarex*)s_pMiarex;
	if(pMiarex && pMiarex->m_pCurPlane)
	{
		planeName = pMiarex->m_pCurPlane->planeName();
	}
	QString texturePath = projectPath+QDir::separator()+planeName+QDir::separator();

	QImage leftTexture  = QImage(QString(texturePath+"body_left.png"));
	if(leftTexture.isNull()) leftTexture = QImage(QString(":/default_textures/body_left.png"));
	m_pLeftBodyTexture  = new QOpenGLTexture(leftTexture);
	QImage rightTexture  = QImage(QString(texturePath+"body_right.png"));
	if(rightTexture.isNull()) rightTexture = QImage(QString(":/default_textures/body_right.png"));
	m_pRightBodyTexture  = new QOpenGLTexture(rightTexture);


	int bufferSize = (pBody->sideLineCount()-1) * (pBody->frameCount()-1); //quads
	bufferSize *= 2;  // two sides
	bufferSize *= 4;  // four vertices per quad
	bufferSize *= 8;  // 8 components per vertex
	float *pBodyVertexArray = new float[bufferSize];

	//Create triangles
	//  indices array size:
	//    NX*NH
	//    2 triangles per/quad
	//    3 indices/triangle
	//    2 sides
	m_iBodyElems = (pBody->sideLineCount()-1) * (pBody->frameCount()-1); //quads
	m_iBodyElems *= 2;    //two sides
	m_iBodyElems *= 2;    //two triangles per quad
	m_iBodyElems *= 3;    //three vertex per triangle

	if(m_BodyIndicesArray) delete[] m_BodyIndicesArray;
	m_BodyIndicesArray = new unsigned short[m_iBodyElems];

	int iv=0;
	unsigned int ii=0;

	float fnh = pBody->sideLineCount();
	float fLength = pBody->length();

	float tip = 0.0;
	if(pBody->frameCount()) tip = pBody->frame(0)->m_Position.x;

	//surfaces
	for (int k=0; k<pBody->sideLineCount()-1;k++)
	{
		for (int j=0; j<pBody->frameCount()-1;j++)
		{
			Tj.set(pBody->frame(j)->m_Position.x,     0.0, 0.0);
			Tjp1.set(pBody->frame(j+1)->m_Position.x, 0.0, 0.0);

			P1 = pBody->frame(j)->m_CtrlPoint[k];       P1.x = pBody->frame(j)->m_Position.x;
			P2 = pBody->frame(j+1)->m_CtrlPoint[k];     P2.x = pBody->frame(j+1)->m_Position.x;
			P3 = pBody->frame(j+1)->m_CtrlPoint[k+1];   P3.x = pBody->frame(j+1)->m_Position.x;
			P4 = pBody->frame(j)->m_CtrlPoint[k+1];     P4.x = pBody->frame(j)->m_Position.x;

			P1P3 = P3-P1;
			P2P4 = P4-P2;
			N = P1P3 * P2P4;
			N.normalize();

			int i1 = iv/8;
			int i2 = i1+1;
			int i3 = i2+1;
			int i4 = i3+1;

			pBodyVertexArray[iv++] = P1.x;
			pBodyVertexArray[iv++] = P1.y;
			pBodyVertexArray[iv++] = P1.z;
			pBodyVertexArray[iv++] = N.x;
			pBodyVertexArray[iv++] = N.y;
			pBodyVertexArray[iv++] = N.z;
			pBodyVertexArray[iv++] = 1.0-(P1.x-tip)/fLength;
			pBodyVertexArray[iv++] = (float)k/fnh;
			pBodyVertexArray[iv++] = P2.x;
			pBodyVertexArray[iv++] = P2.y;
			pBodyVertexArray[iv++] = P2.z;
			pBodyVertexArray[iv++] = N.x;
			pBodyVertexArray[iv++] = N.y;
			pBodyVertexArray[iv++] = N.z;
			pBodyVertexArray[iv++] = 1.0-(P2.x-tip)/fLength;
			pBodyVertexArray[iv++] = (float)k/fnh;
			pBodyVertexArray[iv++] = P3.x;
			pBodyVertexArray[iv++] = P3.y;
			pBodyVertexArray[iv++] = P3.z;
			pBodyVertexArray[iv++] = N.x;
			pBodyVertexArray[iv++] = N.y;
			pBodyVertexArray[iv++] = N.z;
			pBodyVertexArray[iv++] = 1.0-(P3.x-tip)/fLength;
			pBodyVertexArray[iv++] = (float)(k+1)/fnh;
			pBodyVertexArray[iv++] = P4.x;
			pBodyVertexArray[iv++] = P4.y;
			pBodyVertexArray[iv++] = P4.z;
			pBodyVertexArray[iv++] = N.x;
			pBodyVertexArray[iv++] = N.y;
			pBodyVertexArray[iv++] = N.z;
			pBodyVertexArray[iv++] = 1.0-(P4.x-tip)/fLength;
			pBodyVertexArray[iv++] = (float)(k+1)/fnh;

			//first triangle
			m_BodyIndicesArray[ii]   = i1;
			m_BodyIndicesArray[ii+1] = i2;
			m_BodyIndicesArray[ii+2] = i3;

			//second triangle
			m_BodyIndicesArray[ii+3] = i3;
			m_BodyIndicesArray[ii+4] = i4;
			m_BodyIndicesArray[ii+5] = i1;
			ii += 6;
		}
	}
	for (int k=0; k<pBody->sideLineCount()-1;k++)
	{
		for (int j=0; j<pBody->frameCount()-1;j++)
		{
			Tj.set(pBody->frame(j)->m_Position.x,     0.0, 0.0);
			Tjp1.set(pBody->frame(j+1)->m_Position.x, 0.0, 0.0);

			P1 = pBody->frame(j)->m_CtrlPoint[k];       P1.x = pBody->frame(j)->m_Position.x;
			P2 = pBody->frame(j+1)->m_CtrlPoint[k];     P2.x = pBody->frame(j+1)->m_Position.x;
			P3 = pBody->frame(j+1)->m_CtrlPoint[k+1];   P3.x = pBody->frame(j+1)->m_Position.x;
			P4 = pBody->frame(j)->m_CtrlPoint[k+1];     P4.x = pBody->frame(j)->m_Position.x;

			P1P3 = P3-P1;
			P2P4 = P4-P2;
			N = P1P3 * P2P4;
			N.normalize();

			P1.y = -P1.y;
			P2.y = -P2.y;
			P3.y = -P3.y;
			P4.y = -P4.y;
			N.y = -N.y;

			int i1 = iv/8;
			int i2 = i1+1;
			int i3 = i2+1;
			int i4 = i3+1;

			pBodyVertexArray[iv++] = P1.x;
			pBodyVertexArray[iv++] = P1.y;
			pBodyVertexArray[iv++] = P1.z;
			pBodyVertexArray[iv++] = N.x;
			pBodyVertexArray[iv++] = N.y;
			pBodyVertexArray[iv++] = N.z;
			pBodyVertexArray[iv++] = (P1.x-tip)/fLength;
			pBodyVertexArray[iv++] = (float)k/fnh;
			pBodyVertexArray[iv++] = P2.x;
			pBodyVertexArray[iv++] = P2.y;
			pBodyVertexArray[iv++] = P2.z;
			pBodyVertexArray[iv++] = N.x;
			pBodyVertexArray[iv++] = N.y;
			pBodyVertexArray[iv++] = N.z;
			pBodyVertexArray[iv++] = (P2.x-tip)/fLength;
			pBodyVertexArray[iv++] = (float)k/fnh;
			pBodyVertexArray[iv++] = P3.x;
			pBodyVertexArray[iv++] = P3.y;
			pBodyVertexArray[iv++] = P3.z;
			pBodyVertexArray[iv++] = N.x;
			pBodyVertexArray[iv++] = N.y;
			pBodyVertexArray[iv++] = N.z;
			pBodyVertexArray[iv++] = (P3.x-tip)/fLength;
			pBodyVertexArray[iv++] = (float)(k+1)/fnh;
			pBodyVertexArray[iv++] = P4.x;
			pBodyVertexArray[iv++] = P4.y;
			pBodyVertexArray[iv++] = P4.z;
			pBodyVertexArray[iv++] = N.x;
			pBodyVertexArray[iv++] = N.y;
			pBodyVertexArray[iv++] = N.z;
			pBodyVertexArray[iv++] = (P4.x-tip)/fLength;
			pBodyVertexArray[iv++] = (float)(k+1)/fnh;

			//first triangle
			m_BodyIndicesArray[ii]   = i1;
			m_BodyIndicesArray[ii+1] = i2;
			m_BodyIndicesArray[ii+2] = i3;

			//second triangle
			m_BodyIndicesArray[ii+3] = i3;
			m_BodyIndicesArray[ii+4] = i4;
			m_BodyIndicesArray[ii+5] = i1;
			ii += 6;
		}
	}
	Q_ASSERT(iv==bufferSize);
	Q_ASSERT(ii==m_iBodyElems);

	m_vboBody.destroy();
	m_vboBody.create();
	m_vboBody.bind();
	m_vboBody.allocate(pBodyVertexArray, bufferSize * sizeof(GLfloat));
	m_vboBody.release();

	delete [] pBodyVertexArray;
}



void gl3dView::glMakeBodySplines(Body *pBody)
{
	int NXXXX = W3dPrefsDlg::bodyAxialRes();
	int NHOOOP = W3dPrefsDlg::bodyHoopRes();
	Vector3d *m_T = new Vector3d[(NXXXX+1)*(NHOOOP+1)]; //temporary points to save calculation times for body NURBS surfaces
	Vector3d TALB, LATB;
	int j, k, l, p;
	double v;

	if(!pBody)
	{
		delete [] m_T;
		return;
	}

	Vector3d Point;
	double hinc, u;
	Vector3d N;

	if(m_pLeftBodyTexture)  delete m_pLeftBodyTexture;
	if(m_pRightBodyTexture) delete m_pRightBodyTexture;

	QString projectPath = Settings::s_LastDirName + QDir::separator() + MainFrame::s_ProjectName+ "_textures";
	QString planeName;
	Miarex *pMiarex = (Miarex*)s_pMiarex;
	if(pMiarex && pMiarex->m_pCurPlane)
	{
		planeName = pMiarex->m_pCurPlane->planeName();
	}
	QString texturePath = projectPath+QDir::separator()+planeName+QDir::separator();

	QImage leftTexture  = QImage(QString(texturePath+"body_left.png"));
	if(leftTexture.isNull()) leftTexture = QImage(QString(":/default_textures/body_left.png"));
	m_pLeftBodyTexture  = new QOpenGLTexture(leftTexture);
	QImage rightTexture  = QImage(QString(texturePath+"body_right.png"));
	if(rightTexture.isNull()) rightTexture = QImage(QString(":/default_textures/body_right.png"));
	m_pRightBodyTexture  = new QOpenGLTexture(rightTexture);

	//vertices array size:
	// surface:
	//     (NX+1)*(NH+1) : from 0 to NX, and from 0 to NH
	//     x2 : 2 sides
	// outline:
	//     frameSize()*(NH+1)*2 : frames
	//     (NX+1) + (NX+1)      : top and bottom lines
	//
	// x8 : 3 vertices components, 3 normal components, 2 texture componenents
	int bodyVertexSize;
	bodyVertexSize  =   (NXXXX+1)*(NHOOOP+1) *2             // side surfaces
					  + pBody->frameCount()*(NHOOOP+1)*2 // frames
					  + (NXXXX+1)                       // top outline
					  + (NXXXX+1);                      // bot outline

	bodyVertexSize *= 8; // 3 vertex components, 3 normal components, 2 uv components

	float *pBodyVertexArray = new float[bodyVertexSize];

	p = 0;
	for (k=0; k<=NXXXX; k++)
	{
		u = (double)k / (double)NXXXX;
		for (l=0; l<=NHOOOP; l++)
		{
			v = (double)l / (double)NHOOOP;
			pBody->getPoint(u,  v, true, m_T[p]);
			p++;
		}
	}

	int iv=0; //index of vertex components

	//right side first;
	p=0;
	for (k=0; k<=NXXXX; k++)
	{
		for (l=0; l<=NHOOOP; l++)
		{
			pBodyVertexArray[iv++] = m_T[p].x;
			pBodyVertexArray[iv++] = m_T[p].y;
			pBodyVertexArray[iv++] = m_T[p].z;

			if(k==0)       N.set(-1.0, 0.0, 0.0);
			else if(k==NXXXX) N.set(1.0, 0.0, 0.0);
			else if(l==0)				N.set(0.0, 0.0, 1.0);
			else if(l==NHOOOP)				N.set(0.0,0.0, -1.0);
			else
			{
				LATB = m_T[p+NHOOOP+1] - m_T[p+1];     //	LATB = TB - LA;
				TALB = m_T[p]  - m_T[p+NHOOOP+2];      //	TALB = LB - TA;
				N = TALB * LATB;
				N.normalize();
			}

			pBodyVertexArray[iv++] =  N.x;
			pBodyVertexArray[iv++] =  N.y;
			pBodyVertexArray[iv++] =  N.z;

			pBodyVertexArray[iv++] = (float)(NXXXX-k)/(float)NXXXX;
			pBodyVertexArray[iv++] = (float)l/(float)NHOOOP;
			p++;
		}
	}


	//left side next;
	p=0;
	for (k=0; k<=NXXXX; k++)
	{
		for (l=0; l<=NHOOOP; l++)
		{
			pBodyVertexArray[iv++] =  m_T[p].x;
			pBodyVertexArray[iv++] = -m_T[p].y;
			pBodyVertexArray[iv++] =  m_T[p].z;

			if(k==0) N.set(-1.0, 0.0, 0.0);
			else if(k==NXXXX) N.set(1.0, 0.0, 0.0);
			else if(l==0)  N.set(0.0, 0.0, 1.0);
			else if(l==NHOOOP) N.set(0.0,0.0, -1.0);
			else
			{
				LATB = m_T[p+NHOOOP+1] - m_T[p+1];     //	LATB = TB - LA;
				TALB = m_T[p]  - m_T[p+NHOOOP+2];      //	TALB = LB - TA;
				N = TALB * LATB;
				N.normalize();
			}
			pBodyVertexArray[iv++] =  N.x;
			pBodyVertexArray[iv++] = -N.y;
			pBodyVertexArray[iv++] =  N.z;

			pBodyVertexArray[iv++] = (float)k/(float)NXXXX;
			pBodyVertexArray[iv++] = (float)l/(float)NHOOOP;
			p++;
		}
	}

	//OUTLINE
	hinc=1./(double)NHOOOP;
	u=0.0; v = 0.0;

	// frames : frameCount() x (NH+1)
	for (int iFr=0; iFr<pBody->frameCount(); iFr++)
	{
		u = pBody->getu(pBody->frame(iFr)->m_Position.x);
		for (j=0; j<=NHOOOP; j++)
		{
			v = (double)j*hinc;
			pBody->getPoint(u,v,true, Point);
			pBodyVertexArray[iv++] = Point.x;
			pBodyVertexArray[iv++] = Point.y;
			pBodyVertexArray[iv++] = Point.z;

			N = Vector3d(0.0, Point.y, Point.z);
			N.normalize();
			pBodyVertexArray[iv++] =  N.x;
			pBodyVertexArray[iv++] =  N.y;
			pBodyVertexArray[iv++] =  N.z;

			pBodyVertexArray[iv++] = u;
			pBodyVertexArray[iv++] = v;
		}

		for (j=NHOOOP; j>=0; j--)
		{
			v = (double)j*hinc;
			pBody->getPoint(u,v,false, Point);
			pBodyVertexArray[iv++] = Point.x;
			pBodyVertexArray[iv++] = Point.y;
			pBodyVertexArray[iv++] = Point.z;
			N = Vector3d(0.0, Point.y, Point.z);
			N.normalize();
			pBodyVertexArray[iv++] =  N.x;
			pBodyVertexArray[iv++] =  N.y;
			pBodyVertexArray[iv++] =  N.z;

			pBodyVertexArray[iv++] = u;
			pBodyVertexArray[iv++] = v;
		}
	}

	//top line: NX+1
	v = 0.0;
	for (int iu=0; iu<=NXXXX; iu++)
	{
		pBody->getPoint((double)iu/(double)NXXXX,v, true, Point);
		pBodyVertexArray[iv++] = Point.x;
		pBodyVertexArray[iv++] = Point.y;
		pBodyVertexArray[iv++] = Point.z;

		pBodyVertexArray[iv++] = N.x;
		pBodyVertexArray[iv++] = N.y;
		pBodyVertexArray[iv++] = N.z;

		pBodyVertexArray[iv++] = (float)iu/(float)NXXXX;
		pBodyVertexArray[iv++] = v;
	}

	//bottom line: NX+1
	v = 1.0;
	for (int iu=0; iu<=NXXXX; iu++)
	{
		pBody->getPoint((double)iu/(double)NXXXX,v, true, Point);
		pBodyVertexArray[iv++] = Point.x;
		pBodyVertexArray[iv++] = Point.y;
		pBodyVertexArray[iv++] = Point.z;
		pBodyVertexArray[iv++] = N.x;
		pBodyVertexArray[iv++] = N.y;
		pBodyVertexArray[iv++] = N.z;

		pBodyVertexArray[iv++] = (float)iu/(float)NXXXX;
		pBodyVertexArray[iv++] = v;
	}
	Q_ASSERT(iv==bodyVertexSize);


	//Create triangles
	//  indices array size:
	//    NX*NH
	//    2 triangles per/quad
	//    3 indices/triangle
	//    2 sides
	if(m_BodyIndicesArray) delete[] m_BodyIndicesArray;
	m_BodyIndicesArray = new unsigned short[NXXXX*NHOOOP*2*3*2];

	int ii=0;
	int nV;

	//left side;
	for (k=0; k<NXXXX; k++)
	{
		for (l=0; l<NHOOOP; l++)
		{
			nV = k*(NHOOOP+1)+l; // id of the vertex at the bottom left of the quad
			//first triangle
			m_BodyIndicesArray[ii]   = nV;
			m_BodyIndicesArray[ii+1] = nV+NHOOOP+1;
			m_BodyIndicesArray[ii+2] = nV+1;

			//second triangle
			m_BodyIndicesArray[ii+3] = nV+NHOOOP+1;
			m_BodyIndicesArray[ii+4] = nV+1;
			m_BodyIndicesArray[ii+5] = nV+NHOOOP+1+1;
			ii += 6;
		}
	}

	//right side
	for (k=0; k<NXXXX; k++)
	{
		for (l=0; l<NHOOOP; l++)
		{
			nV = (NXXXX+1)*(NHOOOP+1) + k*(NHOOOP+1)+l; // id of the vertex at the bottom left of the quad
			//first triangle
			m_BodyIndicesArray[ii]   = nV;
			m_BodyIndicesArray[ii+1] = nV+NHOOOP+1;
			m_BodyIndicesArray[ii+2] = nV+1;

			//second triangle
			m_BodyIndicesArray[ii+3] = nV+NHOOOP+1;
			m_BodyIndicesArray[ii+4] = nV+1;
			m_BodyIndicesArray[ii+5] = nV+NHOOOP+1+1;
			ii += 6;
		}
	}
	m_iBodyElems = ii;

	pBody = NULL;

	m_vboBody.destroy();
	m_vboBody.create();
	m_vboBody.bind();
	m_vboBody.allocate(pBodyVertexArray, bodyVertexSize * sizeof(GLfloat));
	m_vboBody.release();

	delete [] pBodyVertexArray;
	delete [] m_T;
}


void gl3dView::initializeGL()
{
	QSurfaceFormat ctxtFormat = format();
	m_bUse120StyleShaders = (ctxtFormat.majorVersion()*10+ctxtFormat.minorVersion())<33;

	Trace("");
	Trace("****************gl3dView********************");
	Trace("Initializing GL");

	printFormat(format());

	QString vendor, renderer, version, glslVersion;
	const GLubyte *p;
	if ((p = glGetString(GL_VENDOR)))
		vendor = QString::fromLatin1(reinterpret_cast<const char *>(p));
	if ((p = glGetString(GL_RENDERER)))
		renderer = QString::fromLatin1(reinterpret_cast<const char *>(p));
	if ((p = glGetString(GL_VERSION)))
		version = QString::fromLatin1(reinterpret_cast<const char *>(p));
	if ((p = glGetString(GL_SHADING_LANGUAGE_VERSION)))
		glslVersion = QString::fromLatin1(reinterpret_cast<const char *>(p));

	Trace("   *** Context information ***");
	Trace(QString("   Vendor: %1").arg(vendor));
	Trace(QString("   Renderer: %1").arg(renderer));
	Trace(QString("   OpenGL version: %1").arg(version));
	Trace(QString("   GLSL version: %1").arg(glslVersion));
	if(m_bUse120StyleShaders) Trace("Using glsl v120 style shaders");
	else                      Trace("Using glsl v330 style shaders");
	Trace("/****************gl3dView********************");


	glMakeUnitSphere();
	glMakeArcBall();
	glMakeArcPoint();

	//setup the shader to paint lines
	QString vsrc = m_bUse120StyleShaders ? ":/shaders/line_vertexshader_120.glsl" : ":/shaders/line_vertexshader.glsl";
	QString fsrc = m_bUse120StyleShaders ? ":/shaders/line_fragmentshader_120.glsl" : ":/shaders/line_fragmentshader.glsl";

	m_ShaderProgramLine.addShaderFromSourceFile(QOpenGLShader::Vertex, (vsrc));
	if(m_ShaderProgramLine.log().length()) Trace("Line vertex shader log:"+m_ShaderProgramLine.log());
	m_ShaderProgramLine.addShaderFromSourceFile(QOpenGLShader::Fragment, (fsrc));
	if(m_ShaderProgramLine.log().length()) Trace("Line fragment shader log:"+m_ShaderProgramLine.log());
	m_ShaderProgramLine.link();
	m_ShaderProgramLine.bind();
	m_VertexLocationLine       = m_ShaderProgramLine.attributeLocation("vertex");
	m_mMatrixLocationLine      = m_ShaderProgramLine.uniformLocation("mMatrix");
	m_vMatrixLocationLine      = m_ShaderProgramLine.uniformLocation("vMatrix");
	m_pvmMatrixLocationLine    = m_ShaderProgramLine.uniformLocation("pvmMatrix");
	m_ColorLocationLine        = m_ShaderProgramLine.uniformLocation("color");
	m_ClipPlaneLocationLine    = m_ShaderProgramLine.uniformLocation("clipPlane0");
	m_ShaderProgramLine.release();


	//setup the shader to paint the Cp and other gradients
	vsrc = m_bUse120StyleShaders ? ":/shaders/gradient_vertexshader_120.glsl" : ":/shaders/gradient_vertexshader.glsl";
	fsrc = m_bUse120StyleShaders ? ":/shaders/gradient_fragmentshader_120.glsl" : ":/shaders/gradient_fragmentshader.glsl";
	m_ShaderProgramGradient.addShaderFromSourceFile(QOpenGLShader::Vertex, vsrc);
	if(m_ShaderProgramGradient.log().length()) Trace("Gradient vertex shader log:"+m_ShaderProgramGradient.log());

	m_ShaderProgramGradient.addShaderFromSourceFile(QOpenGLShader::Fragment, fsrc);
	if(m_ShaderProgramGradient.log().length()) Trace("Gradient fragment shader log:"+m_ShaderProgramGradient.log());

	m_ShaderProgramGradient.link();
	m_ShaderProgramGradient.bind();
	m_VertexLocationGradient = m_ShaderProgramGradient.attributeLocation("vertexPosition_modelSpace");
	m_pvmMatrixLocationGradient = m_ShaderProgramGradient.uniformLocation("pvmMatrix");
	m_ColorLocationGradient  = m_ShaderProgramGradient.attributeLocation("vertexColor");
	m_ShaderProgramGradient.release();


	//setup the shader to paint colored surfaces
	vsrc = m_bUse120StyleShaders ? ":/shaders/surface_vertexshader_120.glsl" : ":/shaders/surface_vertexshader.glsl";
	fsrc = m_bUse120StyleShaders ? ":/shaders/surface_fragmentshader_120.glsl" : ":/shaders/surface_fragmentshader.glsl";
	m_ShaderProgramSurface.addShaderFromSourceFile(QOpenGLShader::Vertex, vsrc);
	if(m_ShaderProgramSurface.log().length()) Trace("Surface vertex shader log:"+m_ShaderProgramSurface.log());

	m_ShaderProgramSurface.addShaderFromSourceFile(QOpenGLShader::Fragment, fsrc);
	if(m_ShaderProgramSurface.log().length()) Trace("Surface fragment shader log:"+m_ShaderProgramSurface.log());

	m_ShaderProgramSurface.link();
	m_ShaderProgramSurface.bind();
	m_VertexLocationSurface = m_ShaderProgramSurface.attributeLocation("vertexPosition_modelSpace");
	m_NormalLocationSurface = m_ShaderProgramSurface.attributeLocation("vertexNormal_modelSpace");
	m_ClipPlaneLocationSurface     = m_ShaderProgramSurface.uniformLocation("clipPlane0");
	m_pvmMatrixLocationSurface     = m_ShaderProgramSurface.uniformLocation("pvmMatrix");
	m_vMatrixLocationSurface       = m_ShaderProgramSurface.uniformLocation("vMatrix");
	m_mMatrixLocationSurface       = m_ShaderProgramSurface.uniformLocation("mMatrix");
	m_EyePosLocationSurface        = m_ShaderProgramSurface.uniformLocation("EyePosition_viewSpace");
	m_LightPosLocationSurface      = m_ShaderProgramSurface.uniformLocation("LightPosition_viewSpace");
	m_LightColorLocationSurface    = m_ShaderProgramSurface.uniformLocation("LightColor");
	m_LightAmbientLocationSurface  = m_ShaderProgramSurface.uniformLocation("LightAmbient");
	m_LightDiffuseLocationSurface  = m_ShaderProgramSurface.uniformLocation("LightDiffuse");
	m_LightSpecularLocationSurface = m_ShaderProgramSurface.uniformLocation("LightSpecular");
	m_ColorLocationSurface         = m_ShaderProgramSurface.uniformLocation("incolor");
	m_LightLocationSurface         = m_ShaderProgramSurface.uniformLocation("lightOn");
	m_SurfaceLocationSurface       = m_ShaderProgramSurface.uniformLocation("hasSurface");
	m_MaterialShininessSurface     = m_ShaderProgramSurface.uniformLocation("MaterialShininess");
	m_AttenuationConstantSurface   = m_ShaderProgramSurface.uniformLocation("Kc");
	m_AttenuationLinearSurface     = m_ShaderProgramSurface.uniformLocation("Kl");
	m_AttenuationQuadraticSurface  = m_ShaderProgramSurface.uniformLocation("Kq");
	m_ShaderProgramSurface.release();

	//setup the shader to paint textured surfaces
	vsrc = m_bUse120StyleShaders ? ":/shaders/texture_vertexshader_120.glsl" : ":/shaders/texture_vertexshader.glsl";
	fsrc = m_bUse120StyleShaders ? ":/shaders/texture_fragmentshader_120.glsl" : ":/shaders/texture_fragmentshader.glsl";
	m_ShaderProgramTexture.addShaderFromSourceFile(QOpenGLShader::Vertex, vsrc);
	if(m_ShaderProgramTexture.log().length()) Trace("Texture vertex shader log:"+m_ShaderProgramTexture.log());

	m_ShaderProgramTexture.addShaderFromSourceFile(QOpenGLShader::Fragment, fsrc);
	if(m_ShaderProgramTexture.log().length()) Trace("Texture fragment shader log:"+m_ShaderProgramTexture.log());

	m_ShaderProgramTexture.link();
	m_ShaderProgramTexture.bind();
	m_VertexLocationTexture = m_ShaderProgramTexture.attributeLocation("vertexPosition_modelSpace");
	m_NormalLocationTexture = m_ShaderProgramTexture.attributeLocation("vertexNormal_modelSpace");
	m_UVLocationTexture     = m_ShaderProgramTexture.attributeLocation("vertexUV");
	m_ClipPlaneLocationTexture     = m_ShaderProgramTexture.uniformLocation("clipPlane0");
	m_pvmMatrixLocationTexture     = m_ShaderProgramTexture.uniformLocation("pvmMatrix");
	m_vMatrixLocationTexture       = m_ShaderProgramTexture.uniformLocation("vMatrix");
	m_mMatrixLocationTexture       = m_ShaderProgramTexture.uniformLocation("mMatrix");
	m_EyePosLocationTexture        = m_ShaderProgramTexture.uniformLocation("EyePosition_viewSpace");
	m_LightPosLocationTexture      = m_ShaderProgramTexture.uniformLocation("LightPosition_viewSpace");
	m_LightColorLocationTexture    = m_ShaderProgramTexture.uniformLocation("LightColor");
	m_LightAmbientLocationTexture  = m_ShaderProgramTexture.uniformLocation("LightAmbient");
	m_LightDiffuseLocationTexture  = m_ShaderProgramTexture.uniformLocation("LightDiffuse");
	m_LightSpecularLocationTexture = m_ShaderProgramTexture.uniformLocation("LightSpecular");
	m_LightLocationTexture         = m_ShaderProgramTexture.uniformLocation("lightOn");
	m_MaterialShininessTexture     = m_ShaderProgramTexture.uniformLocation("MaterialShininess");
	m_AttenuationConstantTexture   = m_ShaderProgramTexture.uniformLocation("Kc");
	m_AttenuationLinearTexture     = m_ShaderProgramTexture.uniformLocation("Kl");
	m_AttenuationQuadraticTexture  = m_ShaderProgramTexture.uniformLocation("Kq");
	m_ShaderProgramTexture.release();

	glSetupLight();
}



void gl3dView::glSetupLight()
{
	QColor LightColor;
	LightColor.setRedF(  GLLightDlg::s_Light.m_Red);
	LightColor.setGreenF(GLLightDlg::s_Light.m_Green);
	LightColor.setBlueF( GLLightDlg::s_Light.m_Blue);

	m_ShaderProgramSurface.bind();
	if(GLLightDlg::s_Light.m_bIsLightOn) m_ShaderProgramSurface.setUniformValue(m_LightLocationSurface, 1);
	else                                 m_ShaderProgramSurface.setUniformValue(m_LightLocationSurface, 0);
	m_ShaderProgramSurface.setUniformValue(m_LightPosLocationSurface,      (GLfloat)(GLLightDlg::s_Light.m_X), (GLfloat)(GLLightDlg::s_Light.m_Y), (GLfloat)(GLLightDlg::s_Light.m_Z));
	m_ShaderProgramSurface.setUniformValue(m_LightColorLocationSurface,    LightColor);
	m_ShaderProgramSurface.setUniformValue(m_LightAmbientLocationSurface,  GLLightDlg::s_Light.m_Ambient);
	m_ShaderProgramSurface.setUniformValue(m_LightDiffuseLocationSurface,  GLLightDlg::s_Light.m_Diffuse);
	m_ShaderProgramSurface.setUniformValue(m_LightSpecularLocationSurface, GLLightDlg::s_Light.m_Specular);
	m_ShaderProgramSurface.setUniformValue(m_MaterialShininessSurface,     GLLightDlg::s_iShininess);
	m_ShaderProgramSurface.setUniformValue(m_AttenuationConstantSurface,   GLLightDlg::s_Attenuation.m_Constant);
	m_ShaderProgramSurface.setUniformValue(m_AttenuationLinearSurface,     GLLightDlg::s_Attenuation.m_Linear);
	m_ShaderProgramSurface.setUniformValue(m_AttenuationQuadraticSurface,  GLLightDlg::s_Attenuation.m_Quadratic);
	m_ShaderProgramSurface.release();

	m_ShaderProgramTexture.bind();
	if(GLLightDlg::s_Light.m_bIsLightOn) m_ShaderProgramTexture.setUniformValue(m_LightLocationTexture, 1);
	else                                 m_ShaderProgramTexture.setUniformValue(m_LightLocationTexture, 0);
	m_ShaderProgramTexture.setUniformValue(m_LightPosLocationTexture,      (GLfloat)(GLLightDlg::s_Light.m_X), (GLfloat)(GLLightDlg::s_Light.m_Y), (GLfloat)(GLLightDlg::s_Light.m_Z));
	m_ShaderProgramTexture.setUniformValue(m_LightColorLocationTexture,    LightColor);
	m_ShaderProgramTexture.setUniformValue(m_LightAmbientLocationTexture,  GLLightDlg::s_Light.m_Ambient);
	m_ShaderProgramTexture.setUniformValue(m_LightDiffuseLocationTexture,  GLLightDlg::s_Light.m_Diffuse);
	m_ShaderProgramTexture.setUniformValue(m_LightSpecularLocationTexture, GLLightDlg::s_Light.m_Specular);
	m_ShaderProgramTexture.setUniformValue(m_MaterialShininessTexture,     GLLightDlg::s_iShininess);
	m_ShaderProgramTexture.setUniformValue(m_AttenuationConstantTexture,   GLLightDlg::s_Attenuation.m_Constant);
	m_ShaderProgramTexture.setUniformValue(m_AttenuationLinearTexture,     GLLightDlg::s_Attenuation.m_Linear);
	m_ShaderProgramTexture.setUniformValue(m_AttenuationQuadraticTexture,  GLLightDlg::s_Attenuation.m_Quadratic);
	m_ShaderProgramTexture.release();
}



void gl3dView::paintGL()
{
	paintGL3();
	paintOverlay();
}


void gl3dView::paintGL3()
{
//	makeCurrent();
	int width, height;

	double s = 1.0;
	double pixelRatio = devicePixelRatio();

	glClearColor(Settings::backgroundColor().redF(), Settings::backgroundColor().greenF(), Settings::backgroundColor().blueF(), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	m_ShaderProgramSurface.bind();
	m_ShaderProgramSurface.setUniformValue(m_EyePosLocationSurface, QVector3D(0.0,0.0,50.0*s));
	m_ShaderProgramSurface.release();
	m_ShaderProgramTexture.bind();
	m_ShaderProgramTexture.setUniformValue(m_EyePosLocationTexture, QVector3D(0.0,0.0,50.0*s));
	m_ShaderProgramTexture.release();

	QVector4D clipplane(0.0,0.0,-1,m_ClipPlanePos);

	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.setUniformValue(m_ClipPlaneLocationLine, clipplane);
	m_ShaderProgramLine.release();

	m_ShaderProgramSurface.bind();
	m_ShaderProgramSurface.setUniformValue(m_ClipPlaneLocationSurface, clipplane);
	m_ShaderProgramSurface.release();

	m_ShaderProgramTexture.bind();
	m_ShaderProgramTexture.setUniformValue(m_ClipPlaneLocationTexture, clipplane);
	m_ShaderProgramTexture.release();

	width  = geometry().width() * pixelRatio;
	height = geometry().height() * pixelRatio;

	m_orthoMatrix.setToIdentity();
	m_orthoMatrix.ortho(-s,s,-(height*s)/width,(height*s)/width,-50.0*s,50.0*s);

	QMatrix4x4 matQuat(m_ArcBall.ab_quat);

	m_modelMatrix.setToIdentity();//keep identity
	m_viewMatrix = matQuat.transposed();
	m_pvmMatrix = m_orthoMatrix * m_viewMatrix * m_modelMatrix;

	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.setUniformValue(m_mMatrixLocationLine, m_modelMatrix);
	m_ShaderProgramLine.setUniformValue(m_vMatrixLocationLine, m_viewMatrix);
	m_ShaderProgramLine.setUniformValue(m_pvmMatrixLocationLine, m_pvmMatrix);
	m_ShaderProgramLine.release();

	if(m_bArcball) paintArcBall();

	if(s_pMainFrame->m_glLightDlg.isVisible())
	{
		Vector3d lightPos(GLLightDlg::s_Light.m_X, GLLightDlg::s_Light.m_Y, GLLightDlg::s_Light.m_Z);
		double radius = (GLLightDlg::s_Light.m_Z+2.0)/73.0;
		QColor lightColor;
		lightColor.setRedF(GLLightDlg::s_Light.m_Red);
		lightColor.setGreenF(GLLightDlg::s_Light.m_Green);
		lightColor.setBlueF(GLLightDlg::s_Light.m_Blue);
		lightColor.setAlphaF(1.0);
		m_pvmMatrix = m_orthoMatrix;
		paintSphere(lightPos, radius, lightColor, false);
	}

	m_viewMatrix.scale(m_glScaled, m_glScaled, m_glScaled);
	m_viewMatrix.translate(m_glRotCenter.x, m_glRotCenter.y, m_glRotCenter.z);
	m_pvmMatrix = m_orthoMatrix * m_viewMatrix * m_modelMatrix;

	if(m_bAxes)  paintAxes();

	glRenderView();

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
}


void gl3dView::setScale(double refLength)
{
	m_glScaled = 1.5/refLength;
}


void gl3dView::paintFoilNames(void *pWingPtr)
{
	int j;
	Foil *pFoil;
	Wing *pWing = (Wing*)pWingPtr;

	for(j=0; j<pWing->m_Surface.size(); j++)
	{
		pFoil = pWing->m_Surface.at(j)->foilA();

		if(pFoil) glRenderText(pWing->m_Surface.at(j)->m_TA.x, pWing->m_Surface.at(j)->m_TA.y, pWing->m_Surface.at(j)->m_TA.z,
							   pFoil->foilName(),
							   QColor(Qt::cyan).lighter(175));
	}

	j = pWing->m_Surface.size()-1;
	pFoil = pWing->m_Surface.at(j)->foilB();
	if(pFoil) glRenderText(pWing->m_Surface.at(j)->m_TB.x, pWing->m_Surface.at(j)->m_TB.y, pWing->m_Surface.at(j)->m_TB.z,
						 pFoil->foilName(),
						 QColor(Qt::cyan).lighter(175));
}



/**
 * Draws the point masses, the object masses, and the CG position in the OpenGL viewport
*/
void gl3dView::glDrawMasses(Plane *pPlane)
{
	if(!pPlane) return;
	double delta = 0.02/m_glScaled;

	for(int iw=0; iw<MAXWINGS; iw++)
	{
		if(pPlane->wing(iw))
		{
			paintMasses(pPlane->wing(iw)->m_VolumeMass, pPlane->WingLE(iw),
						pPlane->wing(iw)->m_WingName,   pPlane->wing(iw)->m_PointMass);
		}
	}

	paintMasses(0.0, Vector3d(0.0,0.0,0.0),"",pPlane->m_PointMass);


	if(pPlane->body())
	{
		Body *pCurBody = pPlane->body();

		paintMasses(pCurBody->m_VolumeMass,
					pPlane->bodyPos(),
					pCurBody->m_BodyName,
					pCurBody->m_PointMass);
	}

	//plot CG
	Vector3d Place(pPlane->CoG().x, pPlane->CoG().y, pPlane->CoG().z);
	paintSphere(Place, W3dPrefsDlg::s_MassRadius*2.0/m_glScaled,
					   W3dPrefsDlg::s_MassColor.lighter());

	glRenderText(pPlane->CoG().x, pPlane->CoG().y, pPlane->CoG().z + delta,
				 "CoG "+QString("%1").arg(pPlane->totalMass()*Units::kgtoUnit(), 7,'g',3)
				 +Units::weightUnitLabel(), W3dPrefsDlg::s_MassColor.lighter(125));
}



void gl3dView::paintMasses(double volumeMass, Vector3d pos, QString tag, const QList<PointMass*> &ptMasses)
{
	double delta = 0.02/m_glScaled;
	if(qAbs(volumeMass)>PRECISION)
	{
		glRenderText(pos.x, pos.y, pos.z + delta,
					 tag + QString(" (%1").arg(volumeMass*Units::kgtoUnit(), 0,'g',3) + Units::weightUnitLabel()+")", W3dPrefsDlg::s_MassColor.lighter(125));
	}

	for(int im=0; im<ptMasses.size(); im++)
	{
		paintSphere(ptMasses[im]->position() +pos,
					W3dPrefsDlg::s_MassRadius/m_glScaled,
					W3dPrefsDlg::s_MassColor.lighter(),
					true);
		glRenderText(ptMasses[im]->position().x + pos.x,
					 ptMasses[im]->position().y + pos.y,
					 ptMasses[im]->position().z + pos.z + delta,
					 ptMasses[im]->tag()+QString(" (%1").arg(ptMasses[im]->mass()*Units::kgtoUnit(), 0,'g',3)+Units::weightUnitLabel()+")", W3dPrefsDlg::s_MassColor.lighter(125));
	}
}



/** used only in GL3DWingDlg*/
void gl3dView::paintSectionHighlight()
{
	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
	m_vboHighlight.bind();
	m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));
	m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, QColor(255,0,0));
	glLineWidth(5);

	int pos = 0;
	for(int iLines=0; iLines<m_nHighlightLines; iLines++)
	{
		glDrawArrays(GL_LINE_STRIP, pos, m_HighlightLineSize);
		pos += m_HighlightLineSize;
	}

	m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
	m_vboHighlight.release();
	m_ShaderProgramLine.release();
}


/** Default mesh, if no polar has been defined */
void gl3dView::paintEditWingMesh(QOpenGLBuffer &vbo)
{
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.setUniformValue(m_mMatrixLocationLine, m_modelMatrix);
	m_ShaderProgramLine.setUniformValue(m_vMatrixLocationLine, m_viewMatrix);
	m_ShaderProgramLine.setUniformValue(m_pvmMatrixLocationLine, m_pvmMatrix);
	vbo.bind();
	m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3);
	m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_VLMColor);

/*	glEnable (GL_LINE_STIPPLE);
	switch(W3dPrefsDlg::s_VLMStyle)
	{
		case 1:  glLineStipple (1, 0xCFCF); break;
		case 2:  glLineStipple (1, 0x6666); break;
		case 3:  glLineStipple (1, 0xFF18); break;
		case 4:  glLineStipple (1, 0x7E66); break;
		default: glLineStipple (1, 0xFFFF); break;
	}*/

	int nTriangles = vbo.size()/3.0/3.0/sizeof(float); // three vertices and three components

	f->glLineWidth(W3dPrefsDlg::s_VLMWidth);
	int pos = 0;
	for(int p=0; p<nTriangles; p++)
	{
		f->glDrawArrays(GL_LINE_STRIP, pos, 3);
		pos +=3 ;
	}

	m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, Settings::backgroundColor());

	f->glEnable(GL_POLYGON_OFFSET_FILL);
	f->glPolygonOffset(1.0, 1.0);
	f->glDrawArrays(GL_TRIANGLES, 0, nTriangles*3);
	f->glDisable(GL_POLYGON_OFFSET_FILL);

	vbo.release();
	m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.release();
//	f->glDisable(GL_LINE_STIPPLE);
	f->glDisable(GL_POLYGON_OFFSET_FILL);
}




void gl3dView::paintArcBall()
{
	int pos;
	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
	m_vboArcBall.bind();
	m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3, 0);
	m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, QColor(50,55,80,255));

	glLineWidth(1.0);
	pos=0;
	for (int col=0; col<NUMCIRCLES*2; col++)
	{
		glDrawArrays(GL_LINE_STRIP, pos, NUMANGLES-2);
		pos += NUMANGLES-2;
	}
	glDrawArrays(GL_LINE_STRIP, pos, NUMPERIM-1);
	pos += NUMPERIM-1;
	glDrawArrays(GL_LINE_STRIP, pos, NUMPERIM-1);
	pos += NUMPERIM-1;
	m_vboArcBall.release();

	if(m_bCrossPoint)
	{
		QMatrix4x4 pvmCP(m_orthoMatrix);
		m_ArcBall.rotateCrossPoint();
		pvmCP.rotate(m_ArcBall.angle, m_ArcBall.p.x, m_ArcBall.p.y, m_ArcBall.p.z);
		m_ShaderProgramLine.setUniformValue(m_pvmMatrixLocationLine, pvmCP);

		m_vboArcPoint.bind();
		m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3, 0);
		m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, QColor(70, 25, 40));
		pos=0;
		glLineWidth(3.0);
		glDrawArrays(GL_LINE_STRIP, pos, 2*NUMARCPOINTS);
		pos += 2*NUMARCPOINTS;
		glDrawArrays(GL_LINE_STRIP, pos, 2*NUMARCPOINTS);
		glLineWidth(1.0);
		m_vboArcPoint.release();
	}
	m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.release();
}


void gl3dView::paintAxes()
{
	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3, 0);
	m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_3DAxisColor);
	m_ShaderProgramLine.setUniformValue(m_mMatrixLocationLine, m_modelMatrix);
	m_ShaderProgramLine.setUniformValue(m_vMatrixLocationLine, m_viewMatrix);
	m_ShaderProgramLine.setUniformValue(m_pvmMatrixLocationLine, m_pvmMatrix);

	//draw Axis
	glLineWidth(W3dPrefsDlg::s_3DAxisWidth);
	glEnable (GL_LINE_STIPPLE);
	glLineStipple (1, 0xFF18);

	m_ShaderProgramLine.setAttributeArray(m_VertexLocationLine, x_axis, 3);
	glDrawArrays(GL_LINE_STRIP, 0, 6);
	m_ShaderProgramLine.setAttributeArray(m_VertexLocationLine, y_axis, 3);
	glDrawArrays(GL_LINE_STRIP, 0, 6);
	m_ShaderProgramLine.setAttributeArray(m_VertexLocationLine, z_axis, 3);
	glDrawArrays(GL_LINE_STRIP, 0, 6);

	glDisable (GL_LINE_STIPPLE);
	m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.release();
	glRenderText(1.0, 0.015, 0.015, "X");
	glRenderText(0.015, 1.0, 0.015, "Y");
	glRenderText(0.015, 0.015, 1.0, "Z");
}


void gl3dView::setSpanStations(Plane *pPlane, WPolar *pWPolar, PlaneOpp *pPOpp)
{
	if(!pPlane || !pWPolar || !pPOpp) return;
	Wing *pWing;

	if(pWPolar->isLLTMethod())
	{
		if(pPOpp)
		{
			m_Ny[0] = pPOpp->m_pPlaneWOpp[0]->m_NStation-1;
		}
		else
		{
			m_Ny[0] = LLTAnalysis::nSpanStations();
		}

		m_Ny[1] = m_Ny[2] = m_Ny[3] = 0;
	}
	else
	{
		for(int iWing=0; iWing<MAXWINGS; iWing++)
		{
			pWing = pPlane->wing(iWing);
			if(pWing)
			{
				m_Ny[iWing]=0;
				for (int j=0; j<pWing->m_Surface.size(); j++)
				{
					m_Ny[iWing] += pWing->m_Surface[j]->NYPanels();
				}
			}
		}
	}
}



void gl3dView::paintBody(Body *pBody)
{
	if(!pBody) return;
	int pos = 0;
	int NXXXX = W3dPrefsDlg::bodyAxialRes();
	int NHOOOP = W3dPrefsDlg::bodyHoopRes();

	bool bTextures = pBody->textures() && (m_pLeftBodyTexture && m_pRightBodyTexture);
	if(bTextures)
	{
		m_ShaderProgramTexture.bind();
		m_vboBody.bind();

		m_ShaderProgramTexture.setUniformValue(m_mMatrixLocationTexture, m_modelMatrix);
		m_ShaderProgramTexture.setUniformValue(m_vMatrixLocationTexture, m_viewMatrix);
		m_ShaderProgramTexture.setUniformValue(m_pvmMatrixLocationTexture, m_pvmMatrix);
		if(GLLightDlg::s_Light.m_bIsLightOn) m_ShaderProgramTexture.setUniformValue(m_LightLocationTexture, 1);
		else                                 m_ShaderProgramTexture.setUniformValue(m_LightLocationTexture, 0);
		m_ShaderProgramTexture.enableAttributeArray(m_VertexLocationTexture);
		m_ShaderProgramTexture.enableAttributeArray(m_NormalLocationTexture);
		m_ShaderProgramTexture.enableAttributeArray(m_UVLocationTexture);
		m_ShaderProgramTexture.setAttributeBuffer(m_VertexLocationTexture, GL_FLOAT, 0,                  3, 8 * sizeof(GLfloat));
		m_ShaderProgramTexture.setAttributeBuffer(m_NormalLocationTexture, GL_FLOAT, 3* sizeof(GLfloat), 3, 8 * sizeof(GLfloat));
		m_ShaderProgramTexture.setAttributeBuffer(m_UVLocationTexture,     GL_FLOAT, 6* sizeof(GLfloat), 2, 8 * sizeof(GLfloat));
	}
	else
	{
		m_ShaderProgramSurface.bind();
		m_vboBody.bind();

		m_ShaderProgramSurface.setUniformValue(m_mMatrixLocationSurface, m_modelMatrix);
		m_ShaderProgramSurface.setUniformValue(m_vMatrixLocationSurface, m_viewMatrix);
		m_ShaderProgramSurface.setUniformValue(m_pvmMatrixLocationSurface, m_pvmMatrix);
		if(GLLightDlg::s_Light.m_bIsLightOn) m_ShaderProgramSurface.setUniformValue(m_LightLocationSurface, 1);
		else                                 m_ShaderProgramSurface.setUniformValue(m_LightLocationSurface, 0);
		m_ShaderProgramSurface.setUniformValue(m_ColorLocationSurface, color(pBody->bodyColor()));
		m_ShaderProgramSurface.enableAttributeArray(m_VertexLocationSurface);
		m_ShaderProgramSurface.enableAttributeArray(m_NormalLocationSurface);
		m_ShaderProgramSurface.setAttributeBuffer(m_VertexLocationSurface, GL_FLOAT, 0,                  3, 8 * sizeof(GLfloat));
		m_ShaderProgramSurface.setAttributeBuffer(m_NormalLocationSurface, GL_FLOAT, 3* sizeof(GLfloat), 3, 8 * sizeof(GLfloat));
	}


	if(m_bSurfaces)
	{
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0, 1.0);

		if(bTextures) m_pRightBodyTexture->bind();
		glDrawElements(GL_TRIANGLES, m_iBodyElems/2, GL_UNSIGNED_SHORT, m_BodyIndicesArray);
		if(bTextures) m_pRightBodyTexture->release();
		if(bTextures) m_pLeftBodyTexture->bind();
		glDrawElements(GL_TRIANGLES, m_iBodyElems/2, GL_UNSIGNED_SHORT, m_BodyIndicesArray+m_iBodyElems/2);
		if(bTextures) m_pLeftBodyTexture->release();

		glDisable(GL_POLYGON_OFFSET_FILL);
	}
	m_vboBody.release();

	if(bTextures)
	{
		m_ShaderProgramTexture.disableAttributeArray(m_VertexLocationTexture);
		m_ShaderProgramTexture.disableAttributeArray(m_NormalLocationTexture);
		m_ShaderProgramTexture.disableAttributeArray(m_UVLocationTexture);
		m_ShaderProgramTexture.release();
	}
	else
	{
		m_ShaderProgramSurface.disableAttributeArray(m_VertexLocationSurface);
		m_ShaderProgramSurface.disableAttributeArray(m_NormalLocationSurface);
		m_ShaderProgramSurface.release();
	}


	if(m_bOutline)
	{
		m_ShaderProgramLine.bind();
		m_vboBody.bind();
		m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
		m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3, 8 * sizeof(GLfloat));
		m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_OutlineColor);

		m_ShaderProgramLine.setUniformValue(m_mMatrixLocationLine, m_modelMatrix);
		m_ShaderProgramLine.setUniformValue(m_vMatrixLocationLine, m_viewMatrix);
		m_ShaderProgramLine.setUniformValue(m_pvmMatrixLocationLine, m_pvmMatrix);

		glLineWidth(W3dPrefsDlg::s_OutlineWidth);

		if(pBody->isSplineType())
		{
			pos = (NXXXX+1) * (NHOOOP+1) * 2;
			for(int iFr=0; iFr<pBody->frameCount(); iFr++)
			{
				glDrawArrays(GL_LINE_STRIP, pos, (NHOOOP+1)*2);
				pos += (NHOOOP+1)*2;
			}
			glDrawArrays(GL_LINE_STRIP, pos, NXXXX+1);
			pos += NXXXX+1;
			glDrawArrays(GL_LINE_STRIP, pos, NXXXX+1);
		}
		else if(pBody->isFlatPanelType())
		{
			int pos=0;
			for(uint i=0; i<m_iBodyElems/2; i++)
			{
				glDrawArrays(GL_LINE_STRIP, pos, 4);
				pos +=4;
			}
		}

		m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
		m_vboBody.release();
		m_ShaderProgramLine.release();
	}
}


/** Default mesh, if no polar has been defined */
void gl3dView::paintEditBodyMesh(Body *pBody)
{
	if(!pBody) return;
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.setUniformValue(m_mMatrixLocationLine, m_modelMatrix);
	m_ShaderProgramLine.setUniformValue(m_vMatrixLocationLine, m_viewMatrix);
	m_ShaderProgramLine.setUniformValue(m_pvmMatrixLocationLine, m_pvmMatrix);
	m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
	m_vboEditBodyMesh.bind();
	m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3, 3*sizeof(GLfloat));
	m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_VLMColor);
//	m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, Qt::red);

	if(pBody->isFlatPanelType())
	{
		f->glLineWidth(W3dPrefsDlg::s_VLMWidth);

//		f->glPolygonOffset(1.0, 1.0);
		f->glDrawArrays(GL_LINES, 0, m_iBodyMeshLines*2);
	}
	else if(pBody->isSplineType())
	{
		int pos=0;
		int NXXXX = W3dPrefsDlg::bodyAxialRes();
		int NHOOOP = W3dPrefsDlg::bodyHoopRes();
		f->glLineWidth(W3dPrefsDlg::s_VLMWidth);

		pos=0;
		//x-lines
		for (int l=0; l<2*pBody->m_nhPanels; l++)
		{
			f->glDrawArrays(GL_LINE_STRIP, pos, NXXXX);
			pos += NXXXX;
		}

		//hoop lines;
		for (int k=0; k<2*pBody->m_nxPanels; k++)
		{
			f->glDrawArrays(GL_LINE_STRIP, pos, NHOOOP);
			pos += NHOOOP;
		}
	}

	m_vboEditBodyMesh.release();

	//mesh background
	m_ShaderProgramSurface.bind();
	m_ShaderProgramSurface.setUniformValue(m_mMatrixLocationSurface, m_modelMatrix);
	m_ShaderProgramSurface.setUniformValue(m_vMatrixLocationSurface, m_viewMatrix);
	m_ShaderProgramSurface.setUniformValue(m_pvmMatrixLocationSurface, m_pvmMatrix);
	m_ShaderProgramSurface.setUniformValue(m_LightLocationSurface, 0); // no light for the background
	m_ShaderProgramSurface.enableAttributeArray(m_VertexLocationSurface);
	m_ShaderProgramSurface.enableAttributeArray(m_NormalLocationSurface);
	m_ShaderProgramSurface.setUniformValue(m_ColorLocationSurface, Settings::backgroundColor());

	m_vboEditBodyMesh.bind();
	m_ShaderProgramSurface.setAttributeBuffer(m_VertexLocationSurface, GL_FLOAT, 0,                  3, 6*sizeof(GLfloat));
	m_ShaderProgramSurface.setAttributeBuffer(m_NormalLocationSurface, GL_FLOAT, 3* sizeof(GLfloat), 3, 6*sizeof(GLfloat));

	f->glEnable(GL_POLYGON_OFFSET_FILL);
	f->glPolygonOffset(1.0, 1.0);
	f->glDrawElements(GL_TRIANGLES, m_iBodyElems/2, GL_UNSIGNED_SHORT, m_BodyIndicesArray);
	f->glDrawElements(GL_TRIANGLES, m_iBodyElems/2, GL_UNSIGNED_SHORT, m_BodyIndicesArray+m_iBodyElems/2);
	f->glDisable(GL_POLYGON_OFFSET_FILL);

	m_ShaderProgramSurface.disableAttributeArray(m_VertexLocationSurface);
	m_ShaderProgramSurface.disableAttributeArray(m_NormalLocationSurface);
	m_ShaderProgramSurface.release();


	m_vboEditBodyMesh.release();
	m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.release();
}


void gl3dView::paintWing(int iWing, Wing *pWing)
{
	if(!pWing) return;

	int CHORDPOINTS = W3dPrefsDlg::chordwiseRes();

	if(m_bSurfaces)
	{
		unsigned short *wingIndicesArray = m_WingIndicesArray[iWing];

		int pos = 0;

		bool bTextures = pWing->textures() &&
				(m_pWingBotLeftTexture[iWing] && m_pWingBotRightTexture[iWing] && m_pWingTopLeftTexture[iWing] && m_pWingTopRightTexture[iWing]);

		if(bTextures)
		{
			m_ShaderProgramTexture.bind();
			m_vboWingSurface[iWing].bind();

			m_ShaderProgramTexture.setUniformValue(m_mMatrixLocationTexture, m_modelMatrix);
			m_ShaderProgramTexture.setUniformValue(m_vMatrixLocationTexture, m_viewMatrix);
			m_ShaderProgramTexture.setUniformValue(m_pvmMatrixLocationTexture, m_pvmMatrix);
			if(GLLightDlg::s_Light.m_bIsLightOn) m_ShaderProgramTexture.setUniformValue(m_LightLocationTexture, 1);
			else                                 m_ShaderProgramTexture.setUniformValue(m_LightLocationTexture, 0);

			m_ShaderProgramTexture.enableAttributeArray(m_VertexLocationTexture);
			m_ShaderProgramTexture.enableAttributeArray(m_NormalLocationTexture);
			m_ShaderProgramTexture.enableAttributeArray(m_UVLocationTexture);
			m_ShaderProgramTexture.setAttributeBuffer(m_VertexLocationTexture, GL_FLOAT, 0,                  3, 8 * sizeof(GLfloat));
			m_ShaderProgramTexture.setAttributeBuffer(m_NormalLocationTexture, GL_FLOAT, 3* sizeof(GLfloat), 3, 8 * sizeof(GLfloat));
			m_ShaderProgramTexture.setAttributeBuffer(m_UVLocationTexture,     GL_FLOAT, 6* sizeof(GLfloat), 2, 8 * sizeof(GLfloat));
		}
		else
		{
			m_ShaderProgramSurface.bind();
			m_vboWingSurface[iWing].bind();
			m_ShaderProgramSurface.setUniformValue(m_mMatrixLocationSurface, m_modelMatrix);
			m_ShaderProgramSurface.setUniformValue(m_vMatrixLocationSurface, m_viewMatrix);
			m_ShaderProgramSurface.setUniformValue(m_pvmMatrixLocationSurface, m_pvmMatrix);
			if(GLLightDlg::s_Light.m_bIsLightOn) m_ShaderProgramSurface.setUniformValue(m_LightLocationSurface, 1);
			else                                 m_ShaderProgramSurface.setUniformValue(m_LightLocationSurface, 0);
			m_ShaderProgramSurface.setUniformValue(m_ColorLocationSurface, color(pWing->wingColor()));

			m_ShaderProgramSurface.enableAttributeArray(m_VertexLocationSurface);
			m_ShaderProgramSurface.enableAttributeArray(m_NormalLocationSurface);
			m_ShaderProgramSurface.setAttributeBuffer(m_VertexLocationSurface, GL_FLOAT, 0,                  3, 8 * sizeof(GLfloat));
			m_ShaderProgramSurface.setAttributeBuffer(m_NormalLocationSurface, GL_FLOAT, 3* sizeof(GLfloat), 3, 8 * sizeof(GLfloat));
		}


		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0, 1.0);

		//indices array size:
		//  Top & bottom surfaces
		//      NSurfaces
		//      x (ChordPoints-1)quads
		//      x2 triangles per/quad
		//      x2 top and bottom surfaces
		//      x3 indices/triangle

		pos = 0;
		for (int j=0; j<pWing->m_Surface.count(); j++)
		{
			//top surface
			if(bTextures)
			{
				if(pWing->m_Surface.at(j)->isLeftSurf()) m_pWingTopLeftTexture[iWing]->bind();
				else                                     m_pWingTopRightTexture[iWing]->bind();
			}
			glDrawElements(GL_TRIANGLES, (CHORDPOINTS-1)*2*3, GL_UNSIGNED_SHORT, wingIndicesArray+pos);
			if(bTextures)
			{
				if(pWing->m_Surface.at(j)->isLeftSurf()) m_pWingTopLeftTexture[iWing]->release();
				else                                     m_pWingTopRightTexture[iWing]->release();
			}
			pos += (CHORDPOINTS-1)*2*3;
			// bottom surface
			if(bTextures)
			{
				if(pWing->m_Surface.at(j)->isLeftSurf()) m_pWingBotLeftTexture[iWing]->bind();
				else                                     m_pWingBotRightTexture[iWing]->bind();
			}
			glDrawElements(GL_TRIANGLES, (CHORDPOINTS-1)*2*3, GL_UNSIGNED_SHORT, wingIndicesArray+pos);
			if(bTextures)
			{
				if(pWing->m_Surface.at(j)->isLeftSurf()) m_pWingBotLeftTexture[iWing]->release();
				else                                     m_pWingBotRightTexture[iWing]->release();
			}
			pos += (CHORDPOINTS-1)*2*3;
		}

		for (int j=0; j<pWing->m_Surface.count(); j++)
		{
			//tip ssurface
			if(pWing->m_Surface.at(j)->isTipLeft())
			{
				glDrawElements(GL_TRIANGLES, (CHORDPOINTS-1)*2*3, GL_UNSIGNED_SHORT, wingIndicesArray+pos);
				pos += (CHORDPOINTS-1)*2*3;
			}

			if(pWing->m_Surface.at(j)->isTipRight())
			{
				glDrawElements(GL_TRIANGLES, (CHORDPOINTS-1)*2*3, GL_UNSIGNED_SHORT, wingIndicesArray+pos);
				pos += (CHORDPOINTS-1)*2*3;
			}
		}

		glDisable(GL_POLYGON_OFFSET_FILL);

		m_vboWingSurface[iWing].release();
		if(bTextures)
		{
			m_ShaderProgramTexture.disableAttributeArray(m_VertexLocationTexture);
			m_ShaderProgramTexture.disableAttributeArray(m_NormalLocationTexture);
			m_ShaderProgramTexture.disableAttributeArray(m_UVLocationTexture);
			m_ShaderProgramTexture.release();
		}
		else
		{
			m_ShaderProgramSurface.disableAttributeArray(m_VertexLocationSurface);
			m_ShaderProgramSurface.disableAttributeArray(m_NormalLocationSurface);
			m_ShaderProgramSurface.release();
		}
	}

	if(m_bOutline)
	{
		m_ShaderProgramLine.bind();
		m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_OutlineColor);
		m_ShaderProgramLine.setUniformValue(m_mMatrixLocationLine, m_modelMatrix);
		m_ShaderProgramLine.setUniformValue(m_vMatrixLocationLine, m_viewMatrix);
		m_ShaderProgramLine.setUniformValue(m_pvmMatrixLocationLine, m_pvmMatrix);

		m_vboWingOutline[iWing].bind();
		m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
		m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3, 3* sizeof(GLfloat));

		glLineWidth(W3dPrefsDlg::s_OutlineWidth);
		glEnable (GL_LINE_STIPPLE);
		switch(W3dPrefsDlg::s_OutlineStyle)
		{
			case 1:  glLineStipple (1, 0xCFCF); break;
			case 2:  glLineStipple (1, 0x6666); break;
			case 3:  glLineStipple (1, 0xFF18); break;
			case 4:  glLineStipple (1, 0x7E66); break;
			default: glLineStipple (1, 0xFFFF); break;
		}

		glDrawArrays(GL_LINES, 0, m_iWingOutlinePoints[iWing]);
		m_vboWingOutline[iWing].release();

		m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
		m_ShaderProgramLine.release();
	}
	glDisable(GL_LINE_STIPPLE);
}



/**
*Creates the OpenGL List for the ArcBall.
*@param ArcBall the ArcBall object associated to the view
*@param GLScale the overall scaling factor for the view
*/
void gl3dView::glMakeArcBall()
{
	float GLScale = 1.0f;
	int row, col;
	double Radius=0.1, lat_incr, lon_incr, phi, theta;
	Vector3d eye(0.0, 0.0, 1.0);
	Vector3d up(0.0, 1.0, 0.0);
	m_ArcBall.setZoom(0.45, eye, up);

	Radius = m_ArcBall.ab_sphere;
	lat_incr =  90.0 / (double)NUMANGLES;
	lon_incr = 360.0 / (double)NUMCIRCLES;

	int iv=0;

	int bufferSize = ((NUMCIRCLES*2)*(NUMANGLES-2) + (NUMPERIM-1)*2)*3;
	float *arcBallVertexArray  = new float[bufferSize];

	//ARCBALL
	for (col=0; col<NUMCIRCLES; col++)
	{
		//first
		phi = (col * lon_incr) * PI/180.0;
		for (row=1; row<NUMANGLES-1; row++)
		{
			theta = (row * lat_incr) * PI/180.0;
			arcBallVertexArray[iv++] = Radius*cos(phi)*cos(theta)*GLScale;
			arcBallVertexArray[iv++] = Radius*sin(theta)*GLScale;
			arcBallVertexArray[iv++] = Radius*sin(phi)*cos(theta)*GLScale;
		}
	}

	for (col=0; col<NUMCIRCLES; col++)
	{
		//Second
		phi = (col * lon_incr ) * PI/180.0;
		for (row=1; row<NUMANGLES-1; row++)
		{
			theta = -(row * lat_incr) * PI/180.0;
			arcBallVertexArray[iv++] = Radius*cos(phi)*cos(theta)*GLScale;
			arcBallVertexArray[iv++] = Radius*sin(theta)*GLScale;
			arcBallVertexArray[iv++] = Radius*sin(phi)*cos(theta)*GLScale;
		}
	}

	theta = 0.;
	for(col=1; col<NUMPERIM; col++)
	{
		phi = (0.0 + (double)col*360.0/72.0) * PI/180.0;
		arcBallVertexArray[iv++] = Radius * cos(phi) * cos(theta)*GLScale;
		arcBallVertexArray[iv++] = Radius * sin(theta)*GLScale;
		arcBallVertexArray[iv++] = Radius * sin(phi) * cos(theta)*GLScale;
	}

	theta = 0.;
	for(col=1; col<NUMPERIM; col++)
	{
		phi = (0.0 + (double)col*360.0/72.0) * PI/180.0;
		arcBallVertexArray[iv++] = Radius * cos(-phi) * cos(theta)*GLScale;
		arcBallVertexArray[iv++] = Radius * sin(theta)*GLScale;
		arcBallVertexArray[iv++] = Radius * sin(-phi) * cos(theta)*GLScale;
	}
	Q_ASSERT(iv==bufferSize);

	m_vboArcBall.destroy();
	m_vboArcBall.create();
	m_vboArcBall.bind();
	m_vboArcBall.allocate(arcBallVertexArray, iv * sizeof(GLfloat));
	m_vboArcBall.release();

	delete [] arcBallVertexArray;
}

#define NUMLONG  43
#define NUMLAT   37

/**
Creates a list for a sphere with unit radius
*/
void gl3dView::glMakeUnitSphere()
{
	double start_lat, start_lon,lat_incr, lon_incr;
	double phi, theta;
	int iLat, iLong;

	start_lat = -90 * PI/180.0;
	start_lon = 0.0 * PI/180.0;

	lat_incr = 180.0 / (NUMLAT-1) * PI/180.0;
	lon_incr = 360.0 / (NUMLONG-1) * PI/180.0;

	int bufferSize = NUMLONG * NUMLAT * 2 *3;
	GLfloat *sphereVertexArray = new GLfloat[bufferSize];
	m_SphereIndicesArray  = new unsigned short[(NUMLONG-1) * NUMLAT * 2];

	int iv = 0;

	for (iLong=0; iLong<NUMLONG; iLong++)
	{
		phi = (start_lon + iLong * lon_incr) ;
		for (iLat=0; iLat<NUMLAT; iLat++)
		{
			theta = (start_lat + iLat * lat_incr);
			// the point
			sphereVertexArray[iv++] = cos(phi) * cos(theta);//x
			sphereVertexArray[iv++] = sin(phi) * cos(theta);//z
			sphereVertexArray[iv++] = sin(theta);//y
			 //the normal
			sphereVertexArray[iv++] = cos(phi) * cos(theta);//x
			sphereVertexArray[iv++] = sin(phi) * cos(theta);//z
			sphereVertexArray[iv++] = sin(theta);//y
		}
	}

	Q_ASSERT(iv==bufferSize);

	int in=0;
	for (iLong=0; iLong<NUMLONG-1; iLong++)
	{
		for (iLat=0; iLat<NUMLAT; iLat++)
		{
			m_SphereIndicesArray[in++] =   iLong   *NUMLAT  + iLat;
			m_SphereIndicesArray[in++] =  (iLong+1)*NUMLAT  + iLat;
		}
	}
	Q_ASSERT(in==(NUMLONG-1) * NUMLAT * 2);

	m_vboSphere.create();
	m_vboSphere.bind();
	m_vboSphere.allocate(sphereVertexArray, iv * sizeof(GLfloat));
	m_vboSphere.release();
	delete [] sphereVertexArray;
}


void gl3dView::paintSphere(Vector3d place, double radius, QColor sphereColor, bool bLight)
{
	QMatrix4x4 mSphere; //is identity
	mSphere.translate(place.x, place.y, place.z);
	mSphere.scale(radius);

	m_ShaderProgramSurface.bind();
	m_ShaderProgramSurface.setUniformValue(m_mMatrixLocationSurface, mSphere);
	m_ShaderProgramSurface.setUniformValue(m_vMatrixLocationSurface, m_viewMatrix);
	m_ShaderProgramSurface.setUniformValue(m_pvmMatrixLocationSurface, m_pvmMatrix * mSphere);
	if(bLight) m_ShaderProgramSurface.setUniformValue(m_LightLocationSurface, 1);
	else       m_ShaderProgramSurface.setUniformValue(m_LightLocationSurface, 0);
	m_ShaderProgramSurface.setUniformValue(m_ColorLocationSurface, sphereColor);

	m_ShaderProgramSurface.enableAttributeArray(m_VertexLocationSurface);
	m_ShaderProgramSurface.enableAttributeArray(m_NormalLocationSurface);

	m_vboSphere.bind();
	m_ShaderProgramSurface.setAttributeBuffer(m_VertexLocationSurface, GL_FLOAT, 0,                  3, 6 * sizeof(GLfloat));
	m_ShaderProgramSurface.setAttributeBuffer(m_NormalLocationSurface, GL_FLOAT, 3* sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

	for(int iLong=0; iLong<NUMLONG-1; iLong++)
	{
		glDrawElements(GL_TRIANGLE_STRIP, NUMLAT*2, GL_UNSIGNED_SHORT, m_SphereIndicesArray+iLong*NUMLAT*2);
	}
	m_vboSphere.release();

	m_ShaderProgramSurface.disableAttributeArray(m_VertexLocationSurface);
	m_ShaderProgramSurface.disableAttributeArray(m_NormalLocationSurface);
	m_ShaderProgramSurface.release();
}



void gl3dView::glMakeWingGeometry(int iWing, Wing *pWing, Body *pBody)
{
	int CHORDPOINTS = W3dPrefsDlg::chordwiseRes();

	int j, l ;
	Vector3d N, Pt;
	Vector3d *NormalA = new Vector3d[CHORDPOINTS];
	Vector3d *NormalB = new Vector3d[CHORDPOINTS];
	Vector3d *PtBotLeft, *PtBotRight, *PtTopLeft, *PtTopRight;
	PtBotLeft  = new Vector3d[pWing->m_Surface.count() * CHORDPOINTS];
	PtBotRight = new Vector3d[pWing->m_Surface.count() * CHORDPOINTS];
	PtTopLeft  = new Vector3d[pWing->m_Surface.count() * CHORDPOINTS];
	PtTopRight = new Vector3d[pWing->m_Surface.count() * CHORDPOINTS];

	double *leftV= new double[CHORDPOINTS];
	double *rightV = new double[CHORDPOINTS];
	double leftU=0.0, rightU=1.0;
	memset(NormalA, 0, sizeof(CHORDPOINTS*sizeof(Vector3d)));
	memset(NormalB, 0, sizeof(CHORDPOINTS*sizeof(Vector3d)));
	//vertices array size:
	// surface:
	//     pWing->NSurfaces
	//     xCHORDPOINTS : from 0 to CHORDPOINTS
	//     x2  for A and B sides
	//     x2  for top and bottom
	// outline
	//     2 points mLA & mLB for leading edge
	//     2 points mTA & mTB for trailing edge
	//
	// x8  : for 3 vertex components, 3 normal components, 2 texture components

	int bufferSize = pWing->m_Surface.count()*CHORDPOINTS*2*2 ;
	bufferSize *= 8;

	float *wingVertexArray = new float[bufferSize];

	N.set(0.0, 0.0, 0.0);
	int iv=0; //index of vertex components

	//SURFACE
	for (j=0; j<pWing->m_Surface.size(); j++)
	{
		//top surface
		pWing->m_Surface.at(j)->getSidePoints(TOPSURFACE, pBody, PtTopLeft+j*CHORDPOINTS, PtTopRight+j*CHORDPOINTS,
											  NormalA, NormalB, CHORDPOINTS);
		pWing->getTextureUV(j, leftV, rightV, leftU, rightU, CHORDPOINTS);

		//left side vertices
		for (l=0; l<CHORDPOINTS; l++)
		{
			wingVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS+l].x;
			wingVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS+l].y;
			wingVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS+l].z;
			wingVertexArray[iv++] = NormalA[l].x;
			wingVertexArray[iv++] = NormalA[l].y;
			wingVertexArray[iv++] = NormalA[l].z;
			wingVertexArray[iv++] = leftU;
			wingVertexArray[iv++] = leftV[l];
		}
		//right side vertices
		for (l=0; l<CHORDPOINTS; l++)
		{
			wingVertexArray[iv++] = PtTopRight[j*CHORDPOINTS+l].x;
			wingVertexArray[iv++] = PtTopRight[j*CHORDPOINTS+l].y;
			wingVertexArray[iv++] = PtTopRight[j*CHORDPOINTS+l].z;
			wingVertexArray[iv++] = NormalB[l].x;
			wingVertexArray[iv++] = NormalB[l].y;
			wingVertexArray[iv++] = NormalB[l].z;
			wingVertexArray[iv++] = rightU;
			wingVertexArray[iv++] = rightV[l];
		}


		//bottom surface
		pWing->m_Surface.at(j)->getSidePoints(BOTSURFACE, pBody, PtBotLeft+j*CHORDPOINTS, PtBotRight+j*CHORDPOINTS,
											  NormalA, NormalB, CHORDPOINTS);

		//left side vertices
		for (l=0; l<CHORDPOINTS; l++)
		{
			wingVertexArray[iv++] = PtBotLeft[j*CHORDPOINTS+l].x;
			wingVertexArray[iv++] = PtBotLeft[j*CHORDPOINTS+l].y;
			wingVertexArray[iv++] = PtBotLeft[j*CHORDPOINTS+l].z;
			wingVertexArray[iv++] = NormalA[l].x;
			wingVertexArray[iv++] = NormalA[l].y;
			wingVertexArray[iv++] = NormalA[l].z;
			wingVertexArray[iv++] = 1.0f-leftU;
			wingVertexArray[iv++] = leftV[l];
		}

		//right side vertices
		for (l=0; l<CHORDPOINTS; l++)
		{
			wingVertexArray[iv++] = PtBotRight[j*CHORDPOINTS+l].x;
			wingVertexArray[iv++] = PtBotRight[j*CHORDPOINTS+l].y;
			wingVertexArray[iv++] = PtBotRight[j*CHORDPOINTS+l].z;
			wingVertexArray[iv++] = NormalB[l].x;
			wingVertexArray[iv++] = NormalB[l].y;
			wingVertexArray[iv++] = NormalB[l].z;
			wingVertexArray[iv++] = 1.0f-rightU;
			wingVertexArray[iv++] = rightV[l];
		}
	}


	Q_ASSERT(iv==bufferSize);

	//indices array size:
	//  Top & bottom surfaces
	//      NSurfaces
	//      x (ChordPoints-1)quads
	//      x2 triangles per/quad
	//      x2 top and bottom surfaces
	//      x3 indices/triangle
	//  Tip patches
	//      (CHORDPOINTS-1) quads
	//      x2 triangles per/quad
	//      x2 tip patches
	//      x3 indices/triangle

	m_iWingElems[iWing] =  pWing->m_Surface.count()* (CHORDPOINTS-1) *2 *2 *3
						 + (CHORDPOINTS-1) *2 *2 *3;
	if(m_WingIndicesArray[iWing]) delete[] m_WingIndicesArray[iWing];
	m_WingIndicesArray[iWing] = new unsigned short[m_iWingElems[iWing]];
	unsigned short *wingIndicesArray = m_WingIndicesArray[iWing];
	uint ii = 0;
	int nV=0;
	for (j=0; j<pWing->m_Surface.count(); j++)
	{
		//topsurface
		for (l=0; l<CHORDPOINTS-1; l++)
		{
			Q_ASSERT(ii < m_iWingElems[iWing]);
			//first triangle
			wingIndicesArray[ii]   = nV;
			wingIndicesArray[ii+1] = nV+1;
			wingIndicesArray[ii+2] = nV+CHORDPOINTS;
			//second triangle
			wingIndicesArray[ii+3] = nV+CHORDPOINTS;
			wingIndicesArray[ii+4] = nV+1;
			wingIndicesArray[ii+5] = nV+CHORDPOINTS+1;
			ii += 6;
			nV++;
		}
		nV +=CHORDPOINTS+1;

		//botsurface
		for (l=0; l<CHORDPOINTS-1; l++)
		{
			Q_ASSERT(ii < m_iWingElems[iWing]);
			//first triangle
			wingIndicesArray[ii]   = nV;
			wingIndicesArray[ii+1] = nV+1;
			wingIndicesArray[ii+2] = nV+CHORDPOINTS;
			//second triangle
			wingIndicesArray[ii+3] = nV+CHORDPOINTS;
			wingIndicesArray[ii+4] = nV+1;
			wingIndicesArray[ii+5] = nV+CHORDPOINTS+1;
			ii += 6;
			nV++;
		}
		nV +=CHORDPOINTS+1;
	}

	//TIP PATCHES
	nV=0;
	for (j=0; j<pWing->m_Surface.count(); j++)
	{
		if(pWing->m_Surface.at(j)->isTipLeft())
		{
			Q_ASSERT(ii+5 < m_iWingElems[iWing]);
			for (l=0; l<CHORDPOINTS-1; l++)
			{
				//first triangle
				wingIndicesArray[ii]   = nV;
				wingIndicesArray[ii+1] = nV+1;
				wingIndicesArray[ii+2] = nV+2*CHORDPOINTS;
				//second triangle
				wingIndicesArray[ii+3] = nV+2*CHORDPOINTS;
				wingIndicesArray[ii+4] = nV+1;
				wingIndicesArray[ii+5] = nV+2*CHORDPOINTS+1;
				ii += 6;
				nV++; //move one vertex
			}
			nV++; //skip the last vertex
		}

		if(pWing->m_Surface.at(j)->isTipRight())
		{
			if(!pWing->m_Surface.at(j)->isTipLeft()) nV += CHORDPOINTS;

			Q_ASSERT(ii+5 < m_iWingElems[iWing]);
			for (l=0; l<CHORDPOINTS-1; l++)
			{
				//first triangle
				wingIndicesArray[ii]   = nV;
				wingIndicesArray[ii+1] = nV+1;
				wingIndicesArray[ii+2] = nV+2*CHORDPOINTS;

				//second triangle
				wingIndicesArray[ii+3] = nV+2*CHORDPOINTS;
				wingIndicesArray[ii+4] = nV+1;
				wingIndicesArray[ii+5] = nV+2*CHORDPOINTS+1;
				ii += 6;
				nV++; //move one vertex
			}
			nV++; //skip the last vertex;
			nV += CHORDPOINTS; //skip the bottom line of this wing section
		}

		if(pWing->m_Surface.at(j)->isTipLeft())
		{
			nV+= 3*CHORDPOINTS;
		}
		else if(pWing->m_Surface.at(j)->isTipRight())
		{
		}
		else
		{
			nV +=4*CHORDPOINTS;
		}
	}
	Q_ASSERT(ii==m_iWingElems[iWing]);

	if(m_pWingBotLeftTexture[iWing])  delete m_pWingBotLeftTexture[iWing];
	if(m_pWingTopLeftTexture[iWing])  delete m_pWingTopLeftTexture[iWing];
	if(m_pWingBotRightTexture[iWing]) delete m_pWingBotRightTexture[iWing];
	if(m_pWingTopRightTexture[iWing]) delete m_pWingTopRightTexture[iWing];


	QString planeName;
	QString textureName;

	if(s_pMiarex && s_pMiarex->m_pCurPlane)
	{
		planeName = s_pMiarex->m_pCurPlane->planeName();
		switch(pWing->wingType())
		{
			case XFLR5::MAINWING:
				textureName = "wing_";
				break;
			case XFLR5::SECONDWING:
				textureName = "wing2_";
				break;
			case XFLR5::ELEVATOR:
				textureName = "elevator_";
				break;
			case XFLR5::FIN:
				textureName = "fin_";
				break;
			default:
				textureName="wing_";
			break;
		}
	}
	else
	{
		textureName="wing_";
	}

	QImage topLeftTexture;
	getTextureFile(planeName, textureName+"top_left", topLeftTexture);
	m_pWingTopLeftTexture[iWing] = new QOpenGLTexture(topLeftTexture);

	QImage botLeftTexture;
	getTextureFile(planeName, textureName+"bottom_left", botLeftTexture);
	m_pWingBotLeftTexture[iWing] = new QOpenGLTexture(botLeftTexture);

	QImage topRightTexture;
	getTextureFile(planeName, textureName+"top_right", topRightTexture);
	m_pWingTopRightTexture[iWing] = new QOpenGLTexture(topRightTexture);

	QImage botRightTexture;
	getTextureFile(planeName, textureName+"bottom_right", botRightTexture);
	m_pWingBotRightTexture[iWing] = new QOpenGLTexture(botRightTexture);

	m_vboWingSurface[iWing].destroy();
	m_vboWingSurface[iWing].create();
	m_vboWingSurface[iWing].bind();
	m_vboWingSurface[iWing].allocate(wingVertexArray, bufferSize * sizeof(GLfloat));
	m_vboWingSurface[iWing].release();


	//make OUTLINE
	//vertices array size:
	// surface:
	//     pWing->NSurfaces
	//     x(CHORDPOINTS-1)*2 : segments from i to i+1, times two vertices
	//                          so that we can make only one call to GL_LINES later on
	//     x2  for A and B sides
	//     x2  for top and bottom
	// flaps
	m_iWingOutlinePoints[iWing]  = pWing->m_Surface.count()*(CHORDPOINTS-1)*2*2*2;

	// outline
	//     2 points mLA & mLB for leading edge
	//     2 points mTA & mTB for trailing edge
	m_iWingOutlinePoints[iWing] += pWing->m_Surface.size()*2*2;

	//TE flap outline....
	for (j=0; j<pWing->m_Surface.size(); j++)
	{
		Foil *pFoilA = pWing->m_Surface[j]->m_pFoilA;
		Foil *pFoilB = pWing->m_Surface[j]->m_pFoilB;
		if(pFoilA && pFoilB && pFoilA->m_bTEFlap && pFoilB->m_bTEFlap)
		{
			m_iWingOutlinePoints[iWing] += 4;//two vertices for the top line and two for the bottom line
		}
	}
	//LE flap outline....
	for (j=0; j<pWing->m_Surface.size(); j++)
	{
		Foil *pFoilA = pWing->m_Surface[j]->m_pFoilA;
		Foil *pFoilB = pWing->m_Surface[j]->m_pFoilB;
		if(pFoilA && pFoilB && pFoilA->m_bLEFlap && pFoilB->m_bLEFlap)
		{
			m_iWingOutlinePoints[iWing] += 4;//two vertices for the top line and two for the bottom line
		}
	}

	// x3  : for 3 vertex components
	float *wingOutlineVertexArray = new float[m_iWingOutlinePoints[iWing] * 3];

	iv=0; //index of vertex components

	//SECTIONS OUTLINE
	for (j=0; j<pWing->m_Surface.size(); j++)
	{
		//top surface
		//left side vertices
		for (l=0; l<CHORDPOINTS-1; l++)
		{
			wingOutlineVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS+l].x;
			wingOutlineVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS+l].y;
			wingOutlineVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS+l].z;
			wingOutlineVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS+l+1].x;
			wingOutlineVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS+l+1].y;
			wingOutlineVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS+l+1].z;
		}
		//right side vertices
		for (l=0; l<CHORDPOINTS-1; l++)
		{
			wingOutlineVertexArray[iv++] = PtTopRight[j*CHORDPOINTS+l].x;
			wingOutlineVertexArray[iv++] = PtTopRight[j*CHORDPOINTS+l].y;
			wingOutlineVertexArray[iv++] = PtTopRight[j*CHORDPOINTS+l].z;
			wingOutlineVertexArray[iv++] = PtTopRight[j*CHORDPOINTS+l+1].x;
			wingOutlineVertexArray[iv++] = PtTopRight[j*CHORDPOINTS+l+1].y;
			wingOutlineVertexArray[iv++] = PtTopRight[j*CHORDPOINTS+l+1].z;
		}


		//bottom surface

		//left side vertices
		for (l=0; l<CHORDPOINTS-1; l++)
		{
			wingOutlineVertexArray[iv++] = PtBotLeft[j*CHORDPOINTS+l].x;
			wingOutlineVertexArray[iv++] = PtBotLeft[j*CHORDPOINTS+l].y;
			wingOutlineVertexArray[iv++] = PtBotLeft[j*CHORDPOINTS+l].z;
			wingOutlineVertexArray[iv++] = PtBotLeft[j*CHORDPOINTS+l+1].x;
			wingOutlineVertexArray[iv++] = PtBotLeft[j*CHORDPOINTS+l+1].y;
			wingOutlineVertexArray[iv++] = PtBotLeft[j*CHORDPOINTS+l+1].z;
		}

		//right side vertices
		for (l=0; l<CHORDPOINTS-1; l++)
		{
			wingOutlineVertexArray[iv++] = PtBotRight[j*CHORDPOINTS+l].x;
			wingOutlineVertexArray[iv++] = PtBotRight[j*CHORDPOINTS+l].y;
			wingOutlineVertexArray[iv++] = PtBotRight[j*CHORDPOINTS+l].z;
			wingOutlineVertexArray[iv++] = PtBotRight[j*CHORDPOINTS+l+1].x;
			wingOutlineVertexArray[iv++] = PtBotRight[j*CHORDPOINTS+l+1].y;
			wingOutlineVertexArray[iv++] = PtBotRight[j*CHORDPOINTS+l+1].z;
		}

		//Leading edge
		wingOutlineVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS+0].x;
		wingOutlineVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS+0].y;
		wingOutlineVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS+0].z;
		wingOutlineVertexArray[iv++] = PtTopRight[j*CHORDPOINTS+0].x;
		wingOutlineVertexArray[iv++] = PtTopRight[j*CHORDPOINTS+0].y;
		wingOutlineVertexArray[iv++] = PtTopRight[j*CHORDPOINTS+0].z;

		//trailing edge
		wingOutlineVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS + CHORDPOINTS-1].x;
		wingOutlineVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS + CHORDPOINTS-1].y;
		wingOutlineVertexArray[iv++] = PtTopLeft[j*CHORDPOINTS + CHORDPOINTS-1].z;
		wingOutlineVertexArray[iv++] = PtTopRight[j*CHORDPOINTS + CHORDPOINTS-1].x;
		wingOutlineVertexArray[iv++] = PtTopRight[j*CHORDPOINTS + CHORDPOINTS-1].y;
		wingOutlineVertexArray[iv++] = PtTopRight[j*CHORDPOINTS + CHORDPOINTS-1].z;
	}

	//TE flap outline....
	for (j=0; j<pWing->m_Surface.size(); j++)
	{
		Foil *pFoilA = pWing->m_Surface[j]->m_pFoilA;
		Foil *pFoilB = pWing->m_Surface[j]->m_pFoilB;
		if(pFoilA && pFoilB && pFoilA->m_bTEFlap && pFoilB->m_bTEFlap)
		{
			pWing->m_Surface[j]->getSurfacePoint(pWing->m_Surface[j]->m_pFoilA->m_TEXHinge/100.0,
												 pWing->m_Surface[j]->m_pFoilA->m_TEXHinge/100.0,
												 0.0, TOPSURFACE, Pt, N);
			wingOutlineVertexArray[iv++] = Pt.x;
			wingOutlineVertexArray[iv++] = Pt.y;
			wingOutlineVertexArray[iv++] = Pt.z;

			pWing->m_Surface[j]->getSurfacePoint(pWing->m_Surface[j]->m_pFoilB->m_TEXHinge/100.0,
												 pWing->m_Surface[j]->m_pFoilB->m_TEXHinge/100.0,
												 1.0, TOPSURFACE, Pt, N);
			wingOutlineVertexArray[iv++] = Pt.x;
			wingOutlineVertexArray[iv++] = Pt.y;
			wingOutlineVertexArray[iv++] = Pt.z;


			pWing->m_Surface[j]->getSurfacePoint(pWing->m_Surface[j]->m_pFoilA->m_TEXHinge/100.0,
												 pWing->m_Surface[j]->m_pFoilA->m_TEXHinge/100.0,
												 0.0, BOTSURFACE, Pt, N);
			wingOutlineVertexArray[iv++] = Pt.x;
			wingOutlineVertexArray[iv++] = Pt.y;
			wingOutlineVertexArray[iv++] = Pt.z;


			pWing->m_Surface[j]->getSurfacePoint(pWing->m_Surface[j]->m_pFoilB->m_TEXHinge/100.0,
												 pWing->m_Surface[j]->m_pFoilB->m_TEXHinge/100.0,
												 1.0, BOTSURFACE, Pt, N);
			wingOutlineVertexArray[iv++] = Pt.x;
			wingOutlineVertexArray[iv++] = Pt.y;
			wingOutlineVertexArray[iv++] = Pt.z;
		}
	}
	//LE flap outline....
	for (j=0; j<pWing->m_Surface.size(); j++)
	{
		Foil *pFoilA = pWing->m_Surface[j]->m_pFoilA;
		Foil *pFoilB = pWing->m_Surface[j]->m_pFoilB;
		if(pFoilA && pFoilB && pFoilA->m_bLEFlap && pFoilB->m_bLEFlap)
		{
			pWing->m_Surface[j]->getSurfacePoint(pWing->m_Surface[j]->m_pFoilA->m_LEXHinge/100.0,
												 pWing->m_Surface[j]->m_pFoilA->m_LEXHinge/100.0,
												 0.0, TOPSURFACE, Pt, N);
			wingOutlineVertexArray[iv++] = Pt.x;
			wingOutlineVertexArray[iv++] = Pt.y;
			wingOutlineVertexArray[iv++] = Pt.z;

			pWing->m_Surface[j]->getSurfacePoint(pWing->m_Surface[j]->m_pFoilB->m_LEXHinge/100.0,
												 pWing->m_Surface[j]->m_pFoilB->m_LEXHinge/100.0,
												 1.0, TOPSURFACE, Pt, N);
			wingOutlineVertexArray[iv++] = Pt.x;
			wingOutlineVertexArray[iv++] = Pt.y;
			wingOutlineVertexArray[iv++] = Pt.z;

			pWing->m_Surface[j]->getSurfacePoint(pWing->m_Surface[j]->m_pFoilA->m_LEXHinge/100.0,
												 pWing->m_Surface[j]->m_pFoilA->m_LEXHinge/100.0,
												 0.0, BOTSURFACE, Pt, N);
			wingOutlineVertexArray[iv++] = Pt.x;
			wingOutlineVertexArray[iv++] = Pt.y;
			wingOutlineVertexArray[iv++] = Pt.z;

			pWing->m_Surface[j]->getSurfacePoint(pWing->m_Surface[j]->m_pFoilB->m_LEXHinge/100.0,
												 pWing->m_Surface[j]->m_pFoilB->m_LEXHinge/100.0,
												 1.0, BOTSURFACE, Pt, N);
			wingOutlineVertexArray[iv++] = Pt.x;
			wingOutlineVertexArray[iv++] = Pt.y;
			wingOutlineVertexArray[iv++] = Pt.z;
		}
	}

	Q_ASSERT(iv==m_iWingOutlinePoints[iWing] * 3);

	m_vboWingOutline[iWing].destroy();
	m_vboWingOutline[iWing].create();
	m_vboWingOutline[iWing].bind();
	m_vboWingOutline[iWing].allocate(wingOutlineVertexArray, m_iWingOutlinePoints[iWing] * 3 * sizeof(GLfloat));
	m_vboWingOutline[iWing].release();

	delete[] wingOutlineVertexArray;

	delete[] wingVertexArray;
	delete[] PtTopLeft;
	delete[] PtTopRight;
	delete[] PtBotLeft;
	delete[] PtBotRight;
	delete[] NormalA;
	delete[] NormalB;
	delete[] leftV;
	delete[] rightV;
}


void gl3dView::getTextureFile(QString planeName, QString surfaceName, QImage &textureImage)
{
	QString projectPath = Settings::s_LastDirName + QDir::separator() + MainFrame::s_ProjectName+ "_textures";
	QString texturePath = projectPath+QDir::separator()+planeName+QDir::separator()+surfaceName;

	textureImage =  QImage(QString(texturePath+".png"));
	if(textureImage.isNull())
	{
		textureImage  = QImage(QString(texturePath+".jpg"));
		if(textureImage.isNull())
		{
			textureImage  = QImage(QString(texturePath+".jpeg"));
			if(textureImage.isNull())
			{
				textureImage = QImage(QString(":/default_textures/"+surfaceName+".png"));
			}
		}
	}
}


/** Default mesh, if no polar has been defined */
void gl3dView::glMakeWingEditMesh(QOpenGLBuffer &vbo, Wing *pWing)
{
	int l,k;
	//not necessarily the same Nx for all surfaces, so we need to count the quad panels
	int bufferSize = 0;
	for (int j=0; j<pWing->m_Surface.size(); j++)
	{
		Surface *pSurf = pWing->m_Surface[j];
		//tip patches
		if(pSurf->isTipLeft())  bufferSize += (pSurf->NXPanels());
		if(pSurf->isTipRight()) bufferSize += (pSurf->NXPanels());

		// top and bottom surfaces
		bufferSize += pSurf->NXPanels()*2 * (pSurf->NYPanels());
	}
	bufferSize *=2;    // 2 triangles/quad
	bufferSize *=3;    // 3 vertex for each triangle
	bufferSize *=3;    // 3 components for each node

	std::vector<float> meshVertexArray(bufferSize);

	int iv=0;

	Vector3d A,B,C,D;

	//tip patches
	for (int j=0; j<pWing->m_Surface.size(); j++)
	{
		Surface *pSurf = pWing->m_Surface[j];
		if(pSurf->isTipLeft())
		{
			for (l=0; l<pSurf->NXPanels(); l++)
			{
				pSurf->getPanel(0,l,TOPSURFACE);
				A = pSurf->TA;
				B = pSurf->LA;
				pSurf->getPanel(0,l,BOTSURFACE);
				C = pSurf->LA;
				D = pSurf->TA;

				//first triangle
				meshVertexArray[iv++] = A.x;
				meshVertexArray[iv++] = A.y;
				meshVertexArray[iv++] = A.z;
				meshVertexArray[iv++] = B.x;
				meshVertexArray[iv++] = B.y;
				meshVertexArray[iv++] = B.z;
				meshVertexArray[iv++] = C.x;
				meshVertexArray[iv++] = C.y;
				meshVertexArray[iv++] = C.z;

				//second triangle
				meshVertexArray[iv++] = C.x;
				meshVertexArray[iv++] = C.y;
				meshVertexArray[iv++] = C.z;
				meshVertexArray[iv++] = D.x;
				meshVertexArray[iv++] = D.y;
				meshVertexArray[iv++] = D.z;
				meshVertexArray[iv++] = A.x;
				meshVertexArray[iv++] = A.y;
				meshVertexArray[iv++] = A.z;
			}
		}
		if(pSurf->isTipRight())
		{
			for (l=0; l<pSurf->NXPanels(); l++)
			{
				pSurf->getPanel(pSurf->NYPanels()-1,l,TOPSURFACE);
				A = pSurf->TB;
				B = pSurf->LB;
				pSurf->getPanel(pSurf->NYPanels()-1,l,BOTSURFACE);
				C = pSurf->LB;
				D = pSurf->TB;

				//first triangle
				meshVertexArray[iv++] = C.x;
				meshVertexArray[iv++] = C.y;
				meshVertexArray[iv++] = C.z;
				meshVertexArray[iv++] = B.x;
				meshVertexArray[iv++] = B.y;
				meshVertexArray[iv++] = B.z;
				meshVertexArray[iv++] = A.x;
				meshVertexArray[iv++] = A.y;
				meshVertexArray[iv++] = A.z;

				//second triangle
				meshVertexArray[iv++] = A.x;
				meshVertexArray[iv++] = A.y;
				meshVertexArray[iv++] = A.z;
				meshVertexArray[iv++] = D.x;
				meshVertexArray[iv++] = D.y;
				meshVertexArray[iv++] = D.z;
				meshVertexArray[iv++] = C.x;
				meshVertexArray[iv++] = C.y;
				meshVertexArray[iv++] = C.z;
			}
		}
	}

	//background surface
	for (int j=0; j<pWing->m_Surface.size(); j++)
	{
		Surface *pSurf = pWing->m_Surface[j];
		for(k=0; k<pSurf->NYPanels(); k++)
		{
			for (l=0; l<pSurf->NXPanels(); l++)
			{
				pSurf->getPanel(k,l,TOPSURFACE);

				// first triangle
				meshVertexArray[iv++] = pSurf->LA.x;
				meshVertexArray[iv++] = pSurf->LA.y;
				meshVertexArray[iv++] = pSurf->LA.z;
				meshVertexArray[iv++] = pSurf->TA.x;
				meshVertexArray[iv++] = pSurf->TA.y;
				meshVertexArray[iv++] = pSurf->TA.z;
				meshVertexArray[iv++] = pSurf->TB.x;
				meshVertexArray[iv++] = pSurf->TB.y;
				meshVertexArray[iv++] = pSurf->TB.z;

				//second triangle
				meshVertexArray[iv++] = pSurf->TB.x;
				meshVertexArray[iv++] = pSurf->TB.y;
				meshVertexArray[iv++] = pSurf->TB.z;
				meshVertexArray[iv++] = pSurf->LB.x;
				meshVertexArray[iv++] = pSurf->LB.y;
				meshVertexArray[iv++] = pSurf->LB.z;
				meshVertexArray[iv++] = pSurf->LA.x;
				meshVertexArray[iv++] = pSurf->LA.y;
				meshVertexArray[iv++] = pSurf->LA.z;
			}

			for (l=0; l<pSurf->NXPanels(); l++)
			{
				pSurf->getPanel(k,l,BOTSURFACE);
				//first triangle
				meshVertexArray[iv++] = pSurf->TB.x;
				meshVertexArray[iv++] = pSurf->TB.y;
				meshVertexArray[iv++] = pSurf->TB.z;
				meshVertexArray[iv++] = pSurf->TA.x;
				meshVertexArray[iv++] = pSurf->TA.y;
				meshVertexArray[iv++] = pSurf->TA.z;
				meshVertexArray[iv++] = pSurf->LA.x;
				meshVertexArray[iv++] = pSurf->LA.y;
				meshVertexArray[iv++] = pSurf->LA.z;

				//second triangle
				meshVertexArray[iv++] = pSurf->LA.x;
				meshVertexArray[iv++] = pSurf->LA.y;
				meshVertexArray[iv++] = pSurf->LA.z;
				meshVertexArray[iv++] = pSurf->LB.x;
				meshVertexArray[iv++] = pSurf->LB.y;
				meshVertexArray[iv++] = pSurf->LB.z;
				meshVertexArray[iv++] = pSurf->TB.x;
				meshVertexArray[iv++] = pSurf->TB.y;
				meshVertexArray[iv++] = pSurf->TB.z;
			}
		}
	}


	Q_ASSERT(iv==bufferSize);


	Q_ASSERT(iv==bufferSize);

//	m_iWingMeshElems = ii/3;
	vbo.destroy();
	vbo.create();
	vbo.bind();
	vbo.allocate(meshVertexArray.data(), bufferSize * sizeof(GLfloat));
	vbo.release();
}




void gl3dView::glMakeWingSectionHighlight(Wing *pWing, int iSectionHighLight, bool bRightSide)
{
	Vector3d Point, Normal;

	int CHORDPOINTS = W3dPrefsDlg::chordwiseRes();
	int iSection = 0;
	int jSurf = 0;
	for(int jSection=0; jSection<pWing->NWingSection(); jSection++)
	{
		if(jSection==iSectionHighLight) break;
		if(jSection<pWing->NWingSection()-1 &&	fabs(pWing->YPosition(jSection+1)-pWing->YPosition(jSection)) > Wing::s_MinPanelSize)
			iSection++;
	}

	m_HighlightLineSize = CHORDPOINTS * 2;
	int bufferSize = m_HighlightLineSize *2 *3 ;
	float *pHighlightVertexArray = new float[bufferSize];

	m_nHighlightLines = 0;
	int iv=0;
	if(iSection==0)
	{
		m_nHighlightLines++;
		//define the inner left side surface
		if(!pWing->isFin())  jSurf = pWing->m_Surface.size()/2 - 1;
		else                 jSurf = pWing->m_Surface.size()   - 1;

		for (int lx=0; lx<CHORDPOINTS; lx++)
		{
			double xRel = (double)lx/(double)(CHORDPOINTS-1);
			pWing->m_Surface.at(jSurf)->getSidePoint(xRel, true, TOPSURFACE, Point, Normal);
			pHighlightVertexArray[iv++] = Point.x;
			pHighlightVertexArray[iv++] = Point.y;
			pHighlightVertexArray[iv++] = Point.z;
		}
		for (int lx=CHORDPOINTS-1; lx>=0; lx--)
		{
			double xRel = (double)lx/(double)(CHORDPOINTS-1);
			pWing->m_Surface.at(jSurf)->getSidePoint(xRel, true, BOTSURFACE, Point, Normal);
			pHighlightVertexArray[iv++] = Point.x;
			pHighlightVertexArray[iv++] = Point.y;
			pHighlightVertexArray[iv++] = Point.z;
		}
	}
	else
	{
		if((pWing->m_bSymetric || bRightSide) && !pWing->m_bIsFin)
		{
			m_nHighlightLines++;
			jSurf = pWing->m_Surface.size()/2 + iSection -1;

			for (int lx=0; lx<CHORDPOINTS; lx++)
			{
				double xRel = (double)lx/(double)(CHORDPOINTS-1);
				pWing->m_Surface.at(jSurf)->getSidePoint(xRel, true, TOPSURFACE, Point, Normal);
				pHighlightVertexArray[iv++] = Point.x;
				pHighlightVertexArray[iv++] = Point.y;
				pHighlightVertexArray[iv++] = Point.z;
			}
			for (int lx=CHORDPOINTS-1; lx>=0; lx--)
			{
				double xRel = (double)lx/(double)(CHORDPOINTS-1);
				pWing->m_Surface.at(jSurf)->getSidePoint(xRel, true, BOTSURFACE, Point, Normal);
				pHighlightVertexArray[iv++] = Point.x;
				pHighlightVertexArray[iv++] = Point.y;
				pHighlightVertexArray[iv++] = Point.z;
			}
		}

		if(pWing->m_bSymetric || !bRightSide)
		{
			m_nHighlightLines++;
			if(!pWing->m_bIsFin) jSurf = pWing->m_Surface.size()/2 - iSection;
			else                 jSurf = pWing->m_Surface.size()   - iSection;

			//plot A side outline
			for (int lx=0; lx<CHORDPOINTS; lx++)
			{
				double xRel = (double)lx/(double)(CHORDPOINTS-1);
				pWing->m_Surface.at(jSurf)->getSidePoint(xRel, false, TOPSURFACE, Point, Normal);
				pHighlightVertexArray[iv++] = Point.x;
				pHighlightVertexArray[iv++] = Point.y;
				pHighlightVertexArray[iv++] = Point.z;
			}

			for (int lx=CHORDPOINTS-1; lx>=0; lx--)
			{
				double xRel = (double)lx/(double)(CHORDPOINTS-1);
				pWing->m_Surface.at(jSurf)->getSidePoint(xRel, false, BOTSURFACE, Point, Normal);
				pHighlightVertexArray[iv++] = Point.x;
				pHighlightVertexArray[iv++] = Point.y;
				pHighlightVertexArray[iv++] = Point.z;
			}
		}
	}

	m_vboHighlight.destroy();
	m_vboHighlight.create();
	m_vboHighlight.bind();
	m_vboHighlight.allocate(pHighlightVertexArray, bufferSize*sizeof(float));
	m_vboHighlight.release();
	delete [] pHighlightVertexArray;
}



void gl3dView::glMakeBodyFrameHighlight(Body *pBody, Vector3d bodyPos, int iFrame)
{
//	int NXXXX = W3dPrefsDlg::bodyAxialRes();
	int NHOOOP = W3dPrefsDlg::bodyHoopRes();
	int k;
	Vector3d Point;
	double hinc, u, v;
	if(iFrame<0) return;

	Frame *pFrame = pBody->frame(iFrame);
//	xinc = 0.1;
	hinc = 1.0/(double)(NHOOOP-1);

	int bufferSize = 0;
	float *pHighlightVertexArray = NULL;

	m_nHighlightLines = 2; // left and right - could make one instead

	//create 3D Splines or Lines to overlay on the body
	int iv = 0;

	if(pBody->isFlatPanelType())
	{
		m_HighlightLineSize = pFrame->pointCount();
		bufferSize = m_nHighlightLines * m_HighlightLineSize *3 ;
		pHighlightVertexArray = new float[bufferSize];
		for (k=0; k<pFrame->pointCount();k++)
		{
			pHighlightVertexArray[iv++] = pFrame->m_Position.x+bodyPos.x;
			pHighlightVertexArray[iv++] = pFrame->m_CtrlPoint[k].y;
			pHighlightVertexArray[iv++] = pFrame->m_CtrlPoint[k].z+bodyPos.z;
		}

		for (k=0; k<pFrame->pointCount();k++)
		{
			pHighlightVertexArray[iv++] =  pFrame->m_Position.x+bodyPos.x;
			pHighlightVertexArray[iv++] = -pFrame->m_CtrlPoint[k].y;
			pHighlightVertexArray[iv++] =  pFrame->m_CtrlPoint[k].z+bodyPos.z;
		}
	}
	else if(pBody->isSplineType())
	{
		m_HighlightLineSize = NHOOOP;
		bufferSize = m_nHighlightLines * m_HighlightLineSize *3 ;
		pHighlightVertexArray = new float[bufferSize];

		if(pBody->activeFrame())
		{
			u = pBody->getu(pFrame->m_Position.x);
			v = 0.0;
			for (k=0; k<NHOOOP; k++)
			{
				pBody->getPoint(u,v,true, Point);
				pHighlightVertexArray[iv++] = Point.x+bodyPos.x;
				pHighlightVertexArray[iv++] = Point.y;
				pHighlightVertexArray[iv++] = Point.z+bodyPos.z;
				v += hinc;
			}

			v = 1.0;
			for (k=0; k<NHOOOP; k++)
			{
				pBody->getPoint(u,v,false, Point);
				pHighlightVertexArray[iv++] = Point.x+bodyPos.x;
				pHighlightVertexArray[iv++] = Point.y;
				pHighlightVertexArray[iv++] = Point.z+bodyPos.z;
				v -= hinc;
			}
		}
	}
	Q_ASSERT(iv==bufferSize);

	m_vboHighlight.destroy();
	m_vboHighlight.create();
	m_vboHighlight.bind();
	m_vboHighlight.allocate(pHighlightVertexArray, bufferSize*sizeof(float));
	m_vboHighlight.release();
	delete [] pHighlightVertexArray;
}







void gl3dView::onRotationIncrement()
{
	if(m_iTransitionInc>=30)
	{
		m_pTransitionTimer->stop();
		delete m_pTransitionTimer;
		m_pTransitionTimer = NULL;
		return;
	}
	for(int iq=0; iq<16; iq++)
	{
		m_ArcBall.ab_quat[iq] = ab_old[iq] + float(m_iTransitionInc)/29.0f * (ab_new[iq]-ab_old[iq]);
	}
	reset3DRotationCenter();
	update();
	m_iTransitionInc++;
}


void gl3dView::startRotationTimer()
{
	if(W3dPrefsDlg::s_bAnimateTransitions)
	{
		m_iTransitionInc = 0;
		if(m_pTransitionTimer) delete m_pTransitionTimer;
		m_pTransitionTimer = new QTimer(this);
		connect(m_pTransitionTimer, SIGNAL(timeout()), this, SLOT(onRotationIncrement()));
		m_pTransitionTimer->start(10);//33 ms x 30 times is approximately one second
	}
	else
	{
		reset3DRotationCenter();
		update();
	}
}


void gl3dView::startTranslationTimer(Vector3d PP)
{
	glInverseMatrix();
	if(W3dPrefsDlg::s_bAnimateTransitions)
	{
		m_transIncrement.x = (-PP.x -m_glRotCenter.x)/30.0;
		m_transIncrement.y = (-PP.y -m_glRotCenter.y)/30.0;
		m_transIncrement.z = (-PP.z -m_glRotCenter.z)/30.0;

		m_iTransitionInc = 0;
		if(m_pTransitionTimer) delete m_pTransitionTimer;
		m_pTransitionTimer = new QTimer(this);
		connect(m_pTransitionTimer, SIGNAL(timeout()), this, SLOT(onTranslationIncrement()));
		m_pTransitionTimer->start(10);//33 ms x 30 times is approximately one second
	}
	else
	{
		m_glRotCenter -= PP;
		m_glViewportTrans.x =  (MatOut[0][0]*m_glRotCenter.x + MatOut[0][1]*m_glRotCenter.y + MatOut[0][2]*m_glRotCenter.z);
		m_glViewportTrans.y = -(MatOut[1][0]*m_glRotCenter.x + MatOut[1][1]*m_glRotCenter.y + MatOut[1][2]*m_glRotCenter.z);
		m_glViewportTrans.z=   (MatOut[2][0]*m_glRotCenter.x + MatOut[2][1]*m_glRotCenter.y + MatOut[2][2]*m_glRotCenter.z);

		update();
	}
}



void gl3dView::onTranslationIncrement()
{
	if(m_iTransitionInc>=30)
	{
		m_pTransitionTimer->stop();
		delete m_pTransitionTimer;
		m_pTransitionTimer = NULL;
		return;
	}

	m_glRotCenter +=m_transIncrement;
	m_glViewportTrans.x =  (MatOut[0][0]*m_glRotCenter.x + MatOut[0][1]*m_glRotCenter.y + MatOut[0][2]*m_glRotCenter.z);
	m_glViewportTrans.y = -(MatOut[1][0]*m_glRotCenter.x + MatOut[1][1]*m_glRotCenter.y + MatOut[1][2]*m_glRotCenter.z);
	m_glViewportTrans.z=   (MatOut[2][0]*m_glRotCenter.x + MatOut[2][1]*m_glRotCenter.y + MatOut[2][2]*m_glRotCenter.z);


	update();
	m_iTransitionInc++;
}


/**
 * Sets an automatic scale for the wing or plane in the 3D view, depending on wing span.
 */
void gl3dView::set3DScale(double length)
{
	if(length>0.0) m_glScaledRef = (GLfloat)(4./5.*2.0/length);
	m_glScaled = m_glScaledRef;
	m_glViewportTrans.set(0.0, 0.0, 0.0);
	reset3DRotationCenter();
	update();
}


void gl3dView::onResetIncrement()
{
	if(m_iTransitionInc>=30)
	{
		m_pTransitionTimer->stop();
		delete m_pTransitionTimer;
		m_pTransitionTimer = NULL;
		return;
	}

	m_glScaled += m_glScaleIncrement;
	m_glViewportTrans += m_transIncrement;

	reset3DRotationCenter();
	update();
	m_iTransitionInc++;
}


void gl3dView::startResetTimer(double length)
{
	if(W3dPrefsDlg::s_bAnimateTransitions)
	{
		m_iTransitionInc = 0;

		m_glScaleIncrement = (m_glScaledRef-m_glScaled)/30.0f;
		m_transIncrement = (Vector3d(0.0,0.0,0.0)-m_glViewportTrans)/30.0;

		if(m_pTransitionTimer) delete m_pTransitionTimer;
		m_pTransitionTimer = new QTimer(this);
		connect(m_pTransitionTimer, SIGNAL(timeout()), this, SLOT(onResetIncrement()));
		m_pTransitionTimer->start(10);//33 ms x 30 times is approximately one second
	}
	else
	{
		set3DScale(length);
		update();
	}
}



/** Default mesh, if no polar has been defined */
void gl3dView::glMakeEditBodyMesh(Body *pBody, Vector3d BodyPosition)
{
	if(!pBody) return;
	int NXXXX = W3dPrefsDlg::bodyAxialRes();
	int NHOOOP = W3dPrefsDlg::bodyHoopRes();
	int nx, nh;
	Vector3d Pt;
	Vector3d P1, P2, P3, P4, PStart, PEnd;
	float *meshVertexArray = NULL;
	int bufferSize = 0;
	m_iBodyMeshLines = 0;

	double dx = BodyPosition.x;
	double dy = BodyPosition.y;
	double dz = BodyPosition.z;

	int iv=0;

	if(pBody->isFlatPanelType()) //LINES
	{
		bufferSize = 0;
		for (int j=0; j<pBody->frameCount()-1;j++)
		{
			for (int k=0; k<pBody->sideLineCount()-1;k++)
			{
				for(int jp=0; jp<=pBody->m_xPanels[j]; jp++)
				{
					bufferSize += 6;
				}
				for(int kp=0; kp<=pBody->m_hPanels[k]; kp++)
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

				P1.x+=dx; P2.x+=dx; P3.x+=dx; P4.x+=dx;
				P1.y+=dy; P2.y+=dy; P3.y+=dy; P4.y+=dy;
				P1.z+=dz; P2.z+=dz; P3.z+=dz; P4.z+=dz;

				//left side panels
				for(int jp=0; jp<=pBody->m_xPanels[j]; jp++)
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
				for(int kp=0; kp<=pBody->m_hPanels[k]; kp++)
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
				for(int jp=0; jp<=pBody->m_xPanels[j]; jp++)
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
				for(int kp=0; kp<=pBody->m_hPanels[k]; kp++)
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
				meshVertexArray[iv++] = Pt.x + dx;
				meshVertexArray[iv++] = Pt.y + dy;
				meshVertexArray[iv++] = Pt.z + dz;
			}
		}
		for (int l=0; l<nh; l++)
		{
			double v = (double)l/(double)(nh-1);
			for (int k=0; k<NXXXX; k++)
			{
				double u = (double)k/(double)(NXXXX-1);
				pBody->getPoint(u,  v, false, Pt);
				meshVertexArray[iv++] = Pt.x + dx;
				meshVertexArray[iv++] = Pt.y + dy;
				meshVertexArray[iv++] = Pt.z + dz;
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
				meshVertexArray[iv++] = Pt.x + dx;
				meshVertexArray[iv++] = Pt.y + dy;
				meshVertexArray[iv++] = Pt.z + dz;
			}
		}
		for (int k=0; k<nx; k++)
		{
			double uk = pBody->m_XPanelPos[k];
			for (int l=0; l<NHOOOP; l++)
			{
				double v = (double)l/(double)(NHOOOP-1);
				pBody->getPoint(uk,  v, false, Pt);
				meshVertexArray[iv++] = Pt.x + dx;
				meshVertexArray[iv++] = Pt.y + dy;
				meshVertexArray[iv++] = Pt.z + dz;
			}
		}
	}
	Q_ASSERT(iv==bufferSize);

	m_vboEditBodyMesh.destroy();
	m_vboEditBodyMesh.create();
	m_vboEditBodyMesh.bind();
	m_vboEditBodyMesh.allocate(meshVertexArray, bufferSize * sizeof(GLfloat));
	m_vboEditBodyMesh.release();

	delete[] meshVertexArray;
}

