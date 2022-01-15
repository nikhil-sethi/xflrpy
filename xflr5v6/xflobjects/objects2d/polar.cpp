/****************************************************************************

    Polar Class
    Copyright (C) 2003-2019 André Deperrois

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

#include <QRandomGenerator>

#include "polar.h"
#include <xflcore/constants.h>
#include <xflobjects/objects2d/foil.h>


/**
*The public constructor.
*/
Polar::Polar()
{
    m_theStyle.m_bIsVisible = true;
    m_theStyle.m_Symbol = Line::NOSYMBOL;
    m_theStyle.m_Stipple = Line::SOLID;
    m_theStyle.m_Width = 2;

    m_theStyle.m_Color.setHsv(QRandomGenerator::global()->bounded(360),
                              QRandomGenerator::global()->bounded(55)+30,
                              QRandomGenerator::global()->bounded(55)+150);

    m_ASpec     = 0.0;
    m_PolarType = xfl::FIXEDSPEEDPOLAR;
    m_ReType    = 1;
    m_MaType    = 1;
    m_Reynolds  = 100000.0;
    m_Mach      = 0.0;
    m_NCrit     = 9.0;
    m_XTop      = 1.0;
    m_XBot      = 1.0;
    m_FoilName.clear();
}


/**
 * Exports the data of the polar to a text file
 * @param out the instance of output QtextStream
 * @param FileType TXT if the data is separated by spaces, CSV for a comma separator
 * @param bDataOnly true if the analysis parameters should not be output
 */
