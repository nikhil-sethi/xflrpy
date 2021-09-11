/****************************************************************************

    LLTAnalysis Class
    Copyright (C) 2008-2017 André Deperrois

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

#include <QCoreApplication>

#include <QtDebug>
#include <QString>


#include "lltanalysis.h"
#include <xflobjects/objects3d/planeopp.h>
#include <xflobjects/objects3d/wpolar.h>
#include <xflobjects/objects3d/wing.h>
#include <xflcore/matrix.h>

int LLTAnalysis::s_IterLim = 100;
int LLTAnalysis::s_NLLTStations = 20;
double LLTAnalysis::s_RelaxMax = 20.0;
double LLTAnalysis::s_CvPrec = 0.01;
bool LLTAnalysis::s_bInitCalc = true;


/** The public constructor */
LLTAnalysis::LLTAnalysis()
{
    m_pWing = nullptr;
    m_pWPolar = nullptr;
    m_pX = m_pY = nullptr;

    m_poaPolar = nullptr;
    resetVariables();
}


/**
* Initializes the variables to default values
*/
void LLTAnalysis::resetVariables()
{
    m_nPoints = 1;
    m_bSequence = false;
    m_vMin = m_vMax = m_vDelta = 0.0;

    m_bCancel    = false;
    m_bConverged = false;
    m_bWingOut   = false;
    m_bError     = false;
    m_bWarning   = false;

    memset(m_Chord,         0, sizeof(m_Chord));
    memset(m_Offset,        0, sizeof(m_Offset));
    memset(m_Twist,         0, sizeof(m_Twist));
    memset(m_SpanPos,       0, sizeof(m_SpanPos));
    memset(m_StripArea,     0, sizeof(m_StripArea));
    memset(m_Re,            0, sizeof(m_Re));
    memset(m_Cl,            0, sizeof(m_Cl));
    memset(m_Ai,            0, sizeof(m_Ai));
    memset(m_ICd,           0, sizeof(m_ICd));
    memset(m_PCd,           0, sizeof(m_PCd));
    memset(m_Cm,            0, sizeof(m_Cm));
    memset(m_CmAirf,        0, sizeof(m_CmAirf));
    memset(m_XCPSpanRel,    0, sizeof(m_XCPSpanRel));
    memset(m_XCPSpanAbs,    0, sizeof(m_XCPSpanAbs));
    memset(m_BendingMoment, 0, sizeof(m_BendingMoment));
    memset(m_XTrTop,        0, sizeof(m_XCPSpanAbs));
    memset(m_XTrBot,        0, sizeof(m_BendingMoment));

    m_LengthUnit.clear();
    m_mtoUnit = 0.0;

    m_QInf0 = 0.0;
    m_Maxa  = 0.0;

    m_CL = 0.0;
    m_CDi = 0.0;
    m_CDv = 0.0;

    m_VYm = m_IYm = m_GYm = 0.0;
    m_VCm = m_ICm = m_GCm = 0.0;
    m_GRm = 0.0;

    m_CP.set(0.0,0.0,0.0);
}


/** 
* Initializes the Reynolds numbers and and the non-linear lift coefficients for the first iteration in LLT
 * @param QInf the freestream velocity, in m/s
 * @param Alpha the angle of attack, in degrees
*/
void LLTAnalysis::setVelocity(double &QInf)
{
    if(m_pWPolar->isFixedSpeedPolar())
    {
        QInf = m_pWPolar->m_QInfSpec;
    }
    else if(m_pWPolar->isFixedLiftPolar())
    {
        double Lift=0.0;// required for Type 2
        for (int k=1; k<s_NLLTStations; k++)
        {
            Lift += Eta(k) * m_Cl[k] * m_Chord[k] /m_pWing->planformSpan();
        }
        if(Lift<=0.0) return;
        QInf  = m_QInf0 / sqrt(Lift);
    }

    for (int k=1; k<s_NLLTStations; k++)
    {
        m_Re[k] = m_Chord[k] * QInf /m_pWPolar->viscosity();
    }
}


/**
 * The multiplier for the lift, drag and pitching-moment coefficients in the case of asymmetrical distributions.
 * cf. NACA TN-1269 for details
 * @param m the index of the span station
 * @return the value of the eta factor
*/
double LLTAnalysis::Eta(int m)
{
    return PI/2.0/double(m_pWing->m_NStation) * sin(double(m)*PI/double(m_pWing->m_NStation)) ;
}


/**
 * The multiplier for the yawing and rolling moment coefficients in the case of asymmetrical distributions.
 * cf. NACA TN-1269 for details
 * @param m
 * @return the value of the sigma factor
*/
double LLTAnalysis::Sigma(int m)
{
    return PI/8.0/double(m_pWing->m_NStation) * sin(2.*double(m)*PI/double(m_pWing->m_NStation)) ;
}



/**
 * The multiplier for the induced angle of attack in the case of asymmetrical distributions.
 * cf. NACA TN-1269 for details
 * @param m
 * @param k
 * @return the value of the beta factor
*/
double LLTAnalysis::Beta(int m, int k)
{
    double b=0;
    double fk = double(k);
    double fm = double(m);
    double fr = double(m_pWing->m_NStation);

    if (m==k) b = 180.0*fr/8.0/PI/sin(fk*PI/fr);
    else if ((m+k)%2==0) b=0.0;
    else
    {
        double c1 = 180.0/4.0/PI/fr/sin(fk*PI/fr);
        double c2 =   1.0/(1.0-cos((fk+fm)*PI/fr))
                - 1.0/(1.0-cos((fk-fm)*PI/fr));
        b = c1 * c2;
    }
    return b;
}


/**
 * Once the solution has converged, computes the wing's aerodynamic coefficients and stores them in the member variables.
 * @param QInf the freestream velocity, in m/s
 * @param Alpha the angle of attack, in degrees
 * @param ErrorMessage a reference to the output string which is filled with the error messages
 */
