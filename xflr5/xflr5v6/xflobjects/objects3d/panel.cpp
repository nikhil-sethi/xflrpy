/****************************************************************************

    Panel Class
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



#include <QtCore>
#include "panel.h"


double Panel::s_CoreSize = 0.000001;
double Panel::s_VortexPos = 0.25;
double Panel::s_CtrlPos   = 0.75;

Vector3d const *Panel::s_pNode(nullptr);
Vector3d const *Panel::s_pWakeNode(nullptr);

//temporary variables

#define RFF 10.0         /**< factor used to determine if a point is at a far distance from the panel >*/
#define eps 1.e-7        /**< factor used to determine if a point is on the panel >*/

/**
* The public constructor
*/
Panel::Panel()
{
    reset();
}


/**
* Resets the panel geometry to default initialization values
*/
void Panel::reset()
{
    dl     = 0.0;
    Size   = 0.0;
    Area   = 0.0;
    SMP = SMQ = 0.0;

    m_bIsLeading     = false;
    m_bIsTrailing    = false;
    m_bIsInSymPlane  = false;
    m_bIsLeftPanel   = false;
    m_bIsWakePanel   = false;

    m_Pos         =  xfl::MIDSURFACE;

    m_iElement    = -1;
    m_iLA         = -1;
    m_iLB         = -1;
    m_iTA         = -1;
    m_iTB         = -1;
    m_iPL         = -1;
    m_iPR         = -1;
    m_iPU         = -1;
    m_iPD         = -1;
    m_iWake       = -1;
    m_iWakeColumn = -1;

    memset(lij, 0, sizeof(lij));
}


/**
* Defines the vortex and panel geometrical properties necessary for the VLM and panel calculations.
*/
void Panel::setPanelFrame()
{
    //set the boundary conditions from existing nodes
    setPanelFrame(s_pNode[m_iLA], s_pNode[m_iLB], s_pNode[m_iTA], s_pNode[m_iTB]);
}


