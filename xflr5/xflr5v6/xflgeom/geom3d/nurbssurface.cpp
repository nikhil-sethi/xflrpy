/****************************************************************************

    NURBSSurface Class
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


#include "nurbssurface.h"
#include <xflanalysis/analysis3d_params.h>
#include <xflcore/constants.h>


/**
 * The public constructor.
 * @param iAxis defines along which preferred axis the paramater u is directed; v is in the y-direction
 */
NURBSSurface::NURBSSurface(int iAxis)
{
    m_pFrame.clear();
#if QT_VERSION >= 0x040700
    m_pFrame.reserve(10);
#endif

    m_uAxis = iAxis;//directed in x direction, mainly
    m_vAxis = 2;//directed in y direction, mainly

    m_iuDegree = 2;
    m_ivDegree = 2;
    m_nuKnots = 0;
    m_nvKnots = 0;

    m_iRes = 31;

    m_Bunch  = 0.0;
    m_EdgeWeightu = 1.0;
    m_EdgeWeightv = 1.0;
}


NURBSSurface::~NURBSSurface()
{
    for(int iFr=m_pFrame.size()-1; iFr>=0; iFr--)
    {
        delete m_pFrame.at(iFr);
        m_pFrame.removeAt(iFr);
    }
}


/**
 * Returns the u-parameter for a given value along the axis and a given v parameter
 * Proceeds by iteration - time consuming,
 * @param pos the point coordinate for which the parameter u is requested
 * @param v the specified value of the v-parameter
 * @return the value of the u-parameter
 */
double NURBSSurface::getu(double pos, double v) const
{
    if(pos<=m_pFrame.first()->m_Position.coord(m_uAxis)) return 0.0;
    if(pos>=m_pFrame.last()->m_Position.coord(m_uAxis))  return 1.0;
    if(qAbs(m_pFrame.last()->m_Position.coord(m_uAxis) - m_pFrame.first()->m_Position.coord(m_uAxis))<0.0000001) return 0.0;

    int iter=0;
    double u2(0), u1(0), b(0), c(0), u(0), zz(0), zh(0);
    u1 = 0.0; u2 = 1.0;

    //    v = 0.0;//use top line, but doesn't matter
    while(qAbs(u2-u1)>1.0e-6 && iter<200)
    {
        u=(u1+u2)/2.0;
        zz = 0.0;
        for(int iu=0; iu<frameCount(); iu++) //browse all points
        {
            zh = 0.0;
            for(int jv=0; jv<framePointCount(); jv++)
            {
                c =  splineBlend(jv, m_ivDegree, v, m_vKnots);
                zh += m_pFrame[iu]->m_Position.coord(m_uAxis) * c;
            }
            b = splineBlend(iu, m_iuDegree, u, m_uKnots);
            zz += zh * b;
        }
        if(zz>pos) u2 = u;
        else       u1 = u;
        iter++;
    }
    return (u1+u2)/2.0;
}


/**
* Calculates the blending value of a point on a BSpline. This is done recursively.
* If the numerator and denominator are 0 the expression is 0.
* If the denominator is 0 the expression is 0
*
* @param index   the control point's index
* @param p       the spline's degree
* @param t       the spline parameter
* @param knots   a pointer to the vector of knots
* @return the spline function value
*/
#define EPS 0.0001
double NURBSSurface::splineBlend(int const &index, int const &p, double const &t, double const *knots) const
{
    if(p==0)
    {
        if ((knots[index] <= t) && (t < knots[index+1]) ) return 1.0;
        else                                              return 0.0;
    }
    else
    {
        if (qAbs(knots[index+p] - knots[index])<EPS && qAbs(knots[index+p+1] - knots[index+1])<EPS)
            return 0.0;
        else if (qAbs(knots[index+p] - knots[index])<EPS)
            return (knots[index+p+1]-t) / (knots[index+p+1]-knots[index+1])  * splineBlend(index+1, p-1, t, knots);
        else if (qAbs(knots[index+p+1] - knots[index+1])<EPS)
            return (t-knots[index])     / (knots[index+p] - knots[index])    * splineBlend(index,   p-1, t, knots);
        else
            return (t-knots[index])     / (knots[index+p] - knots[index])    * splineBlend(index,   p-1, t, knots) +
                    (knots[index+p+1]-t) / (knots[index+p+1]-knots[index+1]) * splineBlend(index+1 ,p-1, t, knots);
    }
}

/**
 * Returns the v-parameter for a given value  of u and a geometrical point
 * Proceeds by iteration - time consuming,
 * @param u the specified value of the u-parameter
 * @param r the point for which v is requested
 * @return the value of the v-parameter
 */