void Polar::exportPolar(QTextStream &out, QString versionName, bool bCSV, bool bDataOnly) const
{
    QString Header, strong;

    if(!bDataOnly)
    {
        strong = versionName;
        strong += "\n\n";
        out << strong;
        strong =(" Calculated polar for: ");
        strong += m_FoilName + "\n\n";
        out << strong;

        strong = QString(" %1 %2").arg(m_ReType).arg(m_MaType);

        if(m_ReType==1)      strong += (" Reynolds number fixed       ");
        else if(m_ReType==2) strong += (" Reynolds number ~ 1/sqrt(CL)");
        else if(m_ReType==3) strong += (" Reynolds number ~ 1/CL      ");

        if(m_MaType==1)      strong += ("   Mach number fixed         ");
        else if(m_MaType==2) strong += ("   Mach number ~ 1/sqrt(CL)  ");
        else if(m_MaType==3) strong += ("   Mach number ~ 1/CL        ");
        strong +="\n\n";
        out << strong;

        strong=QString((" xtrf =   %1 (top)        %2 (bottom)\n"))
                        .arg(m_XTop,0,'f',3).arg(m_XBot,0,'f',3);
        out << strong;

        strong = QString(" Mach = %1     Re = %2 e 6     Ncrit = %3\n\n")
                 .arg(m_Mach,7,'f',3).arg(m_Reynolds/1.e6,9,'f',3).arg(m_NCrit,7,'f',3);
        out << strong;
    }

    if(m_PolarType!=xfl::FIXEDAOAPOLAR)
    {
        if(!bCSV) Header = ("  alpha     CL        CD       CDp       Cm    Top Xtr Bot Xtr   Cpmin    Chinge    XCp    \n");
        else      Header = ("alpha,CL,CD,CDp,Cm,Top Xtr,Bot Xtr,Cpmin,Chinge,XCp\n");
        out << Header;
        if(!bCSV)
        {
            Header=QString(" ------- -------- --------- --------- -------- ------- ------- -------- --------- ---------\n");
            out << Header;
        }
        for (int j=0; j<m_Alpha.size(); j++)
        {
            if(!bCSV) strong = QString(" %1  %2  %3  %4  %5")
                                            .arg(m_Alpha[j],7,'f',3)
                                            .arg(m_Cl[j],7,'f',4)
                                            .arg(m_Cd[j],8,'f',5)
                                            .arg(m_Cdp[j],8,'f',5)
                                            .arg(m_Cm[j],7,'f',4);
            else      strong = QString("%1,%2,%3,%4,%5")
                                            .arg(m_Alpha[j],7,'f',3)
                                            .arg(m_Cl[j],7,'f',4)
                                            .arg(m_Cd[j],8,'f',5)
                                            .arg(m_Cdp[j],8,'f',5)
                                            .arg(m_Cm[j],7,'f',4);

            out << strong;
            if(m_XTr1[j]<990.0)
            {
                if(!bCSV) strong=QString("  %1  %2").arg(m_XTr1[j],6,'f',4).arg( m_XTr2[j],6,'f',4);
                else      strong=QString(",%1,%2").arg(m_XTr1[j],6,'f',4).arg( m_XTr2[j],6,'f',4);
                out << strong;
            }
            if(!bCSV) strong=QString("  %1  %2  %3\n").arg(m_Cpmn[j],7,'f',4).arg(m_HMom[j],7,'f',4).arg(m_XCp[j],7,'f',4);
            else      strong=QString(",%1,%2,%3\n").arg(m_Cpmn[j],7,'f',4).arg(m_HMom[j],7,'f',4).arg(m_XCp[j],7,'f',4);
            out << strong;
            }
    }
    else 
    {
        if(!bCSV) Header=QString(("  alpha     Re      CL        CD       CDp       Cm    Top Xtr Bot Xtr   Cpmin    Chinge     XCp    \n"));
        else      Header=QString(("alpha,Re,CL,CD,CDp,Cm,Top Xtr,Bot Xtr,Cpmin,Chinge,XCp\n"));
        out << Header;
        if(!bCSV)
        {
            Header=QString(" ------- -------- -------- --------- --------- -------- ------- ------- -------- --------- ---------\n");
            out << Header;
        }
        for (int j=0; j<m_Alpha.size(); j++)
        {
            if(!bCSV) strong=QString(" %1 %2  %3  %4  %5  %6")
                                            .arg(m_Alpha[j],7,'f',3)
                                            .arg( m_Re[j],8,'f',0)
                                            .arg( m_Cl[j],7,'f',4)
                                            .arg( m_Cd[j],8,'f',5)
                                            .arg(m_Cdp[j],8,'f',5)
                                            .arg(m_Cm[j],7,'f',4);
            else      strong=QString(" %1,%2,%3,%4,%5,%6")
                                            .arg(m_Alpha[j],7,'f',3)
                                            .arg( m_Re[j],8,'f',0)
                                            .arg( m_Cl[j],7,'f',4)
                                            .arg( m_Cd[j],8,'f',5)
                                            .arg(m_Cdp[j],8,'f',5)
                                            .arg(m_Cm[j],7,'f',4);
            out << strong;
            if(m_XTr1[j]<990.0)
            {
                if(!bCSV) strong=QString("  %1  %2").arg(m_XTr1[j],6,'f',4).arg(m_XTr2[j],6,'f',4);
                else      strong=QString(",%1,%2").arg(m_XTr1[j],6,'f',4).arg(m_XTr2[j],6,'f',4);
                out << strong;
            }
            if(!bCSV) strong=QString("  %1  %2  %3\n").arg(m_Cpmn[j],7,'f',4).arg(m_HMom[j],7,'f',4).arg(m_XCp[j],7,'f',4);
            else      strong=QString(",%1,%2,%3\n").arg(m_Cpmn[j],7,'f',4).arg(m_HMom[j],7,'f',4).arg(m_XCp[j],7,'f',4);
            out << strong;
        }
    }
    out << "\n\n";
}



/**
 * Remove all data from the polar object
 */
void Polar::resetPolar()
{
    m_Alpha.clear();
    m_Cl.clear();
    m_Cd.clear();
    m_Cdp.clear();
    m_Cm.clear();
    m_XTr1.clear();
    m_XTr2.clear();
    m_HMom.clear();
    m_Cpmn.clear();
    m_ClCd.clear();
    m_RtCl.clear();
    m_Cl32Cd.clear();
    m_Re.clear();
    m_XCp.clear();
}



