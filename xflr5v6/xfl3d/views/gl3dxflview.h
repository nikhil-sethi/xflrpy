/****************************************************************************

    gl3dXflView Class
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

#include <xfl3d/views/gl3dview.h>

#include <xflanalysis/analysis3d_params.h>

class Miarex;
class MainFrame;
class Plane;
class Body;
class Wing;
class WingOpp;
class WPolar;
class PlaneOpp;
class Panel;
class PointMass;

class gl3dXflView : public gl3dView
{
    Q_OBJECT

    friend class MainFrame;
    friend class Miarex;
    friend class WingDlg;
    friend class BodyDlg;
    friend class EditBodyDlg;
    friend class EditPlaneDlg;
    friend class PlaneDlg;

    public:
        gl3dXflView(QWidget *pParent = nullptr);
        ~gl3dXflView();

    protected:
        void glMakeWingEditMesh(QOpenGLBuffer &vbo, const Wing *pWing);
        void glMakeFuseFlatPanels(const Body *pBody);
        void glMakeFuseFlatPanelsOutline(const Body *pBody);
        void glMakeFuseSplines(Body const *pBody);
        void glMakeFuseSplinesOutline(const Body *pBody);
        void glMakeBodyFrameHighlight(Body const *pBody, Vector3d const&bodyPos, int iFrame);
        void glMakeEditBodyMesh(Body *pBody, const Vector3d &pos);

        void glMakeWingSurface(Wing const *pWing, Body const *pBody, QOpenGLBuffer &vboSurf) const;
        void glMakeWingOutline(Wing const *pWing, Body const *pBody, QOpenGLBuffer &vboOutline) const;

        void paintMasses(double volumeMass, const Vector3d &pos, const QString &tag, const QVector<PointMass> &ptMasses);
        void paintMasses(double volumeMass, Vector3d const&pos, QString const&tag, QVector<PointMass *> const &ptMasses);
        void paintMasses(Plane const *pPlane);
        void paintEditWingMesh(QOpenGLBuffer &vbo);
        void paintSectionHighlight();
        void paintFoilNames(const Wing *pWing);
        void paintNormals(QOpenGLBuffer &vbo);
        void paintMesh(QOpenGLBuffer &vbo, bool bBackGround);
        void setSpanStations(Plane const *pPlane, WPolar const *pWPolar, PlaneOpp const *pPOpp);

    private:
        void enterEvent(QEvent *pEvent) override;

    protected slots:
        void onSurfaces(  bool bChecked);
        void onPanels(    bool bChecked);
        void onOutline(   bool bChecked);
        void onFoilNames( bool bChecked);
        void onShowMasses(bool bChecked);

    protected:

        QOpenGLBuffer m_vboWingSurface[MAXWINGS], m_vboWingOutline[MAXWINGS];
        QOpenGLBuffer m_vboEditWingMesh[MAXWINGS];
        QOpenGLBuffer m_vboEditBodyMesh;
        QOpenGLBuffer m_vboHighlight;
        QOpenGLBuffer m_vboNormals;

        QOpenGLBuffer m_vboBody;
        QOpenGLBuffer m_vboFuseLeft, m_vboFuseRight, m_vboFuseOutline;

        int m_Ny[MAXWINGS];
        int m_nHighlightLines, m_HighlightLineSize;

        int m_iMomentPoints;

        bool m_bOutline;                   /**< true if the surface outlines are to be displayed in the 3D view*/
        bool m_bSurfaces;                  /**< true if the surfaces are to be displayed in the 3D view*/
        bool m_bVLMPanels;                 /**< true if the panels are to be displayed in the 3D view*/
        bool m_bShowMasses;                /**< true if the point masses are to be displayed on the openGL 3D view */
        bool m_bFoilNames;                 /**< true if the foil names are to be displayed on the openGL 3D view */

        static Miarex *s_pMiarex;     /**< A void pointer to the instance of the QMiarex widget.*/
        static MainFrame *s_pMainFrame;  /**< A void pointer to the instance of the MainFrame widget.*/

};