/**
* Constructs the vortex and panel properties necessary for the VLM and panel calculations, 
  based on the absolute position of the four corner nodes.
*
* @param LA the position of the leading edge left node.
* @param LB the position of the leading edge right node.
* @param TA the position of the trailing edge left node.
* @param TB the position of the trailing edge rightt node.
*/
void Panel::setPanelFrame(Vector3d const &LA, Vector3d const &LB, Vector3d const &TA, Vector3d const &TB)
{
    Vector3d TALB, LATB, MidA, MidB;
    Vector3d smp, smq;

    if(qAbs(LA.y)<1.e-5 && qAbs(TA.y)<1.e-5 && qAbs(LB.y)<1.e-5 && qAbs(TB.y)<1.e-5)
        m_bIsInSymPlane = true;
    else m_bIsInSymPlane = false;

    LATB.x = TB.x - LA.x;
    LATB.y = TB.y - LA.y;
    LATB.z = TB.z - LA.z;
    TALB.x = LB.x - TA.x;
    TALB.y = LB.y - TA.y;
    TALB.z = LB.z - TA.z;

    Normal = LATB * TALB;
    Area = Normal.norm()/2.0;
    Normal.normalize();

    VA.x = LA.x*(1.0-s_VortexPos) + TA.x*s_VortexPos;
    VA.y = LA.y*(1.0-s_VortexPos) + TA.y*s_VortexPos;
    VA.z = LA.z*(1.0-s_VortexPos) + TA.z*s_VortexPos;

    VB.x = LB.x*(1.0-s_VortexPos) + TB.x*s_VortexPos;
    VB.y = LB.y*(1.0-s_VortexPos) + TB.y*s_VortexPos;
    VB.z = LB.z*(1.0-s_VortexPos) + TB.z*s_VortexPos;

    Vortex.x = VB.x - VA.x;
    Vortex.y = VB.y - VA.y;
    Vortex.z = VB.z - VA.z;

    dl = Vortex.norm();

    VortexPos.x = (VA.x+VB.x)/2.0;
    VortexPos.y = (VA.y+VB.y)/2.0;
    VortexPos.z = (VA.z+VB.z)/2.0;


    MidA.x = LA.x*(1.0-s_CtrlPos) + TA.x*s_CtrlPos;
    MidA.y = LA.y*(1.0-s_CtrlPos) + TA.y*s_CtrlPos;
    MidA.z = LA.z*(1.0-s_CtrlPos) + TA.z*s_CtrlPos;

    MidB.x = LB.x*(1.0-s_CtrlPos) + TB.x*s_CtrlPos;
    MidB.y = LB.y*(1.0-s_CtrlPos) + TB.y*s_CtrlPos;
    MidB.z = LB.z*(1.0-s_CtrlPos) + TB.z*s_CtrlPos;

    CtrlPt.x = (MidA.x+MidB.x)/2.0;
    CtrlPt.y = (MidA.y+MidB.y)/2.0;
    CtrlPt.z = (MidA.z+MidB.z)/2.0;

    CollPt.x = (LA.x + LB.x + TA.x + TB.x)/4.0;
    CollPt.y = (LA.y + LB.y + TA.y + TB.y)/4.0;
    CollPt.z = (LA.z + LB.z + TA.z + TB.z)/4.0;

    //Use VSAERO figure 8. p23
    //    if(m_iPos==THINSURFACE || m_Pos==TOPSURFACE || m_Pos==BODYSURFACE)
    //    {
    m.x = (LB.x + TB.x) *0.5 - CollPt.x;
    m.y = (LB.y + TB.y) *0.5 - CollPt.y;
    m.z = (LB.z + TB.z) *0.5 - CollPt.z;
    /*    }
    else
    {
        m.x = (LA.x + TA.x) *0.5 - CollPt.x;
        m.y = (LA.y + TA.y) *0.5 - CollPt.y;
        m.z = (LA.z + TA.z) *0.5 - CollPt.z;
    }*/
    m.normalize();

    l.x =  m.y * Normal.z - m.z * Normal.y;
    l.y = -m.x * Normal.z + m.z * Normal.x;
    l.z =  m.x * Normal.y - m.y * Normal.x;

    smq.x  = (LB.x + TB.x) * 0.5 - CollPt.x;
    smq.y  = (LB.y + TB.y) * 0.5 - CollPt.y;
    smq.z  = (LB.z + TB.z) * 0.5 - CollPt.z;
    smp.x  = (TB.x + TA.x) * 0.5 - CollPt.x;
    smp.y  = (TB.y + TA.y) * 0.5 - CollPt.y;
    smp.z  = (TB.z + TA.z) * 0.5 - CollPt.z;

    SMP = smp.norm();
    SMQ = smq.norm();

    Size = SMP + SMQ;

    //create the transformation matrix
    lij[0]=l.x;        lij[1]=m.x;       lij[2]=Normal.x;
    lij[3]=l.y;        lij[4]=m.y;       lij[5]=Normal.y;
    lij[6]=l.z;        lij[7]=m.z;       lij[8]=Normal.z;

    invert33(lij);

    if(m_Pos>xfl::MIDSURFACE)
    {
        P1.x = lij[0]*(LA.x-CollPt.x) + lij[1]*(LA.y-CollPt.y) + lij[2]*(LA.z-CollPt.z);
        P1.y = lij[3]*(LA.x-CollPt.x) + lij[4]*(LA.y-CollPt.y) + lij[5]*(LA.z-CollPt.z);
        P1.z = lij[6]*(LA.x-CollPt.x) + lij[7]*(LA.y-CollPt.y) + lij[8]*(LA.z-CollPt.z);
        P2.x = lij[0]*(LB.x-CollPt.x) + lij[1]*(LB.y-CollPt.y) + lij[2]*(LB.z-CollPt.z);
        P2.y = lij[3]*(LB.x-CollPt.x) + lij[4]*(LB.y-CollPt.y) + lij[5]*(LB.z-CollPt.z);
        P2.z = lij[6]*(LB.x-CollPt.x) + lij[7]*(LB.y-CollPt.y) + lij[8]*(LB.z-CollPt.z);
        P3.x = lij[0]*(TB.x-CollPt.x) + lij[1]*(TB.y-CollPt.y) + lij[2]*(TB.z-CollPt.z);
        P3.y = lij[3]*(TB.x-CollPt.x) + lij[4]*(TB.y-CollPt.y) + lij[5]*(TB.z-CollPt.z);
        P3.z = lij[6]*(TB.x-CollPt.x) + lij[7]*(TB.y-CollPt.y) + lij[8]*(TB.z-CollPt.z);
        P4.x = lij[0]*(TA.x-CollPt.x) + lij[1]*(TA.y-CollPt.y) + lij[2]*(TA.z-CollPt.z);
        P4.y = lij[3]*(TA.x-CollPt.x) + lij[4]*(TA.y-CollPt.y) + lij[5]*(TA.z-CollPt.z);
        P4.z = lij[6]*(TA.x-CollPt.x) + lij[7]*(TA.y-CollPt.y) + lij[8]*(TA.z-CollPt.z);
    }
    else
    {
        P1.x = lij[0]*(LB.x-CollPt.x) + lij[1]*(LB.y-CollPt.y) + lij[2]*(LB.z-CollPt.z);
        P1.y = lij[3]*(LB.x-CollPt.x) + lij[4]*(LB.y-CollPt.y) + lij[5]*(LB.z-CollPt.z);
        P1.z = lij[6]*(LB.x-CollPt.x) + lij[7]*(LB.y-CollPt.y) + lij[8]*(LB.z-CollPt.z);
        P2.x = lij[0]*(LA.x-CollPt.x) + lij[1]*(LA.y-CollPt.y) + lij[2]*(LA.z-CollPt.z);
        P2.y = lij[3]*(LA.x-CollPt.x) + lij[4]*(LA.y-CollPt.y) + lij[5]*(LA.z-CollPt.z);
        P2.z = lij[6]*(LA.x-CollPt.x) + lij[7]*(LA.y-CollPt.y) + lij[8]*(LA.z-CollPt.z);
        P3.x = lij[0]*(TA.x-CollPt.x) + lij[1]*(TA.y-CollPt.y) + lij[2]*(TA.z-CollPt.z);
        P3.y = lij[3]*(TA.x-CollPt.x) + lij[4]*(TA.y-CollPt.y) + lij[5]*(TA.z-CollPt.z);
        P3.z = lij[6]*(TA.x-CollPt.x) + lij[7]*(TA.y-CollPt.y) + lij[8]*(TA.z-CollPt.z);
        P4.x = lij[0]*(TB.x-CollPt.x) + lij[1]*(TB.y-CollPt.y) + lij[2]*(TB.z-CollPt.z);
        P4.y = lij[3]*(TB.x-CollPt.x) + lij[4]*(TB.y-CollPt.y) + lij[5]*(TB.z-CollPt.z);
        P4.z = lij[6]*(TB.x-CollPt.x) + lij[7]*(TB.y-CollPt.y) + lij[8]*(TB.z-CollPt.z);
    }
}


