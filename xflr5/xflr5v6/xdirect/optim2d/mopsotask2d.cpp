/****************************************************************************

    MOPSOTask2d Class
    Copyright (C) 2021 André Deperrois

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

#include <QDebug>

#include "mopsotask2d.h"
#include <xflcore/constants.h>
#include <xfoil.h>
#include <xflobjects/objects2d/foil.h>
#include <xdirect/analysis/xfoiltask.h>
#include <xflcore/mathelem.h>

MOPSOTask2d::MOPSOTask2d() : MOPSOTask()
{
    m_pFoil = nullptr;
    m_pPolar = nullptr;

    m_iLE = -1;

    m_HHt1            = 1.0;
    m_HHt2            = 1.0;
    m_HHmax           = 0.02;  // unused, replaced by the variable amplitude
    m_Alpha           = 0.0;
}


void MOPSOTask2d::makeFoil(Particle const &particle, Foil *pFoil) const
{
    pFoil->copyFoil(m_pFoil, false);

    double t1(0), hh(0), x(0);

    int halfdim = particle.dimension()/2; //less the flap dim

    for(int i=0; i<m_iLE; i++)
    {
        x = pFoil->m_xb[i];
        for(int j=0; j<halfdim; j++)
        {
            t1 = m_HHt1*double(j+1)/double(2*halfdim+1);
            hh = HH(x, t1, m_HHt2) * particle.pos(j);
            pFoil->m_xb[i] += pFoil->m_nx[i] *hh;
            pFoil->m_yb[i] += pFoil->m_ny[i] *hh;
        }
    }

    for(int i=m_iLE+1; i<pFoil->m_nb; i++)
    {
        x = pFoil->m_x[i];
        for(int j=0; j<halfdim; j++)
        {
            t1 = m_HHt1*double(j+1)/double(2*halfdim+1);
            hh = HH(x, t1, m_HHt2) * particle.pos(halfdim+j);
            pFoil->m_xb[i] += pFoil->m_nx[i] *hh;
            pFoil->m_yb[i] += pFoil->m_ny[i] *hh;
        }
    }

    pFoil->initFoil();

    if(isOdd(particle.dimension()))
    {
        double angle = particle.position().last();
        pFoil->setTEFlapAngle(angle);
        pFoil->setFlap();
    }
}


void MOPSOTask2d::calcFitness(Particle *pParticle) const
{
    Foil tempfoil;
    makeFoil(*pParticle, &tempfoil);

    bool bViscous  = true;
    bool bInitBL = true;

    QString strange;

    XFoilTask *task = new XFoilTask;
    XFoil const &xfoil = task->m_XFoilInstance;
    task->m_OutStream.setString(&strange);
    task->setSequence(true, m_Alpha, m_Alpha, 0.0);
    task->initializeXFoilTask(&tempfoil, m_pPolar, bViscous, bInitBL, false);
    task->run();

    bool bConverged = xfoil.lvconv;


    for(int i=0; i<m_Objective.size(); i++)
    {
        if      (m_Objective.at(i).m_Name=="Cl")     pParticle->setFitness(i, xfoil.cl);
        else if (m_Objective.at(i).m_Name=="Cd")     pParticle->setFitness(i, xfoil.cd);
        else if (m_Objective.at(i).m_Name=="Cl/Cd")  pParticle->setFitness(i, xfoil.cl/xfoil.cd);
        else if (m_Objective.at(i).m_Name=="Cp_min") pParticle->setFitness(i, xfoil.cpmn);
        else if (m_Objective.at(i).m_Name=="Cm")     pParticle->setFitness(i, xfoil.cm);
        else if (m_Objective.at(i).m_Name=="Cm0")
        {
            task->setSequence(false, 0, 0, 0);
            task->initializeXFoilTask(&tempfoil, m_pPolar, bViscous, bInitBL, false);
            task->run();
            bConverged = bConverged && xfoil.lvconv;
            pParticle->setFitness(i, xfoil.cm);
        }
    }

    pParticle->setConverged(bConverged);

    delete task;
}


double MOPSOTask2d::error(const Particle *pParticle, int iObjective) const
{
    if(!pParticle->isConverged()) return LARGEVALUE;

    switch (m_Objective.at(iObjective).m_Type)
    {
        case xfl::MINIMIZE:
        {
            if(pParticle->fitness(iObjective) <= m_Objective.at(iObjective).m_Target)
                return 0;
            else
                return fabs(pParticle->fitness(iObjective)-m_Objective.at(iObjective).m_Target);
        }
        case xfl::MAXIMIZE:
            if(pParticle->fitness(iObjective) >= m_Objective.at(iObjective).m_Target)
                return 0;
            else
                return fabs(pParticle->fitness(iObjective)-m_Objective.at(iObjective).m_Target);
        default:
        case xfl::EQUALIZE:
            return fabs(pParticle->fitness(iObjective)-m_Objective.at(iObjective).m_Target);
    }
}
