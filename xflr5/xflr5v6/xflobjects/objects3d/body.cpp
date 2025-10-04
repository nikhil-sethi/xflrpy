/****************************************************************************

    Body Class
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

#include <QStringList>


#include "body.h"
#include <xflobjects/objects_global.h>
#include <xflcore/xflcore.h>

/**
 * The public constructor
 */
Body::Body()
{
    m_Name = QObject::tr("Body Name");

    m_Color = QColor(98,102,156);

    m_iActiveFrame =  1;
    m_iHighlightFrame   = -1;
    m_LineType     =  xfl::BODYSPLINETYPE;

    m_nxPanels = 19;
    m_nhPanels = 11;

    m_NElements = m_nxPanels * m_nhPanels * 2;

    //    m_BodyLEPosition.Set(0.0,0.0,0.0);
    m_CoG.set(0.0,0.0,0.0);
    m_VolumeMass = 0.0;        //for inertia calculations
    m_CoGIxx = m_CoGIyy = m_CoGIzz = m_CoGIxz = 0.0;
    clearPointMasses();

    m_Bunch  = 0.0;

    m_SplineSurface.m_iuDegree = 3;
    m_SplineSurface.m_ivDegree = 3;


    //    m_NSideLines = 5;
    //    m_SplineSurface.m_nvLines = SideLines();
    m_xPanels.clear();
    m_hPanels.clear();
    for(int ifr=0; ifr<7; ifr++)
    {
        m_SplineSurface.m_pFrame.append(new Frame);
        m_SplineSurface.m_pFrame[ifr]->m_CtrlPoint.clear();
        m_xPanels.append(1);
        m_hPanels.append(1);
        for(int is=0; is<5; is++)
        {
            m_SplineSurface.m_pFrame[ifr]->m_CtrlPoint.append(Vector3d(0.0,0.0,0.0));
        }
    }

    frame(0)->setuPosition(-0.243);
    frame(1)->setuPosition(-0.228);
    frame(2)->setuPosition(-0.051);
    frame(3)->setuPosition( 0.094);
    frame(4)->setuPosition( 0.279);
    frame(5)->setuPosition( 0.705);
    frame(6)->setuPosition( 0.719);

    frame(0)->m_CtrlPoint[0].set(-0.243, 0.0, -0.0172);
    frame(0)->m_CtrlPoint[1].set(-0.243, 0.0, -0.0172);
    frame(0)->m_CtrlPoint[2].set(-0.243, 0.0, -0.0172);
    frame(0)->m_CtrlPoint[3].set(-0.243, 0.0, -0.0172);
    frame(0)->m_CtrlPoint[4].set(-0.243, 0.0, -0.0172);

    frame(1)->m_CtrlPoint[0].set(-0.228, 0.000,  0.005);
    frame(1)->m_CtrlPoint[1].set(-0.228, 0.011,  0.004);
    frame(1)->m_CtrlPoint[2].set(-0.228, 0.013, -0.018);
    frame(1)->m_CtrlPoint[3].set(-0.228, 0.011, -0.030);
    frame(1)->m_CtrlPoint[4].set(-0.228, 0.000, -0.031);

    frame(2)->m_CtrlPoint[0].set(-0.051, 0.000,  0.033);
    frame(2)->m_CtrlPoint[1].set(-0.051, 0.028,  0.036);
    frame(2)->m_CtrlPoint[2].set(-0.051, 0.037, -0.003);
    frame(2)->m_CtrlPoint[3].set(-0.051, 0.034, -0.045);
    frame(2)->m_CtrlPoint[4].set(-0.051, 0.000, -0.049);

    frame(3)->m_CtrlPoint[0].set(0.094, 0.000,  0.025);
    frame(3)->m_CtrlPoint[1].set(0.094, 0.012,  0.019);
    frame(3)->m_CtrlPoint[2].set(0.094, 0.018,  0.001);
    frame(3)->m_CtrlPoint[3].set(0.094, 0.012, -0.017);
    frame(3)->m_CtrlPoint[4].set(0.094, 0.000, -0.023);

    frame(4)->m_CtrlPoint[0].set(0.279, 0.000,  0.007);
    frame(4)->m_CtrlPoint[1].set(0.279, 0.006,  0.008);
    frame(4)->m_CtrlPoint[2].set(0.279, 0.009,  0.000);
    frame(4)->m_CtrlPoint[3].set(0.279, 0.007, -0.006);
    frame(4)->m_CtrlPoint[4].set(0.279, 0.000, -0.005);

    frame(5)->m_CtrlPoint[0].set(0.705, 0.000,  0.0124);
    frame(5)->m_CtrlPoint[1].set(0.705, 0.010,  0.0118);
    frame(5)->m_CtrlPoint[2].set(0.705, 0.012, -0.0015);
    frame(5)->m_CtrlPoint[3].set(0.705, 0.010, -0.0116);
    frame(5)->m_CtrlPoint[4].set(0.705, 0.000, -0.012);

    frame(6)->m_CtrlPoint[0].set(0.719, 0.00,  0.0);
    frame(6)->m_CtrlPoint[1].set(0.719, 0.00,  0.0);
    frame(6)->m_CtrlPoint[2].set(0.719, 0.00,  0.0);
    frame(6)->m_CtrlPoint[3].set(0.719, 0.00, -0.0);
    frame(6)->m_CtrlPoint[4].set(0.719, 0.00, -0.0);


    setNURBSKnots();
}

Body::~Body()
{
    clearPointMasses();
}

void Body::setNURBSKnots()
{
    m_SplineSurface.setKnots();
}


/**
 * Computes and returns the aerodynamic coefficients based on the array of Cp coefficients.
 * Aero coefficients are inviscid only. No viscous model for the body. Besides, since the body
 * does not shed any wake, all forces are calculated on-body and not in the far field plane.
 * @param Cp a pointer to the array of Cp coefficients, in input.
 * @param XCP the x-position of the center of pressure of body forces, in meters.
 * @param YCP the y-position of the center of pressure of body forces, in meters.
 * @param ZCP the z-position of the center of pressure of body forces, in meters.
 * @param GCm the total pitching moment coefficient induced by panel forces wrt the CoG
 * @param GRm the total rolling moment coefficient induced by panel forces wrt the CoG.
 * @param GYm the total yawing moment coefficient induced by panel forces wrt the CoG.
 * @param Alpha the angle of attack, in degrees
 * @param CoG the position of the CoG
 */
void Body::computeAero(double *Cp, double &XCP, double &YCP, double &ZCP,
                       double &GCm, double &GRm, double &GYm, double &Alpha, Vector3d const &CoG,
                       Panel const*pPanel) const
{
    Vector3d PanelForce, LeverArm, WindNormal, WindDirection;
    Vector3d GeomMoment;

    double cosa = cos(Alpha*PI/180.0);
    double sina = sin(Alpha*PI/180.0);

    //   Define wind axis
    WindNormal.set(   -sina, 0.0, cosa);
    WindDirection.set( cosa, 0.0, sina);

    for (int p=0; p<m_NElements; p++)
    {
        int ip = m_FirstPanelIndex+p;
        Panel const &panel = pPanel[ip];
        PanelForce.x = panel.Normal.x * (-Cp[p]) * panel.Area;
        PanelForce.y = panel.Normal.y * (-Cp[p]) * panel.Area;
        PanelForce.z = panel.Normal.z * (-Cp[p]) * panel.Area; // N/q

        double PanelLift = PanelForce.dot(WindNormal);
        XCP   += panel.CollPt.x * PanelLift;
        YCP   += panel.CollPt.y * PanelLift;
        ZCP   += panel.CollPt.z * PanelLift;

        LeverArm.set(panel.CollPt.x - CoG.x, panel.CollPt.y, panel.CollPt.z-CoG.z);
        GeomMoment = LeverArm * PanelForce; // N.m/q

        GCm  += GeomMoment.y;
        GRm  += GeomMoment.dot(WindDirection);
        GYm  += GeomMoment.dot(WindNormal);
    }
}


/**
 * Copies the data of an existing Body object to this Body
 * @param pBody the source Body object
 */
void Body::duplicate(Body const *pBody)
{
    if(!pBody) return;

    m_Name        = pBody->m_Name;
    m_Color       = pBody->m_Color;
    m_nxPanels        = pBody->m_nxPanels;
    m_nhPanels        = pBody->m_nhPanels;
    m_LineType        = pBody->m_LineType;
    m_Description = pBody->m_Description;

    //copy the splined surface data
    m_SplineSurface.m_iuDegree    = pBody->m_SplineSurface.m_iuDegree;
    m_SplineSurface.m_ivDegree    = pBody->m_SplineSurface.m_ivDegree;
    m_SplineSurface.m_iRes        = pBody->m_SplineSurface.m_iRes;
    m_SplineSurface.m_Bunch       = pBody->m_SplineSurface.m_Bunch;
    m_SplineSurface.m_EdgeWeightu = pBody->m_SplineSurface.m_EdgeWeightu;
    m_SplineSurface.m_EdgeWeightv = pBody->m_SplineSurface.m_EdgeWeightv;

    m_SplineSurface.clearFrames();
    m_xPanels.clear();

    for(int i=0; i<pBody->frameCount(); i++)
    {
        m_SplineSurface.m_pFrame.append(new Frame);
        m_SplineSurface.m_pFrame[i]->copyFrame(pBody->m_SplineSurface.m_pFrame[i]);
    }


    m_xPanels = pBody->m_xPanels;
    m_hPanels = pBody->m_hPanels;

    m_XPanelPos = pBody->m_XPanelPos;

    setNURBSKnots();

    clearPointMasses();
    for(int im=0; im<pBody->m_PointMass.size(); im++)
    {
        m_PointMass.append(PointMass(pBody->m_PointMass.at(im)));
    }
    m_VolumeMass = pBody->m_VolumeMass;
}


/**
 * Calculates and returns the length of the Body measured from nose to tail.
 * @return the Body length
 */
double Body::length() const
{
    if(m_SplineSurface.m_pFrame.size()) return qAbs(m_SplineSurface.m_pFrame.last()->m_Position.x - m_SplineSurface.m_pFrame.first()->m_Position.x);
    else                                return 0.0;
}


/**
 * Returns the posistion of the Body nose
 * @return the Vector3d which defines the position of the Body nose.
 */
Vector3d Body::leadingPoint() const
{
    if(m_SplineSurface.m_pFrame.size())
    {
        return Vector3d(m_SplineSurface.m_pFrame[0]->m_Position.x,
                0.0,
                (m_SplineSurface.m_pFrame[0]->m_CtrlPoint.first().z + m_SplineSurface.m_pFrame[0]->m_CtrlPoint.last().z)/2.0 );
    }
    else return Vector3d(0.0, 0.0, 0.0);
}


/**
 * Returns the length of a 360 degree hoop arc at a given axial position of the Body
 * Used to evaluate local volume and inertias.
 * @param x the longitudinal position at which the arc length is to be calculated.
 * @return  the arc length, in meters.
 */
double Body::getSectionArcLength(double x) const
{
    //NURBS only
    if(m_LineType==xfl::BODYPANELTYPE) return 0.0;
    // aproximate arc length, used for inertia estimations
    double length = 0.0;
    double ux = getu(x);
    Vector3d Pt, Pt1;
    getPoint(ux, 0.0, true, Pt1);

    int NPoints = 10;//why not ?
    for(int i=1; i<=NPoints; i++)
    {
        getPoint(ux, double(i)/double(NPoints), true, Pt);
        length += sqrt((Pt.y-Pt1.y)*(Pt.y-Pt1.y) + (Pt.z-Pt1.z)*(Pt.z-Pt1.z));
        Pt1.y = Pt.y;
        Pt1.z = Pt.z;
    }
    return length*2.0; //to account for left side.
}


Vector3d Body::centerPoint(double u) const
{
    Vector3d Top, Bot;
    getPoint(u, 0.0, true, Top);
    getPoint(u, 1.0, true, Bot);
    return (Top+Bot)/2.0;
}


/**
 * Calculates the absolute position of a point on the NURBS from its parametric coordinates.
 * @param u the value of the parameter in the longitudinal direction
 * @param v the value of the parameter in the hoop direction
 * @param bRight if true, the position of the point will be returned for the right side,
 * and for the left side if false
 * @param Pt the calculated point position
 */