/**
* Inverts in place a 3x3 matrix
*/
bool Panel::invert33(double *l)
{
    /*        a0 b1 c2
        d3 e4 f5
        g6 h7 i8

              1                     (ei-fh)   (bi-ch)   (bf-ce)
-----------------------------   x   (fg-di)   (ai-cg)   (cd-af)
a(ei-fh) - b(di-fg) + c(dh-eg)      (dh-eg)   (bg-ah)   (ae-bd)*/

    double det;         /**< temporary variable */
    double mat[9];      /**< temporary array  */
    memcpy(mat,l,sizeof(mat));


    det  = mat[0] *(mat[4] * mat[8] - mat[5]* mat[7]);
    det -= mat[1] *(mat[3] * mat[8] - mat[5]* mat[6]);
    det += mat[2] *(mat[3] * mat[7] - mat[4]* mat[6]);
    if(det==0.0) return false;

    * l     = (mat[4] * mat[8] - mat[5] * mat[7])/det;
    *(l+1)  = (mat[2] * mat[7] - mat[1] * mat[8])/det;
    *(l+2)  = (mat[1] * mat[5] - mat[2] * mat[4])/det;

    *(l+3)  = (mat[5] * mat[6] - mat[3] * mat[8])/det;
    *(l+4)  = (mat[0] * mat[8] - mat[2] * mat[6])/det;
    *(l+5)  = (mat[2] * mat[3] - mat[0] * mat[5])/det;

    *(l+6)  = (mat[3] * mat[7] - mat[4] * mat[6])/det;
    *(l+7)  = (mat[1] * mat[6] - mat[0] * mat[7])/det;
    *(l+8)  = (mat[0] * mat[4] - mat[1] * mat[3])/det;

    return true;
}



/**
* Converts the global coordinates of the input vector in local panel coordinates.
*@param  V the global coordinates
*@param  V the calculated local coordinates
*/
void Panel::globalToLocal(Vector3d const &V, Vector3d &VLocal)
{
    VLocal.x = lij[0]*V.x +lij[1]*V.y +lij[2]*V.z;
    VLocal.y = lij[3]*V.x +lij[4]*V.y +lij[5]*V.z;
    VLocal.z = lij[6]*V.x +lij[7]*V.y +lij[8]*V.z;
}



/**
* Converts the global coordinates of the input vector in local panel coordinates.
*@param  V the global coordinates
*@return The Vector3d holding the local coordinates
*/
Vector3d Panel::globalToLocal(Vector3d const &V)
{
    Vector3d L;
    L.x = lij[0]*V.x +lij[1]*V.y +lij[2]*V.z;
    L.y = lij[3]*V.x +lij[4]*V.y +lij[5]*V.z;
    L.z = lij[6]*V.x +lij[7]*V.y +lij[8]*V.z;
    return L;
}



