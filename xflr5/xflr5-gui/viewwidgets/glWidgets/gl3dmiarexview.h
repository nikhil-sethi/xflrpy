/****************************************************************************

    gl3dMiarexView Class
    Copyright (C) 2003-2019 Andre Deperrois

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


#ifndef GL3DMIAREXVIEW_H
#define GL3DMIAREXVIEW_H


#include <gl3dview.h>

class gl3dMiarexView : public gl3dView
{
public:
    gl3dMiarexView(QWidget *parent = nullptr);
	~gl3dMiarexView();

private:
	void glRenderView();
	void contextMenuEvent (QContextMenuEvent * event);
	void paintGL();
	void paintOverlay();
	void set3DRotationCenter(QPoint point);
	void resizeGL(int width, int height);

public:
    void glMake3dObjects();
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
	void glMakePanels(QOpenGLBuffer &vbo, int nPanels, int nNodes, Vector3d *pNode, Panel *pPanel, PlaneOpp *pPOpp);

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
	void paintMesh(int nPanels);

public slots:
	void on3DReset();

public:
	QOpenGLBuffer m_vboSurfaceVelocities, m_vboPanelCp, m_vboPanelForces, m_vboStreamLines;
	QOpenGLBuffer m_vboLiftForce, m_vboMoments;
	QOpenGLBuffer m_vboICd[MAXWINGS], m_vboVCd[MAXWINGS], m_vboLiftStrips[MAXWINGS], m_vboTransitions[MAXWINGS], m_vboDownwash[MAXWINGS];
	QOpenGLBuffer m_vboMesh, m_vboLegendColor;

    uint m_NStreamLines;


    static bool s_bResetglGeom;               /**< true if the geometry OpenGL list needs to be re-generated */
    static bool s_bResetglMesh;               /**< true if the mesh OpenGL list needs to be re-generated */
    static bool s_bResetglWake;               /**< true if the wake OpenGL list needs to be re-generated */
    static bool s_bResetglOpp;                /**< true if the OpenGL lists need to be re-generated */
    static bool s_bResetglLift;               /**< true if the OpenGL lists need to be re-generated */
    static bool s_bResetglDrag;               /**< true if the OpenGL lists need to be re-generated */
    static bool s_bResetglDownwash;           /**< true if the OpenGL lists need to be re-generated */
    static bool s_bResetglPanelForce;         /**< true if the OpenGL lists need to be re-generated */
    static bool s_bResetglPanelCp;            /**< true if the OpenGL lists need to be re-generated */
    static bool s_bResetglStream;             /**< true if the streamlines OpenGL list needs to be re-generated */
    static bool s_bResetglLegend;             /**< true if the legend needs to be reset if the window has been resized */
    static bool s_bResetglBody;               /**< true if the openGL list for the body needs to be re-generated */
    static bool s_bResetglSurfVelocities;     /**< true if the crossflow OpenGL list needs to be refreshed */

    static double s_LiftScale;                /**< scaling factor for the lift display in 3D view */
    static double s_VelocityScale;            /**< scaling factor for the velocity display in 3D view */
    static double s_DragScale;                /**< scaling factor for the drag display in 3D view */

    static bool s_bAutoCpScale;		          /**< true if the Cp scale should be set automatically */
    static double s_LegendMin;                /**< minimum value of the Cp scale in 3D view */
    static double s_LegendMax;                /**< maximum value of the Cp scale in 3D view */


    bool m_bSurfVelocitiesDone;
    bool m_bStreamlinesDone;

    bool m_bSurfVelocities;            /**< true if the velocities should be displayed in the operating point or 3D view*/
    bool m_bStream;                    /**< true if the streamlines should be displayed in the operating point or 3D view*/

};

#endif // GL3DMIAREXVIEW_H
