/****************************************************************************

    Panel Class
    Copyright (C) 2André Deperrois

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




/**
 *@file
 *
 * This file defines the classes for quad panel object used both in VLM and in 3d-panel analysis
 *
 */

#pragma once



#include <xflcore/core_enums.h>
#include <xflgeom/geom3d/quaternion.h>
#include <xflgeom/geom3d/vector3d.h>


/**
*@class Panel
*@brief    This class defines the quad panel object used both in VLM and in 3d-panel analysis

*    The class provides member variables which define the geometric properties of the panel, and functions used in the 3D analysis.
*
*    The name of the variables follows closely the naming used in the document NASA Contractor report 4023 "Program VSAERO Theory Document".
    Refer to this document for detailed explanations on the description of the panel and the meaning of the variables.
    The nodes are defined in a separate global array. The index of the nodes at the four corners are stored as
    member variables of this panel.
*
*    For VLM calculations, the position and length vector of the bound vortex at the panel's quarter-chord are
    stored as member variables.
*/
class Panel
{
    friend class Surface;
    friend class Wing;
    friend class Miarex;
    friend class Objects3D;
    friend class Body;
    friend class PlaneTask;
    friend class PanelAnalysis;
    friend class PanelAnalysisDlg;
    friend class GL3dBodyDlg;
    friend class GL3dWingDlg;
    friend class gl3dSurfacePlot;

    public:
        Panel();

        void VLMCmn(Vector3d const &C, Vector3d &VTest, bool const &bAll);
        void doubletNASA4023(Vector3d const &C, Vector3d &VTest, double &phi, bool bWake) const;
        void sourceNASA4023(Vector3d const &C, Vector3d &VTest, double &phi) const;

        void rotateBC(Vector3d const &HA, Quaternion & Qt);
        void reset();
        void setPanelFrame();
        void setPanelFrame(Vector3d const &LA, Vector3d const &LB, Vector3d const &TA, Vector3d const &TB);
        bool intersect(Vector3d const &A, Vector3d const &U, Vector3d &I, double &dist);
        bool invert33(double *l);
        void globalToLocal(Vector3d const &V, Vector3d &VLocal);
        Vector3d globalToLocal(Vector3d const &VTest);
        Vector3d globalToLocal(double const &Vx, double const &Vy, double const &Vz);
        Vector3d localToGlobal(Vector3d const &VTest);

        double width() const;
        double area() const {return Area;}
        Vector3d ctrlPt() const {return CtrlPt;}
        Vector3d collPt() const {return CollPt;}
        Vector3d normal() const {return Normal;}

        bool isTopSurface()  const {return m_Pos==xfl::TOPSURFACE;}
        bool isBotSurface()  const {return m_Pos==xfl::BOTSURFACE;}
        bool isMidSurface()  const {return m_Pos==xfl::MIDSURFACE;}
        bool isSideSurface() const {return m_Pos==xfl::SIDESURFACE;}
        bool isBodySurface() const {return m_Pos==xfl::BODYSURFACE;}

        void printPanel();

        static void setCoreSize(double CoreSize) { s_CoreSize=CoreSize;    }
        static double coreSize() { return s_CoreSize; }


    protected:
        bool m_bIsInSymPlane;    /**< true if the panel lies in the plane's xz plane of symetry at y=0*/
        bool m_bIsLeftPanel;     /**< true if the panel lies on the left (port) wing */
        bool m_bIsWakePanel;     /**< true if the panel lies on the wake of a winf */

        int m_iElement;          /**< panel identification number ; used when the panel array is re-arranged in non sequential order to reduce the matrix size in symetrical calculations */
        int m_iPL;               /**< index of the panel which lies left of this panel, or -1 if none */
        int m_iPR;               /**< index of the panel which lies right of this panel, or -1 if none */
        int m_iPU;               /**< index of the panel which lies upstream of this panel, or -1 if none */
        int m_iPD;               /**< index of the panel which lies downstream of this panel, or -1 if none */
        int m_iWake;             /**< -1 if not followed by a wake panel, else equal to wake panel number */
        int m_iWakeColumn;       /**< index of the wake column shed by this panel, numbered from left tip to right tip, or -1 if none */

        //Local frame of refernce
        Vector3d VortexPos;       /**< the absolute position of the mid point of the bound vortex at the panel's quarter chord */
        Vector3d Vortex;          /**< the bound vortex vector at the panel's quarter chord */
        Vector3d P1;              /**< the coordinates of the panel's corners, in local coordinates */
        Vector3d P2;              /**< the coordinates of the panel's corners, in local coordinates */
        Vector3d P3;              /**< the coordinates of the panel's corners, in local coordinates */
        Vector3d P4;              /**< the coordinates of the panel's corners, in local coordinates */
        Vector3d m, l;            /**< the unit vectors which lie in the panel's plane. Cf. document NACA 4023 */

        double dl;               /**< The length of the bound vortex */
        double Area;             /**< The panel's area; */
        double SMP, SMQ;
        double Size;             /**< = SMP + SMQ and provides an estimation of the panel's size.
                                      This is used to determine if the far-field approximation can be used in
                                      the evaluation of the source and doublet influent at a distant point */
        double lij[9];           /**< The 3x3 matrix used to transform local coordinates in absolute coordinates */

        static Vector3d const *s_pNode;       /**< A static pointer to the global array of panel nodes */
        static Vector3d const *s_pWakeNode;   /**< A static pointer to the global array of wake panel nodes */

        static double s_VortexPos; /**< Defines the relative position of the bound vortex in the streamwise direction. Usually the vortex is positioned at the panel's quarter chord i.e. s_VortexPos=0.25 */
        static double s_CtrlPos;   /**< Defines the relative position of the panel's control point in VLM. Usually the control point is positioned at the panel's 3/4 chord : s_VortexPos=0.75 */


    public:
        xfl::enumSurfacePosition m_Pos;   /**< defines if the panel is positioned on a top, middle, bottom, side or body surface */
        bool m_bIsLeading;         /**< true if the panel is positioned on a leading edge */
        bool m_bIsTrailing;        /**< true if the panel is positioned on a trailing edge */
        int m_iLA;                 /**< index of the leading left node in the node array */
        int m_iLB;                 /**< index of the leading right node in the node array */
        int m_iTA;                 /**< index of the trailing left node in the node array */
        int m_iTB;                 /**< index of the trailing right node in the node array */
        Vector3d Normal;            /**< the unit vector normal to the panel */
        Vector3d CtrlPt;            /**< the position of the control point for VLM analysis or 3D/Thin panels analysis */
        Vector3d CollPt;            /**< the collocation point for 3d panel analysis */
        Vector3d VA;                /**< the left end point of the bound quarter-chord vortex on this panel */
        Vector3d VB;                /**< the rightt end point of the bound quarter-chord vortex on this panel */

        static double s_CoreSize;
};