/**
* Converts the global coordinates of the input vector in local panel coordinates.
*@param  Vx the global x-coordinate
*@param  Vy the global y-coordinate
*@param  Vz the global z-coordinate
*@return The Vector3d holding the local coordinates
*/
Vector3d Panel::globalToLocal(double const &Vx, double const &Vy, double const &Vz)
{
    Vector3d L;
    L.x = lij[0]*Vx +lij[1]*Vy +lij[2]*Vz;
    L.y = lij[3]*Vx +lij[4]*Vy +lij[5]*Vz;
    L.z = lij[6]*Vx +lij[7]*Vy +lij[8]*Vz;
    return L;
}


/**
* Converts the local coordinates of the input vector in the global referential
*@param  V the locaal coordinates
*@return The Vector3d holding the global coordinates
*/
Vector3d Panel::localToGlobal(Vector3d const &V)
{
    Vector3d L;
    L.x = V.x * l.x + V.y * m.x + V.z * Normal.x;
    L.y = V.x * l.y + V.y * m.y + V.z * Normal.y;
    L.z = V.x * l.z + V.y * m.z + V.z * Normal.z;
    return L;
}


/**
* Finds the intersection point of a ray with the panel. 
* The ray is defined by a point and a direction vector.
*@param A the ray's origin
*@param U the ray's direction
*@param I the intersection point
*@param dist the distance of A to the panel in the direction of the panel's normal
*/
bool Panel::intersect(Vector3d const &A, Vector3d const &U, Vector3d &I, double &dist)
{
    Vector3d ILA, ILB, ITA, ITB;
    Vector3d T, V, W, P;
    bool b1, b2, b3, b4;
    double r,s;

    ILA.copy(s_pNode[m_iLA]);
    ITA.copy(s_pNode[m_iTA]);
    ILB.copy(s_pNode[m_iLB]);
    ITB.copy(s_pNode[m_iTB]);

    r = (CollPt.x-A.x)*Normal.x + (CollPt.y-A.y)*Normal.y + (CollPt.z-A.z)*Normal.z ;
    s = U.x*Normal.x + U.y*Normal.y + U.z*Normal.z;

    dist = 10000.0;

    if(qAbs(s)>0.0)
    {
        dist = r/s;

        //inline operations to save time
        P.x = A.x + U.x * dist;
        P.y = A.y + U.y * dist;
        P.z = A.z + U.z * dist;

        // P is inside the panel if on left side of each panel side
        W.x = P.x  - ITA.x;
        W.y = P.y  - ITA.y;
        W.z = P.z  - ITA.z;
        V.x = ITB.x - ITA.x;
        V.y = ITB.y - ITA.y;
        V.z = ITB.z - ITA.z;
        T.x =  V.y * W.z - V.z * W.y;
        T.y = -V.x * W.z + V.z * W.x;
        T.z =  V.x * W.y - V.y * W.x;
        if(T.x*T.x+T.y*T.y+T.z*T.z <1.0e-10 || T.x*Normal.x+T.y*Normal.y+T.z*Normal.z>=0.0) b1 = true; else b1 = false;

        W.x = P.x  - ITB.x;
        W.y = P.y  - ITB.y;
        W.z = P.z  - ITB.z;
        V.x = ILB.x - ITB.x;
        V.y = ILB.y - ITB.y;
        V.z = ILB.z - ITB.z;
        T.x =  V.y * W.z - V.z * W.y;
        T.y = -V.x * W.z + V.z * W.x;
        T.z =  V.x * W.y - V.y * W.x;
        if(T.x*T.x+T.y*T.y+T.z*T.z <1.0e-10 || T.x*Normal.x+T.y*Normal.y+T.z*Normal.z>=0.0) b2 = true; else b2 = false;

        W.x = P.x  - ILB.x;
        W.y = P.y  - ILB.y;
        W.z = P.z  - ILB.z;
        V.x = ILA.x - ILB.x;
        V.y = ILA.y - ILB.y;
        V.z = ILA.z - ILB.z;
        T.x =  V.y * W.z - V.z * W.y;
        T.y = -V.x * W.z + V.z * W.x;
        T.z =  V.x * W.y - V.y * W.x;
        if(T.x*T.x+T.y*T.y+T.z*T.z <1.0e-10 || T.x*Normal.x+T.y*Normal.y+T.z*Normal.z>=0.0) b3 = true; else b3 = false;

        W.x = P.x  - ILA.x;
        W.y = P.y  - ILA.y;
        W.z = P.z  - ILA.z;
        V.x = ITA.x - ILA.x;
        V.y = ITA.y - ILA.y;
        V.z = ITA.z - ILA.z;
        T.x =  V.y * W.z - V.z * W.y;
        T.y = -V.x * W.z + V.z * W.x;
        T.z =  V.x * W.y - V.y * W.x;
        if(T.x*T.x+T.y*T.y+T.z*T.z <1.0e-10 || T.x*Normal.x+T.y*Normal.y+T.z*Normal.z>=0.0) b4 = true; else b4 = false;

        if(b1 && b2 && b3 && b4)
        {
            I.set(P.x, P.y, P.z);
            return true;
        }
    }
    return false;
}