void LLTAnalysis::computeWing(double QInf, double Alpha, QString &ErrorMessage)
{
    Foil * pFoil0 = nullptr;
    Foil * pFoil1 = nullptr;

    QString strange;
    double yob=0, tau=0, c4=0, zpos=0;

    double Integral0           = 0.0;
    double Integral1           = 0.0;
    double Integral2           = 0.0;
    double Integral3           = 0.0;
    double InducedDrag         = 0.0;
    double ViscousDrag         = 0.0;
    double InducedYawingMoment = 0.0;
    double ViscousYawingMoment = 0.0;
    double PitchingMoment      = 0.0;
    double VCm                 = 0.0;
    double ICm                 = 0.0;
    double eta=0, sigma=0;
    double Cm0=0;

    bool bOutRe=false, bError=false;
    bool bPointOutRe=false, bPointOutAlpha=false;
    m_bWingOut = false;

    ErrorMessage.clear();

    for (int m=1; m<s_NLLTStations; m++)
    {
        bPointOutRe    = false;
        bPointOutAlpha = false;
        yob   = cos(double(m)*PI/double(s_NLLTStations));
        m_pWing->getFoils(&pFoil0, &pFoil1, yob*m_pWing->m_PlanformSpan/2.0, tau);

        m_Cl[m]     = getCl(pFoil0, pFoil1, m_Re[m], Alpha+m_Ai[m]+m_Twist[m], tau, bOutRe, bError);
        if(bOutRe) bPointOutRe = true;
        if(bError) bPointOutAlpha = true;

        m_PCd[m]    = getCd(pFoil0, pFoil1, m_Re[m], Alpha+m_Ai[m]+m_Twist[m], tau, m_pWing->m_AR, bOutRe, bError);
        if(bOutRe) bPointOutRe = true;
        if(bError) bPointOutAlpha = true;

        m_ICd[m]    = -m_Cl[m] * (m_Ai[m]* PI/180.0);

        m_XTrTop[m] = getXTr(pFoil0, pFoil1, m_Re[m], Alpha+m_Ai[m] + m_Twist[m], tau, true, bOutRe, bError);
        if(bOutRe) bPointOutRe = true;
        if(bError) bPointOutAlpha = true;

        m_XTrBot[m] = getXTr(pFoil0, pFoil1, m_Re[m], Alpha+m_Ai[m]+m_Twist[m], tau, false, bOutRe, bError);
        if(bOutRe) bPointOutRe = true;
        if(bError) bPointOutAlpha = true;

        m_CmAirf[m] = getCm(pFoil0, pFoil1, m_Re[m], Alpha+m_Ai[m]+m_Twist[m], tau, bOutRe, bError);
        if(bOutRe) bPointOutRe = true;
        if(bError) bPointOutAlpha = true;

        m_XCPSpanRel[m] = getXCp(pFoil0, pFoil1, m_Re[m], Alpha+m_Ai[m]+m_Twist[m], tau, bOutRe, bError);

        if(qAbs(m_XCPSpanRel[m])<0.000001)
        {
            //plr mesh was generated prior to v3.15, i.e., without XCp calculations
            Cm0 = getCm0(pFoil0, pFoil1, m_Re[m],tau, bOutRe, bError);
            if(m_Cl[m]!=0.0) m_XCPSpanRel[m] = 0.25 - Cm0/m_Cl[m];
            else             m_XCPSpanRel[m] = 0.25;
        }
        if(bOutRe) bPointOutRe = true;
        if(bError) bPointOutAlpha = true;

        // incorrect up to v6.47, moments should be calculated in wind axes, notwithstanding induced angle and twist
        // makes a non-significant difference on results
/*        double arad = (Alpha+m_Ai[m]+m_Twist[m])*PI/180.0;
        double sina = sin(arad);
        double cosa = cos(arad);*/

        // v6.48
        // added wingLE position to the calculation of lever arms - cf. Ticket 147
        c4   = m_pPlane->wingLE(0).x + m_pWing->C4(yob)                                - m_pWPolar->CoG().x; //m
        zpos = m_pPlane->wingLE(0).z + m_pWing->zPos(yob*m_pWing->m_PlanformSpan/2.0)  - m_pWPolar->CoG().z; //m
        Vector3d LeverArm(c4, 0, zpos); //m
        Vector3d Finv(m_ICd[m], 0.0, m_Cl[m]); // Inviscid force,  N/qS
        Vector3d Fvisc(m_PCd[m],0,0);          // Viscous force,   N/qS
        Vector3d Minv  = LeverArm * Finv;      // Inviscid moment, Nm/qS
        Vector3d Mvisc = LeverArm * Fvisc;     // Viscousmoment,   Nm/qS

        double Cm_i = m_CmAirf[m] + Minv.y /m_pWing->mac();  // N.m/qSc
        double Cm_v =               Mvisc.y/m_pWing->mac();  // N.m/qSc

        m_Cm[m] = Cm_i + Cm_v;                               // N.m/qSc

        eta = Eta(m);
        sigma = Sigma(m);
        Integral0           += eta   * m_Cl[m]   * m_Chord[m];
        Integral1           += sigma * m_Cl[m]   * m_Chord[m];
        Integral2           += eta   * m_Cl[m]   * m_Chord[m] * (m_Offset[m]+m_XCPSpanRel[m]*m_Chord[m]);
        Integral3           += eta   * m_Cl[m]   * m_Chord[m] * (zpos*m_Chord[m]);
        //        Integral3           += eta   * m_Cl[m]  * m_Chord[m] * ((m_XCPSpanRel[m]*m_Chord[m]*cos(-m_Twist[m]*PI/180.0)+m_Offset[m]) * sin(-Alpha*PI/180.0) + (zpos*m_Chord[m]+m_XCPSpanRel[m]*m_Chord[m]*sin(-m_Twist[m]*PI/180.0)) * cos(-Alpha*PI/180.0));
        InducedDrag         += eta   * m_Cl[m]   * m_Chord[m] * (-m_Ai[m]);
        ViscousDrag         += eta   * m_PCd[m]  * m_Chord[m];
        InducedYawingMoment += sigma * m_Cl[m]   * m_Chord[m] * (-m_Ai[m]);
        ViscousYawingMoment += sigma * m_PCd[m]  * m_Chord[m];
        PitchingMoment      += eta   * m_Cm[m]   * m_Chord[m] * m_Chord[m];
        VCm                 += eta   * Cm_v * m_Chord[m] * m_Chord[m];
        ICm                 += eta   * Cm_i * m_Chord[m] * m_Chord[m];

        if(bPointOutAlpha)
        {
            ErrorMessage = QString("       Span pos = %1 ").arg(cos(m*PI/s_NLLTStations)*m_pWing->m_PlanformSpan/2.0*m_mtoUnit,9,'f',2);
            ErrorMessage += m_LengthUnit;
            ErrorMessage += ",  Re = ";
            strange = QString::asprintf("%.0f", m_Re[m]);
            ErrorMessage += strange;

            strange = QString(" ,  A+Ai+Twist = %1 could not be interpolated\n").arg(Alpha+m_Ai[m] + m_Twist[m],6,'f',1);
            ErrorMessage+=strange;

            m_bWingOut = true;
            m_bConverged = false;
        }
        else if(bPointOutRe)
        {
            ErrorMessage = QString("       Span pos = %1 ").arg(cos(m*PI/s_NLLTStations)*m_pWing->m_PlanformSpan/2.0*m_mtoUnit,9,'f',2);
            ErrorMessage += m_LengthUnit;
            ErrorMessage += ",  Re = ";
            strange = QString::asprintf("%.0f", m_Re[m]);
            ErrorMessage += strange;

            strange = QString(" ,  A+Ai+Twist = %1 is outside the flight envelope\n").arg(Alpha+m_Ai[m] + m_Twist[m],6,'f',1);
            ErrorMessage+=strange;

            m_bWingOut = true;
        }
    }

    m_CL    =  Integral0   * m_pWing->m_AR /m_pWing->m_PlanformSpan;
    m_CDi   =  InducedDrag * m_pWing->m_AR /m_pWing->m_PlanformSpan  * PI / 180.0;
    m_CDv   =  ViscousDrag / m_pWing->m_GChord;

    m_VYm = ViscousYawingMoment /m_pWing->m_GChord;
    m_IYm = InducedYawingMoment /m_pWing->m_PlanformSpan * PI * m_pWing->m_AR /180.0;
    m_GYm = m_VYm + m_IYm;
    //    m_GCm = PitchingMoment / m_GChord / m_MAChord;
    m_VCm = VCm / m_pWing->m_GChord / m_pWing->m_MAChord;
    m_ICm = ICm / m_pWing->m_GChord / m_pWing->m_MAChord;
    m_GCm = m_VCm + m_ICm;

    m_GRm = -Integral1   * m_pWing->m_AR /m_pWing->m_PlanformSpan;

    if(m_CL !=0.0)
    {
        m_CP.x = Integral2 * m_pWing->m_AR /m_pWing->m_PlanformSpan/m_CL;
        //        m_ZCP = Integral3 * m_pWing->m_AR /m_pWing->m_PlanformSpan/m_CL;
        m_CP.z=0.0;//the ZCP position may make physical sense in 3D panel analysis, but not in LLT

    }
    else
    {
        m_CP.set(0.0,0.0,0.0);
    }
    if(m_pWing->m_bSymetric) m_CP.y = 0.0;
    else                     m_CP.y = m_pWing->m_AR/m_CL * Integral1;

    setBending(QInf);
}



/**
 * Calculates the bending moment at span stations, based on the results of the analysis and on the freestrem velocity.
 * @param QInf the freestream velocity, in m/s
 */
void LLTAnalysis::setBending(double QInf)
{
    //dynamic pressure, kg/m3
    double q = 0.5*m_pWPolar->density() * QInf * QInf;

    double bm=0;
    double y=0, yy=0;

    for (int j=1; j<s_NLLTStations; j++)
    {
        y = m_SpanPos[j];
        bm = 0.0;
        if (y>=0)
        {
            for (int jj=0; jj<j; jj++)
            {
                yy =  m_SpanPos[jj];
                bm += (yy-y) * m_Cl[jj] * m_StripArea[jj];
            }
        }
        else
        {
            for (int jj=j+1; jj<s_NLLTStations; jj++)
            {
                yy =  m_SpanPos[jj];
                bm += (y-yy) * m_Cl[jj] * m_StripArea[jj];
            }
        }
        m_BendingMoment[j] = bm*q;
    }
}