/**
 * Adds the data from the instance of the operating point referenced by pOpPoint to the polar object.
 * The index used to insert the data is the aoa for type 1, 2 and 3 polars, and the freestream velocity for type 4 polars.
 * If a point with identical index exists, the data is replaced.
 * If not, the data is inserted for this index.
 *
 * @param *pPOpPoint a pointer to the foil's operating point from which the data is to be extracted
 */
void Polar::addOpPointData(const OpPoint *pOpPoint)
{
    if(!pOpPoint->m_bViscResults) return;

    bool bInserted = false;
    int size = m_Alpha.size();

    if(size)
    {
        for (int i=0; i<size; i++)
        {
            if(m_PolarType<xfl::FIXEDAOAPOLAR)
            {
                if (qAbs(pOpPoint->aoa()-m_Alpha[i]) < 0.001)
                {
                    replaceOppDataAt(i, pOpPoint);
                    bInserted = true;
                    break;
                }
                else if (pOpPoint->aoa() < m_Alpha[i])
                {
                    insertOppDataAt(i, pOpPoint);
                    bInserted = true;
                    break;
                }
            }
            else if(m_PolarType==xfl::FIXEDAOAPOLAR)
            {
                // type 4, sort by speed
                if (qAbs(pOpPoint->Reynolds() - m_Re[i]) < 0.1)
                {
                    // then erase former result
                    replaceOppDataAt(i, pOpPoint);
                    bInserted = true;
                    break;
                }
                else if (pOpPoint->Reynolds() < m_Re[i])
                {
                    // sort by crescending speed
                    insertOppDataAt(i, pOpPoint);
                    bInserted = true;
                    break;
                }
            }

        }
    }

    if(!bInserted)
    {
        // data is appended at the end
        int size = m_Alpha.size();
        insertOppDataAt(size, pOpPoint);
    }
}


void Polar::replaceOppDataAt(int pos, OpPoint const*pOpp)
{
    if(pos<0 || pos>= m_Alpha.size()) return;

    m_Alpha[pos] =  pOpp->aoa();
    m_Cd[pos]    =  pOpp->Cd;
    m_Cdp[pos]   =  pOpp->Cdp;
    m_Cl[pos]    =  pOpp->Cl;
    m_Cm[pos]    =  pOpp->Cm;
    m_XTr1[pos]  =  pOpp->Xtr1;
    m_XTr2[pos]  =  pOpp->Xtr2;
    m_HMom[pos]  =  pOpp->m_TEHMom;
    m_Cpmn[pos]  =  pOpp->Cpmn;
    m_ClCd[pos]  =  pOpp->Cl/pOpp->Cd;
    m_XCp[pos]   =  pOpp->m_XCP;

//  Bug  if(pOpp->Cl>0.0) m_RtCl[pos] = 1.0/sqrt(pOpp->Cl);
    if(pOpp->Cl>0.0) m_RtCl[pos] = sqrt(pOpp->Cl);
    else             m_RtCl[pos] = 0.0;
    if (pOpp->Cl>=0.0) m_Cl32Cd[pos] =  pow( pOpp->Cl, 1.5)/ pOpp->Cd;
    else               m_Cl32Cd[pos] = -pow(-pOpp->Cl, 1.5)/ pOpp->Cd;

    if(m_PolarType==xfl::FIXEDSPEEDPOLAR)  m_Re[pos] =  pOpp->Reynolds();
    else if (m_PolarType==xfl::FIXEDLIFTPOLAR)
    {
//    Bug    if(pOpp->Cl>0.0) m_Re[pos] =  pOpp->Reynolds()/ sqrt(pOpp->Cl);
        if(pOpp->Cl>0.0) m_Re[pos] =  pOpp->Reynolds();
        else             m_Re[pos] = 0.0;
    }
    else if (m_PolarType==xfl::RUBBERCHORDPOLAR)
    {
        if(pOpp->Cl>0.0) m_Re[pos] =  pOpp->Reynolds()/(pOpp->Cl);
        else             m_Re[pos] = 0.0;
    }

}