void Body::getPoint(double u, double v, bool bRight, Vector3d &Pt) const
{
    m_SplineSurface.getPoint(u, v, Pt);
    if(!bRight)  Pt.y = -Pt.y;
}


/**
 * Returns the absolute position of a point on the NURBS from its parametric coordinates.
 * @param u the value of the parameter in the longitudinal direction
 * @param v the value of the parameter in the hoop direction
 * @param bRight if true, the position of the point will be returned for the right side,
 * and for the left side if false
 */
Vector3d Body::Point(double u, double v, bool bRight) const
{
    Vector3d Pt = m_SplineSurface.point(u, v);
    if(!bRight)  Pt.y = -Pt.y;
    return Pt;
}


/**
 * Returns the value of the longitudinal parameter given the absolute X-position ON the NURBS surface.
 * @param x in input, the longitudinal position
 * @return the longitudinal paramater on the NURBS surface
 */
double Body::getu(double x) const
{
    return m_SplineSurface.getu(x,0.0);
}

/**
 * For a NURBS surface: Given a value of the longitudinal parameter and a vector in the yz plane, returns the
 * value of the hoop paramater for the intersection of a ray originating on the x-axis
 * and directed along the input vector
 * @param u in input, the value of the longitudinal parameter
 * @param r the vector which defines the ray's direction
 * @param bRight true if the intersection should be calculated on the Body's right side, and flase if on the left
 * @return the value of the hoop parameter
 */
double Body::getv(double u, Vector3d r, bool bRight) const
{
    double sine = 10000.0;

    if(u<=0.0)          return 0.0;
    if(u>=1.0)          return 0.0;
    if(r.norm()<1.0e-5) return 0.0;

    int iter=0;
    double v, v1, v2;

    sine = 10000.0;
    iter = 0;
    r.normalize();
    v1 = 0.0; v2 = 1.0;

    while(qAbs(sine)>1.0e-4 && iter<200)
    {
        v=(v1+v2)/2.0;
        t_R = Point(u, v, bRight);
        t_R.x = 0.0;
        t_R.normalize();//t_R is the unit radial vector for u,v

        sine = (r.y*t_R.z - r.z*t_R.y);

        if(bRight)
        {
            if(sine>0.0) v1 = v;
            else         v2 = v;
        }
        else
        {
            if(sine>0.0) v2 = v;
            else         v1 = v;
        }
        iter++;
    }
    return (v1+v2)/2.0;
}




/**
 * Inserts a control point in the selected Frame.
 * @param Real the Vector3d which defines the point to insert
 * @return the index of the control point in the array; control points are indexed from bottom to top on the left side.
 */
int Body::insertPoint(Vector3d Real)
{
    int n = activeFrame()->insertPoint(Real, 3);
    for (int i=0; i<frameCount(); i++)
    {
        Frame *pFrame = m_SplineSurface.m_pFrame[i];
        if(pFrame != activeFrame())
        {
            pFrame->insertPoint(n);
        }
    }

    m_hPanels.insert(n, 1);

    setNURBSKnots();
    return n;
}


/**
 * Inserts a new Frame object in the Body definition
 * @param iFrame the index of the frame before which a new Frame will be inserted
 * @return the index of the Frame which has been inserted; Frame objects are indexed from nose to tail
 */
int Body::insertFrameBefore(int iFrame)
{
    Frame *pFrame = new Frame(sideLineCount());
    if(iFrame==0)
    {
        pFrame->setuPosition(frame(0)->position().x-0.1);
        m_SplineSurface.m_pFrame.prepend(pFrame);
    }
    else
    {
        pFrame->setuPosition((frame(iFrame)->position().x+frame(iFrame-1)->position().x)/2.0);

        int n = iFrame;
        m_SplineSurface.m_pFrame.insert(n, pFrame);

        for (int ic=0; ic<sideLineCount(); ic++)
        {
            m_SplineSurface.m_pFrame[n]->m_CtrlPoint[ic].x = (m_SplineSurface.m_pFrame[n-1]->m_CtrlPoint[ic].x + m_SplineSurface.m_pFrame[n+1]->m_CtrlPoint[ic].x)/2.0;
            m_SplineSurface.m_pFrame[n]->m_CtrlPoint[ic].y = (m_SplineSurface.m_pFrame[n-1]->m_CtrlPoint[ic].y + m_SplineSurface.m_pFrame[n+1]->m_CtrlPoint[ic].y)/2.0;
            m_SplineSurface.m_pFrame[n]->m_CtrlPoint[ic].z = (m_SplineSurface.m_pFrame[n-1]->m_CtrlPoint[ic].z + m_SplineSurface.m_pFrame[n+1]->m_CtrlPoint[ic].z)/2.0;
        }
    }
    m_xPanels.insert(iFrame, 1);

    setNURBSKnots();
    return iFrame;
}


/**
 * Inserts a new Frame object in the Body definition
 * @param iFrame the index of the frame after which a new Frame will be inserted
 * @return the index of the Frame which has been inserted; Frame objects are indexed from nose to tail
 */
int Body::insertFrameAfter(int iFrame)
{
    Frame *pFrame = new Frame(sideLineCount());
    if(iFrame==frameCount()-1)
    {
        pFrame->setuPosition(frame(iFrame)->position().x+0.1);
        m_SplineSurface.m_pFrame.append(pFrame);
    }
    else
    {
        pFrame->setuPosition((frame(iFrame)->position().x+frame(iFrame+1)->position().x)/2.0);

        int n = iFrame+1;
        m_SplineSurface.m_pFrame.insert(n, pFrame);

        for (int ic=0; ic<sideLineCount(); ic++)
        {
            m_SplineSurface.m_pFrame[n]->m_CtrlPoint[ic].x = (m_SplineSurface.m_pFrame[n-1]->m_CtrlPoint[ic].x + m_SplineSurface.m_pFrame[n+1]->m_CtrlPoint[ic].x)/2.0;
            m_SplineSurface.m_pFrame[n]->m_CtrlPoint[ic].y = (m_SplineSurface.m_pFrame[n-1]->m_CtrlPoint[ic].y + m_SplineSurface.m_pFrame[n+1]->m_CtrlPoint[ic].y)/2.0;
            m_SplineSurface.m_pFrame[n]->m_CtrlPoint[ic].z = (m_SplineSurface.m_pFrame[n-1]->m_CtrlPoint[ic].z + m_SplineSurface.m_pFrame[n+1]->m_CtrlPoint[ic].z)/2.0;
        }
    }

    m_xPanels.insert(iFrame+1, 1);

    setNURBSKnots();

    return iFrame+1;
}


/**
 * Inserts a new Frame object in the Body definition
 * @param Real the Vector3d which defines the x and z coordinates of the Frame to insert
 * @return the index of the Frame which has been inserted; Frame objects are indexed from nose to tail
 */
int Body::insertFrame(Vector3d Real)
{
    int k=0, n=0;

    if(Real.x<m_SplineSurface.m_pFrame[0]->m_Position.x)
    {
        m_SplineSurface.m_pFrame.prepend(new Frame(sideLineCount()));
        for (k=0; k<sideLineCount(); k++)
        {
            m_SplineSurface.m_pFrame.first()->m_CtrlPoint[k].set(Real.x,0.0,Real.z);
        }
        m_SplineSurface.m_pFrame.first()->setuPosition(Real.x);
    }
    else if(Real.x>m_SplineSurface.m_pFrame.last()->m_Position.x)
    {
        m_SplineSurface.m_pFrame.append(new Frame(sideLineCount()));

        for (k=0; k<sideLineCount(); k++)
        {
            m_SplineSurface.m_pFrame.last()->m_CtrlPoint[k].set(0.0,0.0,Real.z);
        }
        m_SplineSurface.m_pFrame.last()->setuPosition(Real.x);
    }
    else
    {
        for (n=0; n<frameCount()-1; n++)
        {
            if(m_SplineSurface.m_pFrame[n]->m_Position.x<=Real.x  &&  Real.x<m_SplineSurface.m_pFrame[n+1]->m_Position.x)
            {
                m_SplineSurface.m_pFrame.insert(n+1, new Frame(sideLineCount()));
                m_xPanels.insert(n+1,1);

                for (k=0; k<sideLineCount(); k++)
                {
                    m_SplineSurface.m_pFrame[n+1]->m_CtrlPoint[k].x = (m_SplineSurface.m_pFrame[n]->m_CtrlPoint[k].x + m_SplineSurface.m_pFrame[n+2]->m_CtrlPoint[k].x)/2.0;
                    m_SplineSurface.m_pFrame[n+1]->m_CtrlPoint[k].y = (m_SplineSurface.m_pFrame[n]->m_CtrlPoint[k].y + m_SplineSurface.m_pFrame[n+2]->m_CtrlPoint[k].y)/2.0;
                    m_SplineSurface.m_pFrame[n+1]->m_CtrlPoint[k].z = (m_SplineSurface.m_pFrame[n]->m_CtrlPoint[k].z + m_SplineSurface.m_pFrame[n+2]->m_CtrlPoint[k].z)/2.0;
                }
                break;
            }
        }
        if(n+1<frameCount())
        {
            m_SplineSurface.m_pFrame[n+1]->setuPosition(Real.x);
            double trans = Real.z - (m_SplineSurface.m_pFrame[n+1]->m_CtrlPoint[0].z + m_SplineSurface.m_pFrame[n+1]->m_CtrlPoint.last().z)/2.0;
            for (k=0; k<sideLineCount(); k++)
            {
                m_SplineSurface.m_pFrame[n+1]->m_CtrlPoint[k].z += trans;
            }
        }
    }

    m_iActiveFrame = n+1;

    if(n>=frameCount())    m_iActiveFrame = frameCount();
    if(n<=0)            m_iActiveFrame = 0;
    m_iHighlightFrame = -1;

    m_xPanels.insert(n, 1);

    setNURBSKnots();

    return n+1;
}


/**
 * Intersects a line segment defined by two points with the Body's surface
 * @param A the first point which defines the line
 * @param B the second point which defines the line
 * @param I the intersection point
 * @param bRight true if the intersection was found on the right side, false if found on the left side.
 * @return true if an intersection point has been found, false otherwise.
 */
bool Body::intersect(Vector3d const &A, Vector3d const &B, Vector3d &I, bool bRight) const
{
    if(m_LineType==xfl::BODYPANELTYPE)
    {
        return intersectFlatPanels(A,B,I);
    }
    else if (m_LineType==xfl::BODYSPLINETYPE)
    {
        return intersectNURBS(A,B,I, bRight);
    }
    return false;
}

/**
 * Intersects a line segment defined by two points with a NURBS Body's surface
 * @param A the first point which defines the line
 * @param B the second point which defines the line
 * @param I the intersection point
 * @param bRight true if the intersection was found on the right side, false if found on the left side.
 * @return true if an intersection point has been found, false otherwise.
 */
bool Body::intersectNURBS(Vector3d A, Vector3d B, Vector3d &I, bool bRight) const
{
    //intersect line AB with right or left body surface
    //intersection point is I
    Vector3d N, tmp, M0, M1;
    double u=0, v=0, dist=0, t=0, tp=0;
    int iter = 0;
    int itermax = 20;
    double dmax = 1.0e-5;
    dist = 1000.0;//m

    M0.set(0.0, A.y, A.z);
    M1.set(0.0, B.y, B.z);

    if(M0.norm()<M1.norm())
    {
        tmp = A;        A   = B;        B   = tmp;
    }
    //M0 is the outside Point, M1 is the inside point
    M0 = A; M1 = B;

    //define which side to intersect with
    if(M0.y>=0.0) bRight = true; else bRight = false;

    if(!isInNURBSBody(M1.x, M1.z))
    {
        //consider no intersection (not quite true in special high dihedral cases)
        I = M1;
        return false;
    }

    I = (M0+M1)/2.0; t=0.5;

    while(dist>dmax && iter<itermax)
    {
        //store the previous parameter
        tp = t;
        //first we get the u parameter corresponding to point I
        u = getu(I.x);
        //        t_Q.Set(I.x, 0.0, 0.0);
        //        t_r = (I-t_Q);
        t_r.x = 0.0;
        t_r.y = I.y;
        t_r.z = I.z;
        v = getv(u, t_r, bRight);
        t_N = Point(u, v, bRight);

        //project t_N on M0M1 line
        t = - ( (M0.x - t_N.x) * (M1.x-M0.x) + (M0.y - t_N.y) * (M1.y-M0.y) + (M0.z - t_N.z)*(M1.z-M0.z))
                /( (M1.x -  M0.x) * (M1.x-M0.x) + (M1.y -  M0.y) * (M1.y-M0.y) + (M1.z -  M0.z)*(M1.z-M0.z));

        I.x = M0.x + t * (M1.x-M0.x);
        I.y = M0.y + t * (M1.y-M0.y);
        I.z = M0.z + t * (M1.z-M0.z);

        //        dist = sqrt((t_N.x-I.x)*(t_N.x-I.x) + (t_N.y-I.y)*(t_N.y-I.y) + (t_N.z-I.z)*(t_N.z-I.z));
        dist = qAbs(t-tp);
        iter++;
    }

    return dist<dmax;
}