double NURBSSurface::getv(double u, Vector3d r) const
{
    double sine = 10000.0;

    if(u<=0.0)          return 0.0;
    if(u>=1.0)          return 0.0;
    if(r.norm()<1.0e-5) return 0.0;

    int iter=0;
    double v(0), v1(0), v2(0);
    Vector3d t_R;
    r.normalize();
    v1 = 0.0; v2 = 1.0;

    while(qAbs(sine)>1.0e-4 && iter<200)
    {
        v=(v1+v2)/2.0;
        getPoint(u, v, t_R);
        t_R.x = 0.0;
        t_R.normalize();//t_R is the unit radial vector for u,v

        sine = (r.y*t_R.z - r.z*t_R.y);

        if(sine>0.0) v1 = v;
        else         v2 = v;
        iter++;
    }

    return (v1+v2)/2.0;
}



/**
 * Returns the point corresponding to the pair of parameters (u,v)
 * Assumes that the knots have been set previously
 *
 * Scans the u-direction first, then v-direction
 * @param u the specified u-parameter
 * @param v the specified v-parameter
 * @param Pt a reference to the point defined by the pair (u,v)
*/
void NURBSSurface::getPoint(double u, double v, Vector3d &Pt) const
{
    Vector3d V, Vv;

    if(u>=1.0) u=0.99999999999;
    if(v>=1.0) v=0.99999999999;

    double totalweight = 0.0;
    for(int iu=0; iu<frameCount(); iu++)
    {
        Vv.set(0.0,0.0,0.0);
        double wx = 0.0;
        for(int jv=0; jv<framePointCount(); jv++)
        {
            double cs = splineBlend(jv, m_ivDegree, v, m_vKnots) * weight(m_EdgeWeightv, jv, framePointCount());

            Vv.x += m_pFrame[iu]->m_CtrlPoint[jv].x * cs;
            Vv.y += m_pFrame[iu]->m_CtrlPoint[jv].y * cs;
            Vv.z += m_pFrame[iu]->m_CtrlPoint[jv].z * cs;

            wx += cs;
        }
        double bs = splineBlend(iu, m_iuDegree, u, m_uKnots) * weight(m_EdgeWeightu, iu, frameCount());

        V.x += Vv.x * bs;
        V.y += Vv.y * bs;
        V.z += Vv.z * bs;

        totalweight += wx * bs;
    }

    Pt.x = V.x / totalweight;
    Pt.y = V.y / totalweight;
    Pt.z = V.z / totalweight;
}


void NURBSSurface::getNormal(double u, double v, Vector3d &N) const
{
    Vector3d Su, Sv, Vv, rpt;
    double cs(0), bs(0);

    u=std::max(u, 1e-4);
    v=std::max(v, 1e-4);
    u=std::min(u, 1.0-1.e-4);
    v=std::min(v, 1.0-1.e-4);

    // calculate the u derivative
    for(int iu=0; iu<frameCount(); iu++)
    {
        Frame const *uframe = m_pFrame.at(iu);
        Vv.reset();

        for(int jv=0; jv<framePointCount(); jv++)
        {
             cs = basis(jv, m_ivDegree, v, m_vKnots);

            rpt.x = uframe->ctrlPointAt(jv).x;
            rpt.y = uframe->ctrlPointAt(jv).y;
            rpt.z = uframe->ctrlPointAt(jv).z;

            Vv.x += rpt.x * cs;
            Vv.y += rpt.y * cs;
            Vv.z += rpt.z * cs;

        }

        bs = basisDerivative(iu, m_iuDegree, u, m_uKnots);

        Su.x += Vv.x * bs;
        Su.y += Vv.y * bs;
        Su.z += Vv.z * bs;
    }

    // calculate the u derivative
    for(int iu=0; iu<frameCount(); iu++)
    {
        Frame const *uframe = m_pFrame.at(iu);
        Vv.reset();

        for(int jv=0; jv<framePointCount(); jv++)
        {
             cs = basisDerivative(jv, m_ivDegree, v, m_vKnots);

            rpt.x = uframe->ctrlPointAt(jv).x;
            rpt.y = uframe->ctrlPointAt(jv).y;
            rpt.z = uframe->ctrlPointAt(jv).z;


            Vv.x += rpt.x * cs;
            Vv.y += rpt.y * cs;
            Vv.z += rpt.z * cs;

        }

        bs = basis(iu, m_iuDegree, u, m_uKnots);

        Sv.x += Vv.x * bs;
        Sv.y += Vv.y * bs;
        Sv.z += Vv.z * bs;
    }

    N = (Su * Sv).normalized();
}