/**
 * Calculates the linear solution to the Lifting line problem, for the given wing geometry and angle of attack.
 * This is the starting point for the non-linear iterations.
 * A simplifying assumtion is that the lift slope is Cl = 2.pi (alpha-alpha0+wahshout) at all positions.
 * Numerical experiments have shown however that the non-linear LLT converges in roughly the same amount of iterations
 * whatever the initial state, even if random or asymetric.
 * @param Alpha the angle of attack, in degrees
 * @return true if a linear solution has been set, false otherwise. Should always be true, unless the user has defined some crazy wing configuration. Who knows what a user can do ?
 */
bool LLTAnalysis::setLinearSolution(double Alpha)
{
    QString strange;
    traceLog("Setting initial linear solution\n");

    QVector<double> aij(s_NLLTStations*s_NLLTStations);
    QVector<double> rhs(s_NLLTStations+1);

    memset(aij.data(), 0, ulong(s_NLLTStations*s_NLLTStations)*sizeof(double));
    memset(rhs.data(), 0, ulong(s_NLLTStations+1)*sizeof(double));

    Foil *pFoil0=nullptr, *pFoil1=nullptr;

    int size = s_NLLTStations-1;
    double dn  = double(s_NLLTStations);
    double di=0, dj=0, t0=0, st0=0, snt0=0, ch=0, a0=0, slope=0, tau=0, yob=0, twist=0;
    double cs = m_pWing->rootChord();
    double b  = m_pWing->m_PlanformSpan;

    for (int i=1; i<s_NLLTStations; i++)
    {
        di  = double(i);
        t0  = di * PI/dn;
        yob = cos(t0);
        ch = m_pWing->getChord(yob);      //or m_Chord[i], same
        twist = m_pWing->getTwist(yob);   //or m_Twist[i], same

        st0 = sin(t0);

        for (int j=1; j<s_NLLTStations; j++)
        {
            dj   = double(j);
            snt0 = sin(dj*t0);

            int p = (i-1)*size + (j-1);
            aij[p] = snt0 + ch*PI/b/2.0* dj*snt0/st0;
        }

        m_pWing->getFoils(&pFoil0, &pFoil1, yob*b/2.0, tau);
        a0 = getZeroLiftAngle(pFoil0, pFoil1, m_Re[i], tau);
        rhs[i] = ch/cs * (Alpha-a0+twist)/180.0*PI;
    }

    bool bCancel = false;
    if(!Gauss(aij.data(), s_NLLTStations-1, rhs.data()+1, 1, &bCancel))
    {
        return false;
    }
    traceLog("  station         Cl           Ai\n");
    for (int i=1; i<s_NLLTStations; i++)
    {
        di  = double(i);
        t0  = di * PI/dn;
        yob = cos(t0);

        m_Cl[i] = 0.0;
        for (int j=1; j<s_NLLTStations; j++)
        {
            dj = double(j);
            snt0 = sin(dj*t0);
            m_Cl[i] += rhs[j]* snt0;
        }
        m_pWing->getFoils(&pFoil0, &pFoil1, yob*b/2.0, tau);
        getLinearizedPolar(pFoil0, pFoil1, m_Re[i], tau, a0, slope);
        a0 = getZeroLiftAngle(pFoil0, pFoil1, m_Re[i], tau); //better approximation ?

        m_Cl[i] *= slope*180.0/PI*cs/m_pWing->getChord(yob);
        m_Ai[i]  = -(Alpha-a0+m_pWing->getTwist(yob)) + m_Cl[i]/slope;

//        strange = QString::asprintf("    %2d   %11g  %11g\n", i, m_Cl[i], m_Ai[i]);
//        traceLog(strange);
    }
//    traceLog("\n\n");
    return true;
}


/** 
 * Calculates the induced angle from the lift coefficient and from the Beta factor
 * @param k
 * @return the induced angle, in degrees
 */
double LLTAnalysis::AlphaInduced(int k)
{
    double ai = 0.0;

    for (int m=1; m<m_pWing->m_NStation; m++)
    {
        ai += Beta(m,k) * m_Cl[m] * m_Chord[m]/m_pWing->m_PlanformSpan;
    }
    return ai;
}


/** 
 * Performs the iterations of the non-linear LLT analysis.
 * @param QInf the freestream velocity, in m/s
 * @param Alpha the angle of attack, in degrees
 * @return the number of iterations performed, or -1 if the analysis has been user-cancelled
*/
int LLTAnalysis::iterate(double &QInf, double Alpha)
{
    Foil* pFoil0  = nullptr;
    Foil* pFoil1  = nullptr;
    double  yob=0, tau=0, anext=0;
    bool bOutRe=false, bError=false;
    int iter = 0;

    while(iter<s_IterLim)
    {
        if(m_bCancel) return -1;
        m_Maxa = 0.0;

        for (int k=1; k<s_NLLTStations; k++)
        {
            double a = m_Ai[k];
            anext    = -AlphaInduced(k);
            m_Ai[k]  = a +(anext-a)/s_RelaxMax;
            m_Maxa   = qMax(m_Maxa, qAbs(a-anext));
        }

        double Lift=0.0;// required for Type 2
        for (int k=1; k<s_NLLTStations; k++)
        {
            yob     = cos(k*PI/s_NLLTStations);
            m_pWing->getFoils(&pFoil0, &pFoil1, yob*m_pWing->m_PlanformSpan/2.0, tau);
            m_Cl[k] = getCl( pFoil0, pFoil1, m_Re[k], Alpha + m_Ai[k]+ m_Twist[k], tau, bOutRe, bError);
            if (m_pWPolar->polarType()==xfl::FIXEDLIFTPOLAR)
            {
                Lift += Eta(k) * m_Cl[k] * m_Chord[k];
            }
        }

        if(m_pWPolar->polarType()==xfl::FIXEDLIFTPOLAR)
        {
            Lift *= m_pWing->m_AR / m_pWing->m_PlanformSpan;
            if(Lift<=0.0)  return -1;

            QInf  = m_QInf0 / sqrt(Lift);

            for (int k=1; k<s_NLLTStations; k++)
            {
                m_Re[k] = m_Chord[k] * QInf /m_pWPolar->m_Viscosity;
                yob     = cos(k*PI/s_NLLTStations);
                m_pWing->getFoils(&pFoil0, &pFoil1, yob*m_pWing->m_PlanformSpan/2.0, tau);
                m_Cl[k] = getCl(pFoil0, pFoil1, m_Re[k], Alpha + m_Ai[k]+ m_Twist[k], tau, bOutRe, bError);
            }
        }

        if (m_Maxa<s_CvPrec)
        {
            m_bConverged = true;
            break;
        }

        //        if(m_pCurve) m_pCurve->appendPoint(iter, m_Maxa);
        if(m_pX && m_pY)
        {
            m_pX->append(double(iter));
            m_pY->append(m_Maxa);
        }
        iter++;
    }
    return iter;
}


/**
 * Initializes the geometric data necessary for the LLT calculation
*/
void LLTAnalysis::initializeGeom()
{    
    m_bWingOut = false;
    m_bConverged = false;

    if(m_pWPolar->isFixedLiftPolar()) m_QInf0 = sqrt(2.*m_pWPolar->mass()* 9.81 /m_pWPolar->density()/m_pWing->m_PlanformArea);
    else                              m_QInf0 = 0.0;

    m_pWing->computeChords(s_NLLTStations, m_Chord, m_Offset, m_Twist);

    for (int k=0; k<=s_NLLTStations; k++)
    {
        //        y   = cos(k*PI/s_NLLTStations)* m_pWing->m_PlanformSpan/2.0;
        m_SpanPos[k] = m_pWing->m_PlanformSpan/2.0 * cos(double(k)*PI/double(s_NLLTStations));
    }

    for (int j=1; j<s_NLLTStations; j++)
    {
        double yjp = m_SpanPos[j-1];
        double yjm = m_SpanPos[j+1];
        double yj  = m_SpanPos[j];

        double dy = (yjp-yj)/2.0 + (yj-yjm)/2.0;

        m_StripArea[j] = m_Chord[j]*dy;//m2
    }
}


