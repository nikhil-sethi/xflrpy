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

#include "mopsotask2d.h"
#include <xflcore/constants.h>

#include <xfoil.h>
#include <xflobjects/objects2d/foil.h>
#include <xdirect/analysis/xfoiltask.h>


MOPSOTask2d::MOPSOTask2d()
{
    m_pFoil = nullptr;
    m_pPolar = nullptr;

    m_iLE = -1;

    m_HHn             = 5;
    m_HHt2            = 1.0;
    m_HHmax           = 0.02;  // unused, replaced by the variable amplitude
    m_Alpha           = 0.0;
}


// for this demo case, modifies only the top surface, i.e. from node 0 to node m_iLE
void MOPSOTask2d::makeFoil(Particle const &particle, Foil *pFoil) const
{
    pFoil->copyFoil(m_pFoil, false);

    double t1=0, hh=0, x=0;

    for(int i=0; i<m_iLE; i++)
    {
        x = pFoil->m_x[i];
        for(int j=0; j<particle.dimension(); j++)
        {
            t1 = double(j+1)/double(particle.dimension()+1); // HH undefined for t1=0
            hh = HH(x, t1, m_HHt2) * particle.pos(j);
            pFoil->m_xb[i] += pFoil->m_nx[i] *hh;
            pFoil->m_yb[i] += pFoil->m_ny[i] *hh;
        }
    }

    memcpy(pFoil->m_x, pFoil->m_xb, IBX*sizeof(double));
    memcpy(pFoil->m_y, pFoil->m_yb, IBX*sizeof(double));
    pFoil->normalizeGeometry();
}


void MOPSOTask2d::calcFitness(Particle *pParticle) const
{
    Foil tempfoil;
    makeFoil(*pParticle, &tempfoil);

    bool bViscous  = true;
    bool bInitBL = true;

    QString strange;

    XFoilTask *task = new XFoilTask; // watch the stack
    XFoil const &xfoil = task->m_XFoilInstance;
    task->m_OutStream.setString(&strange);
    task->setSequence(true, m_Alpha, m_Alpha, 0.0);
    task->initializeXFoilTask(&tempfoil, m_pPolar, bViscous, bInitBL, false);
    task->run();

    double Cl = LARGEVALUE;
    double Cd = LARGEVALUE;
    if(xfoil.lvconv)
    {
        Cl = xfoil.cl;
        Cd = xfoil.cd;
    }
    else
    {
        // just to keep a reasonable scale for the Pareto graph
        Cl = 2.0;
        Cd = 0.5;

    }

    delete task;

    pParticle->setFitness(0, Cl);
    pParticle->setFitness(1, Cd);
}


double MOPSOTask2d::error(const Particle *pParticle, int iObjective) const
{
    return fabs(pParticle->fitness(iObjective)-m_Objective.at(iObjective).m_Target);
}


/** Hicks-Henne bump function
 * parameter t1 controls the bump's position and t2 its width
 */
double MOPSOTask2d::HH(double x, double t1, double t2) const
{
    if(x<=0.0 || x>=1.0) return 0.0;
    return pow(sin(PI*pow(x, log(0.5)/log(t1))), t2);
}