#define KNOTPRECISION 1.e-6
double NURBSSurface::basis(int i, int deg, double t, double const *knots) const
{
    double sum = 0.0;
    if(deg==0)
    {
        if(knots[i]<=t && t<knots[i+1]) return 1.0;
        else                            return 0.0;
    }

    if(fabs(knots[i+deg]-knots[i])>KNOTPRECISION)
    {
        sum += (t-knots[i])/(knots[i+deg]-knots[i]) * basis(i, deg-1, t, knots);
    }

    if(fabs(knots[i+deg+1]-knots[i+1])>KNOTPRECISION)
    {
        sum += (knots[i+deg+1]-t)/(knots[i+deg+1]-knots[i+1]) * basis(i+1, deg-1, t, knots);
    }
    return sum;
}


double NURBSSurface::basisDerivative(int i, int deg, double t, double const *knots) const
{
    double der = 0.0;

    if(fabs(knots[i+deg]-knots[i])>KNOTPRECISION)
    {
        der += double(deg)/(knots[i+deg]  -knots[i])   * basis(i, deg-1, t, knots);
    }
    if(fabs(knots[i+deg+1]-knots[i+1])>KNOTPRECISION)
    {
        der -= double(deg)/(knots[i+deg+1]-knots[i+1]) * basis(i+1, deg-1, t, knots);
    }
    return der;
}

/**
 * Returns the point corresponding to the pair of parameters (u,v)
 * Assumes that the knots have been set previously
 *
 * Scans the u-direction first, then v-direction
 * @param u the specified u-parameter
 * @param v the specified v-parameter
 * @param Pt a reference to the point defined by the pair (u,v)
*/
Vector3d NURBSSurface::point(double u, double v) const
{
    Vector3d V, Vv;
    double wx=0, totalweight=0;

    if(u>=1.0) u=0.99999999999;
    if(v>=1.0) v=0.99999999999;

    totalweight = 0.0;
    for(int iu=0; iu<frameCount(); iu++)
    {
        Vv.set(0.0,0.0,0.0);
        wx = 0.0;
        for(int jv=0; jv<framePointCount(); jv++)
        {
            double cs = splineBlend(jv, m_ivDegree, v, m_vKnots) * weight(m_EdgeWeightv, jv, framePointCount());

            Vv.x += m_pFrame[iu]->m_CtrlPoint[jv].x * cs;
            Vv.y += m_pFrame[iu]->m_CtrlPoint[jv].y * cs;
            Vv.z += m_pFrame[iu]->m_CtrlPoint[jv].z * cs;

            wx += cs;
        }
        double bs = splineBlend(iu, m_iuDegree, u, m_uKnots) * weight(m_EdgeWeightu, iu, frameCount());

        V.x += Vv.x * bs;
        V.y += Vv.y * bs;
        V.z += Vv.z * bs;

        totalweight += wx * bs;
    }

    return V/totalweight;
}


/**
 * Returns the weight of the control point
 * @param i the index of the point along the edge
*  @param N the total number of points along the edge
*  @return the point's weight
**/
double NURBSSurface::weight(double const &d, const int &i, int const &N) const
{
    if(qAbs(d-1.0)<PRECISION) return 1.0;
    if(i<(N+1)/2)             return pow(d, i);
    else                      return pow(d, N-i-1);
}


/**
 * Intersects a line segment AB with the NURBS surface. The points are expected to be on each side of the NURBS surface.
 *@param A the first point which defines the ray
 *@param B the second point which defines the ray
 *@param I the intersection point
 *@return true if an intersection point could be determined
 */
