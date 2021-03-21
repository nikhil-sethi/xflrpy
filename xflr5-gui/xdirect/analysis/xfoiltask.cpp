/****************************************************************************

    XFoilTask Class
       Copyright (C) 2011-2017 Andre Deperrois

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
#include "xfoiltask.h"
#include "xfoiltaskevent.h"


#include <QThread>
#include <QCoreApplication>
#include <QtDebug>

#define PI 3.141592654

int XFoilTask::s_IterLim=100;
bool XFoilTask::s_bAutoInitBL = true;
bool XFoilTask::s_bCancel = false;
bool XFoilTask::s_bSkipOpp = false;
bool XFoilTask::s_bSkipPolar = false;

/**
* The public constructor
*/
XFoilTask::XFoilTask(void *pParent)
{
    setAutoDelete(true);
    m_pParent = pParent;
    m_pFoil  = nullptr;
    m_pPolar = nullptr;
    m_bIsFinished = true;

    m_AlphaMin = m_AlphaMax = m_AlphaInc = 0.0;
    m_ClMin    = m_ClMax    = m_ClInc    = 0.0;
    m_ReMin    = m_ReMax    = m_ReInc    = 0.0;

    m_bAlpha    = true;
    m_bFromZero = false;
    m_bInitBL   = true;
    m_bStoreOpp = false;

    m_OutMessage.clear();
    m_OutStream.setDevice(NULL);

    m_bErrors = false;
    m_x0 = m_x1 = m_y0 = m_y1 = nullptr;
}


/**
* Implements the run method of the QRunnable virtual base method
*
* Asssumes that XFoil has been initialized with Foil and Polar
*/
void XFoilTask::run()
{

    if(s_bCancel || !m_pPolar || !m_pFoil)
    {
        m_bIsFinished = true;
        return;
    }

    if(m_pPolar->polarType()!=XFLR5::FIXEDAOAPOLAR) alphaSequence();
    else                                            ReSequence();

    m_bIsFinished = true;

    // For multithreaded analysis, post an event to notify parent window that the task is done
    if(m_pParent)
        qApp->postEvent((QObject*)m_pParent, new XFoilTaskEvent(m_pFoil, m_pPolar));
}

/**
* Initializes the XFoil calculation
* @param pFoil a pointer to the instance of the Foil object for which the calculation is run
* @param pPolar a pointer to the instance of the Polar object for which the calculation is run
* @return true if the initialization of the Foil in XFoil has been sucessful, false otherwise
*/
bool XFoilTask::initializeTask(FoilAnalysis *pFoilAnalysis, bool bStoreOpp, bool bViscous, bool bInitBL, bool bFromZero)
{
    return initializeTask(pFoilAnalysis->pFoil, pFoilAnalysis->pPolar, bStoreOpp, bViscous, bInitBL, bFromZero);
}

/**
* Initializes the XFoil calculation
* @param pFoil a pointer to the instance of the Foil object for which the calculation is run
* @param pPolar a pointer to the instance of the Polar object for which the calculation is run
* @return true if the initialization of the Foil in XFoil has been sucessful, false otherwise
*/
bool XFoilTask::initializeTask(Foil *pFoil, Polar *pPolar, bool bStoreOpp, bool bViscous, bool bInitBL, bool bFromZero)
{
    s_bCancel = false;
    s_bSkipOpp = s_bSkipPolar = false;
    m_bStoreOpp = bStoreOpp;

    XFoil::setCancel(false);
    m_bErrors = false;
    m_pFoil = pFoil;
    m_pPolar = pPolar;

    m_bInitBL = bInitBL;
    m_bFromZero = bFromZero;

    m_bIsFinished = false;

    m_XFoilStream.setString(&m_XFoilLog);
    if(!m_XFoilInstance.initXFoilGeometry(m_pFoil->n, m_pFoil->x,m_pFoil->y, m_pFoil->nx, m_pFoil->ny))  return false;
    if(!m_XFoilInstance.initXFoilAnalysis(m_pPolar->Reynolds(), m_pPolar->aoa(), m_pPolar->Mach(),
                                          m_pPolar->NCrit(), m_pPolar->XtrTop(), m_pPolar->XtrBot(),
                                          m_pPolar->ReType(), m_pPolar->MaType(),
                                          bViscous, m_XFoilStream)) return false;

    return true;
}

/** 
 * Sets the range of aoa or Cl parameters to analyze
 * @param bAlpha true if the input parameter is a range of aoa, false if a range of lift coefficients
 * @param SpMin the minimum value of the range to analyze
 * @param SpMax the maximum value of the range to analyze
 * @param SpInc the increment value for the parameter
 */
