#ifndef GL3DMIAREXVIEW_H
#define GL3DMIAREXVIEW_H


#include <gl3dview.h>

class gl3dMiarexView : public gl3dView
{
public:
	gl3dMiarexView(QWidget *parent = 0);
	~gl3dMiarexView();

private:
	void glRenderView();
	void contextMenuEvent (QContextMenuEvent * event);
	void paintGL();
	void paintOverlay();
	void set3DRotationCenter(QPoint point);
	void resizeGL(int width, int height);

public:
	void glMakeCpLegendClr();
	bool glMakeStreamLines(Wing *PlaneWing[MAXWINGS], Vector3d *pNode, WPolar *pWPolar, PlaneOpp *pPOpp, int nPanels);
	void glMakeSurfVelocities(Panel *pPanel, WPolar *pWPolar, PlaneOpp *pPOpp, int nPanels);
	void glMakeTransitions(int iWing, Wing *pWing, WPolar *pWPolar, WingOpp *pWOpp);
	void glMakeLiftStrip(int iWing, Wing *pWing, WPolar *pWPolar, WingOpp *pWOpp);
	void glMakeLiftForce(WPolar *pWPolar, PlaneOpp *pPOpp);
	void glMakeMoments(Wing *pWing, WPolar *pWPolar, PlaneOpp *pPOpp);
	void glMakeDownwash(int iWing, Wing *pWing, WPolar *pWPolar, WingOpp *pWOpp);
	void glMakeDragStrip(int iWing, Wing *pWing, WPolar *pWPolar, WingOpp *pWOpp, double beta);
	void glMakePanelForces(int nPanels, Panel *pPanel, WPolar *pWPolar, PlaneOpp *pPOpp);

	void paintLift(int iWing);
	void paintMoments();
	void paintDrag(int iWing);
	void paintDownwash(int iWing);
	void paintStreamLines();
	void paintSurfaceVelocities(int nPanels);
	void paintTransitions(int iWing);
	void paintCpLegendClr();
	void paintPanelCp(int nPanels);
	void paintPanelForces(int nPanels);

public slots:
	void on3DReset();

public:
	QOpenGLBuffer m_vboSurfaceVelocities, m_vboPanelCp, m_vboPanelForces, m_vboStreamLines;
	QOpenGLBuffer m_vboLiftForce, m_vboMoments;
	QOpenGLBuffer m_vboICd[MAXWINGS], m_vboVCd[MAXWINGS], m_vboLiftStrips[MAXWINGS], m_vboTransitions[MAXWINGS], m_vboDownwash[MAXWINGS];

};

#endif // GL3DMIAREXVIEW_H