void Polar::insertOppDataAt(int i, const OpPoint *pOpp)
{
    m_Alpha.insert(i, pOpp->aoa());
    m_Cd.insert(   i, pOpp->Cd);
    m_Cdp.insert(  i, pOpp->Cdp);
    m_Cl.insert(   i, pOpp->Cl);
    m_Cm.insert(   i, pOpp->Cm);
    m_XTr1.insert( i, pOpp->Xtr1);
    m_XTr2.insert( i, pOpp->Xtr2);
    m_HMom.insert( i, pOpp->m_TEHMom);
    m_Cpmn.insert( i, pOpp->Cpmn);
    m_ClCd.insert( i, pOpp->Cl/pOpp->Cd);
    m_XCp.insert(  i, pOpp->m_XCP);

//  Bug  if(pOpp->Cl>0.0) m_RtCl.insert(i, 1.0/sqrt(pOpp->Cl));
    if(pOpp->Cl>0.0) m_RtCl.insert(i, sqrt(pOpp->Cl));
    else             m_RtCl.insert(i, 0.0);

    if (pOpp->Cl>=0.0) m_Cl32Cd.insert(i,pow( pOpp->Cl, 1.5) / pOpp->Cd);
    else               m_Cl32Cd.insert(i,-pow(-pOpp->Cl, 1.5)/ pOpp->Cd);

    if(m_PolarType==xfl::FIXEDSPEEDPOLAR)     m_Re.insert(i, pOpp->Reynolds());
    else if (m_PolarType==xfl::FIXEDLIFTPOLAR)
    {
//      Bug  if(pOpp->Cl>0) m_Re.insert(i, pOpp->Reynolds()/sqrt(pOpp->Cl));
        if(pOpp->Cl>0.0) m_Re.insert(i, pOpp->Reynolds());
//      Bug  else           m_Re[i] = 0.0;   -> exception when i doesn't exist in array
        else             m_Re.insert(i, 0.0);
    }
    else if (m_PolarType==xfl::RUBBERCHORDPOLAR)
    {
        if(pOpp->Cl>0.0) m_Re.insert(i, pOpp->Reynolds()/pOpp->Cl);
        else             m_Re.insert(i, 0.0);
    }
    else if (m_PolarType==xfl::FIXEDAOAPOLAR)
    {
        m_Re.insert(i, pOpp->Reynolds());
    }
}



/**
 * Adds the parameter data to the data arrays
 * The index used to insert the data is the aoa for type 1, 2 and 3 polars, and the freestream velocity for type 4 polars.
 * If a point with identical index exists, the data is replaced.
 * If not, the data is inserted for this index.
 */
void Polar::addPoint(double Alpha, double Cd, double Cdp, double Cl, double Cm, double Xtr1,
                     double Xtr2, double HMom, double Cpmn, double Reynolds, double XCp)
{
    OpPoint *pOpp = new OpPoint;
    pOpp->m_bViscResults = true;
    pOpp->m_Alpha    = Alpha;
    pOpp->Cd         = Cd;
    pOpp->Cdp        = Cdp;
    pOpp->Cl         = Cl;
    pOpp->Cm         = Cm;
    pOpp->Xtr1       = Xtr1;
    pOpp->Xtr2       = Xtr2;
    pOpp->m_TEHMom   = HMom;
    pOpp->Cpmn       = Cpmn;
    pOpp->m_Reynolds = Reynolds;
    pOpp->m_XCP      = XCp;

    addOpPointData(pOpp);
    delete pOpp;

}


/**
 * Copies the polar's data from an existing polar
 * @param pPolar a pointer to the instance of the reference Polar object from which the data should be copied
 */