bool LLTAnalysis::loop()
{
    if (m_pWPolar->polarType()!=xfl::FIXEDAOAPOLAR)
    {
        return alphaLoop();
    }
    else
    {
        return QInfLoop();
    }
}


/**
* Launches a type 1 or 2 analysis.
* Loops over the range of specified aoa.
* For each successful aoa, stores the data in the WPolar and Operating Point objects.
*/
bool LLTAnalysis::alphaLoop()
{
    QString str;

    Foil *pFoil0=nullptr, *pFoil1=nullptr;
    bool bOutRe=false, bError=false;
    double tau = 0.0;

    for (int i=0; i<=m_nPoints; i++)
    {
        if(m_pX) m_pX->clear();
        if(m_pY) m_pY->clear();

        double Alpha = m_vMin + double(i) * m_vDelta;
        if(m_bCancel)
        {
            str = "Analysis cancelled on user request....\n";
            traceLog(str);
            break;
        }

        setVelocity(m_pWPolar->m_QInfSpec);
        if(s_bInitCalc) setLinearSolution(Alpha);

        //initialize first iteration
        for (int k=1; k<s_NLLTStations; k++)
        {
            double yob   = cos(double(k)*PI/double(s_NLLTStations));
            m_pWing->getFoils(&pFoil0, &pFoil1, yob*m_pWing->m_PlanformSpan/2.0, tau);
            m_Cl[k] = getCl(pFoil0, pFoil1, m_Re[k], Alpha + m_Ai[k] + m_Twist[k], tau, bOutRe, bError);
        }


        str= QString("Calculating Alpha = %1... ").arg(Alpha,5,'f',2);
        traceLog(str);

        int iter = iterate(m_pWPolar->m_QInfSpec, Alpha);

        if (iter==-1 && !m_bCancel)
        {
            str= QString("    ...negative Lift... Aborting\n");
            m_bError = true;
            s_bInitCalc = true;
            traceLog(str);
        }
        else if (iter<s_IterLim && !m_bCancel)
        {
            //converged,
            str= QString("    ...converged after %1 iterations\n").arg(iter);
            traceLog(str);
            computeWing(m_pWPolar->m_QInfSpec, Alpha, str);// generates wing results,
            traceLog(str);
            if (m_bWingOut) m_bWarning = true;
            PlaneOpp *pPOpp = createPlaneOpp(m_pWPolar->m_QInfSpec, Alpha, m_bWingOut);// Adds WOpp point and adds result to polar
            if(pPOpp) m_PlaneOppList.append(pPOpp);
            s_bInitCalc = false;
        }
        else
        {
            if (m_bWingOut) m_bWarning = true;
            m_bError = true;
            str= QString("    ...unconverged after %1 iterations out of %2\n").arg(iter).arg(s_IterLim);
            traceLog(str);
            s_bInitCalc = true;
        }
    }
    return true;
}


/**
* Launches a type 4 analysis.
* Loops over the range of specified velocities.
* For each successful aoa, stores the data in the WPolar and Operating Point objects.
*/
bool LLTAnalysis::QInfLoop()
{
    QString str;
    double tau=0.0;
    Foil *pFoil0=nullptr, *pFoil1=nullptr;
    bool bOutRe=false, bError=false;

    str = "Initializing analysis...\n";
    traceLog(str);

    double QInf = 0.0;
    double Alpha = m_pWPolar->m_AlphaSpec;

    for (int i=0; i<=m_nPoints; i++)
    {
        QInf = m_vMin + double(i) * m_vDelta;
        if(m_bCancel)
        {
            str = "Analysis cancelled on user request....\n";
            traceLog(str);
            break;
        }

        setVelocity(QInf);
        if(s_bInitCalc) setLinearSolution(m_pWPolar->m_AlphaSpec);

        //initialize first iteration
        for (int k=1; k<s_NLLTStations; k++)
        {
            double yob   = cos(k*PI/s_NLLTStations);
            m_pWing->getFoils(&pFoil0, &pFoil1, yob*m_pWing->m_PlanformSpan/2.0, tau);
            m_Cl[k] = getCl(pFoil0, pFoil1, m_Re[k], Alpha + m_Ai[k] + m_Twist[k], tau, bOutRe, bError);
        }

        str = QString("Calculating QInf = %1... ").arg(QInf,6,'f',2);
        traceLog(str);
        int iter = iterate(QInf, m_pWPolar->m_AlphaSpec);

        if(iter<0)
        {
            //unconverged
            m_bError = true;
            m_bWarning = true;
            str = QString("\n");
            traceLog(str);
            s_bInitCalc = true;
        }
        else if (iter<s_IterLim  && !m_bCancel)
        {
            //converged,
            str = QString("    ...converged after %1 iterations\n").arg(iter);
            traceLog(str);
            computeWing(QInf, m_pWPolar->m_AlphaSpec,str);// generates wing results,
            traceLog(str);
            if (m_bWingOut) m_bWarning = true;
            PlaneOpp *pPOpp = createPlaneOpp(QInf, m_pWPolar->m_AlphaSpec, m_bWingOut);// Adds WOpp point and adds result to polar
            if(pPOpp) m_PlaneOppList.append(pPOpp);

            /*            if(m_bWingOut)
            {
                str = QString("\n");
                traceLog(str);
            }*/
            s_bInitCalc = false;
        }
        else
        {
            if (m_bWingOut) m_bWarning = true;
            m_bError = true;
            str = QString("    ...unconverged after %1 iterations\n").arg(iter);
            traceLog(str);
            s_bInitCalc = true;
        }

        if(m_pX) m_pX->clear();
        if(m_pY) m_pY->clear();
    }
    return true;
}



/**
* Copies the value of the input parameters to the member variables
*/
void LLTAnalysis::setLLTRange(double AlphaMin, double AlphaMax, double DeltaAlpha, bool bSequence)
{
    m_vMin      = AlphaMin;
    m_vMax      = AlphaMax;
    m_vDelta    = DeltaAlpha;
    m_bSequence = bSequence;

    if(m_vMax<m_vMin) m_vDelta = -double(qAbs(m_vDelta));
    m_nPoints  = int(qAbs((m_vMax-m_vMin)*1.001/m_vDelta));
    if(!m_bSequence) m_nPoints = 0;

}



void LLTAnalysis::setLLTData(Plane *pPlane, WPolar *pWPolar)
{
    m_pPlane   = pPlane;
    m_pWing    = pPlane->wing();
    m_pWPolar  = pWPolar;
}


void LLTAnalysis::initializeAnalysis()
{
    m_bWarning = m_bError = false;
    m_PlaneOppList.clear();

    traceLog("\nLaunching the LLT Analysis....\n");

    initializeGeom();
}



/** emits the analysis messages to the world */
void LLTAnalysis::traceLog(QString str)
{
    emit outputMsg(str);
}