/**
 * Intersects a line segment defined by two points with the PANEL defining the Body
 * @param A the first point which defines the line
 * @param B the second point which defines the line
 * @param I the intersection point
 * @return true if an intersection point has been found, false otherwise.
 */
bool Body::intersectFlatPanels(Vector3d const &A, Vector3d const &B, Vector3d &I) const
{
    bool b1=0, b2=0, b3=0, b4=0, b5=0;
    double r=0,s=0,t=0;
    Vector3d LA, TA, LB, TB, U, V, W, H, D1, D2, N, C, P;
    bool bIntersect = false;

    U = B-A;
    U.normalize();

    for (int i=0; i<frameCount()-1; i++)
    {
        for (int k=0; k<sideLineCount()-1; k++)
        {
            //build the four corner points of the Quad Panel
            LB.x =  m_SplineSurface.m_pFrame[i]->m_Position.x     ;
            LB.y =  m_SplineSurface.m_pFrame[i]->m_CtrlPoint[k].y  ;
            LB.z =  m_SplineSurface.m_pFrame[i]->m_CtrlPoint[k].z  ;

            TB.x =  m_SplineSurface.m_pFrame[i+1]->m_Position.x;
            TB.y =  m_SplineSurface.m_pFrame[i+1]->m_CtrlPoint[k].y;
            TB.z =  m_SplineSurface.m_pFrame[i+1]->m_CtrlPoint[k].z;

            LA.x =  m_SplineSurface.m_pFrame[i]->m_Position.x     ;
            LA.y =  m_SplineSurface.m_pFrame[i]->m_CtrlPoint[k+1].y;
            LA.z =  m_SplineSurface.m_pFrame[i]->m_CtrlPoint[k+1].z;

            TA.x =  m_SplineSurface.m_pFrame[i+1]->m_Position.x;
            TA.y =  m_SplineSurface.m_pFrame[i+1]->m_CtrlPoint[k+1].y;
            TA.z =  m_SplineSurface.m_pFrame[i+1]->m_CtrlPoint[k+1].z;

            //does it intersect the right panel ?
            C = (LA + LB + TA + TB)/4.0;

            D1 = LA - TB;
            D2 = LB - TA;

            N = D2 * D1;
            N.normalize();

            r = (C.x-A.x)*N.x + (C.y-A.y)*N.y + (C.z-A.z)*N.z ;
            s = (U.x*N.x + U.y*N.y + U.z*N.z);
            if(qAbs(s)>0.0)
            {
                t = r/s;
                P = A + U * t;

                // P is inside panel if on left side of each panel side
                W = P  - TA;
                V = TB - TA;
                t_Prod = V*W;
                if(t_Prod.norm() <1.0e-4 || t_Prod.dot(N)>=0.0) b1 = true; else b1 = false;

                W = P  - TB;
                V = LB - TB;
                t_Prod = V*W;
                if(t_Prod.norm() <1.0e-4 || t_Prod.dot(N)>=0.0) b2 = true; else b2 = false;

                W = P  - LB;
                V = LA - LB;
                t_Prod = V*W;
                if(t_Prod.norm() <1.0e-4 || t_Prod.dot(N)>=0.0) b3 = true; else b3 = false;

                W = P  - LA;
                V = TA - LA;
                t_Prod = V*W;
                if(t_Prod.norm() <1.0e-4 || t_Prod.dot(N)>=0.0) b4 = true; else b4 = false;

                W = A-P;
                V = B-P;
                if(W.dot(V)<=0.0)      b5 = true; else b5 = false;

                if(b1 && b2 && b3 && b4 && b5)
                {
                    bIntersect = true;
                    break;
                }
            }

            //does it intersect the left panel ?

            LB.y = -LB.y;
            LA.y = -LA.y;
            TB.y = -TB.y;
            TA.y = -TA.y;

            C = (LA + LB + TA + TB)/4.0;

            D1 = LA - TB;
            D2 = LB - TA;

            N = D2 * D1;
            N.normalize();

            r = (C.x-A.x)*N.x + (C.y-A.y)*N.y + (C.z-A.z)*N.z ;
            s = (U.x*N.x + U.y*N.y + U.z*N.z);

            if(qAbs(s)>0.0)
            {
                t = r/s;
                P = A + U * t;

                // P is inside panel if on left side of each panel side
                W = P  - TA;
                V = TB - TA;
                t_Prod = V*W;
                if(t_Prod.norm() <1.0e-4 || t_Prod.dot(N)>=0.0) b1 = true; else b1 = false;

                W = P  - TB;
                V = LB - TB;
                t_Prod = V*W;
                if(t_Prod.norm() <1.0e-4 || t_Prod.dot(N)>=0.0) b2 = true; else b2 = false;

                W = P  - LB;
                V = LA - LB;
                t_Prod = V*W;
                if(t_Prod.norm() <1.0e-4 || t_Prod.dot(N)>=0.0) b3 = true; else b3 = false;

                W = P  - LA;
                V = TA - LA;
                t_Prod = V*W;
                if(t_Prod.norm() <1.0e-4 || t_Prod.dot(N)>=0.0) b4 = true; else b4 = false;

                W = A-P;
                V = B-P;
                if(W.dot(V)<=0.0)       b5 = true; else b5 = false;

                if(b1 && b2 && b3 && b4 && b5)
                {
                    bIntersect = true;
                    break;
                }
            }
        }
        if(bIntersect) break;
    }
    if(bIntersect) I = P;
    return bIntersect;
}


/**
 * returns the index of the Frame  pointed by the input vector, or -10 if none
 * @param Real the input vector
 * @param ZoomFactor the view's scae factor
 * @return the Frame's index, or -10 if none found
 */
int Body::isFramePos(Vector3d Real, double ZoomFactor)
{
    for (int k=0; k<frameCount(); k++)
    {
        if (qAbs(Real.x-m_SplineSurface.m_pFrame[k]->m_Position.x) < 0.01 *length()/ZoomFactor &&
                qAbs(Real.z-m_SplineSurface.m_pFrame[k]->zPos())       < 0.01 *length()/ZoomFactor)
            return k;
    }
    return -10;
}


/**
 * Returns the true if the input point is inside the NURBS Body, false otherwise
 * @param Pt the input point, in the x-z plane, i.e. y=0
 * @return true if the point is inside the Body, false otherwise
 */
bool Body::isInNURBSBody(double x, double z) const
{
    double u = getu(x);
    if (u <= 0.0 || u >= 1.0) return false;

    return (Point(u,1,true).z<z && z<Point(u,0,true).z);
}


/**
 * Returns the true if the input point is inside the NURBS Body, false otherwise
 * @param Pt the input point
 * @return true if the point is inside the Body, false otherwise
 */
bool Body::isInNURBSBodyOld(Vector3d Pt)
{
    double u = getu(Pt.x);

    if (u <= 0.0 || u >= 1.0) return false;

    t_r.set(0.0, Pt.y, Pt.z);

    bool bRight = (Pt.y>=0.0);

    double v = getv(u, t_r, bRight);
    getPoint(u, v, bRight, t_N);

    t_N.x = 0.0;

    if(t_r.norm()>t_N.norm()) return false;
    return true;
}


/**
 * Removes a Frame from the Body
 * @param n the index of the Frame to remove
 * @return the index of the new active Frame object
 */
int Body::removeFrame(int n)
{
    m_SplineSurface.m_pFrame.removeAt(n);

    m_iActiveFrame = qMin(n, m_SplineSurface.m_pFrame.size());
    m_iHighlightFrame = -1;
    setNURBSKnots();
    return m_iActiveFrame;
}


/**
 * Removes the active Frame from the Body
 */
void Body::removeActiveFrame()
{
    m_SplineSurface.removeFrame(m_iActiveFrame);

    m_iHighlightFrame = -1;
    setNURBSKnots();
}

/**
 * Removes a sideline from the Body
 * @param SideLine the index of the sideline to remove
 */
void Body::removeSideLine(int SideLine)
{
    for (int i=0; i<m_SplineSurface.m_pFrame.size(); i++)
    {
        m_SplineSurface.m_pFrame[i]->removePoint(SideLine);
    }
    setNURBSKnots();
}


/**
 * Scales either the Frame or the entire Body object
 * @param XFactor the scaling factor in the x direction
 * @param YFactor the scaling factor in the y direction
 * @param ZFactor the scaling factor in the z direction
 * @param bFrameOnly if true, only a Frame shall be scaled, otherwise the whole Body shall be scaled
 * @param FrameID the index of the Frame to scale
 */
void Body::scale(double XFactor, double YFactor, double ZFactor, bool bFrameOnly, int FrameID)
{
    for (int i=0; i<frameCount(); i++)
    {
        if((bFrameOnly &&  i==FrameID) || !bFrameOnly)
        {
            if(!bFrameOnly) m_SplineSurface.m_pFrame[i]->m_Position.x *= XFactor;

            for(int j=0; j<m_SplineSurface.m_pFrame[i]->m_CtrlPoint.size(); j++)
            {
                m_SplineSurface.m_pFrame[i]->m_CtrlPoint[j].x  = m_SplineSurface.m_pFrame[i]->m_Position.x;
                m_SplineSurface.m_pFrame[i]->m_CtrlPoint[j].y *= YFactor;
                m_SplineSurface.m_pFrame[i]->m_CtrlPoint[j].z *= ZFactor;
            }
        }
    }
    //    ComputeCenterLine();
}


/**
 * For a NURBS Body, sets the default position of the longitudinal parameters
 */
void Body::setPanelPos()
{
    double a = (m_Bunch+1.0)*.48 ;
    a = 1./(1.0-a);

    double norm = 1.0/(1.0+exp(0.5*a));

    m_XPanelPos.resize(m_nxPanels+1);
    for(int i=0; i<=m_nxPanels; i++)
    {
        double x = double(i)/double(m_nxPanels);
        double y = 1.0/(1.0+exp((0.5-x)*a));
        m_XPanelPos[i] = (0.5-((0.5-y)/(0.5-norm))/2.0);
    }
}


/**
 * Translates either a Frame or the whole Body in the xz plane
 * @param XTrans the component of the translation vector along the x-axis
 * @param YTrans unused
 * @param XTrans the component of the translation vector along the z-axis
 * @param bFrameOnly true if only a Frame is to be translated
 * @param FrameID the index of the Frame object to be translated
 */
void Body::translate(double XTrans, double , double ZTrans, bool bFrameOnly, int FrameID)
{
    for (int i=0; i<frameCount(); i++)
    {
        if((bFrameOnly &&  i==FrameID) || !bFrameOnly)
        {
            m_SplineSurface.m_pFrame[i]->m_Position.x += XTrans;
            //            m_SplineSurface.m_pFrame[i]->m_Position.y += YTrans;
            m_SplineSurface.m_pFrame[i]->m_Position.z += ZTrans;

            for(int j=0; j<m_SplineSurface.m_pFrame[i]->m_CtrlPoint.size(); j++)
            {
                m_SplineSurface.m_pFrame[i]->m_CtrlPoint[j].x += XTrans;
                m_SplineSurface.m_pFrame[i]->m_CtrlPoint[j].z += ZTrans;
            }
        }
    }
    //    ComputeCenterLine();
}