void Polar::copyPolar(const Polar *pPolar)
{
    copySpecification(pPolar);

    int size  = m_Alpha.size();
    for(int i=size-1; i>=0; i--)
        removePoint(i);

    size  = pPolar->m_Alpha.size();
    for(int i=0; i<size; i++)
    {
        m_Alpha.insert(i,  pPolar->m_Alpha[i]);
        m_Cd.insert(i,     pPolar->m_Cd[i]);
        m_Cdp.insert(i,    pPolar->m_Cdp[i]);
        m_Cl.insert(i,     pPolar-> m_Cl[i]);
        m_Cm.insert(i,     pPolar->m_Cm[i]);
        m_XTr1.insert(i,   pPolar->m_XTr1[i]);
        m_XTr2.insert(i,   pPolar->m_XTr2[i]);
        m_HMom.insert(i,   pPolar->m_HMom[i]);
        m_Cpmn.insert(i,   pPolar->m_Cpmn[i]);
        m_ClCd.insert(i,   pPolar->m_ClCd[i]);
        m_RtCl.insert(i,   pPolar->m_RtCl[i]);
        m_Cl32Cd.insert(i, pPolar->m_Cl32Cd[i]);
        m_Re.insert(i,     pPolar->m_Re[i]);
        m_XCp.insert(i,    pPolar->m_XCp[i]);
    }
}


/**
 * Copies the polar's data from an existing polar
 * @param pPolar a pointer to the instance of the reference Polar object from which the data should be copied
 */
void Polar::copySpecification(const Polar *pPolar)
{
    m_PolarType = pPolar->m_PolarType;
    m_ReType    = pPolar->m_ReType;
    m_MaType    = pPolar->m_MaType;
    m_Reynolds  = pPolar->m_Reynolds;
    m_ASpec     = pPolar->m_ASpec;
    m_Mach      = pPolar->m_Mach;
    m_NCrit     = pPolar->m_NCrit;
    m_XTop      = pPolar->m_XTop;
    m_XBot      = pPolar->m_XBot;
}


/**
 * Removes the data for the point at a given index of the data arrays
 * @param i the index of the point to be removed
 **/
void Polar::removePoint(int i)
{
    m_Alpha.removeAt(i);
    m_Cl.removeAt(i);
    m_Cd.removeAt(i);
    m_Cdp.removeAt(i);
    m_Cm.removeAt(i);
    m_XTr1.removeAt(i);
    m_XTr2.removeAt(i);
    m_HMom.removeAt(i);
    m_Cpmn.removeAt(i);
    m_ClCd.removeAt(i);
    m_RtCl.removeAt(i);
    m_Cl32Cd.removeAt(i);
    m_Re.removeAt(i);
    m_XCp.removeAt(i);
}


/**
* Returns the minimum and maximum angles of attack stored in the polar.
* Since the data is sorted by crescending aoa, this is a matter of returning the first and last values of the array.
*@param &amin the miminum aoa
*@param &amax the maximum aoa
*/
void Polar::getAlphaLimits(double &amin, double &amax) const
{
    if(!m_Alpha.size())
    {
        amin = 0.0;
        amax = 0.0;
    }
    else
    {
        amin = m_Alpha[0];
        amax = m_Alpha[m_Alpha.size()-1];
    }
}


/**
* Returns the minimum and maximum lift coefficients stored in the polar.
*@param &Clmin the miminum lift coefficient
*@param &Clmax the maximum lift coefficient
*/
void Polar::getClLimits(double &Clmin, double &Clmax) const
{
    if(!m_Cl.size())
    {
        Clmin = 0.0;
        Clmax = 0.0;
    }
    else
    {
        Clmin = 10000.0;
        Clmax =-10000.0;
        double Cl;
        for (int i=0;i<m_Cl.size(); i++)
        {
            Cl = m_Cl[i];
            if(Clmin>Cl) Clmin = Cl;
            if(Clmax<Cl) Clmax = Cl;
        }
    }
}