/**
* Creates the plane's operating point,
* fills it with the input resulting from the  analysis, and returns a pointer to the PlaneOpp object which has been created.

*@param bPointOut: if true, part of the oppoint viscous properties could not be interpolated
*@param Cp: the array of Cp value for each panel
*@param Gamma: the array of circulation or doublet strengths Gamma for each panel
*@param Sigma: the array of source strengths for a panel analysis
*@return a pointer to the PlaneOpp object which has been created
*/
PlaneOpp* LLTAnalysis::createPlaneOpp(double QInf, double Alpha, bool bWingOut)
{
    PlaneOpp *pNewPOpp = new PlaneOpp(m_pPlane, m_pWPolar, 0);

    if(!pNewPOpp)
    {
        traceLog("Not enough memory to store the OpPoint\n");
        return nullptr;
    }
    else
    {
        pNewPOpp->m_Alpha               = Alpha;
        pNewPOpp->m_QInf                = QInf;
        pNewPOpp->m_NPanels             = m_pWing->m_nPanels;
        pNewPOpp->m_bOut                = m_bWingOut;
        pNewPOpp->m_CL                  = m_CL;
        pNewPOpp->m_ICD                 = m_CDi;

        pNewPOpp->m_GCm                 = m_GCm;
        pNewPOpp->m_VCm                 = m_VCm;
        pNewPOpp->m_ICm                 = m_ICm;
        pNewPOpp->m_GRm                 = m_GRm;
        pNewPOpp->m_GYm                 = m_GYm;
        pNewPOpp->m_VYm                 = m_VYm;
        pNewPOpp->m_IYm                 = m_IYm;

        pNewPOpp->m_CP                  = m_CP;
        pNewPOpp->m_VCD                 = m_CDv;

        pNewPOpp->m_bOut                = bWingOut;


        pNewPOpp->addWingOpp(0, 0);
        pNewPOpp->m_pWOpp[0]->createWOpp(m_pWing, m_pWPolar);

        WingOpp *pNewPoint = pNewPOpp->m_pWOpp[0];
        {
            pNewPoint->m_Alpha          = Alpha;
            pNewPoint->m_QInf           = QInf;

            pNewPoint->m_bOut           = m_bWingOut;
            pNewPoint->m_CL             = m_CL;
            pNewPoint->m_ICD            = m_CDi;

            pNewPoint->m_GCm            = m_GCm;
            pNewPoint->m_VCm            = m_VCm;
            pNewPoint->m_ICm            = m_ICm;
            pNewPoint->m_GRm            = m_GRm;
            pNewPoint->m_GYm            = m_GYm;
            pNewPoint->m_VYm            = m_VYm;
            pNewPoint->m_IYm            = m_IYm;

            pNewPoint->m_CP             = m_CP;
            pNewPoint->m_VCD            = m_CDv;

            int nStation = LLTAnalysis::s_NLLTStations;
            pNewPoint->m_NStation = nStation;

            double Cb =0.0;
            for (int l=1; l<nStation; l++)
            {
                pNewPoint->m_SpanPos[l]       = -m_SpanPos[l];
                pNewPoint->m_StripArea[l]     =  m_StripArea[l];
                pNewPoint->m_Ai[l]            =  m_Ai[nStation-l];
                pNewPoint->m_Cl[l]            =  m_Cl[nStation-l];
                pNewPoint->m_PCd[l]           =  m_PCd[nStation-l];
                pNewPoint->m_ICd[l]           =  m_ICd[nStation-l];
                pNewPoint->m_Cm[l]            =  m_Cm[nStation-l];
                pNewPoint->m_CmAirf[l]        =  m_CmAirf[nStation-l];
                pNewPoint->m_XCPSpanRel[l]    =  m_XCPSpanRel[nStation-l];
                pNewPoint->m_XCPSpanAbs[l]    =  m_XCPSpanAbs[nStation-l];
                pNewPoint->m_Re[l]            =  m_Re[nStation-l];
                pNewPoint->m_Chord[l]         =  m_Chord[nStation-l];
                pNewPoint->m_Twist[l]         =  m_Twist[nStation-l];
                pNewPoint->m_XTrTop[l]        =  m_XTrTop[nStation-l];
                pNewPoint->m_XTrBot[l]        =  m_XTrBot[nStation-l];
                pNewPoint->m_BendingMoment[l] =  m_BendingMoment[nStation-l];
                if(qAbs(m_BendingMoment[l])>qAbs(Cb))    Cb = m_BendingMoment[l];
            }

            pNewPoint->m_MaxBending = Cb;
        }
    }

    //add the data to the polar object
    if(PlaneOpp::s_bKeepOutOpps || !pNewPOpp->m_bOut)
        m_pWPolar->addPlaneOpPoint(pNewPOpp);

    return pNewPOpp;
}



void LLTAnalysis::setPlane(Plane *pPlane)
{
    m_pPlane   = pPlane;
    m_pWing    = pPlane->wing();
}


void LLTAnalysis::setWPolar(WPolar *pWPolar)
{
    m_pWPolar  = pWPolar;
}


void LLTAnalysis::clearPOppList()
{
    for(int ip=m_PlaneOppList.count()-1; ip>=0; ip--)
    {
        delete m_PlaneOppList.at(ip);
        m_PlaneOppList.removeAt(ip);
    }
}


void LLTAnalysis::onCancel()
{
    m_bCancel = true;
    traceLog("Cancelling the LLT analysis\n");
}


bool LLTAnalysis::isCancelled() const
{
    return m_bCancel;
}

bool LLTAnalysis::hasWarnings() const
{
    return m_bError || m_bWarning;
}


/**
*Interpolates the lift coefficient on the polar mesh, based on the geometrical position of a point between two sections on a wing.
*@param pFoil0 the pointer to the left foil  of the wing's section.
*@param pFoil1 the pointer to the left foil  of the wing's section.
*@param Re the Reynolds number at the point's position.
*@param Alpha the apparent aoa at the point's position.
*@param Tau the relative position of the point between the two foils.
*@param bOutRe true if Cl is outside the min or max Cl of the polar mesh.
*@param bError if Re is outside the min or max Reynolds number of the polar mesh.
*@return the interpolated value.
*/
double LLTAnalysis::getCl(const Foil *pFoil0, const Foil *pFoil1, double Re, double Alpha, double Tau, bool &bOutRe, bool &bError)
{
    double Cl0=0, Cl1=0;
    bool IsOutRe = false;
    bool IsError  = false;
    bOutRe = false;
    bError = false;

    if(!pFoil0)
        Cl0 = 2.0*PI*(Alpha*PI/180.0);
    else
        Cl0 = getPlrPointFromAlpha(pFoil0, Re, Alpha, 1, IsOutRe, IsError);
    if(IsOutRe) bOutRe = true;
    if(IsError) bError = true;
    if(!pFoil1)
        Cl1 = 2.0*PI*(Alpha*PI/180.0);
    else
        Cl1 = getPlrPointFromAlpha(pFoil1, Re, Alpha, 1, IsOutRe, IsError);
    if(IsOutRe) bOutRe = true;
    if(IsError) bError = true;

    if (Tau<0.0) Tau = 0.0;
    if (Tau>1.0) Tau = 1.0;

    return ((1-Tau) * Cl0 + Tau * Cl1);
}


/**
*Returns the zero-lift moment coefficient interpolated on the polar mesh, based on the geometrical position of a point between two sections on a wing.
*@param pFoil0 the pointer to the left foil  of the wing's section.
*@param pFoil1 the pointer to the left foil  of the wing's section.
*@param Re the Reynolds number at the point's position.
*@param Tau the relative position of the point between the two foils.
*@param bOutRe true if Cl is outside the min or max Cl of the polar mesh.
*@param bError if Re is outside the min or max Reynolds number of the polar mesh.
*@return the interpolated value for the zero-moment lift coefficient.
*/
double LLTAnalysis::getCm0(const Foil *pFoil0, const Foil *pFoil1, double Re, double Tau, bool &bOutRe, bool &bError)
{
    //Find 0-lift angle for local foil
    double Alpha=0;
    double Cm0=0, Cm1=0;
    double Cl0 = 1.0;
    double Cl1=0;
    bOutRe = false;
    bError = false;
    bool IsOutRe=false;
    bool IsError=false;

    bOutRe = false;
    for (int i=-10; i<10; i++)
    {
        Alpha = double(i);
        Cl1 = getCl(pFoil0, pFoil1, Re, Alpha, Tau, IsOutRe, IsError);
        if(Cl1>0.0)
        {
            if(IsOutRe) bOutRe = true;
            if(IsError) bError = true;
            break;
        }
        Cl0 = Cl1;
    }
    if(Cl0>0.0)
    {
        return 0.0;
    }
    Cm0 = getCm(pFoil0, pFoil1, Re, Alpha-1.0, Tau, IsOutRe, IsError);
    if(IsOutRe) bOutRe = true;
    if(IsError) bError = true;
    Cm1 = getCm(pFoil0, pFoil1, Re, Alpha, Tau, IsOutRe, IsError);
    if(IsOutRe) bOutRe = true;
    if(IsError) bError = true;

    double Res = Cm0 + (Cm1-Cm0)*(0.0-Cl0)/(Cl1-Cl0);

    return Res;
}