/**
 * Overloaded function
 * Translates either a Frame or the whole Body in the xz plane
 * @param T the Vector3d which defines the translation
 * @param bFrameOnly true if only a Frame is to be translated
 * @param FrameID the index of the Frame object to be translated
 */
void Body::translate(const Vector3d &T, bool bFrameOnly, int FrameID)
{
    translate(T.x, T.y, T.z, bFrameOnly, FrameID);
}


/**
 * Returns a pointer to the Frame object from its index
 * @param iFrame the index of the Frame object
 * @return a pointer to the Frame object
 */
Frame *Body::frame(int iFrame)
{
    if(iFrame>=0 && iFrame<frameCount()) return m_SplineSurface.m_pFrame[iFrame];
    return nullptr;
}


Frame const *Body::frameAt(int iFrame) const
{
    if(iFrame>=0 && iFrame<frameCount()) return m_SplineSurface.m_pFrame[iFrame];
    return nullptr;
}

/**
 * Returns the longitudinal position of a Frame defined by its index
 * @param iFrame the index of the Frame object
 * @return the absolute longitudinal position, in meters
 */
double Body::framePosition(int iFrame) const
{
    return m_SplineSurface.m_pFrame[iFrame]->m_Position.x;
}


/**
 * Returns a pointer to the active Frame object
 * @return a pointer to the active Frame object
 */
Frame *Body::activeFrame() const
{
    if(m_iActiveFrame>=0 && m_iActiveFrame<frameCount()) return m_SplineSurface.m_pFrame[m_iActiveFrame];
    else                                                 return nullptr;
}


/**
 * Sets as active the Frame pointed in input
 * @param pFrame a pointer to the Frame object to be set as active
 * @return the index of the newly selected Frame
 */
int Body::setActiveFrame(Frame const*pFrame)
{
    for(int ifr=0; ifr<m_SplineSurface.m_pFrame.size(); ifr++)
    {
        if(m_SplineSurface.m_pFrame.at(ifr)==pFrame)
        {
            m_iActiveFrame = ifr;
            return m_iActiveFrame;
        }
    }
    return -1;
}


/**
 * Sets as active the Frame pointed in input
 * @param iFrame the index of the newly selected Frame
 * @return a pointer to the Frame object to be set as active
 */
Frame * Body::setActiveFrame(int iFrame)
{
    m_iActiveFrame = iFrame;
    return frame(iFrame);
}


/**
 * Calculates the inertia tensor in geometrical (body) axis:
 *  - adds the volume inertia AND the point masses of all components
 *  - the body axis is the frame in which all the geometry has been defined
 *  - the origin=BodyFrame;
 *  - the center of gravity is calculated from component masses and is NOT the CoG defined in the polar
 */
void Body::computeBodyAxisInertia()
{
    Vector3d LA, VolumeCoG;
    double Ixx(0), Iyy(0), Izz(0), Ixz(0);

    computeVolumeInertia(VolumeCoG, Ixx, Iyy, Izz, Ixz);
    double TotalMass = m_VolumeMass;

    m_CoG = VolumeCoG *m_VolumeMass;

    // add point masses
    for(int im=0; im<m_PointMass.size(); im++)
    {
        PointMass const &pm = m_PointMass.at(im);
        TotalMass += pm.mass();
        m_CoG += pm.position() * pm.mass();
    }

    if(TotalMass>0) m_CoG = m_CoG/TotalMass;
    else            m_CoG.set(0.0,0.0,0.0);

    // The CoG position is now available, so calculate the inertia w.r.t the CoG
    // using Huygens theorem
    // LA is the distance vector from the centre of mass to the new axis
    LA = VolumeCoG - m_CoG;
    m_CoGIxx = Ixx + m_VolumeMass * (LA.y*LA.y + LA.z*LA.z);
    m_CoGIyy = Iyy + m_VolumeMass * (LA.x*LA.x + LA.z*LA.z);
    m_CoGIzz = Izz + m_VolumeMass * (LA.x*LA.x + LA.y*LA.y);
    m_CoGIxz = Ixz + m_VolumeMass * LA.x*LA.z;

    for(int im=0; im<m_PointMass.size(); im++)
    {
        PointMass const &pm = m_PointMass.at(im);
        LA = pm.position() - m_CoG;
        m_CoGIxx += pm.mass() * (LA.y*LA.y + LA.z*LA.z);
        m_CoGIyy += pm.mass() * (LA.x*LA.x + LA.z*LA.z);
        m_CoGIzz += pm.mass() * (LA.x*LA.x + LA.y*LA.y);
        m_CoGIxz += pm.mass() * (LA.x*LA.z);
    }
}


/**
 * Calculates the inertia of the Body's volume, in geometrical axis
 * Assumes that the mass is distributed homogeneously in the body's skin.
 * Homogeneity is questionable, but is a rather handy assumption.
 * Mass in the body's skin is reasonable, given that the point masses are added manually.
 */
void Body::computeVolumeInertia(Vector3d &CoG, double &CoGIxx, double &CoGIyy, double &CoGIzz, double &CoGIxz) const
{
    double ux(0), rho(0);
    double dj(0), dj1(0);
    double BodyArea(0);
    double SectionArea(0);
    double xpos(0), dl(0);
    Vector3d Pt, LATB, TALB, N, PLA, PTA, PLB, PTB, Top, Bot;

    CoG.set(0.0, 0.0, 0.0);
    CoGIxx = CoGIyy = CoGIzz = CoGIxz = 0.0;

    if(m_LineType==xfl::BODYPANELTYPE)
    {
        // we use the panel division
        //first get the wetted area

        for (int i=0; i<frameCount()-1; i++)
        {
            for (int k=0; k<sideLineCount()-1; k++)
            {
                //build the four corner points of the strips
                PLA.x =  framePosition(i);
                PLA.y =  frameAt(i)->m_CtrlPoint[k].y  ;
                PLA.z =  frameAt(i)->m_CtrlPoint[k].z  ;

                PLB.x = framePosition(i);
                PLB.y = frameAt(i)->m_CtrlPoint[k+1].y ;
                PLB.z = frameAt(i)->m_CtrlPoint[k+1].z ;

                PTA.x = framePosition(i+1);
                PTA.y = frameAt(i+1)->m_CtrlPoint[k].y ;
                PTA.z = frameAt(i+1)->m_CtrlPoint[k].z ;

                PTB.x = framePosition(i+1);
                PTB.y = frameAt(i+1)->m_CtrlPoint[k+1].y;
                PTB.z = frameAt(i+1)->m_CtrlPoint[k+1].z;

                LATB = PTB - PLA;
                TALB = PLB - PTA;
                N = TALB * LATB;//panel area x2
                BodyArea += N.norm() /2.0;
            }
        }

        BodyArea *= 2.0;
        rho = m_VolumeMass/BodyArea;
        //First get the CoG position
        for (int i=0; i<frameCount()-1; i++)
        {
            for (int j=0; j<m_xPanels.at(i); j++)
            {
                dj  = double(j)  /double(m_xPanels.at(i));
                dj1 = double(j+1)/double(m_xPanels.at(i));
                SectionArea = 0.0;

                PLA.x = PLB.x = (1.0- dj) * framePosition(i)  +  dj * framePosition(i+1);
                PTA.x = PTB.x = (1.0-dj1) * framePosition(i)  + dj1 * framePosition(i+1);

                for (int k=0; k<sideLineCount()-1; k++)
                {
                    //build the four corner points of the strips
                    PLB.y = (1.0- dj) * frameAt(i)->m_CtrlPoint.at(k).y   +  dj * frameAt(i+1)->m_CtrlPoint.at(k).y;
                    PLB.z = (1.0- dj) * frameAt(i)->m_CtrlPoint.at(k).z   +  dj * frameAt(i+1)->m_CtrlPoint.at(k).z;

                    PTB.y = (1.0-dj1) * frameAt(i)->m_CtrlPoint.at(k).y   + dj1 * frameAt(i+1)->m_CtrlPoint.at(k).y;
                    PTB.z = (1.0-dj1) * frameAt(i)->m_CtrlPoint.at(k).z   + dj1 * frameAt(i+1)->m_CtrlPoint.at(k).z;

                    PLA.y = (1.0- dj) * frameAt(i)->m_CtrlPoint.at(k+1).y +  dj * frameAt(i+1)->m_CtrlPoint.at(k+1).y;
                    PLA.z = (1.0- dj) * frameAt(i)->m_CtrlPoint.at(k+1).z +  dj * frameAt(i+1)->m_CtrlPoint.at(k+1).z;

                    PTA.y = (1.0-dj1) * frameAt(i)->m_CtrlPoint.at(k+1).y + dj1 * frameAt(i+1)->m_CtrlPoint.at(k+1).y;
                    PTA.z = (1.0-dj1) * frameAt(i)->m_CtrlPoint.at(k+1).z + dj1 * frameAt(i+1)->m_CtrlPoint.at(k+1).z;

                    LATB = PTB - PLA;
                    TALB = PLB - PTA;
                    N = TALB * LATB;//panel area x2
                    SectionArea += N.norm() /2.0;
                }
                SectionArea *= 2.0;// to account for right side;

                // get center point for this section
                Pt.x = (PLA.x + PTA.x)/2.0;
                Pt.y = 0.0;
                Pt.z = ((1.0-dj)  * m_SplineSurface.m_pFrame.at(i)->zPos() + dj  * m_SplineSurface.m_pFrame.at(i+1)->zPos()
                       +(1.0-dj1) * m_SplineSurface.m_pFrame.at(i)->zPos() + dj1 * m_SplineSurface.m_pFrame.at(i+1)->zPos())/2.0;

                CoG.x += SectionArea*rho * Pt.x;
                CoG.y += SectionArea*rho * Pt.y;
                CoG.z += SectionArea*rho * Pt.z;
            }
        }
        if(m_VolumeMass>PRECISION) CoG *= 1.0/ m_VolumeMass;
        else                       CoG.set(0.0, 0.0, 0.0);

        //Then Get Inertias
        // we could do it one calculation, for CG and inertia, by using Huyghens/steiner theorem
        for (int i=0; i<frameCount()-1; i++)
        {
            for (int j=0; j<m_xPanels.at(i); j++)
            {
                dj  = double(j)  /double(m_xPanels.at(i));
                dj1 = double(j+1)/double(m_xPanels.at(i));
                SectionArea = 0.0;

                PLA.x = PLB.x = (1.0- dj) * framePosition(i)   +  dj * framePosition(i+1);
                PTA.x = PTB.x = (1.0-dj1) * framePosition(i)   + dj1 * framePosition(i+1);

                for (int k=0; k<sideLineCount()-1; k++)
                {
                    //build the four corner points of the strips
                    PLB.y = (1.0- dj) * frameAt(i)->m_CtrlPoint.at(k).y   +  dj * frameAt(i+1)->m_CtrlPoint.at(k).y;
                    PLB.z = (1.0- dj) * frameAt(i)->m_CtrlPoint.at(k).z   +  dj * frameAt(i+1)->m_CtrlPoint.at(k).z;

                    PTB.y = (1.0-dj1) * frameAt(i)->m_CtrlPoint.at(k).y   + dj1 * frameAt(i+1)->m_CtrlPoint.at(k).y;
                    PTB.z = (1.0-dj1) * frameAt(i)->m_CtrlPoint.at(k).z   + dj1 * frameAt(i+1)->m_CtrlPoint.at(k).z;

                    PLA.y = (1.0- dj) * frameAt(i)->m_CtrlPoint.at(k+1).y +  dj * frameAt(i+1)->m_CtrlPoint.at(k+1).y;
                    PLA.z = (1.0- dj) * frameAt(i)->m_CtrlPoint.at(k+1).z +  dj * frameAt(i+1)->m_CtrlPoint.at(k+1).z;

                    PTA.y = (1.0-dj1) * frameAt(i)->m_CtrlPoint.at(k+1).y + dj1 * frameAt(i+1)->m_CtrlPoint.at(k+1).y;
                    PTA.z = (1.0-dj1) * frameAt(i)->m_CtrlPoint.at(k+1).z + dj1 * frameAt(i+1)->m_CtrlPoint.at(k+1).z;

                    LATB = PTB - PLA;
                    TALB = PLB - PTA;
                    N = TALB * LATB;//panel area x2
                    SectionArea += N.norm() /2.0;
                }
                SectionArea *= 2.0;// to account for right side;

                // get center point for this section
                Pt.x = (PLA.x + PTA.x)/2.0;
                Pt.y = 0.0;
                Pt.z = ((1.0-dj)  * m_SplineSurface.m_pFrame.at(i)->zPos() + dj  * m_SplineSurface.m_pFrame.at(i+1)->zPos()
                       +(1.0-dj1) * m_SplineSurface.m_pFrame.at(i)->zPos() + dj1 * m_SplineSurface.m_pFrame.at(i+1)->zPos())/2.0;

                CoGIxx += SectionArea*rho * ( (Pt.y-CoG.y)*(Pt.y-CoG.y) + (Pt.z-CoG.z)*(Pt.z-CoG.z) );
                CoGIyy += SectionArea*rho * ( (Pt.x-CoG.x)*(Pt.x-CoG.x) + (Pt.z-CoG.z)*(Pt.z-CoG.z) );
                CoGIzz += SectionArea*rho * ( (Pt.x-CoG.x)*(Pt.x-CoG.x) + (Pt.y-CoG.y)*(Pt.y-CoG.y) );
                CoGIxz += SectionArea*rho * ( (Pt.x-CoG.x)*(Pt.z-CoG.z) );
            }
        }
    }
    else if(m_LineType==xfl::BODYSPLINETYPE)
    {
        int NSections = 20;//why not ?
        xpos = framePosition(0);
        dl = length()/double(NSections-1);

        for (int j=0; j<NSections-1; j++)
        {
            BodyArea += dl * (getSectionArcLength(xpos)+ getSectionArcLength(xpos+dl)) /2.0;
            xpos += dl;
        }

        rho = m_VolumeMass / BodyArea;

        // First evaluate CoG, assuming each section is a point mass
        xpos = framePosition(0);
        for (int j=0; j<NSections-1; j++)
        {
            SectionArea = dl * (getSectionArcLength(xpos)+ getSectionArcLength(xpos+dl))/2.0;
            Pt.x = xpos + dl/2.0;
            Pt.y = 0.0;
            ux = getu(Pt.x);
            getPoint(ux, 0.0, true, Top);
            getPoint(ux, 1.0, true, Bot);
            Pt.z = (Top.z + Bot.z)/2.0;
            xpos += dl;

            CoG.x += SectionArea*rho * Pt.x;
            CoG.y += SectionArea*rho * Pt.y;
            CoG.z += SectionArea*rho * Pt.z;
        }
        if(m_VolumeMass>PRECISION) CoG *= 1.0/ m_VolumeMass;
        else                       CoG.set(0.0, 0.0, 0.0);

        // Next evaluate inertia, assuming each section is a point mass
        xpos = framePosition(0);
        for (int j=0; j<NSections-1; j++)
        {
            SectionArea = dl * (getSectionArcLength(xpos)+ getSectionArcLength(xpos+dl))/2.0;
            Pt.x = xpos + dl/2.0;
            Pt.y = 0.0;
            ux = getu(Pt.x);
            getPoint(ux, 0.0, true, Top);
            getPoint(ux, 1.0, true, Bot);
            Pt.z = (Top.z + Bot.z)/2.0;

            CoGIxx += SectionArea*rho * ( (Pt.y-CoG.y)*(Pt.y-CoG.y) + (Pt.z-CoG.z)*(Pt.z-CoG.z) );
            CoGIyy += SectionArea*rho * ( (Pt.x-CoG.x)*(Pt.x-CoG.x) + (Pt.z-CoG.z)*(Pt.z-CoG.z) );
            CoGIzz += SectionArea*rho * ( (Pt.x-CoG.x)*(Pt.x-CoG.x) + (Pt.y-CoG.y)*(Pt.y-CoG.y) );
            CoGIxz += SectionArea*rho * ( (Pt.x-CoG.x)*(Pt.z-CoG.z) );

            xpos += dl;
        }
    }
}