/**
* Returns the moment coefficient at zero-lift.
* Cm0 is interpolated between the two points in the array such that Cl[i]<0 and Cl[i+1]>0.
* If no such pair is found, the method returns 0.
*@return Cm0
*/
double Polar::getCm0() const
{
    int i;
    double Clmin =  1000.0;
    double Clmax = -1000.0;
    for (i=0; i<m_Cl.size(); i++)
    {
        Clmin = qMin(Clmin, m_Cl[i]);
        Clmax = qMax(Clmax, m_Cl[i]);
    }
    if(!(Clmin<0.0) || !(Clmax>0.0)) return 0.0;
    int k=0;
//    double rr  = m_Cl[k];
//    double rr1 = m_Cl[k+1];

    while (m_Cl[k+1]<0.0)
    {
//        rr  = m_Cl[k];
//        rr1 = m_Cl[k+1];
        k++;
    }
    if(k+1>=m_Cm.size()) return 0.0;
    double Cm0 = m_Cm[k] + (m_Cm[k+1]-m_Cm[k])*(0.0-m_Cl[k])/(m_Cl[k+1]-m_Cl[k]);
    return Cm0;

}


/**
* Returns the value of the aoa such that Cl=0.
* The zero lift angle is interpolated between the two points in the array such that Cl[i]<0 and Cl[i+1]>0.
* If no such pair is found, the method returns 0.
*@return Cm0
*/
double Polar::getZeroLiftAngle() const
{
    double Clmin =  1000.0;
    double Clmax = -1000.0;
    for (int i=0; i<m_Cl.size(); i++)
    {
        Clmin = qMin(Clmin, m_Cl[i]);
        Clmax = qMax(Clmax, m_Cl[i]);
    }
    if(!(Clmin<0.0) || !(Clmax>0.0)) return 0.0;
    int k=0;
//    double rr  = m_Cl[k];
//    double rr1 = m_Cl[k+1];

    while (m_Cl[k+1]<0.0)
    {
//        rr  = m_Cl[k];
//        rr1 = m_Cl[k+1];
        k++;
    }
    if(k+1>=m_Alpha.size()) return 0.0;
    double Alpha0 = m_Alpha[k] + (m_Alpha[k+1]-m_Alpha[k])*(0.0-m_Cl[k])/(m_Cl[k+1]-m_Cl[k]);
    return Alpha0;

}


/**
* Linearizes Cl vs. Alpha set of points by least square method
* @param Alpha0 the zero-lift angle, i.e.such that Cl = 0, in degrees
* @param slope the slope of the curve Cl=f(aoa), in units 1/°
*/
void Polar::getLinearizedCl(double &Alpha0, double &slope) const
{
    int n = m_Cl.size();

    if(n<=1)
    {
        Alpha0 = 0.0;
        slope = 2.0*PI*PI/180.0;
        return;
    }

    double fn = double(n);
    double sum1 = 0.0;
    double sum2 = 0.0;
    double sum3 = 0.0;
    double sum4 = 0.0;
    double b1, b2;

    for (int k=0; k<n; k++)
    {
        sum1 += m_Cl[k] * m_Alpha[k];
        sum2 += m_Alpha[k];
        sum3 += m_Cl[k];
        sum4 += m_Alpha[k] * m_Alpha[k];
    }
    if(fabs(fn*sum4-sum2*sum2)<1.e-10 || fabs(fn*sum1-sum2 * sum3)<1.0e-10)
    {
        //very improbable...
        Alpha0 = 0.0;
        slope = 2.0*PI*PI/180.0;
        return;
    }

    b1 = (fn*sum1 - sum2 * sum3)/(fn*sum4 - sum2*sum2);
    b2 = (sum3 - b1 * sum2)/fn;

    slope  = b1; //in cl/°
    Alpha0 = -b2/b1;
}



/**
* Returns the name of the variable for a given index
*@param iVar the index of the variable
*@param &Name the reference of the QString object to be filled with the variable's name.
*/
QString Polar::variableName(int iVar)
{
    switch (iVar)
    {
        case 0:
            return "Alpha";
        case 1:
            return "Cl";
        case 2:
            return "Cd";
        case 3:
            return "Cd x 10000";
        case 4:
            return "Cdp";
        case 5:
            return "Cm";
        case 6:
            return "Xtr top";
        case 7:
            return "Xtr bot";
        case 8:
            return "HMom";
        case 9:
            return "Cpmin";
        case 10:
            return "Cl/Cd";
        case 11:
            return "|Cl|^(3/2)/Cd";
        case 12:
            return "1/Rt(Cl)";
        case 13:
            return "Re";
        case 14:
            return "XCp";
        default:
            return "Alpha";
    }
}