/**
*Interpolates the moment coefficient on the polar mesh, based on the geometrical position of a point between two sections on a wing.
*@param nVar the index of the variable to interpolate.
*@param pFoil0 the pointer to the left foil  of the wing's section.
*@param pFoil1 the pointer to the left foil  of the wing's section.
*@param Re the Reynolds number at the point's position.
*@param Alpha the apparent aoa  at the point's position.
*@param Tau the relative position of the point between the two foils.
*@param bOutRe true if Cl is outside the min or max Cl of the polar mesh.
*@param bError if Re is outside the min or max Reynolds number of the polar mesh.
*@return the interpolated value.
*/
double LLTAnalysis::getCm(const Foil *pFoil0, const Foil *pFoil1, double Re, double Alpha, double Tau, bool &bOutRe, bool &bError)
{
    double Cm0=0, Cm1=0;
    bool IsOutRe = false;
    bool IsError  = false;
    bOutRe = false;
    bError = false;

    if(!pFoil0)
        Cm0 = 0.0;
    else
        Cm0 = getPlrPointFromAlpha(pFoil0, Re, Alpha, 4, IsOutRe, IsError);
    if(IsOutRe) bOutRe = true;
    if(IsError) bError = true;

    if(!pFoil1)
        Cm1 = 0.0;
    else
        Cm1 = getPlrPointFromAlpha(pFoil1, Re, Alpha, 4, IsOutRe, IsError);
    if(IsOutRe) bOutRe = true;
    if(IsError) bError = true;

    if (Tau<0.0) Tau = 0.0;
    if (Tau>1.0) Tau = 1.0;
    return ((1-Tau) * Cm0 + Tau * Cm1);
}


/**
*Interpolates the drag coefficient on the polar mesh, based on the geometrical position of a point between two sections on a wing.
*@param nVar the index of the variable to interpolate.
*@param pFoil0 the pointer to the left foil  of the wing's section.
*@param pFoil1 the pointer to the left foil  of the wing's section.
*@param Re the Reynolds number at the point's position.
*@param Alpha the apparent aoa  at the point's position.
*@param Tau the relative position of the point between the two foils.
*@param bOutRe true if Cl is outside the min or max Cl of the polar mesh.
*@param bError if Re is outside the min or max Reynolds number of the polar mesh.
*@return the interpolated value.
*/
double LLTAnalysis::getCd(Foil const*pFoil0, Foil const*pFoil1, double Re, double Alpha, double Tau, double AR, bool &bOutRe, bool &bError)
{
    //For LLT calculations
    //returns the interpolated viscous drag
    bool IsOutRe = false;
    bool IsError  = false;
    bOutRe = false;
    bError = false;

    double Cd0, Cd1, Cl;
    if(!pFoil0)
    {
        Cl = 2.0*PI*(Alpha*PI/180.0);
        Cd0 = Cl*Cl/PI/AR;
    }
    else Cd0 = getPlrPointFromAlpha(pFoil0, Re, Alpha,2, IsOutRe, IsError);
    if(IsOutRe) bOutRe = true;
    if(IsError) bError = true;
    if(!pFoil1)
    {
        Cl = 2.0*PI*(Alpha*PI/180.0);
        Cd1 = Cl*Cl/PI/AR;
    }
    else Cd1 = getPlrPointFromAlpha(pFoil1, Re, Alpha,2, IsOutRe, IsError);
    if(IsOutRe) bOutRe = true;
    if(IsError) bError = true;

    if (Tau<0.0) Tau = 0.0;
    if (Tau>1.0) Tau = 1.0;
    return ((1-Tau) * Cd0 + Tau * Cd1);
}


/**
*Interpolates the center of pressure's x-position coefficient on the polar mesh, based on the geometrical position of a point between two sections on a wing.
*@param pFoil0 the pointer to the left foil  of the wing's section.
*@param pFoil1 the pointer to the left foil  of the wing's section.
*@param Re the Reynolds number at the point's position.
*@param Alpha the apparent aoa  at the point's position.
*@param Tau the relative position of the point between the two foils.
*@param bOutRe true if Cl is outside the min or max Cl of the polar mesh.
*@param bError if Re is outside the min or max Reynolds number of the polar mesh.
*@return the interpolated value.
*/
double LLTAnalysis::getXCp(Foil const*pFoil0, Foil const*pFoil1, double Re, double Alpha, double Tau, bool &bOutRe, bool &bError)
{
    //For LLT calculations
    //returns the interpolated center of pressure position

    bool IsOutRe = false;
    bool IsError  = false;
    bOutRe = false;
    bError = false;

    double XCp0=0, XCp1=0;

    if(!pFoil0) return 0.0;
    else XCp0 = getPlrPointFromAlpha(pFoil0, Re, Alpha, 11, IsOutRe, IsError);
    if(IsOutRe) bOutRe = true;
    if(IsError) bError = true;

    if(!pFoil1) return 0.0;
    else XCp1 = getPlrPointFromAlpha(pFoil1, Re, Alpha, 11, IsOutRe, IsError);
    if(IsOutRe) bOutRe = true;
    if(IsError) bError = true;

    if (Tau<0.0) Tau = 0.0;
    if (Tau>1.0) Tau = 1.0;

    return ((1-Tau) * XCp0 + Tau * XCp1);
}


/**
*Interpolates transition locations on the polar mesh, based on the geometrical position of a point between two sections on a wing.
*@param pFoil0 the pointer to the left foil  of the wing's section.
*@param pFoil1 the pointer to the left foil  of the wing's section.
*@param Re the Reynolds number at the point's position.
*@param Alpha the apparent aoa  at the point's position.
*@param Tau the relative position of the point between the two foils.
*@param bTop true if the upper transition is requested, false otherwise
*@param bOutRe true if Cl is outside the min or max Cl of the polar mesh.
*@param bError if Re is outside the min or max Reynolds number of the polar mesh.
*@return the interpolated value.
*/
double LLTAnalysis::getXTr(Foil const*pFoil0, Foil const*pFoil1, double Re, double Alpha, double Tau, bool bTop, bool &bOutRe, bool &bError)
{
    //For LLT calculations
    //returns the interpolated position of the transition on the  surface specified by bTop


    bool IsOutRe = false;
    bool IsError  = false;
    bOutRe = false;
    bError = false;

    double Tr0=0, Tr1=0;
    if(!pFoil0)
    {
        Tr0 = 1.0;
    }
    else
    {
        if(bTop) Tr0 = getPlrPointFromAlpha(pFoil0, Re, Alpha, 5, IsOutRe, IsError);
        else     Tr0 = getPlrPointFromAlpha(pFoil0, Re, Alpha, 6, IsOutRe, IsError);
    }
    if(IsOutRe) bOutRe = true;
    if(IsError) bError = true;
    if(!pFoil1)
    {
        Tr1 = 1.0;
    }
    else
    {
        if(bTop) Tr1 = getPlrPointFromAlpha(pFoil1, Re, Alpha, 5, IsOutRe, IsError);
        else     Tr1 = getPlrPointFromAlpha(pFoil1, Re, Alpha, 6, IsOutRe, IsError);
    }
    if(IsOutRe) bOutRe = true;
    if(IsError) bError = true;

    if (Tau<0.0) Tau = 0.0;
    if (Tau>1.0) Tau = 1.0;
    return ((1-Tau) * Tr0 + Tau * Tr1);
}