void XFoilTask::setSequence(double bAlpha, double SpMin, double SpMax, double SpInc)
{
    m_bAlpha = bAlpha;
    if(bAlpha)
    {
        m_AlphaMin = SpMin;
        m_AlphaMax = SpMax;
        m_AlphaInc = SpInc;
    }
    else
    {
        m_ClMin = SpMin;
        m_ClMax = SpMax;
        m_ClInc = SpInc;
    }
}

/** 
 * Maps the range of Reynolds numbers to analyze to the member variables
 * @param ReMin the minimum value of the range to analyze
 * @param ReMax the maximum value of the range to analyze
 * @param ReInc the increment value for the Reynolds number
 */
void XFoilTask::setReRange(double ReMin, double ReMax, double ReInc)
{
    m_ReMin = ReMin;
    m_ReMax = ReMax;
    m_ReInc = ReInc;
}


/** 
* Performs a sequence of XFoil calculations for a range of aoa or lift coefficients
* @return true if the calculation was successful
*/
bool XFoilTask::alphaSequence()
{
    QString str;

    double alphadeg;
    int ia, iSeries, total, MaxSeries;
    double SpMin, SpMax, SpInc;

    MaxSeries = 1;
    if(m_bAlpha)
    {
        SpMin = m_AlphaMin;
        SpMax = m_AlphaMax;
        SpInc = qAbs(m_AlphaInc);
        if (m_bFromZero && SpMin*SpMax<0)
        {
            MaxSeries = 2;
            SpMin = 0.0;
            //            SpMax = SpMax;
        }
    }
    else
    {
        SpMin = m_ClMin;
        SpMax = m_ClMax;
        SpInc = qAbs(m_ClInc);
    }

    if(SpMin > SpMax) SpInc = -qAbs(SpInc);

    for (iSeries=0; iSeries<MaxSeries; iSeries++)
    {
        if(s_bCancel) break;

        qApp->processEvents();

        total = (int)qAbs((SpMax*1.0001-SpMin)/SpInc);//*1.0001 to make sure upper limit is included

        if(m_bInitBL)
        {
            m_XFoilInstance.setBLInitialized(false);
            m_XFoilInstance.lipan = false;
        }

        for (ia=0; ia<=total; ia++)
        {
            if(s_bCancel) break;
            if(s_bSkipPolar)
            {

                m_XFoilInstance.setBLInitialized(false);
                m_XFoilInstance.lipan = false;
                s_bSkipPolar = false;
                traceLog("    .......skipping polar \n");
                return false;
            }

            if(m_bAlpha)
            {
                alphadeg = SpMin+ia*SpInc;

                m_XFoilInstance.setAlpha(alphadeg * PI/180.0);
                m_XFoilInstance.lalfa = true;
                m_XFoilInstance.setQInf(1.0);
                str = QString("Alpha = %1").arg(alphadeg,9,'f',3);
                traceLog(str);


                // here we go !
                if (!m_XFoilInstance.specal())
                {
                    str = QObject::tr("Invalid Analysis Settings\nCpCalc: local speed too large\n Compressibility corrections invalid ");
                    traceLog(str);
                    m_bErrors = true;
                    return false;
                }
            }
            else
            {
                m_XFoilInstance.lalfa = false;
                m_XFoilInstance.setAlpha(0.0);
                m_XFoilInstance.setQInf(1.0);
                m_XFoilInstance.setClSpec(SpMin+ia*SpInc);
                str = QString(QObject::tr("Cl = %1")).arg(m_XFoilInstance.ClSpec(),9,'f',3);
                traceLog(str);
                if(!m_XFoilInstance.speccl())
                {
                    str = QObject::tr("Invalid Analysis Settings\nCpCalc: local speed too large\n Compressibility corrections invalid ");
                    traceLog(str);
                    m_bErrors = true;
                    return false;
                }
            }

            m_XFoilInstance.lwake = false;
            m_XFoilInstance.lvconv = false;

            m_Iterations = 0;

            while(!iterate()){}

            if(m_XFoilInstance.lvconv)
            {
                str = QString(QObject::tr("   ...converged after %1 iterations\n")).arg(m_Iterations);
                traceLog(str);
            }
            else
            {
                str = QString(QObject::tr("   ...unconverged after %1 iterations\n")).arg(m_Iterations);
                traceLog(str);
                m_bErrors = true;
            }

            if(m_pParent)
            {
                OpPoint *pOpPoint = new OpPoint;
                addXFoilData(pOpPoint, &m_XFoilInstance, m_pFoil);
                qApp->postEvent((QObject*)m_pParent, new XFoilOppEvent(m_pFoil, m_pPolar, pOpPoint));
            }

            if(XFoil::fullReport())
            {
                m_XFoilStream.flush();
                traceLog(m_XFoilLog);
                m_XFoilLog.clear();
            }


            if(m_x0) m_x0->clear();
            if(m_x1) m_x1->clear();
            if(m_y0) m_y0->clear();
            if(m_y1) m_y1->clear();

        }// end Alpha or Cl loop

        SpMin = 0.0;
        SpMax = m_AlphaMin;
        SpInc = -SpInc;
    }
    //        strong+="\n";
    return true;
}