/**
 * Returns the sum of volume and point masses
 * @return the sum of volume and point masses
 */
double Body::totalMass() const
{
    double TotalMass = m_VolumeMass;
    for(int im=0; im<m_PointMass.size(); im++)
        TotalMass += m_PointMass.at(im).mass();
    return TotalMass;
}


/**
 * Sets a non unit weight on the edges of the NURBS surface
 * @param uw the weight on the longitudinal edges ( @todo check )
 * @param uw the weight on the hoop edges ( @todo check )
 */
void Body::setEdgeWeight(double uw, double vw)
{
    m_SplineSurface.m_EdgeWeightu = uw;
    m_SplineSurface.m_EdgeWeightv = vw;
}


/**
 * Exports the definition to text file. The definition is the type of body, the position of the control points,
 * the NURBS degree and other related data.
 * @return true if the Body definition was correctly exported, false otherwise.
 */
bool Body::exportBodyDefinition(QTextStream &outStream, double mtoUnit) const
{
    QString strong;

    strong = "\n# This file defines a body geometry\n";
    outStream << strong;
    strong = "# The frames are defined from nose to tail\n";
    outStream << strong;
    strong = "# The numer of sidelines is defined by the number of points of the first frame\n";
    outStream << strong;
    strong = "# Each of the next frames should have the same number of points as the first\n";
    outStream << strong;
    strong = "# For each frame, the points are defined for the right half of the body, \n";
    outStream << strong;
    strong = "# in the clockwise direction aft looking forward\n\n";
    outStream << strong;

    outStream << (m_Name+"\n\n");
    outStream << ("BODYTYPE\n");
    if(m_LineType==xfl::BODYPANELTYPE)  outStream << (" 1        # Flat Panels (1) or NURBS (2)\n\n");
    if(m_LineType==xfl::BODYSPLINETYPE) outStream << (" 2        # Flat Panels (1) or NURBS (2)\n\n");

    outStream << ("OFFSET\n");
    outStream << ("0.0     0.0     0.0     #Total body offset (Y-coord is ignored)\n\n");

    for(int i=0; i<frameCount(); i++)
    {
        outStream << ("FRAME\n");
        for(int j=0; j<sideLineCount(); j++)
        {
            strong = QString("%1     %2    %3\n")
                    .arg(m_SplineSurface.m_pFrame[i]->m_Position.x     * mtoUnit,14,'f',7)
                    .arg(m_SplineSurface.m_pFrame[i]->m_CtrlPoint[j].y * mtoUnit,14,'f',7)
                    .arg(m_SplineSurface.m_pFrame[i]->m_CtrlPoint[j].z * mtoUnit,14,'f',7);
            outStream << (strong);
        }
        outStream << ("\n");
    }

    return true;

}


/**
 * Exports the points on the surface to text file. No real purpose to this function, except to provide users with
 * some mean to access the surface geometry.
 * @todo remove.
 */
void Body::exportGeometry(QTextStream &outStream, int type, double mtoUnit, int nx, int nh) const
{
    QString strong, LengthUnit,str;
    int k,l;
    double u, v;
    Vector3d Point;


    if(type==1)    str="";
    else        str=", ";

    outStream  << m_Name;
    outStream  << ("\n\n");

    outStream  << (("Right Surface Points\n"));
    if(type==1) strong = "        x("+LengthUnit+")          y("+LengthUnit+")          z("+LengthUnit+")\n";
    else        strong = " x("+LengthUnit+"),"+"y("+LengthUnit+"),"+"z("+LengthUnit+")\n";
    outStream  << (strong);

    for (k=0; k<nx; k++)
    {
        strong = QString(("  Cross Section ")+str+"%1\n").arg(k+1,3);
        outStream  << (strong);

        u = double(k) / double(nx-1);

        for (l=0; l<nh; l++)
        {
            v = double(l) / double(nh-1);
            getPoint(u,  v, true, Point);

            //increased precision i.a.w. request #18
            /*            strong = QString("   %1"+str+"     %2"+str+"     %3\n")
                     .arg(Point.x * mtoUnit,10,'f',3)
                     .arg(Point.y * mtoUnit,10,'f',3)
                     .arg(Point.z * mtoUnit,10,'f',3);
            outStream  << (strong);*/


            strong = QString(" %1"+str+" %2"+str+" %3\n")
                    .arg(Point.x * mtoUnit,16,'f',8)
                    .arg(Point.y * mtoUnit,16,'f',8)
                    .arg(Point.z * mtoUnit,16,'f',8);
            outStream << (strong);
        }
        outStream  << ("\n");
    }

    outStream  << ("\n\n");
}


/**
 * Reads the Body data from the old, obsolete and deprecated .wpa file format
 * @param ar the binary QDataStream from which the data shall be read
 * @param bIsStoring always false, since storing to this format is deprecated
 * @return true if the loading was successful, false otherwise
 */
bool Body::serializeBodyWPA(QDataStream &ar, bool bIsStoring)
{
    int ArchiveFormat=0;
    int k=0, p=0, nStations=0;
    float f=0,h=0;
    double x=0,y=0,z=0;

    if(bIsStoring)
    {
        // not storing to .wpa format anymore
    }
    else
    {
        int r,g,b;
        int NSideLines;
        //1006 : added body LEPosition
        //1005 : added body alpha color + provisions
        //1004 : QFLRv0.03    : added mass properties for inertia calculations
        //1003 : QFLR5 v0.02 : added body description field
        //1002 : Added axial and hoop mesh panel numbers for linetype fuselage
        //1001 : Added bunching parameter
        //1000 : first format
        ar >> ArchiveFormat;
        if(ArchiveFormat<1000 || ArchiveFormat>1100) return false;

        xfl::readCString(ar, m_Name);
        if(ArchiveFormat>=1003) xfl::readCString(ar, m_Description);

        xfl::readCOLORREF(ar, r,g,b);
        m_Color = QColor(r,g,b);
        ar >> k;
        if(k==1) m_LineType = xfl::BODYPANELTYPE;
        else     m_LineType = xfl::BODYSPLINETYPE;
        ar >> NSideLines;
        ar >> nStations;
        ar >> k; //m_iRes
        ar >> k >> k; //ar >> m_nxDegree >> m_nhDegree;
        ar >> m_nxPanels >> m_nhPanels;

        if(ArchiveFormat>=1001)
        {
            ar >> f; m_Bunch = double(f);
        }

        m_xPanels.clear();
        m_hPanels.clear();
        if(ArchiveFormat>=1002)
        {
            for(k=0; k<nStations; k++)
            {
                ar >> p;
                m_xPanels.append(p);
            }
            for(k=0; k<NSideLines; k++)
            {
                ar >> p;
                m_hPanels.append(p);
            }
        }

        ar >> k; // m_bClosedSurface
        if(k!=0 && k!=1) return false;

        m_SplineSurface.clearFrames();
        for(k=0; k<nStations; k++)
        {
            m_SplineSurface.m_pFrame.append(new Frame);
            m_SplineSurface.m_pFrame[k]->serializeFrame(ar, bIsStoring);
        }
        //Serialize Bodyline
        ar >>k;//    ar >> m_NStations; again ?

        for (k=0; k<nStations;k++)
        {
            ar >> f; m_SplineSurface.m_pFrame[k]->setuPosition(double(f));
            for(int ic=0; ic<m_SplineSurface.m_pFrame[k]->m_CtrlPoint.size(); ic++)
            {
                m_SplineSurface.m_pFrame[k]->m_CtrlPoint[ic].x = double(f);
            }
            ar >> f; //m_FramePosition[k].z =f;
        }
        if(ArchiveFormat>=1004)
        {
            ar >> f;  m_VolumeMass = double(f);
            int nMass;
            ar >> nMass;

            QVarLengthArray<double> mass;
            QVarLengthArray<Vector3d> position;
            QVarLengthArray<QString> tag;

            for(int im=0; im<nMass; im++)
            {
                ar >> f;
                mass.append(double(f));
            }
            for(int im=0; im<nMass; im++)
            {
                ar >> f >> g >> h;
                position.append({double(f),double(g),double(h)});
            }
            for(int im=0; im<nMass; im++)
            {
                tag.append("");
                xfl::readCString(ar, tag[im]);
            }

            clearPointMasses();
            for(int im=0; im<nMass; im++)
            {
                m_PointMass.append({mass.at(im), position.at(im), tag.at(im)});
            }
        }
        ar >> f;
        if(ArchiveFormat>=1005) m_Color.setAlphaF(f);
        if(ArchiveFormat>=1006)
        {
            ar >> x >> y >> z;
            //            m_BodyLEPosition.Set(x,y,z);
        }
        //        else m_BodyLEPosition.Set(0.0,0.0,0.0);
        ar >> f;

        setNURBSKnots();
    }
    return true;
}