void Polar::setPolarType(xfl::enumPolarType type)
{
    m_PolarType=type;
    switch (m_PolarType)
    {
        case xfl::FIXEDSPEEDPOLAR:
            m_MaType = 1;
            m_ReType = 1;
            break;
        case xfl::FIXEDLIFTPOLAR:
            m_MaType = 2;
            m_ReType = 2;
            break;
        case xfl::RUBBERCHORDPOLAR:
            m_MaType = 1;
            m_ReType = 3;
            break;
        case xfl::FIXEDAOAPOLAR:
            m_MaType = 1;
            m_ReType = 1;
            break;
        default:
            m_ReType = 1;
            m_MaType = 1;
            break;
    }
}


void Polar::setAutoPolarName()
{
    m_Name = autoPolarName(m_PolarType, m_Reynolds, m_Mach, m_NCrit, m_ASpec, m_XTop, m_XBot);
}



/**
 * Static polar name creation
 * @param polarType
 * @param Re
 * @param Mach
 * @param NCrit
 * @param ASpec
 * @return
 */
QString Polar::autoPolarName(xfl::enumPolarType polarType, double Re, double Mach, double NCrit, double ASpec, double XTop, double XBot)
{
    QString polarName;
    Re = Re/1.e6;
    switch(polarType)
    {
        case xfl::FIXEDSPEEDPOLAR:
        {
            polarName = QString("T1_Re%1_M%2").arg(Re,5,'f',3).arg(Mach,4,'f',2);
            break;
        }
        case xfl::FIXEDLIFTPOLAR:
        {
            polarName = QString("T2_Re%1_M%2").arg(Re,5,'f',3).arg(Mach,4,'f',2);
            break;
        }
        case xfl::RUBBERCHORDPOLAR:
        {
            polarName = QString("T3_Re%1_M%2").arg(Re,5,'f',3).arg(Mach,4,'f',2);
            break;
        }
        case(xfl::FIXEDAOAPOLAR):
        {
            polarName = QString("T4_Al%1_M%2").arg(ASpec,5,'f',2).arg(Mach,4,'f',2);
            break;
        }
        default:
        {
            polarName = QString("T1_Re%1_M%2").arg(Re,5,'f',3).arg(Mach,4,'f',2);
            break;
        }
    }


    QString str = QString("_N%1").arg(NCrit,3,'f',1);
    polarName += str;

    if(XTop<1.0-0.001)
    {
        str = QString("_XtrTop%1%").arg(XTop*100.0,2,'f',0);
        polarName += str;
    }
    if(XBot<1.0-0.001)
    {
        str = QString("_XtrBot%1%").arg(XBot*100.0,2,'f',0);
        polarName += str;
    }

    return polarName;
}


bool Polar::hasOpp(OpPoint const *pOpp) const
{
    return (pOpp->foilName()==m_FoilName && pOpp->polarName()==m_Name);
}

QString Polar::properties() const
{
    QString props;
    getProperties(props);
    return props;
}


/**
 * Returns a QString object holding the description and value of the polar's parameters
 * @param &PolarProperties the reference of the QString object to be filled with the description
 * @param bData true if the analysis data should be appended to the string
 */