/** 
* Performs a sequence of XFoil calculations for a range of Reynolds numbers.
* @return true if the calculation was successful
*/
bool XFoilTask::ReSequence()
{
    QString str;
    int ia;
    double Re;

    if(m_ReMax< m_ReMin) m_ReInc = -qAbs(m_ReInc);

    int total=int((m_ReMax*1.0001-m_ReMin)/m_ReInc);//*1.0001 to make sure upper limit is included

    total = abs(total);

    QString strange;

    for (ia=0; ia<=total; ia++)
    {
        if(s_bCancel) break;
        if(s_bSkipPolar)
        {
            m_XFoilInstance.setBLInitialized(false);
            m_XFoilInstance.lipan = false;
            s_bSkipPolar = false;
            traceLog("    .......skipping polar \n");
            return false;
        }

        qApp->processEvents();

        Re = m_ReMin+ia*m_ReInc;
        strange =QString("Re = %1 ........ ").arg(Re,0,'f',0);
        traceLog(strange);
        m_XFoilInstance.reinf1 = Re;
        m_XFoilInstance.lalfa = true;
        m_XFoilInstance.setQInf(1.0);

        // here we go !
        if (!m_XFoilInstance.specal())
        {
            QString str;
            str = "Invalid Analysis Settings\nCpCalc: local speed too large\n Compressibility corrections invalid ";
            traceLog(str);
            m_bErrors = true;
            return false;
        }

        m_XFoilInstance.lwake = false;
        m_XFoilInstance.lvconv = false;

        while(!iterate()){}
        if(m_XFoilInstance.lvconv)
        {
            str = QString(QObject::tr("   ...converged after %1 iterations\n")).arg(m_Iterations);
            traceLog(str);
        }
        else
        {
            str = QString(QObject::tr("   ...unconverged after %1 iterations\n")).arg(m_Iterations);
            traceLog(str);
            m_bErrors = true;
        }

        qApp->processEvents();

        m_Iterations = 0;

        if(m_pParent)
        {
            OpPoint *pOpPoint = new OpPoint;
            addXFoilData(pOpPoint, &m_XFoilInstance, m_pFoil);
            qApp->postEvent((QObject*)m_pParent, new XFoilOppEvent(m_pFoil, m_pPolar, pOpPoint));
        }

        if(XFoil::fullReport())
        {
            m_XFoilStream.flush();
            traceLog(m_XFoilLog);
            m_XFoilLog.clear();
        }

        if(m_x0) m_x0->clear();
        if(m_x1) m_x1->clear();
        if(m_y0) m_y0->clear();
        if(m_y1) m_y1->clear();
    }
    return true;
}



