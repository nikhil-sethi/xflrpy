/****************************************************************************

    gl3dMiarexView Class
    Copyright (C) André Deperrois

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


#pragma once


#include <xfl3d/views/gl3dxflview.h>

class gl3dMiarexView : public gl3dXflView
{
    public:
        gl3dMiarexView(QWidget *parent = nullptr);
        ~gl3dMiarexView();

    public:
        void glMake3dObjects() override;
        void glMakeCpLegendClr();
        bool glMakeStreamLines(const Wing *PlaneWing[], const Vector3d *pNode, const WPolar *pWPolar, const PlaneOpp *pPOpp);
        void glMakeSurfVelocities(Panel const *pPanel, const WPolar *pWPolar, PlaneOpp const *pPOpp, int nPanels);
        void glMakeTransitions(int iWing, const Wing *pWing, const WPolar *pWPolar, const WingOpp *pWOpp);
        void glMakeLiftStrip(int iWing, Wing const *pWing, WPolar const *pWPolar, WingOpp const*pWOpp);
        void glMakeLiftForce(WPolar const *pWPolar, PlaneOpp const *pPOpp);
        void glMakeMoments(Wing const *pWing, WPolar const *pWPolar, PlaneOpp const*pPOpp);
        void glMakeDownwash(int iWing, Wing const *pWing, WPolar const*pWPolar, WingOpp const*pWOpp);
        void glMakeDragStrip(int iWing, Wing const *pWing, WPolar const *pWPolar, WingOpp const *pWOpp, double beta);
        void glMakePanelForces(int nPanels, Panel const*pPanel, WPolar const*pWPolar, PlaneOpp const*pPOpp);
        void glMakePanels(QOpenGLBuffer &vbo, int nPanels, int nNodes, Vector3d const *pNode, Panel const *pPanel, PlaneOpp const *pPOpp);

        void paintLift(int iWing);
        void paintMoments();
        void paintDrag(int iWing);
        void paintDownwash(int iWing);
        void paintStreamLines();
        void paintSurfaceVelocities(int nPanels);
        void paintTransitions(int iWing);
        void paintPanelCp(int nPanels);
        void paintPanelForces(int nPanels);

    private:
        void glRenderView() override;
        void contextMenuEvent(QContextMenuEvent *pEvent) override;
        void paintOverlay() override;
        bool intersectTheObject(Vector3d const &AA,  Vector3d const &BB, Vector3d &I) override;
        void resizeGL(int width, int height) override;

    public:
        QOpenGLBuffer m_vboSurfaceVelocities, m_vboPanelCp, m_vboPanelForces, m_vboStreamLines;
        QOpenGLBuffer m_vboLiftForce, m_vboMoments;
        QOpenGLBuffer m_vboICd[MAXWINGS], m_vboVCd[MAXWINGS], m_vboLiftStrips[MAXWINGS], m_vboTransitions[MAXWINGS], m_vboDownwash[MAXWINGS];
        QOpenGLBuffer m_vboMesh, m_vboLegendColor;

        int m_NStreamLines;


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

        static bool s_bAutoCpScale;                  /**< true if the Cp scale should be set automatically */
        static double s_LegendMin;                /**< minimum value of the Cp scale in 3D view */
        static double s_LegendMax;                /**< maximum value of the Cp scale in 3D view */


        bool m_bSurfVelocitiesDone;
        bool m_bStreamlinesDone;

        bool m_bSurfVelocities;            /**< true if the velocities should be displayed in the operating point or 3D view*/
        bool m_bStream;                    /**< true if the streamlines should be displayed in the operating point or 3D view*/

};