/**
*Returns the panel's width, measured at the leading edge 
*/
double Panel::width() const
{
    return sqrt( (s_pNode[m_iLB].y - s_pNode[m_iLA].y)*(s_pNode[m_iLB].y - s_pNode[m_iLA].y)
                 +(s_pNode[m_iLB].z - s_pNode[m_iLA].z)*(s_pNode[m_iLB].z - s_pNode[m_iLA].z));
}


/**
*Rotates the boundary condition properties which are used in stability analysis with variable control positions.
*@param HA is the center of rotation
*@param Qt the quaternion which defines the 3D rotation
*/
void Panel::rotateBC(Vector3d const &HA, Quaternion &Qt)
{
    //    Qt.Conjugate(Vortex);
    Vector3d WTest;
    WTest.x = VortexPos.x - HA.x;
    WTest.y = VortexPos.y - HA.y;
    WTest.z = VortexPos.z - HA.z;
    Qt.conjugate(WTest);
    VortexPos.x = WTest.x + HA.x;
    VortexPos.y = WTest.y + HA.y;
    VortexPos.z = WTest.z + HA.z;

    WTest.x = VA.x - HA.x;
    WTest.y = VA.y - HA.y;
    WTest.z = VA.z - HA.z;
    Qt.conjugate(WTest);
    VA.x = WTest.x + HA.x;
    VA.y = WTest.y + HA.y;
    VA.z = WTest.z + HA.z;

    WTest.x = VB.x - HA.x;
    WTest.y = VB.y - HA.y;
    WTest.z = VB.z - HA.z;
    Qt.conjugate(WTest);
    VB.x = WTest.x + HA.x;
    VB.y = WTest.y + HA.y;
    VB.z = WTest.z + HA.z;

    WTest.x = CtrlPt.x - HA.x;
    WTest.y = CtrlPt.y - HA.y;
    WTest.z = CtrlPt.z - HA.z;
    Qt.conjugate(WTest);
    CtrlPt.x = WTest.x + HA.x;
    CtrlPt.y = WTest.y + HA.y;
    CtrlPt.z = WTest.z + HA.z;

    WTest.x = CollPt.x - HA.x;
    WTest.y = CollPt.y - HA.y;
    WTest.z = CollPt.z - HA.z;
    Qt.conjugate(WTest);
    CollPt.x = WTest.x + HA.x;
    CollPt.y = WTest.y + HA.y;
    CollPt.z = WTest.z + HA.z;

    Qt.conjugate(Vortex);
    Qt.conjugate(Normal);
}