/**
*Interpolates the zero-lift angle on the polar mesh, based on the geometrical position of a point between two sections on a wing.
*@param pFoil0 the pointer to the left foil  of the wing's section.
*@param pFoil1 the pointer to the left foil  of the wing's section.
*@param Re the Reynolds number at the point's position.
*@param Tau the relative position of the point between the two foils.
*@return the interpolated value.
*/
double LLTAnalysis::getZeroLiftAngle(const Foil *pFoil0, const Foil *pFoil1, double Re, double Tau)
{
    //returns the 0-lift angle of the foil, at Reynolds=Re
    //if the polar doesn't reach to 0-lift, returns Alpha0 = 0;
    double a01=0, a02=0;
    double Alpha00=0, Alpha01=0;

    //Find the two polars which enclose Reynolds
    int size = 0;
    Polar *pPolar1=nullptr, *pPolar2=nullptr;

    if(!pFoil0) Alpha00 = 0.0;
    else
    {
        pPolar1 = nullptr;
        pPolar2 = nullptr;
        for (int i=0; i<m_poaPolar->size(); i++)
        {
            Polar *pPolar = m_poaPolar->at(i);
            if(pPolar->foilName() == pFoil0->name()) size++;
        }
        if(size)
        {
            for (int i=0; i<m_poaPolar->size(); i++)
            {
                Polar *pOldPolar = m_poaPolar->at(i);
                if(pOldPolar->foilName().compare(pFoil0->name())==0)
                {
                    if(pOldPolar->m_Reynolds < Re) pPolar1=pOldPolar;
                }
            }
            for (int i=0; i<m_poaPolar->size(); i++)
            {
                Polar *pOldPolar = m_poaPolar->at(i);
                if(pOldPolar->foilName() == pFoil0->name())
                {
                    if(pOldPolar->m_Reynolds > Re)
                    {
                        pPolar2 = pOldPolar;
                        break;
                    }
                }
            }
        }
        if(pPolar1 && pPolar2)
        {
            a01 = pPolar1->getZeroLiftAngle();
            a02 = pPolar2->getZeroLiftAngle();
            Alpha00 = a01 + (a02-a01) * (Re-pPolar1->Reynolds())/(pPolar2->Reynolds()-pPolar1->Reynolds());
        }
        else Alpha00 = 0.0;
    }

    if(!pFoil1) Alpha01 = 0.0;
    else
    {
        pPolar1 = nullptr;
        pPolar2 = nullptr;
        for (int i=0; i<m_poaPolar->size(); i++)
        {
            Polar *pPolar = m_poaPolar->at(i);
            if(pPolar->foilName() == pFoil1->name()) size++;
        }
        if(size)
        {
            for (int i=0; i<m_poaPolar->size(); i++)
            {
                Polar *pPolar = m_poaPolar->at(i);
                if(pPolar->foilName() == pFoil1->name())
                {
                    if(pPolar->Reynolds() < Re) pPolar1 = pPolar;
                }
            }
            for (int i=0; i<m_poaPolar->size(); i++)
            {
                Polar *pPolar = m_poaPolar->at(i);
                if(pPolar->foilName() == pFoil1->name())
                {
                    if(pPolar->Reynolds() > Re)
                    {
                        pPolar2 = pPolar;
                        break;
                    }
                }
            }
        }
        if(pPolar1 && pPolar2)
        {
            a01 = pPolar1->getZeroLiftAngle();
            a02 = pPolar2->getZeroLiftAngle();
            Alpha01 = a01 + (a02-a01) * (Re-pPolar1->Reynolds())/(pPolar2->Reynolds()-pPolar1->Reynolds());
        }
        else Alpha01 = 0.0;
    }

    return ((1-Tau) * Alpha00 + Tau * Alpha01);
}


/**
* Returns the value of an aero coefficient, interpolated on a polar mesh, and based on the value of the Reynolds Number and of the aoa.
* Proceeds by identifiying the two polars surronding Re, then interpolating both with the value of Alpha,
* last by interpolating the requested variable between the values measured on the two polars.
*@param pFoil the pointer to the foil
*@param Re the Reynolds number .
*@param Alpha the angle of attack.
*@param PlrVar the index of the variable to interpolate.
*@param bOutRe true if Cl is outside the min or max Cl of the polar mesh.
*@param bError if Re is outside the min or max Reynolds number of the polar mesh.
*@return the interpolated value.
*/
double LLTAnalysis::getPlrPointFromAlpha(Foil const*pFoil, double Re, double Alpha, int PlrVar, bool &bOutRe, bool &bError)
{
    double amin=0, amax=0;
    double Var1=0, Var2=0, u=0;

    Polar *pPolar=nullptr;

    bOutRe = false;
    bError = false;

    if(!pFoil)
    {
        bOutRe = true;
        bError = true;
        return 0.000;
    }


    int size = 0;
    int n = 0;

    // Are there any Type 1 polars available for this foil ?
    for (int i=0; i<m_poaPolar->size(); i++)
    {
        pPolar = m_poaPolar->at(i);
        if((pPolar->polarType()==xfl::FIXEDSPEEDPOLAR) && (pPolar->foilName() == pFoil->name()))
        {
            n++;
            if(n>=2) break;
        }
    }

    //more than one polar - interpolate between  - tough job

    //First Find the two polars with Reynolds number surrounding wanted Re
    Polar * pPolar1 = nullptr;
    Polar * pPolar2 = nullptr;
    int nPolars = m_poaPolar->size();
    //Type 1 Polars are sorted by crescending Re Number

    //if Re is less than that of the first polar, use this one
    for (int i=0; i<nPolars; i++)
    {
        pPolar = m_poaPolar->at(i);
        if(pPolar->isFixedSpeedPolar() && (pPolar->foilName()==pFoil->name()) && pPolar->m_Alpha.size()>0)
        {
            // we have found the first type 1 polar for this foil
            if (Re < pPolar->Reynolds())
            {
                bOutRe = true;
                //interpolate Alpha on this polar
                QVector<double> const &pX = pPolar->getPlrVariable(PlrVar);
                size = pPolar->m_Alpha.size();
                if(Alpha<pPolar->m_Alpha.front())
                {
                    return pX.front();
                }
                else if(Alpha > pPolar->m_Alpha.back())
                {
                    return pX.back();
                }
                for (int i=0; i<size-1; i++)
                {
                    if(pPolar->m_Alpha.at(i) <= Alpha && Alpha < pPolar->m_Alpha.at(i+1))
                    {
                        //interpolate
                        if(pPolar->m_Alpha.at(i+1)-pPolar->m_Alpha.at(i)<0.00001)//do not divide by zero
                            return pX.at(i);
                        else
                        {
                            u = (Alpha - pPolar->m_Alpha.at(i))
                                    /(pPolar->m_Alpha.at(i+1)-pPolar->m_Alpha.at(i));
                            return pX.at(i) + u * (pX.at(i+1)-pX.at(i));
                        }
                    }
                }
                break;
            }
            break;
        }
    }

    // if not Find the two polars
    for (int i=0; i<nPolars; i++)
    {
        pPolar = m_poaPolar->at(i);
        if(pPolar->isFixedSpeedPolar() && (pPolar->foilName() == pFoil->name()) && pPolar->m_Alpha.size()>0)
        {
            // we have found the first type 1 polar for this foil
            pPolar->getAlphaLimits(amin, amax);
            if (pPolar->Reynolds() <= Re)
            {
                if(amin<=Alpha && Alpha<=amax)
                {
                    pPolar1 = pPolar;
                }
            }
            else {
                if(amin<=Alpha && Alpha<=amax)
                {
                    pPolar2 = pPolar;
                    break;
                }
            }
        }
    }

    if (!pPolar2)
    {
        //then Re is greater than that of any polar
        // so use last polar and interpolate alphas on this polar
        bOutRe = true;
        if(!pPolar1)
        {
            bError = true;
            return 0.000;
        }
        size = pPolar1->m_Alpha.size();
        if(!size)
        {
            bError = true;
            return 0.000;
        }

        QVector<double> const &pX1 = pPolar1->getPlrVariable(PlrVar);
        if (Alpha < pPolar1->m_Alpha.front())  return pX1.front();
        if (Alpha > pPolar1->m_Alpha.back())   return pX1.back();
        for (int i=0; i<size-1; i++)
        {
            if(pPolar1->m_Alpha.at(i)<=Alpha && Alpha<pPolar1->m_Alpha.at(i+1))
            {
                //interpolate
                if(pPolar1->m_Alpha.at(i+1)-pPolar1->m_Alpha.at(i) < 0.00001){//do not divide by zero
                    return pX1.at(i);
                }
                else
                {
                    u = (Alpha - pPolar1->m_Alpha.at(i))  /(pPolar1->m_Alpha.at(i+1)-pPolar1->m_Alpha.at(i));
                    return pX1.at(i) + u * (pX1.at(i+1)-pX1.at(i));
                }
            }
        }
        //Out in Re, out in alpha...
        return pX1.back();
    }
    else
    {
        // Re is between that of polars 1 and 2
        // so interpolate alphas for each

        if(!pPolar1)
        {
            bOutRe = true;
            bError = true;
            return 0.000;
        }
        size = pPolar1->m_Alpha.size();
        if(!size)
        {
            bOutRe = true;
            bError = true;
            return 0.000;
        }

        QVector<double> const &pX1 = pPolar1->getPlrVariable(PlrVar);
        if(Alpha < pPolar1->m_Alpha.front())     Var1 = pX1.front();
        else if(Alpha > pPolar1->m_Alpha.back()) Var1 = pX1.back();
        else
        {
            for (int i=0; i<size-1; i++)
            {
                if(pPolar1->m_Alpha.at(i) <= Alpha && Alpha < pPolar1->m_Alpha.at(i+1)){
                    //interpolate
                    if(pPolar1->m_Alpha.at(i+1)-pPolar1->m_Alpha.at(i) < 0.00001)//do not divide by zero
                        Var1 = pX1.at(i);
                    else
                    {
                        u = (Alpha - pPolar1->m_Alpha.at(i)) / (pPolar1->m_Alpha.at(i+1)-pPolar1->m_Alpha.at(i));
                        Var1 = pX1.at(i) + u * (pX1.at(i+1)-pX1.at(i));
                    }
                }
            }
        }

        size = pPolar2->m_Alpha.size();
        if(!size)
        {
            bOutRe = true;
            bError = true;
            return 0.000;
        }
        QVector<double> const &pX2 = pPolar2->getPlrVariable(PlrVar);
        if(Alpha < pPolar2->m_Alpha.front())
        {
            bOutRe = true;
            bError = true;
            Var2 = pX2.front();
        }
        else if(Alpha > pPolar2->m_Alpha.back())
        {
            bOutRe = true;
            bError = true;
            Var2 = pX2.back();
        }
        else{
            for (int i=0; i<size-1; i++)
            {
                if(pPolar2->m_Alpha.at(i)<=Alpha && Alpha<pPolar2->m_Alpha.at(i+1))
                {
                    //interpolate
                    QVector<double> const &pX2 = pPolar2->getPlrVariable(PlrVar);
                    if(pPolar2->m_Alpha.at(i+1)-pPolar2->m_Alpha.at(i) < 0.00001)//do not divide by zero
                        Var2 =pX2.at(i);
                    else
                    {
                        u = (Alpha - pPolar2->m_Alpha.at(i)) /(pPolar2->m_Alpha.at(i+1)-pPolar2->m_Alpha.at(i));
                        Var2 = pX2.at(i) + u * (pX2.at(i+1)-pX2.at(i));
                    }
                }
            }
        }
        // then interpolate Variable

        double v = (Re - pPolar1->Reynolds()) / (pPolar2->Reynolds() - pPolar1->Reynolds());
        double Var = Var1 + v * (Var2-Var1);
        return Var;
    }

    //    bOutRe = true;
    //    bError = true;
    //    return 0.000;// we missed something somewhere...
}