void Polar::getProperties(QString &polarProps) const
{
    QString strong;
    polarProps = m_Name +"\n\n";

//    PolarProperties += QObject::tr("Parent foil")+" = "+ m_FoilName+"\n";

//    strong = QString(QObject::tr("Analysis Type")+" = %1\n").arg(m_PolarType);
    polarProps.clear();

    int iPolarNumber = 0;
    if (m_PolarType==xfl::FIXEDSPEEDPOLAR)     iPolarNumber = 1;
    else if (m_PolarType==xfl::FIXEDLIFTPOLAR) iPolarNumber = 2;
    else if (m_PolarType==xfl::FIXEDAOAPOLAR)  iPolarNumber = 4;
    else if (m_PolarType==xfl::STABILITYPOLAR) iPolarNumber = 7;
    else if (m_PolarType==xfl::BETAPOLAR)      iPolarNumber = 5;
    strong = QString(QObject::tr("Type")+" = %1").arg(iPolarNumber);

    if(m_PolarType==xfl::FIXEDSPEEDPOLAR)      strong += " ("+QObject::tr("Fixed speed") +")\n";
    else if(m_PolarType==xfl::FIXEDLIFTPOLAR) strong += " ("+QObject::tr("Fixed lift") +")\n";
    else if(m_PolarType==xfl::FIXEDAOAPOLAR) strong += " ("+QObject::tr("Fixed angle of attack") +")\n";
    polarProps += strong;

    if(m_PolarType==xfl::FIXEDSPEEDPOLAR)
    {
        strong = QString(QObject::tr("Reynolds number")+" = %L1\n").arg(m_Reynolds,0,'f',0);
        polarProps += strong;
        strong = QString(QObject::tr("Mach number") + " = %L1\n").arg(m_Mach,5,'f',2);
        polarProps += strong;
    }
    else if(m_PolarType==xfl::FIXEDLIFTPOLAR)
    {
        strong = QString("Re.sqrt(Cl) = %L1\n").arg(m_Reynolds,0,'f',0);
        polarProps += strong;
        strong = QString("Ma.sqrt(Cl) = %L1\n").arg(m_Mach,5,'f',2);
        polarProps += strong;
    }
    else if(m_PolarType==xfl::RUBBERCHORDPOLAR)
    {
        strong = QString(QObject::tr("Re.Cl")+" = %L1\n").arg(m_Reynolds,0,'f',0);
        polarProps += strong;
        strong = QString(QObject::tr("Mach number") + " = %L1\n").arg(m_Mach,5,'f',2);
        polarProps += strong;
    }
    else if(m_PolarType==xfl::FIXEDAOAPOLAR)
    {
        strong = QString(QObject::tr("Alpha")+" = %L1"+QChar(0260)+"\n").arg(m_ASpec,7,'f',2);
        polarProps += strong;
        strong = QString(QObject::tr("Mach number") + " = %L1\n").arg(m_Mach,5,'f',2);
        polarProps += strong;
    }


    strong = QString(QObject::tr("NCrit") + " = %L1\n").arg(m_NCrit,6,'f',2);
    polarProps += strong;

    strong = QString(QObject::tr("Forced top trans.   ") + " = %L1\n").arg(m_XTop,6,'f',2);
    polarProps += strong;

    strong = QString(QObject::tr("Forced bottom trans.") + " = %L1\n").arg(m_XBot,6,'f',2);
    polarProps += strong;

    strong = QString(QObject::tr("Number of data points") +" = %L1").arg(m_Alpha    .size());
    polarProps += "\n" +strong;
}





/**
* Returns a pointer to a variable array of a polar object, based on the variable's index
* @param pPolar the pointer to the polar object
* @param iVar the index of the variable
* @return the pointer to the array holding the values of the variable
*/
QVector<double> const & Polar::getPlrVariable(int iVar) const
{
    switch (iVar)
    {
        case 0:
            return m_Alpha;
        case 1:
            return m_Cl;
        case 2:
            return m_Cd;
        case 3:
            return m_Cdp;
        case 4:
            return m_Cm;
        case 5:
            return m_XTr1;
        case 6:
            return m_XTr2;
        case 7:
            return m_HMom;
        case 8:
            return m_Cpmn;
        case 9:
            return m_ClCd;
        case 10:
            return m_Cl32Cd;
        case 11:
            return m_XCp;
        default:
            return m_Alpha;
    }
}