/**
* Evaluates the influence of a uniform source at a point outside the panel.
*
* Follows the method provided in the VSAERO theory Manual NASA 4023.
*
* Vectorial operations are written inline to save computing times -->longer code, but 4x more efficient.
*
*@param C the point where the influence is to be evaluated
*@param pPanel a pointer to the Panel with the doublet strength
*@param V the perturbation velocity at point C
*@param phi the potential at point C
*/
void Panel::sourceNASA4023(Vector3d const &C,  Vector3d &V, double &phi) const
{
    double RNUM(0), DNOM(0), pjk(0), CJKi(0);
    double PN(0), A(0), B(0), PA(0), PB(0), SM(0), SL(0), AM(0), AL(0), Al(0);
    double side(0), sign(0), S(0), GL(0);
    Vector3d PJK, a, b, s, T1, T2, h;
    Vector3d const*m_pR[5];
    //we use a default core size, unless the user has specified one
    double CoreSize = 0.00000;
    if(qAbs(s_CoreSize)>PRECISION) CoreSize = s_CoreSize;

    phi = 0.0;
    V.x=0.0; V.y=0.0; V.z=0.0;

    PJK.x = C.x - CollPt.x;
    PJK.y = C.y - CollPt.y;
    PJK.z = C.z - CollPt.z;

    PN  = PJK.x*Normal.x + PJK.y*Normal.y + PJK.z*Normal.z;
    pjk = sqrt(PJK.x*PJK.x + PJK.y*PJK.y + PJK.z*PJK.z);

    if(pjk> RFF*Size)
    {
        // use far-field formula
        phi = Area /pjk;
        V.x = PJK.x * Area/pjk/pjk/pjk;
        V.y = PJK.y * Area/pjk/pjk/pjk;
        V.z = PJK.z * Area/pjk/pjk/pjk;
        return;
    }

    if(m_Pos>=xfl::MIDSURFACE)
    {
        m_pR[0] = s_pNode + m_iLA;
        m_pR[1] = s_pNode + m_iTA;
        m_pR[2] = s_pNode + m_iTB;
        m_pR[3] = s_pNode + m_iLB;
        m_pR[4] = s_pNode + m_iLA;
    }
    else
    {
        m_pR[0] = s_pNode + m_iLB;
        m_pR[1] = s_pNode + m_iTB;
        m_pR[2] = s_pNode + m_iTA;
        m_pR[3] = s_pNode + m_iLA;
        m_pR[4] = s_pNode + m_iLB;
    }

    for (int i=0; i<4; i++)
    {
        a.x  = C.x - m_pR[i]->x;
        a.y  = C.y - m_pR[i]->y;
        a.z  = C.z - m_pR[i]->z;

        b.x  = C.x - m_pR[i+1]->x;
        b.y  = C.y - m_pR[i+1]->y;
        b.z  = C.z - m_pR[i+1]->z;

        s.x  = m_pR[i+1]->x - m_pR[i]->x;
        s.y  = m_pR[i+1]->y - m_pR[i]->y;
        s.z  = m_pR[i+1]->z - m_pR[i]->z;

        A    = sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
        B    = sqrt(b.x*b.x + b.y*b.y + b.z*b.z);
        S    = sqrt(s.x*s.x + s.y*s.y + s.z*s.z);
        SM   = s.x*m.x + s.y*m.y + s.z*m.z;
        SL   = s.x*l.x + s.y*l.y + s.z*l.z;
        AM   = a.x*m.x + a.y*m.y + a.z*m.z;
        AL   = a.x*l.x + a.y*l.y + a.z*l.z;
        Al   = AM*SL - AL*SM;
        PA   = PN*PN*SL + Al*AM;
        PB   = PA - Al*SM;

        //get the distance of the TestPoint to the panel's side
        h.x =  a.y*s.z - a.z*s.y;
        h.y = -a.x*s.z + a.z*s.x;
        h.z =  a.x*s.y - a.y*s.x;

        if(m_pR[i]->isSame(*m_pR[i+1]))
        {
            //no contribution from this side
            CJKi = 0.0;
        }
        else if ((((h.x*h.x+h.y*h.y+h.z*h.z)/(s.x*s.x+s.y*s.y+s.z*s.z) <= CoreSize*CoreSize) && a.x*s.x+a.y*s.y+a.z*s.z>=0.0 && b.x*s.x+b.y*s.y+b.z*s.z<=0.0) ||
                 A < CoreSize || B < CoreSize)
        {
            //if lying on the panel's side... no contribution
            CJKi = 0.0;
        }
        else
        {
            //first the potential
            if(fabs(A+B-S)>0.0)    GL = 1.0/S * log(fabs((A+B+S)/(A+B-S)));
            else                GL = 0.0;

            RNUM = SM*PN * (B*PA-A*PB);
            DNOM = PA*PB + PN*PN*A*B*SM*SM;

            if(qAbs(PN)<eps)
            {
                // side is >0 if the point is on the panel's right side
                side = Normal.x*h.x + Normal.y*h.y + Normal.z*h.z;
                if(side >=0.0) sign = 1.0; else sign = -1.0;
                if(DNOM<0.0)
                {
                    if(PN>0.0)    CJKi =  PI * sign;
                    else        CJKi = -PI * sign;
                }
                else if(DNOM == 0.0)
                {
                    if(PN>0.0)    CJKi =  PI/2.0 * sign;
                    else        CJKi = -PI/2.0 * sign;
                }
                else
                    CJKi = 0.0;
            }
            else
            {
                CJKi = atan2(RNUM, DNOM);
            }

            phi += Al*GL - PN*CJKi;

            // next the induced velocity
            T1.x   = l.x      * SM*GL;
            T1.y   = l.y      * SM*GL;
            T1.z   = l.z      * SM*GL;
            T2.x   = m.x      * SL*GL;
            T2.y   = m.y      * SL*GL;
            T2.z   = m.z      * SL*GL;

            V.x   += Normal.x * CJKi + T1.x - T2.x;
            V.y   += Normal.y * CJKi + T1.y - T2.y;
            V.z   += Normal.z * CJKi + T1.z - T2.z;
        }
    }
}