bool NURBSSurface::intersectNURBS(Vector3d A, Vector3d B, Vector3d &I) const
{
    Vector3d tmp, M0, M1;
    double u(0), v(0), dist(0), t(0), tp(0);
    int iter = 0;
    int itermax = 20;
    double dmax = 1.0e-5;
    dist = 1000.0;//m

    M0.set(0.0, A.y, A.z);
    M1.set(0.0, B.y, B.z);

    if(M0.norm()<M1.norm())
    {
        tmp = A;        A = B;          B = tmp;
    }

    //M0 is the outside Point, M1 is the inside point
    M0 = A; M1 = B;

    Vector3d t_Q, t_r, t_N;

    I = (M0+M1)/2.0; t=0.5;

    while(dist>dmax && iter<itermax)
    {
        //first we get the u parameter corresponding to point I
        tp = t;
        u = getu(I.z, 0.0);
        t_Q.set(I.x, 0.0, 0.0);
        t_r = (I-t_Q);
        v = getv(u, t_r);
        getPoint(u, v, t_N);

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
 * Creates the knot array for the two directions
 */
void NURBSSurface::setKnots()
{
    if(!frameCount())return;
    if(!framePointCount())return;

    m_iuDegree = std::max(1, m_iuDegree);
    m_iuDegree = std::min(m_iuDegree, frameCount()-1);
    m_nuKnots  = m_iuDegree + frameCount() + 1;
    double b = double(m_nuKnots-2*m_iuDegree-1);

    for (int j=0; j<m_nuKnots; j++)
    {
        if (j<m_iuDegree+1)  m_uKnots[j] = 0.0;
        else
        {
            if (j<frameCount())
            {
                if(qAbs(b)>0.0) m_uKnots[j] = double(j-m_iuDegree)/b;
                else            m_uKnots[j] = 1.0;
            }
            else m_uKnots[j] = 1.0;
        }
    }

    m_ivDegree = std::max(1,m_ivDegree);
    m_ivDegree = std::min(m_ivDegree, firstFrame()->pointCount()-1);

    m_nvKnots  = m_ivDegree + framePointCount() + 1;
    b = double(m_nvKnots-2*m_ivDegree-1);

    for (int j=0; j<m_nvKnots; j++)
    {
        if (j<m_ivDegree+1)  m_vKnots[j] = 0.0;
        else
        {
            if (j<framePointCount())
            {
                if(qAbs(b)>0.0) m_vKnots[j] = double(j-m_ivDegree)/b;
                else            m_vKnots[j] = 1.0;
            }
            else m_vKnots[j] = 1.0;
        }
    }

    /*    qDebug("u-knots\n");
    for(int iu=0; iu<m_nuKnots; iu++)
        qDebug(" %d  %7.3f", iu, m_uKnots[iu]);
    qDebug("v-knots\n");
    for(int iv=0; iv<m_nvKnots; iv++)
        qDebug(" %d  %7.3f", iv, m_vKnots[iv]);*/
}


/**
 * Specifies the degree in the u-direction.
 * If the degree specified by the input parameter is equal or greater than the Frame count,
 * then the degree is set to the number of frames minus 1.
 * @param nvDegree the specified degree
 * @return the degree which has been set
 */
int NURBSSurface::setvDegree(int nvDegree)
{
    m_ivDegree = nvDegree;
    return m_ivDegree;
}


/**
 * Specifies the degree in the u-direction.
 * If the degree specified by the input parameter is equal or greater than the Frame count,
 * then the degree is set to the number of frames minus 1.
 * @param nuDegree the specified degree
 * @return the degree which has been set
 */
int NURBSSurface::setuDegree(int nuDegree)
{
    m_iuDegree = nuDegree;
    return m_iuDegree;
}


/**
 * Removes a Frame from the array
 * @param iFrame the index of the frame to remove
 */
void NURBSSurface::removeFrame(int iFrame)
{
    delete m_pFrame.at(iFrame);
    m_pFrame.removeAt(iFrame);
}



/**
 * Removes all the Frame objects from the array
 */
void NURBSSurface::clearFrames()
{
    if(!frameCount()) return;
    for(int ifr=frameCount()-1; ifr>=0; ifr--)
    {
        removeFrame(ifr);
    }
}

/**
 * Inserts a Frame in the array. The Frame is positioned in crescending position along the u-axis
 * @param pNewFrame a pointer to the Frame object to insert.
 */
void NURBSSurface::insertFrame(Frame *pNewFrame)
{
    for(int ifr=0; ifr<frameCount(); ifr++)
    {
        if(pNewFrame->m_Position.coord(m_uAxis) < m_pFrame.at(ifr)->m_Position.coord(m_uAxis))
        {
            m_pFrame.insert(ifr, pNewFrame);
            return;
        }
    }

    m_pFrame.append(pNewFrame); //either the first if none, either the last...
}

/**
 * Appends a new Frame at the end of the array
 * @return a pointer to the Frame which has been created.
 */
Frame * NURBSSurface::appendNewFrame()
{
    m_pFrame.append(new Frame);
    return m_pFrame.last();
}


/**
 * Appends an existing Frame at the end of the array
 * @return a pointer to the Frame which has been created.
 */
void NURBSSurface::appendFrame(Frame*pFrame)
{
    if(!pFrame)return;
    m_pFrame.append(pFrame);
}


int NURBSSurface::framePointCount() const
{
    if(m_pFrame.size())    return m_pFrame.first()->pointCount();
    else return 0;
}