/**
* Manages the viscous iterations of the XFoil calculation.
* @return true if the analysis has been successful.
*/
bool XFoilTask::iterate()
{
    if(!m_XFoilInstance.viscal())
    {
        m_XFoilInstance.lvconv = false;
        //        QString str =QObject::tr("CpCalc: local speed too large\n Compressibility corrections invalid");
        return false;
    }

    while(m_Iterations<s_IterLim && !m_XFoilInstance.lvconv && !s_bCancel)
    {
        if(m_XFoilInstance.ViscousIter())
        {
            if(m_x0 && m_y0)
            {
                m_x0->append((double)m_Iterations);
                m_y0->append(m_XFoilInstance.rmsbl);
            }
            if(m_x1 && m_y1)
            {
                m_x1->append((double)m_Iterations);
                m_y1->append(m_XFoilInstance.rmxbl);
            }
            m_Iterations++;
        }
        else m_Iterations = s_IterLim;

        if(s_bSkipOpp || s_bSkipPolar)
        {
            m_XFoilInstance.setBLInitialized(false);
            m_XFoilInstance.lipan = false;
            s_bSkipOpp = false;
            return true;
        }
    }

    if(s_bCancel)  return true;// to exit loop


    if(!m_XFoilInstance.ViscalEnd())
    {
        m_XFoilInstance.lvconv = false;//point is unconverged

        m_XFoilInstance.setBLInitialized(false);
        m_XFoilInstance.lipan = false;
        m_bErrors = true;
        return true;// to exit loop
    }

    if(m_Iterations>=s_IterLim && !m_XFoilInstance.lvconv)
    {
        if(s_bAutoInitBL)
        {
            m_XFoilInstance.setBLInitialized(false);
            m_XFoilInstance.lipan = false;
        }
        m_XFoilInstance.fcpmin();// Is it of any use ?
        return true;
    }
    if(!m_XFoilInstance.lvconv)
    {
        m_bErrors = true;
        m_XFoilInstance.fcpmin();// Is it of any use ?
        return false;
    }
    else
    {
        //converged at last
        m_XFoilInstance.fcpmin();// Is it of any use ?
        return true;
    }
    return false;
}



/** 
 * Sends the analysis messages to the specified text output stream
 * @param str the message to output.
 */
void XFoilTask::traceLog(QString str)
{
    if(m_OutStream.device() || m_OutStream.string())
    {
        m_OutStream << str;
        m_OutMessage += str;
        qApp->processEvents();
    }
}