/**
 * Serialize the Body data to or from a QDataStream associated to a .xfl file
 * @param ar the binary QDataStream from/to which the data shall be directed
 * @param bIsStoring true if storing, false if loading the data
 * @return true if the operation was successful, false otherwise
 */
bool Body::serializeBodyXFL(QDataStream &ar, bool bIsStoring)
{
    int ArchiveFormat;
    int i(0),k(0),n(0),p(0);

    double dble(0),m(0),px(0),py(0),pz(0);
    QString str;

    if(bIsStoring)
    {
        ar << 100006;

        ar << m_Name;
        ar << m_Description;

        xfl::writeQColor(ar, m_Color.red(), m_Color.green(), m_Color.blue(), m_Color.alpha());

        if(m_LineType==xfl::BODYPANELTYPE) ar << 1;
        else                                 ar << 2;

        ar << 0;
        ar << m_nxPanels << m_nhPanels;
        ar << m_Bunch;

        ar << sideLineCount();
        for(k=0; k<sideLineCount(); k++) ar << m_hPanels[k];

        ar << frameCount();
        for(k=0; k<frameCount(); k++)
        {
            ar << m_xPanels[k];
            ar << framePosition(k);
            m_SplineSurface.m_pFrame[k]->serializeFrame(ar, bIsStoring);
        }

        ar << m_VolumeMass;
        ar << m_PointMass.size();
        for(i=0; i<m_PointMass.size(); i++)
        {
            ar << m_PointMass.at(i).mass();
            ar << m_PointMass.at(i).position().x << m_PointMass.at(i).position().y << m_PointMass.at(i).position().z;
            ar << m_PointMass.at(i).tag();
        }

        // space allocation for the future storage of more data, without need to change the format
        ar << 1; //formerly bTextures
        for (int i=1; i<18; i++) ar << 0;
        ar << m_SplineSurface.uDegree()<<m_SplineSurface.vDegree();
        double dble=0.0;
        for (int i=0; i<50; i++) ar << dble;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<100000 || ArchiveFormat>200000) return false;

        ar >> m_Name;
        ar >> m_Description;

/*        int a,r,g,b;
        xfl::readQColor(ar, r, g, b, a);
        m_BodyColor = QColor(r,g,b,a); */
        ar >> m_Color;

        ar >> k;
        if(k==1) m_LineType = xfl::BODYPANELTYPE;
        else     m_LineType = xfl::BODYSPLINETYPE;

        ar >> k; //m_iRes
        ar >> m_nxPanels >> m_nhPanels;
        ar >> m_Bunch;

        m_hPanels.clear();
        ar >> n;
        for(k=0; k<n; k++)
        {
            ar >> p;
            m_hPanels.append(p);
        }

        m_SplineSurface.clearFrames();
        m_xPanels.clear();
        ar >> n;
        for(k=0; k<n; k++)
        {
            m_SplineSurface.m_pFrame.append(new Frame);

            ar >> p;
            m_xPanels.append(p);

            ar >> dble;
            m_SplineSurface.m_pFrame[k]->setuPosition(dble);
            for(int ic=0; ic<m_SplineSurface.m_pFrame[k]->m_CtrlPoint.size(); ic++)
            {
                m_SplineSurface.m_pFrame[k]->m_CtrlPoint[ic].x = dble;
            }

            m_SplineSurface.m_pFrame[k]->serializeFrame(ar, bIsStoring);
        }


        ar >> m_VolumeMass;

        clearPointMasses();
        ar >> k;
        for(i=0; i<k; i++)
        {
            ar >> m >> px >> py >> pz;
            ar >> str;
            m_PointMass.append({m, Vector3d(px, py, pz), str});
        }

        // space allocation
        ar >>k; // formerly bTextures

        for (int i=1; i<18; i++) ar >> k;
        ar >> k; m_SplineSurface.setuDegree(std::max(k,3));
        ar >> k; m_SplineSurface.setvDegree(std::max(k,3));
        for (int i=0; i<50; i++) ar >> dble;
    }
    return true;
}


/**
*Rewinds one line from a QTextStream opened for reading
*@param in the stream opened for reading
*@param Line the index of the next Line to read
*@param strong the line to =ewind
*/
bool Body::Rewind1Line(QTextStream &in, int &Line, QString &strong) const
{
    int length = strong.length() * 1+2;//1 char takes one byte in the file ?

    int pos = int(in.pos());
    in.seek(pos-length);
    Line--;
    return true;
}

/**
* Extracts three double values from a QString, and returns the number of extracted values.
*/
int Body::readValues(QString line, double &x, double &y, double &z) const
{
    /*    char *sx = new char[30];
    char *sy = new char[30];
    char *text;*/
    int res=0;

    QString str;
    bool bOK;

    line = line.simplified();
    int pos = line.indexOf(" ");


    if(pos>0)
    {
        str = line.left(pos);
        line = line.right(line.length()-pos);
    }
    else
    {
        str = line;
        line = "";
    }
    x = str.toDouble(&bOK);
    if(bOK) res++;
    else
    {
        y=z=0.0;
        return res;
    }

    line = line.trimmed();
    pos = line.indexOf(" ");
    if(pos>0)
    {
        str = line.left(pos);
        line = line.right(line.length()-pos);
    }
    else
    {
        str = line;
        line = "";
    }
    y = str.toDouble(&bOK);
    if(bOK) res++;
    else
    {
        z=0.0;
        return res;
    }

    line = line.trimmed();
    if(!line.isEmpty())
    {
        z = line.toDouble(&bOK);
        if(bOK) res++;
    }
    else z=0.0;

    return res;
}



/**
 * Imports the definition from a text file. cf. Export the definition for details.
 * @return true if the Body definition was correctly imported, false otherwise.
 */
bool Body::importDefinition(QTextStream &inStream, double mtoUnit, QString &errorMessage)
{
    int res=0, Line=0, NSideLines=0;
    QString strong;
    bool bRead, bOK;
    double xo,yo,zo;
    xo = yo = zo = 0.0;

    Line = 0;
    bRead  = xfl::readAVLString(inStream, Line, strong);
    m_Name = strong.trimmed();
    m_SplineSurface.clearFrames();
    m_xPanels.clear();
    m_hPanels.clear();
    //Header data

    bRead = true;
    while (bRead)
    {
        bRead  = xfl::readAVLString(inStream, Line, strong);
        if(!bRead) break;
        if (strong.indexOf("BODYTYPE") >=0)
        {
            bRead  = xfl::readAVLString(inStream, Line, strong);
            if(!bRead) break;
            res = strong.toInt(&bOK);

            if(bOK)
            {
                if(res==1) m_LineType = xfl::BODYPANELTYPE;
                else       m_LineType = xfl::BODYSPLINETYPE;
            }
        }
        else if (strong.indexOf("OFFSET") >=0)
        {
            bRead  = xfl::readAVLString(inStream, Line, strong);
            if(!bRead) break;

            //Do this the C++ way
            QStringList values;
#if QT_VERSION >= 0x050F00
            values = values = strong.split(" ", Qt::SkipEmptyParts);
#else
    values = values = strong.split(" ", QString::SkipEmptyParts);
#endif

            if(values.length()==3)
            {
                xo  = values.at(0).toDouble();
                yo  = values.at(1).toDouble();
                zo  = values.at(2).toDouble();
                xo /= mtoUnit;
                zo /= mtoUnit;
                //y0 is ignored, body is assumed centered along x-z plane
            }
        }
        else if (strong.indexOf("FRAME", 0)  >=0)
        {
            Frame *pNewFrame = new Frame;
            NSideLines = readFrame(inStream, Line, pNewFrame, mtoUnit);

            if (NSideLines)
            {
                m_SplineSurface.insertFrame(pNewFrame);
                m_xPanels.append(3);
                m_hPanels.append(7);
            }
        }
    }



    for(int i=1; i<frameCount(); i++)
    {
        if(m_SplineSurface.m_pFrame[i]->m_CtrlPoint.size() != m_SplineSurface.m_pFrame[i-1]->m_CtrlPoint.size())
        {
            errorMessage = QObject::tr("Error reading ")+m_Name+QObject::tr("\nFrames have different number of side points");
            return false;
        }
    }

    for(int i=0; i<frameCount(); i++)
    {
        m_SplineSurface.m_pFrame[i]->m_Position.x =  m_SplineSurface.m_pFrame[i]->m_CtrlPoint[0].x + xo;
        for(int j=0; j<sideLineCount(); j++)
        {
            m_SplineSurface.m_pFrame[i]->m_CtrlPoint[j].z += zo;
        }
    }

    m_SplineSurface.setKnots();
    return true;
}


/**
 * Reads a Frame definition from a text stream, and adds the control point data to the Object pointed in input
 * @param in the QTextStream from which the data is to be read
 * @param Line a counter to the line number currently read
 * @param pFrame a pointer to the Frame object to be filled with the control point data
 * @param Unit the unit used to read from the datastream
 * @return the number of control points read from the stream
 */
int Body::readFrame(QTextStream &in, int &Line, Frame *pFrame, double const &Unit)
{
    double x,y,z;

    QString strong;
    int i;
    i = 0;
    x=y=z=0.0;

    bool bRead =true;

    pFrame->m_CtrlPoint.clear();

    while (bRead)
    {
        if(!xfl::readAVLString(in, Line,  strong)) bRead = false;

        if(readValues(strong, x,y,z)!=3)
        {
            bRead = false;
            Rewind1Line(in, Line, strong);
        }
        else
        {
            pFrame->m_CtrlPoint.append(Vector3d(x/Unit, y/Unit, z/Unit));
            i++;
        }
    }

    if(pFrame->m_CtrlPoint.size()) pFrame->m_Position.x = pFrame->m_CtrlPoint.first().x;
    return i;
}




/**
 * Export the wing geometry to a binary file in STL Format.
 * @param out the instance of the QTextStream to which the output will be directed
 */
void Body::exportSTLBinary(QDataStream &outStream, int nXPanels, int nHoopPanels, double unit) const
{
    /***
     *  UINT8[80] – Header
     *     UINT32 – Number of triangles
     *
     *     foreach triangle
     *     REAL32[3] – Normal vector
     *     REAL32[3] – Vertex 1
     *     REAL32[3] – Vertex 2
     *     REAL32[3] – Vertex 3
     *     UINT16 – Attribute byte count
     *     end
    */

    //    80 character header, avoid word "solid"
    //                       0123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789
    QString strong =     "binary STL file                                                                ";
    xfl::writeCString(outStream, strong);

    if(m_LineType==xfl::BODYSPLINETYPE) exportSTLBinarySplines(outStream, nXPanels, nHoopPanels, unit);
    else                                  exportSTLBinaryFlatPanels(outStream, unit);

}