/**
 * Evaluates the influence of a doublet at a point outside the panel.
 *
 * Follows the method provided in the VSAERO theory Manual NASA 4023.
 *
 * Vectorial operations are written inline to save computing times -->longer code, but 4x more efficient.
 *
 * @param C the point where the influence is to be evaluated
 * @param pPanel a pointer to the Panel with the doublet strength
 * @param V the perturbation velocity at point C
 * @param phi the potential at point C
 * @param bWake true if the panel is a wake panel, false if it is a surface panel
 */
void Panel::doubletNASA4023(Vector3d const &C, Vector3d &V, double &phi, bool bWake) const
{
    Vector3d const *m_pR[5];
    Vector3d PJK, a, b, s, T1, h;
    double RNUM(0), DNOM(0), pjk(0), CJKi(0);
    double PN(0), A(0), B(0), PA(0), PB(0), SM(0), SL(0), AM(0), AL(0), Al(0);
    double side(0), sign(0), GL(0);


    //we use a default core size, unless the user has specified one
    double CoreSize = 0.00000;
    if(qAbs(s_CoreSize)>PRECISION) CoreSize = s_CoreSize;

    Vector3d const *pNode;
    if(!bWake) pNode = s_pNode;
    else       pNode = s_pWakeNode;

    phi = 0.0;

    V.x=0.0; V.y=0.0; V.z=0.0;

    PJK.x = C.x - CollPt.x;
    PJK.y = C.y - CollPt.y;
    PJK.z = C.z - CollPt.z;

    PN  = PJK.x*Normal.x + PJK.y*Normal.y + PJK.z*Normal.z;
    pjk = sqrt(PJK.x*PJK.x + PJK.y*PJK.y + PJK.z*PJK.z);

    if(pjk> RFF*Size)
    {
        // use far-field formula
        phi = PN * Area /pjk/pjk/pjk;
        T1.x =PJK.x*3.0*PN - Normal.x*pjk*pjk;
        T1.y =PJK.y*3.0*PN - Normal.y*pjk*pjk;
        T1.z =PJK.z*3.0*PN - Normal.z*pjk*pjk;
        V.x   = T1.x * Area /pjk/pjk/pjk/pjk/pjk;
        V.y   = T1.y * Area /pjk/pjk/pjk/pjk/pjk;
        V.z   = T1.z * Area /pjk/pjk/pjk/pjk/pjk;
        return;
    }

    if(m_Pos>=xfl::MIDSURFACE)
    {
        m_pR[0] = pNode + m_iLA;
        m_pR[1] = pNode + m_iTA;
        m_pR[2] = pNode + m_iTB;
        m_pR[3] = pNode + m_iLB;
        m_pR[4] = pNode + m_iLA;
    }
    else
    {
        m_pR[0] = pNode + m_iLB;
        m_pR[1] = pNode + m_iTB;
        m_pR[2] = pNode + m_iTA;
        m_pR[3] = pNode + m_iLA;
        m_pR[4] = pNode + m_iLB;
    }

    for (int i=0; i<4; i++)
    {
        a.x  = C.x - m_pR[i]->x;
        a.y  = C.y - m_pR[i]->y;
        a.z  = C.z - m_pR[i]->z;
        b.x  = C.x - m_pR[i+1]->x;
        b.y  = C.y - m_pR[i+1]->y;
        b.z  = C.z - m_pR[i+1]->z;
        s.x  = m_pR[i+1]->x - m_pR[i]->x;
        s.y  = m_pR[i+1]->y - m_pR[i]->y;
        s.z  = m_pR[i+1]->z - m_pR[i]->z;
        A    = sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
        B    = sqrt(b.x*b.x + b.y*b.y + b.z*b.z);
        SM   = s.x*m.x + s.y*m.y + s.z*m.z;
        SL   = s.x*l.x + s.y*l.y + s.z*l.z;
        AM   = a.x*m.x + a.y*m.y + a.z*m.z;
        AL   = a.x*l.x + a.y*l.y + a.z*l.z;
        Al   = AM*SL - AL*SM;
        PA   = PN*PN*SL + Al*AM;
        PB   = PA - Al*SM;

        //get the distance of the TestPoint to the panel's side
        h.x =  a.y*s.z - a.z*s.y;
        h.y = -a.x*s.z + a.z*s.x;
        h.z =  a.x*s.y - a.y*s.x;

        //first the potential
        if(m_pR[i]->isSame(*m_pR[i+1]))
        {
            CJKi = 0.0;
            //no contribution to speed either
        }
        else if ((((h.x*h.x+h.y*h.y+h.z*h.z)/(s.x*s.x+s.y*s.y+s.z*s.z) <= CoreSize*CoreSize) && a.x*s.x+a.y*s.y+a.z*s.z>=0.0  && b.x*s.x+b.y*s.y+b.z*s.z<=0.0)
                 ||  A < CoreSize || B < CoreSize)
        {
            CJKi = 0.0;//speed is singular at panel edge, the value of the potential is unknown
        }
        else
        {
            RNUM = SM*PN * (B*PA-A*PB);
            DNOM = PA*PB + PN*PN*A*B*SM*SM;
            if(qAbs(PN)<eps)
            {
                // side is >0 if on the panel's right side
                side = Normal.x*h.x +Normal.y*h.y +Normal.z*h.z;

                if(side >=0.0) sign = 1.0; else sign = -1.0;
                if(DNOM<0.0)
                {
                    if(PN>0.0)    CJKi =  PI * sign;
                    else        CJKi = -PI * sign;
                }
                else if(DNOM == 0.0)
                {
                    if(PN>0.0)    CJKi =  PI/2.0 * sign;
                    else        CJKi = -PI/2.0 * sign;
                }
                else
                    CJKi = 0.0;
            }
            else
            {
                CJKi = atan2(RNUM,DNOM);
            }
            // next the induced velocity
            h.x =  a.y*b.z - a.z*b.y;
            h.y = -a.x*b.z + a.z*b.x;
            h.z =  a.x*b.y - a.y*b.x;
            GL = ((A+B) /A/B/ (A*B + a.x*b.x+a.y*b.y+a.z*b.z));
            V.x += h.x * GL;
            V.y += h.y * GL;
            V.z += h.z * GL;
        }
        phi += CJKi;

    }
    if (( (C.x-CollPt.x)*(C.x-CollPt.x)
          +(C.y-CollPt.y)*(C.y-CollPt.y)
          +(C.z-CollPt.z)*(C.z-CollPt.z))<1.e-10)
    {
        //        if(m_R[0]->IsSame(*m_R[1]) || m_R[1]->IsSame(*m_R[2]) || m_R[2]->IsSame(*m_R[3]) || m_R[3]->IsSame(*m_R[0]))
        //            phi = -3.0*pi/2.0;
        //        else
        phi  = -2.0*PI;
    }
}