/**
* Adds the results of the XFoil Calculation to the OpPoint object
* @param pOpPoint a pointer to the instance of the OpPoint to be filled with the data from the XFoil object.
*/
void XFoilTask::addXFoilData(OpPoint *pOpp, XFoil *pXFoil, Foil *pFoil)
{
    int i=0, j=0, ibl=0, is=0;
    pOpp->m_Alpha      = pXFoil->alfa*180.0/PI;
    pOpp->n            = pXFoil->n;
    pOpp->Cd           = pXFoil->cd;
    pOpp->Cdp          = pXFoil->cdp;
    pOpp->Cl           = pXFoil->cl;
    pOpp->m_XCP        = pXFoil->xcp;
    pOpp->Cm           = pXFoil->cm;
    pOpp->m_Reynolds   = pXFoil->reinf;
    pOpp->m_Mach       = pXFoil->minf;
    pOpp->ACrit        = pXFoil->acrit;

    pOpp->m_bTEFlap    = pFoil->m_bTEFlap;
    pOpp->m_bLEFlap    = pFoil->m_bLEFlap;

    pOpp->Cpmn   = pXFoil->cpmn;

    for (int k=0; k<pXFoil->n; k++)
    {
        //        x[k]   = m_pXFoil->x[k+1];
        //        y[k]   = m_pXFoil->y[k+1];
        //        s[k]   = m_pXFoil->s[k+1];
        pOpp->Cpi[k] = pXFoil->cpi[k+1];
        pOpp->Qi[k]  = pXFoil->qgamm[k+1];
    }

    if(pXFoil->lvisc && pXFoil->lvconv)
    {
        pOpp->Xtr1 =pXFoil->xoctr[1];
        pOpp->Xtr2 =pXFoil->xoctr[2];
        pOpp->m_bViscResults = true;
        pOpp->m_bBL = true;
        for (int k=0; k<pXFoil->n; k++)
        {
            pOpp->Cpv[k] = pXFoil->cpv[k+1];
            pOpp->Qv[k] = pXFoil->qvis[k+1];
        }
    }
    else
    {
        pOpp->m_bViscResults = false;
    }

    if(pOpp->m_bTEFlap || pOpp->m_bLEFlap)
    {
        pOpp->setHingeMoments(pFoil);
        /*        m_TEHMom = m_pXFoil->hmom;
        XForce   = m_pXFoil->hfx;
        YForce   = m_pXFoil->hfy;*/
    }

    if(!pXFoil->lvisc || !pXFoil->lvconv)    return;

    //---- add boundary layer on both sides of airfoil
    pOpp->blx.nd1=0;
    pOpp->blx.nd2=0;
    pOpp->blx.nd3=0;
    for (is=1; is<=2; is++)
    {
        for (ibl=2; ibl<=pXFoil->iblte[is];ibl++)
        {
            i = pXFoil->ipan[ibl][is];
            pOpp->blx.xd1[i] = pXFoil->x[i] + pXFoil->nx[i]*pXFoil->dstr[ibl][is];
            pOpp->blx.yd1[i] = pXFoil->y[i] + pXFoil->ny[i]*pXFoil->dstr[ibl][is];
            pOpp->blx.nd1++;
        }
    }

    //---- set upper and lower wake dstar fractions based on first wake point
    is=2;
    double dstrte = pXFoil->dstr[pXFoil->iblte[is]+1][is];
    double dsf1, dsf2;
    if(dstrte!=0.0) // d* at TE
    {
        dsf1 = (pXFoil->dstr[pXFoil->iblte[1]][1] + 0.5*pXFoil->ante) / dstrte;
        dsf2 = (pXFoil->dstr[pXFoil->iblte[2]][2] + 0.5*pXFoil->ante) / dstrte;
    }
    else
    {
        dsf1 = 0.5;
        dsf2 = 0.5;
    }

    //---- plot upper wake displacement surface
    ibl = pXFoil->iblte[1];
    i = pXFoil->ipan[ibl][1];
    pOpp->blx.xd2[0] = pXFoil->x[i] + pXFoil->nx[i]*pXFoil->dstr[ibl][1];
    pOpp->blx.yd2[0] = pXFoil->y[i] + pXFoil->ny[i]*pXFoil->dstr[ibl][1];
    pOpp->blx.nd2++;

    j= pXFoil->ipan[pXFoil->iblte[is]+1][is]  -1;
    for (ibl=pXFoil->iblte[is]+1; ibl<=pXFoil->nbl[is]; ibl++)
    {
        i = pXFoil->ipan[ibl][is];
        pOpp->blx.xd2[i-j] = pXFoil->x[i] - pXFoil->nx[i]*pXFoil->dstr[ibl][is]*dsf1;
        pOpp->blx.yd2[i-j] = pXFoil->y[i] - pXFoil->ny[i]*pXFoil->dstr[ibl][is]*dsf1;
        pOpp->blx.nd2++;
    }

    //---- plot lower wake displacement surface
    ibl = pXFoil->iblte[2];
    i = pXFoil->ipan[ibl][2];
    pOpp->blx.xd3[0] = pXFoil->x[i] + pXFoil->nx[i]*pXFoil->dstr[ibl][2];
    pOpp->blx.yd3[0] = pXFoil->y[i] + pXFoil->ny[i]*pXFoil->dstr[ibl][2];
    pOpp->blx.nd3++;

    j = pXFoil->ipan[pXFoil->iblte[is]+1][is]  -1;
    for (ibl=pXFoil->iblte[is]+1; ibl<=pXFoil->nbl[is]; ibl++)
    {
        i = pXFoil->ipan[ibl][is];
        pOpp->blx.xd3[i-j] = pXFoil->x[i] + pXFoil->nx[i]*pXFoil->dstr[ibl][is]*dsf2;
        pOpp->blx.yd3[i-j] = pXFoil->y[i] + pXFoil->ny[i]*pXFoil->dstr[ibl][is]*dsf2;
        pOpp->blx.nd3++;
    }

    pOpp->blx.tklam = pXFoil->tklam;
    pOpp->blx.qinf = pXFoil->qinf;

    memcpy(pOpp->blx.thet, pXFoil->thet, IVX * ISX * sizeof(double));
    memcpy(pOpp->blx.tau,  pXFoil->tau,  IVX * ISX * sizeof(double));
    memcpy(pOpp->blx.ctau, pXFoil->ctau, IVX * ISX * sizeof(double));
    memcpy(pOpp->blx.ctq,  pXFoil->ctq,  IVX * ISX * sizeof(double));
    memcpy(pOpp->blx.dis,  pXFoil->dis,  IVX * ISX * sizeof(double));
    memcpy(pOpp->blx.uedg, pXFoil->uedg, IVX * ISX * sizeof(double));
    memcpy(pOpp->blx.dstr, pXFoil->dstr, IVX * ISX * sizeof(double));
    memcpy(pOpp->blx.itran, pXFoil->itran, 3 * sizeof(int));

    pXFoil->createXBL();
    pXFoil->fillHk();
    pXFoil->fillRTheta();
    memcpy(pOpp->blx.xbl, pXFoil->xbl, IVX * ISX * sizeof(double));
    memcpy(pOpp->blx.Hk, pXFoil->Hk, IVX * ISX * sizeof(double));
    memcpy(pOpp->blx.RTheta, pXFoil->RTheta, IVX * ISX * sizeof(double));
    pOpp->blx.nside1 = pXFoil->m_nSide1;
    pOpp->blx.nside2 = pXFoil->m_nSide2;
}