void Body::exportSTLBinarySplines(QDataStream &outStream, int nXPanels, int nHoopPanels, double unitd) const
{
    Vector3d N, Pt;
    QVector<Vector3d> m_T((nXPanels+1)*(nHoopPanels+1)); //temporary points to save calculation times for body NURBS surfaces
    Vector3d TALB, LATB;

    float unitf = float(unitd);

    int p = 0;
    for (int k=0; k<=nXPanels; k++)
    {
        double u = double(k) / double(nXPanels);
        for (int l=0; l<=nHoopPanels; l++)
        {
            double v = double(l) / double(nHoopPanels);
            getPoint(u,  v, true, m_T[p]);

            p++;
        }
    }


    //Number of triangles
    //     NX*NH : quads
    //     x2 : 2 triangles/quad
    //     x2 : 2 sides

    int nTriangles = nXPanels *nHoopPanels *2*2;
    outStream << nTriangles;

    short zero = 0;

    N.set(0.0, 0.0, 0.0);
    int iTriangles = 0;

    char buffer[12];

    //right side first;
    p=0;
    for (int k=0; k<nXPanels; k++)
    {
        for (int l=0; l<nHoopPanels; l++)
        {
            LATB = m_T[p+nHoopPanels+1] - m_T[p+1];     //    LATB = TB - LA;
            TALB = m_T[p+nHoopPanels+2] - m_T[p];      //    TALB = LB - TA;
            N = TALB * LATB;
            N.normalize();

            //1st triangle
            xfl::writeFloat(outStream, N.xf());
            xfl::writeFloat(outStream, N.yf());
            xfl::writeFloat(outStream, N.zf());
            Pt = m_T[p];
            xfl::writeFloat(outStream, Pt.xf()*unitf);
            xfl::writeFloat(outStream, Pt.yf()*unitf);
            xfl::writeFloat(outStream, Pt.zf()*unitf);
            Pt = m_T[p+nHoopPanels+1];
            xfl::writeFloat(outStream, Pt.xf()*unitf);
            xfl::writeFloat(outStream, Pt.yf()*unitf);
            xfl::writeFloat(outStream, Pt.zf()*unitf);
            Pt = m_T[p+1];
            xfl::writeFloat(outStream, Pt.xf()*unitf);
            xfl::writeFloat(outStream, Pt.yf()*unitf);
            xfl::writeFloat(outStream, Pt.zf()*unitf);

            memcpy(buffer, &zero, sizeof(short));
            outStream.writeRawData(buffer, 2);

            //2nd triangle
            xfl::writeFloat(outStream, N.xf());
            xfl::writeFloat(outStream, N.yf());
            xfl::writeFloat(outStream, N.zf());
            Pt = m_T[p+nHoopPanels+1];
            xfl::writeFloat(outStream, Pt.xf()*unitf);
            xfl::writeFloat(outStream, Pt.yf()*unitf);
            xfl::writeFloat(outStream, Pt.zf()*unitf);
            Pt = m_T[p+nHoopPanels+2];
            xfl::writeFloat(outStream, Pt.xf()*unitf);
            xfl::writeFloat(outStream, Pt.yf()*unitf);
            xfl::writeFloat(outStream, Pt.zf()*unitf);
            Pt = m_T[p+1];
            xfl::writeFloat(outStream, Pt.xf()*unitf);
            xfl::writeFloat(outStream, Pt.yf()*unitf);
            xfl::writeFloat(outStream, Pt.zf()*unitf);

            memcpy(buffer, &zero, sizeof(short));
            outStream.writeRawData(buffer, 2);

            iTriangles+=2;
            p++;
        }
        p++;
    }

    //left side next;
    p=0;
    for (int k=0; k<nXPanels; k++)
    {
        for (int l=0; l<nHoopPanels; l++)
        {
            LATB = m_T[p+nHoopPanels+1] - m_T[p+1];     //    LATB = TB - LA;
            TALB = m_T[p+nHoopPanels+2] - m_T[p];      //    TALB = LB - TA;
            N = TALB * LATB;
            N.normalize();
            //1st triangle
            xfl::writeFloat(outStream, -N.xf());
            xfl::writeFloat(outStream, -N.yf());
            xfl::writeFloat(outStream, -N.zf());
            Pt = m_T[p];
            xfl::writeFloat(outStream,  Pt.xf()*unitf);
            xfl::writeFloat(outStream, -Pt.yf()*unitf);
            xfl::writeFloat(outStream,  Pt.zf()*unitf);
            Pt = m_T[p+1];
            xfl::writeFloat(outStream,  Pt.xf()*unitf);
            xfl::writeFloat(outStream, -Pt.yf()*unitf);
            xfl::writeFloat(outStream,  Pt.zf()*unitf);
            Pt = m_T[p+nHoopPanels+1];
            xfl::writeFloat(outStream,  Pt.xf()*unitf);
            xfl::writeFloat(outStream, -Pt.yf()*unitf);
            xfl::writeFloat(outStream,  Pt.zf()*unitf);

            memcpy(buffer, &zero, sizeof(short));
            outStream.writeRawData(buffer, 2);

            //2nd triangle
            xfl::writeFloat(outStream, -N.xf());
            xfl::writeFloat(outStream, -N.yf());
            xfl::writeFloat(outStream, -N.zf());
            Pt = m_T[p+nHoopPanels+1];
            xfl::writeFloat(outStream,  Pt.xf()*unitf);
            xfl::writeFloat(outStream, -Pt.yf()*unitf);
            xfl::writeFloat(outStream,  Pt.zf()*unitf);
            Pt = m_T[p+1];
            xfl::writeFloat(outStream,  Pt.xf()*unitf);
            xfl::writeFloat(outStream, -Pt.yf()*unitf);
            xfl::writeFloat(outStream,  Pt.zf()*unitf);
            Pt = m_T[p+nHoopPanels+2];
            xfl::writeFloat(outStream,  Pt.xf()*unitf);
            xfl::writeFloat(outStream, -Pt.yf()*unitf);
            xfl::writeFloat(outStream,  Pt.zf()*unitf);

            memcpy(buffer, &zero, sizeof(short));
            outStream.writeRawData(buffer, 2);

            iTriangles+=2;
            p++;
        }
        p++;
    }

    //    Q_ASSERT(iTriangles==nTriangles);

}


void Body::exportSTLBinaryFlatPanels(QDataStream &outStream, double unitd) const
{
    Vector3d P1, P2, P3, P4, N, P1P3, P2P4;

    float unitf = float(unitd);

    // count the non-null triangles
    int nTriangles=0;
    for (int k=0; k<sideLineCount()-1;k++)
    {
        for (int j=0; j<frameCount()-1;j++)
        {
            P1 = frameAt(j)->ctrlPointAt(k);       P1.x = frameAt(j)->position().x;
            P2 = frameAt(j+1)->ctrlPointAt(k);     P2.x = frameAt(j+1)->position().x;
            P3 = frameAt(j+1)->ctrlPointAt(k+1);   P3.x = frameAt(j+1)->position().x;
            P4 = frameAt(j)->ctrlPointAt(k+1);     P4.x = frameAt(j)->position().x;
            // check if triangle P1-P2-P4 is not NULL
            if(!P1.isSame(P2) && !P2.isSame(P4) && !P4.isSame(P1)) nTriangles++;
            // check if triangle P4-P2-P3 is not NULL
            if(!P4.isSame(P2) && !P2.isSame(P3) && !P3.isSame(P4)) nTriangles++;
        }
    }
    nTriangles *= 2;  // two sides

    outStream << nTriangles;
    short zero = 0;

    N.set(0.0, 0.0, 0.0);
    int iTriangles = 0;

    char buffer[12];


    //right side
    for (int k=0; k<sideLineCount()-1;k++)
    {
        for (int j=0; j<frameCount()-1;j++)
        {
            P1 = frameAt(j)->ctrlPointAt(k);       P1.x = frameAt(j)->position().x;
            P2 = frameAt(j+1)->ctrlPointAt(k);     P2.x = frameAt(j+1)->position().x;
            P3 = frameAt(j+1)->ctrlPointAt(k+1);   P3.x = frameAt(j+1)->position().x;
            P4 = frameAt(j)->ctrlPointAt(k+1);     P4.x = frameAt(j)->position().x;

            if(!P1.isSame(P2) && !P2.isSame(P4) && !P4.isSame(P1))
            {
                P1P3 = P3-P1;
                P2P4 = P4-P2;
                N = P1P3 * P2P4;
                N.normalize();

                // 1st triangle
                xfl::writeFloat(outStream, N.xf());
                xfl::writeFloat(outStream, N.yf());
                xfl::writeFloat(outStream, N.zf());

                xfl::writeFloat(outStream, P1.xf()*unitf);
                xfl::writeFloat(outStream, P1.yf()*unitf);
                xfl::writeFloat(outStream, P1.zf()*unitf);

                xfl::writeFloat(outStream, P2.xf()*unitf);
                xfl::writeFloat(outStream, P2.yf()*unitf);
                xfl::writeFloat(outStream, P2.zf()*unitf);

                xfl::writeFloat(outStream, P4.xf()*unitf);
                xfl::writeFloat(outStream, P4.yf()*unitf);
                xfl::writeFloat(outStream, P4.zf()*unitf);

                memcpy(buffer, &zero, sizeof(short));
                outStream.writeRawData(buffer, 2);
            }

            // 2nd triangle
            if(!P4.isSame(P2) && !P2.isSame(P3) && !P3.isSame(P4))
            {
                xfl::writeFloat(outStream, N.xf());
                xfl::writeFloat(outStream, N.yf());
                xfl::writeFloat(outStream, N.zf());

                xfl::writeFloat(outStream, P4.xf()*unitf);
                xfl::writeFloat(outStream, P4.yf()*unitf);
                xfl::writeFloat(outStream, P4.zf()*unitf);

                xfl::writeFloat(outStream, P2.xf()*unitf);
                xfl::writeFloat(outStream, P2.yf()*unitf);
                xfl::writeFloat(outStream, P2.zf()*unitf);

                xfl::writeFloat(outStream, P3.xf()*unitf);
                xfl::writeFloat(outStream, P3.yf()*unitf);
                xfl::writeFloat(outStream, P3.zf()*unitf);

                memcpy(buffer, &zero, sizeof(short));
                outStream.writeRawData(buffer, 2);
                iTriangles +=2;
            }
        }
    }

    //left side
    for (int k=0; k<sideLineCount()-1;k++)
    {
        for (int j=0; j<frameCount()-1;j++)
        {
            P1 = frameAt(j)->ctrlPointAt(k);       P1.x = frameAt(j)->position().x;
            P2 = frameAt(j+1)->ctrlPointAt(k);     P2.x = frameAt(j+1)->position().x;
            P3 = frameAt(j+1)->ctrlPointAt(k+1);   P3.x = frameAt(j+1)->position().x;
            P4 = frameAt(j)->ctrlPointAt(k+1);     P4.x = frameAt(j)->position().x;

            P1.y = -P1.y;
            P2.y = -P2.y;
            P3.y = -P3.y;
            P4.y = -P4.y;

            if(!P1.isSame(P2) && !P2.isSame(P4) && !P4.isSame(P1))
            {
                P1P3 = P3-P1;
                P2P4 = P4-P2;
                N = P1P3 * P2P4;
                N.normalize();

                // 1st triangle
                xfl::writeFloat(outStream, -N.xf());
                xfl::writeFloat(outStream, -N.yf());
                xfl::writeFloat(outStream, -N.zf());

                xfl::writeFloat(outStream, P2.xf()*unitf);
                xfl::writeFloat(outStream, P2.yf()*unitf);
                xfl::writeFloat(outStream, P2.zf()*unitf);

                xfl::writeFloat(outStream, P1.xf()*unitf);
                xfl::writeFloat(outStream, P1.yf()*unitf);
                xfl::writeFloat(outStream, P1.zf()*unitf);

                xfl::writeFloat(outStream, P4.xf()*unitf);
                xfl::writeFloat(outStream, P4.yf()*unitf);
                xfl::writeFloat(outStream, P4.zf()*unitf);

                memcpy(buffer, &zero, sizeof(short));
                outStream.writeRawData(buffer, 2);
            }

            // 2nd triangle
            if(!P4.isSame(P2) && !P2.isSame(P3) && !P3.isSame(P4))
            {
                xfl::writeFloat(outStream, -N.xf());
                xfl::writeFloat(outStream, -N.yf());
                xfl::writeFloat(outStream, -N.zf());

                xfl::writeFloat(outStream, P2.xf()*unitf);
                xfl::writeFloat(outStream, P2.yf()*unitf);
                xfl::writeFloat(outStream, P2.zf()*unitf);

                xfl::writeFloat(outStream, P4.xf()*unitf);
                xfl::writeFloat(outStream, P4.yf()*unitf);
                xfl::writeFloat(outStream, P4.zf()*unitf);

                xfl::writeFloat(outStream, P3.xf()*unitf);
                xfl::writeFloat(outStream, P3.yf()*unitf);
                xfl::writeFloat(outStream, P3.zf()*unitf);

                memcpy(buffer, &zero, sizeof(short));
                outStream.writeRawData(buffer, 2);
                iTriangles +=2;
            }
        }
    }

    Q_ASSERT(iTriangles==nTriangles);
}