/** output the panel's properties - debug only */
void Panel::printPanel()
{
    qDebug("Panel %d:", m_iElement);
    qDebug("  neighbour panels:  PU=%3d    PD=%3d   PL=%3d   PR=%3d", m_iPU, m_iPD, m_iPL, m_iPR);
    qDebug("  TrailingWakeElem=%2d  TrailingWakeColumn=%2d", m_iWake, m_iWakeColumn);
    qDebug("  isLeading=%1d    isTrailing=%1d", m_bIsLeading, m_bIsTrailing);
    qDebug("  isInSymPlane=%1d    isLeftWingPanel=%d    isWakePanel=%d", m_bIsInSymPlane, m_bIsLeftPanel, m_bIsWakePanel);
    qDebug("  Area=%13.5g  Size=%13.5g", Area, Size);
    setPanelFrame(s_pNode[m_iLA], s_pNode[m_iLB], s_pNode[m_iTA], s_pNode[m_iTB]);
    s_pNode[m_iLA].listCoords("  LA");
    s_pNode[m_iLB].listCoords("  LB");
    s_pNode[m_iTA].listCoords("  TA");
    s_pNode[m_iTB].listCoords("  TB");
    qDebug("  Normal: %13.7f  %13.7f  %13.7f", Normal.x, Normal.y, Normal.z);
    qDebug("  CollPt: %13.7f  %13.7f  %13.7f", CollPt.x, CollPt.y, CollPt.z);
    qDebug("  CtrlPt: %13.7f  %13.7f  %13.7f", CtrlPt.x, CtrlPt.y, CtrlPt.z);
    qDebug("  Vortex: %13.7f  %13.7f  %13.7f", Vortex.x, Vortex.y, Vortex.z);
    qDebug("  VtxPos: %13.7f  %13.7f  %13.7f", VortexPos.x, VortexPos.y, VortexPos.z);
    qDebug(" ");
}