/**
 * Returns the coefficient of an approximate linearized Cl=f(aoa) curve, based on the geometrical position of a point between two sections on a wing.
 * @param pFoil0 the pointer to the left foil  of the wing's section.
 * @param pFoil1 the pointer to the left foil  of the wing's section.
 * @param Re the Reynolds number at the point's position.
 * @param Tau the relative position of the point between the two foils.
 * @param Alpha0 the zero-lift angle; if the interpolation fails, returns Alpha0 = 0
 * @param Slope the slope of the lift curve; if the interpolation fails, returns Slope = 2 PI
 */
void LLTAnalysis::getLinearizedPolar(Foil *pFoil0, Foil *pFoil1, double Re, double Tau, double &Alpha0, double &Slope)
{
    double Alpha00=0, Alpha01=0;
    double Slope0=0, Slope1=0;
    double AlphaTemp1=0, AlphaTemp2=0, SlopeTemp1=0, SlopeTemp2=0;

    //Find the two polars which enclose the Reynolds number
    int size = 0;
    Polar *pPolar=nullptr, *pPolar1=nullptr, *pPolar2=nullptr;

    if(!pFoil0)
    {
        Alpha00 = 0.0;
        Slope0 = 2.0 * PI *PI/180.0;
    }
    else
    {
        pPolar1 = nullptr;
        pPolar2 = nullptr;
        for (int i=0; i<m_poaPolar->size(); i++)
        {
            pPolar = m_poaPolar->at(i);
            if(pPolar->foilName() == pFoil0->name()) size++;
        }
        if(size)
        {
            for (int i=0; i<m_poaPolar->size(); i++)
            {
                pPolar = m_poaPolar->at(i);
                if(pPolar->foilName() == pFoil0->name())
                {
                    if(pPolar->Reynolds() < Re) pPolar1 = pPolar;
                }
            }
            for (int i=0; i<m_poaPolar->size(); i++)
            {
                pPolar = m_poaPolar->at(i);
                if(pPolar->foilName() == pFoil0->name())
                {
                    if(pPolar->Reynolds() > Re)
                    {
                        pPolar2 = pPolar;
                        break;
                    }
                }
            }
        }
        if(pPolar1 && pPolar2)
        {
            pPolar1->getLinearizedCl(AlphaTemp1, SlopeTemp1);
            pPolar2->getLinearizedCl(AlphaTemp2, SlopeTemp2);
            Alpha00 = AlphaTemp1 +
                    (AlphaTemp2-AlphaTemp1) * (Re-pPolar1->Reynolds())/(pPolar2->Reynolds()-pPolar1->Reynolds());
            Slope0  = SlopeTemp1 +
                    (SlopeTemp2-SlopeTemp1) * (Re-pPolar1->Reynolds())/(pPolar2->Reynolds()-pPolar1->Reynolds());
        }
        else
        {
            Alpha00 = 0.0;
            Slope0  = 2.0 * PI *PI/180.0;
        }
    }

    if(!pFoil1)
    {
        Alpha01 = 0.0;
        Slope1 = 2.0*PI *PI/180.0;
    }
    else
    {
        pPolar1 = nullptr;
        pPolar2 = nullptr;
        for (int i=0; i<m_poaPolar->size(); i++)
        {
            pPolar = m_poaPolar->at(i);
            if(pPolar->foilName() == pFoil1->name()) size++;
        }
        if(size)
        {
            for (int i=0; i<m_poaPolar->size(); i++)
            {
                pPolar = m_poaPolar->at(i);
                if(pPolar->foilName() == pFoil1->name())
                {
                    if(pPolar->Reynolds() < Re) pPolar1 = pPolar;
                }
            }
            for (int i=0; i<m_poaPolar->size(); i++)
            {
                pPolar = m_poaPolar->at(i);
                if(pPolar->foilName() == pFoil1->name())
                {
                    if(pPolar->Reynolds() > Re)
                    {
                        pPolar2 = pPolar;
                        break;
                    }
                }
            }
        }
        if(pPolar1 && pPolar2)
        {
            pPolar1->getLinearizedCl(AlphaTemp1, SlopeTemp1);
            pPolar2->getLinearizedCl(AlphaTemp2, SlopeTemp2);
            Alpha01 = AlphaTemp1 + (AlphaTemp2-AlphaTemp1) * (Re-pPolar1->Reynolds())/(pPolar2->Reynolds()-pPolar1->Reynolds());
            Slope1  = SlopeTemp1 + (SlopeTemp2-SlopeTemp1) * (Re-pPolar1->Reynolds())/(pPolar2->Reynolds()-pPolar1->Reynolds());
        }
        else
        {
            Alpha01 = 0.0;
            Slope1 = 2.0*PI *PI/180.0;
        }
    }

    Alpha0 = ((1-Tau) * Alpha00 + Tau * Alpha01);
    Slope  = ((1-Tau) * Slope0  + Tau * Slope1);
}