int Body::isNode(Vector3d &Pt, QVector<Vector3d> &m_Node)
{
    for (int in=m_Node.size()-1; in>=0; in--)
    {
        if(Pt.isSame(m_Node[in])) return in;
    }
    return -1;
}


int Body::makePanels(int nFirst, Vector3d const &pos, QVector<Panel> &panels, QVector<Vector3d> &nodes)
{
    double uk(0), uk1(0), v(0), dj(0), dj1(0), dl1(0);
    double dpx(0), dpz(0);
    Vector3d LATB, TALB;
    Vector3d LA, LB, TA, TB;
    Vector3d PLA, PTA, PLB, PTB;

    int n0(0), n1(0), n2(0), n3(0), lnx(0), lnh(0);
    int nx = m_nxPanels;
    int nh = m_nhPanels;
    setPanelPos();

    int p = 0;

    int FullSize = 0;
    int nPanels = 0;
    int nNodes = 0;

    lnx = 0;

    dpx = pos.x;
    dpz = pos.z;

    if(isFlatPanelType())
    {
        nx = 0;
        for(int i=0; i<frameCount()-1; i++) nx+=m_xPanels[i];
        nh = 0;
        for(int i=0; i<sideLineCount()-1; i++) nh+=m_hPanels[i];
        FullSize = nx*nh*2;
        setNXPanels(nx);
        setNHPanels(nh);

        for (int i=0; i<frameCount()-1; i++)
        {
            for (int j=0; j<m_xPanels[i]; j++)
            {
                dj  = double( j) /double(m_xPanels[i]);
                dj1 = double(j+1)/double(m_xPanels[i]);

                //body left side
                lnh = 0;
                for (int k=0; k<sideLineCount()-1; k++)
                {
                    //build the four corner points of the strips
                    PLB.x =  (1.0- dj) * framePosition(i)                +  dj * framePosition(i+1)                 +dpx;
                    PLB.y = -(1.0- dj) * frame(i)->m_CtrlPoint.at(k).y   -  dj * frame(i+1)->m_CtrlPoint.at(k).y;
                    PLB.z =  (1.0- dj) * frame(i)->m_CtrlPoint.at(k).z   +  dj * frame(i+1)->m_CtrlPoint.at(k).z    +dpz;

                    PTB.x =  (1.0-dj1) * framePosition(i)                + dj1 * framePosition(i+1)                 +dpx;
                    PTB.y = -(1.0-dj1) * frame(i)->m_CtrlPoint.at(k).y   - dj1 * frame(i+1)->m_CtrlPoint.at(k).y;
                    PTB.z =  (1.0-dj1) * frame(i)->m_CtrlPoint.at(k).z   + dj1 * frame(i+1)->m_CtrlPoint.at(k).z    +dpz;

                    PLA.x =  (1.0- dj) * framePosition(i)                +  dj * framePosition(i+1)                 +dpx;
                    PLA.y = -(1.0- dj) * frame(i)->m_CtrlPoint.at(k+1).y -  dj * frame(i+1)->m_CtrlPoint.at(k+1).y;
                    PLA.z =  (1.0- dj) * frame(i)->m_CtrlPoint.at(k+1).z +  dj * frame(i+1)->m_CtrlPoint.at(k+1).z  +dpz;

                    PTA.x =  (1.0-dj1) * framePosition(i)                + dj1 * framePosition(i+1)                 +dpx;
                    PTA.y = -(1.0-dj1) * frame(i)->m_CtrlPoint.at(k+1).y - dj1 * frame(i+1)->m_CtrlPoint.at(k+1).y;
                    PTA.z =  (1.0-dj1) * frame(i)->m_CtrlPoint.at(k+1).z + dj1 * frame(i+1)->m_CtrlPoint.at(k+1).z  +dpz;

                    LB = PLB;
                    TB = PTB;

                    for (int l=0; l<m_hPanels[k]; l++)
                    {
                        dl1  = double(l+1) / double(m_hPanels[k]);
                        LA = PLB * (1.0- dl1) + PLA * dl1;
                        TA = PTB * (1.0- dl1) + PTA * dl1;

                        n0 = isNode(LA, nodes);
                        n1 = isNode(TA, nodes);
                        n2 = isNode(LB, nodes);
                        n3 = isNode(TB, nodes);

                        panels.append(Panel());
                        Panel &panel = panels.last();
                        if(n0>=0) {
                            panel.m_iLA = n0;
                        }
                        else {
                            panel.m_iLA = nNodes;
                            nodes.push_back(LA);
                            nNodes++;
                        }

                        if(n1>=0) {
                            panel.m_iTA = n1;
                        }
                        else {
                            panel.m_iTA = nNodes;
                            nodes.push_back(TA);
                            nNodes++;
                        }

                        if(n2>=0) {
                            panel.m_iLB = n2;
                        }
                        else {
                            panel.m_iLB = nNodes;
                            nodes.push_back(LB);
                            nNodes++;
                        }

                        if(n3 >=0) {
                            panel.m_iTB = n3;
                        }
                        else {
                            panel.m_iTB = nNodes;
                            nodes.push_back(TB);
                            nNodes++;
                        }

                        LATB = TB - LA;
                        TALB = LB - TA;
                        panel.Normal = LATB * TALB;
                        panel.Area =  panel.Normal.norm()/2.0;
                        panel.Normal.normalize();

                        panel.m_bIsInSymPlane  = false;
                        panel.m_bIsLeading     = false;
                        panel.m_bIsTrailing    = false;
                        panel.m_Pos = xfl::BODYSURFACE;
                        panel.m_iElement = nPanels;
                        panel.m_bIsLeftPanel  = true;
                        panel.setPanelFrame(LA, LB, TA, TB);

                        // set neighbour panels

                        panel.m_iPD = nPanels + nh;
                        panel.m_iPU = nPanels - nh;

                        if(lnx==0)      panel.m_iPU = -1;// no panel downstream
                        if(lnx==nx-1)    panel.m_iPD = -1;// no panel upstream

                        panel.m_iPL = nPanels + 1;
                        panel.m_iPR = nPanels - 1;

                        if(lnh==0)     panel.m_iPR = nFirst + FullSize - p - 1;
                        if(lnh==nh-1)  panel.m_iPL = nFirst + FullSize - p - 1;

                        nPanels++;
                        p++;
                        LB = LA;
                        TB = TA;
                        lnh++;
                    }
                }
                lnx++;
            }
        }
    }
    else if(isSplineType())
    {
        FullSize = 2*nx*nh;
        //start with left side... same as for wings
        for (int k=0; k<nx; k++)
        {
            uk  = m_XPanelPos.at(k);
            uk1 = m_XPanelPos.at(k+1);

            getPoint(uk,  0, false, LB);
            getPoint(uk1, 0, false, TB);

            LB.x += dpx;
            LB.z += dpz;
            TB.x += dpx;
            TB.z += dpz;

            for (int l=0; l<nh; l++)
            {
                //start with left side... same as for wings
                v = double(l+1) / double(nh);
                getPoint(uk,  v, false, LA);
                getPoint(uk1, v, false, TA);

                LA.x += dpx;
                LA.z += dpz;
                TA.x += dpx;
                TA.z += dpz;

                n0 = isNode(LA, nodes);
                n1 = isNode(TA, nodes);
                n2 = isNode(LB, nodes);
                n3 = isNode(TB, nodes);

                panels.append(Panel());
                Panel &panel = panels.last();
                if(n0>=0) {
                    panel.m_iLA = n0;
                }
                else {
                    panel.m_iLA = nNodes;
                    nodes.push_back(LA);
                    nNodes++;
                }

                if(n1>=0) {
                    panel.m_iTA = n1;
                }
                else {
                    panel.m_iTA = nNodes;
                    nodes.push_back(TA);
                    nNodes++;
                }

                if(n2>=0) {
                    panel.m_iLB = n2;
                }
                else {
                    panel.m_iLB = nNodes;
                    nodes.push_back(LB);
                    nNodes++;
                }

                if(n3 >=0) {
                    panel.m_iTB = n3;
                }
                else {
                    panel.m_iTB = nNodes;
                    nodes.push_back(TB);
                    nNodes++;
                }

                LATB = TB - LA;
                TALB = LB - TA;
                panel.Normal = LATB * TALB;
                panel.Area =  panel.Normal.norm()/2.0;
                panel.Normal.normalize();

                panel.m_bIsInSymPlane  = false;
                panel.m_bIsLeading     = false;
                panel.m_bIsTrailing    = false;
                panel.m_Pos = xfl::BODYSURFACE;
                panel.m_iElement = nPanels;
                panel.m_bIsLeftPanel  = true;
                panel.setPanelFrame(LA, LB, TA, TB);

                // set neighbour panels

                panel.m_iPD = nPanels + nh;
                panel.m_iPU = nPanels - nh;

                if(k==0)    panel.m_iPU = -1;// no panel downstream
                if(k==nx-1) panel.m_iPD = -1;// no panel upstream

                panel.m_iPL = nPanels + 1;
                panel.m_iPR = nPanels - 1;

                if(l==0)     panel.m_iPR = nFirst + FullSize - p - 1;
                if(l==nh-1)  panel.m_iPL = nFirst + FullSize - p - 1;

                LB = LA;
                TB = TA;
                nPanels++;
                p++;
            }
        }
    }

    //right side next
    int i = nPanels;

    for (int k=nx-1; k>=0; k--)
    {
        for (int l=nh-1; l>=0; l--)
        {
            i--;
            LA = nodes[panels[i].m_iLB];
            TA = nodes[panels[i].m_iTB];
            LB = nodes[panels[i].m_iLA];
            TB = nodes[panels[i].m_iTA];

            LA.y = -LA.y;
            LB.y = -LB.y;
            TA.y = -TA.y;
            TB.y = -TB.y;

            n0 = isNode(LA, nodes);
            n1 = isNode(TA, nodes);
            n2 = isNode(LB, nodes);
            n3 = isNode(TB, nodes);

            panels.append(Panel());
            Panel &panel = panels.last();
            if(n0>=0) {
                panel.m_iLA = n0;
            }
            else {
                panel.m_iLA = nNodes;
                nodes.push_back(LA);
                nNodes++;
            }

            if(n1>=0) {
                panel.m_iTA = n1;
            }
            else {
                panel.m_iTA = nNodes;
                nodes.push_back(TA);
                nNodes++;
            }

            if(n2>=0) {
                panel.m_iLB = n2;
            }
            else {
                panel.m_iLB = nNodes;
                nodes.push_back(LB);
                nNodes++;
            }

            if(n3 >=0) {
                panel.m_iTB = n3;
            }
            else {
                panel.m_iTB = nNodes;
                nodes.push_back(TB);
                nNodes++;
            }

            LATB = TB - LA;
            TALB = LB - TA;
            panel.Normal = LATB * TALB;
            panel.Area =  panel.Normal.norm()/2.0;
            panel.Normal.normalize();

            panel.m_bIsInSymPlane  = false;
            panel.m_bIsLeading     = false;
            panel.m_bIsTrailing    = false;
            panel.m_Pos = xfl::BODYSURFACE;
            panel.m_iElement = nPanels;
            panel.m_bIsLeftPanel  = false;
            panel.setPanelFrame(LA, LB, TA, TB);

            // set neighbour panels
            // valid only for Panel Analysis

            panel.m_iPD = nPanels - nh;
            panel.m_iPU = nPanels + nh;

            if(k==0)    panel.m_iPU = -1;// no panel downstream
            if(k==nx-1) panel.m_iPD = -1;// no panel upstream

            panel.m_iPL = nPanels + 1;
            panel.m_iPR = nPanels - 1;

            if(l==0)     panel.m_iPL = nFirst + FullSize - p - 1;
            if(l==nh-1)  panel.m_iPR = nFirst + FullSize - p - 1;

            LB = LA;
            TB = TA;
            nPanels++;
            p++;
        }
    }
    m_NElements = nPanels-nFirst;
    return m_NElements;
}
