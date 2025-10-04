/****************************************************************************

    GATask Class
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

#include <QApplication>
#include <QtConcurrent/QtConcurrent>
#include <QFutureSynchronizer>
#include <QRandomGenerator>

#include "gatask.h"

#include <xflobjects/objects2d/foil.h>
#include <xflcore/constants.h>
#include <xdirect/analysis/xfoiltask.h>
//#include <xdirect/optim2d/optimevent.h>


double GATask::s_ProbXOver       = 0.5;
double GATask::s_ProbMutation    = 0.15;
double GATask::s_SigmaMutation   = 0.2;


GATask::GATask()
{
    m_pFoil = nullptr;
    m_pPolar = nullptr;
    m_iLE   = -1;
    m_iBest = -1;
    m_Error = LARGEVALUE;

    m_HHn   = 6;   //GA seems to converge a little better if even number so that there is no bump function centered on the LE
    m_HHt2  = 1.0;
    m_HHmax = 0.025;
}


void GATask::setDimension(int n)
{
    OptimTask::setDimension(n);
    m_BestPosition.resize(n);
}


void GATask::makeSwarm()
{
    m_Swarm.resize(s_PopSize);
    m_BestPosition.resize(m_HHn);

    if(s_bMultiThreaded)
    {
        QFutureSynchronizer<void> futureSync;
        for (int isw=0; isw<m_Swarm.size(); isw++)
        {
            Particle &particle = m_Swarm[isw];
            futureSync.addFuture(QtConcurrent::run(this, &GATask::makeRandomParticle, &particle));
        }
        futureSync.waitForFinished();
        outputMsg("   ...done");
    }
    else
    {
        for (int isw=0; isw<m_Swarm.size(); isw++)
        {
            Particle &particle = m_Swarm[isw];
            makeRandomParticle(&particle);
            outputMsg(QString::asprintf("   created particle %d", isw));
        }
    }
    outputMsg(QString::asprintf("Made %d random particles\n", s_PopSize));
}


/**
 * Makes a particle for a single objective PSO or a GA
 * note: QFutureSync requires that the parameters be passed by pointer and not by reference
 */
void GATask::makeRandomParticle(Particle *pParticle) const
{
    int dim=m_HHn, nobj=1, nbest=1;
    pParticle->resizeArrays(dim, nobj, nbest);

    double pos=0, posmin=0, posmax=0;
    double deltapos = posmax-posmin;

    for(int i=0; i<pParticle->dimension(); i++)
    {
        posmin = m_Variable.at(i).m_Min;
        posmax = m_Variable.at(i).m_Max;
        deltapos = posmax-posmin;
        pos = posmin + QRandomGenerator::global()->bounded(deltapos);
        pParticle->setPos(i, pos);
        pParticle->initializeBest();

        pParticle->setVel(i, 0);
    }
}


void GATask::restoreDefaults()
{

}


/** Posted when an iteration has ended */
void GATask::postIterEvent(int iBest)
{
    OptimEvent *pIterEvent = new OptimEvent(OPTIM_ITER_EVENT, m_Iter, iBest, m_Swarm.at(iBest));
    qApp->postEvent(m_pParent, pIterEvent);
}



void GATask::onIteration()
{
    makeNewGen();

    //make best
    for (int isw=0; isw<m_Swarm.size(); isw++)
    {
        Particle &particle = m_Swarm[isw];
        if(particle.error(0)<m_Error)
        {
            m_BestPosition = particle.position();
            m_Error = particle.error(0);
            m_iBest = isw;
        }
    }

    m_Iter++;
    postIterEvent(m_iBest);

    if(m_Iter>=s_MaxIter || m_Error<m_Objective.m_MaxError)
    {
        outputMsg(QString::asprintf("The winner is particle %d\n", m_iBest));
        outputMsg(QString::asprintf("Residual error = %7.3g\n", m_Error));

        m_Status = xfl::FINISHED;

        postOptEndEvent(); // tell the GUI that the task is done

        // in case this task has been run in a worker thread, move it back to the main GUI thread so that it may be resumed
        moveToThread(QApplication::instance()->thread());
    }
}


void GATask::makeNewGen()
{
    makeSelection();
    crossOver();
    mutateGaussian();
    evaluatePopulation();
}


void GATask::makeSelection()
{
    QVector<double>error(m_Swarm.size()), cumul(m_Swarm.size());

    // find the error of the worst particle
    double worsterror = 0.0;
    for(int i=0; i<m_Swarm.size(); i++)
    {
        if(m_Swarm.at(i).error(0)>worsterror) worsterror = m_Swarm.at(i).error(0);
    }

    // define probability as: worsterror - particle.error
    // this ensures that the worst has probability 0 and the best has max probability
    for(int i=0; i<m_Swarm.size(); i++)
        error[i] = worsterror - m_Swarm.at(i).error(0);

    cumul.first() = error.first();
    for(int i=1; i<m_Swarm.size(); i++) cumul[i] = cumul.at(i-1) + error.at(i);

    QVector<Particle> newpop(m_Swarm);
    for(int i=0; i<m_Swarm.size(); i++)
    {
        double p = QRandomGenerator::global()->bounded(cumul.last());
        bool bFound = false;
        for(int j=1; j<m_Swarm.size(); j++)
        {
            if(p<=cumul.at(j))
            {
                newpop[i] = m_Swarm.at(j);
                bFound = true;
                break;
            }
        }
        Q_ASSERT(bFound);
    }

    m_Swarm = newpop;
}


/**
 * @todo: in the standard GA, the best does not carry over to the next generation
 * so that there may be a regression
 */
void GATask::crossOver()
{
    double const alpha = 0.5;
    double frac=0;
    QVector<Particle> oldpop(m_Swarm);
    m_Swarm.clear();
    Particle parent[2], children[2];

    while (oldpop.size()>=2)
    {
        // extract two parents
        int ifirst = QRandomGenerator::global()->bounded(oldpop.size());
        parent[0] = oldpop.takeAt(ifirst);

        int isecond = QRandomGenerator::global()->bounded(oldpop.size());
        parent[1] = oldpop.takeAt(isecond);

        double prob = QRandomGenerator::global()->bounded(1.0);
        if(prob<s_ProbXOver)
        {
            // create two random children
            for(int iChild=0; iChild<2; iChild++)
            {
                Particle &child = children[iChild];
                child.resizeArrays(parent[0].dimension(), 1, 1);
                for(int i=0; i<child.dimension(); i++)
                {
                    frac = -alpha + QRandomGenerator::global()->bounded(1.0+alpha);
                    child.setPos(i, frac*parent[0].pos(i)+(1.0-frac)*parent[1].pos(i));
                }

                checkBounds(child);
            }
            if(children[0].error(0)>LARGEVALUE/10.0 || children[1].error(0)>LARGEVALUE/10.0)
            {
                // XFoil has failed to converge: restore the parents
                children[0] = parent[0];
                children[1] = parent[1];
            }
        }
        else
        {
            children[0] = parent[0];
            children[1] = parent[1];
        }
        m_Swarm.append(children[0]);
        m_Swarm.append(children[1]);
    }

    m_Swarm.append(oldpop); // add the remaining single parent if odd population
}


/**
 * @todo: in the standard GA, the best can mutate, so that there may be a regression
 */
void GATask::mutateGaussian()
{
    std::default_random_engine generator;
    std::normal_distribution<double> distribution(0.0, s_SigmaMutation*m_HHmax);
    double prob=0, randomvariation=0, newgene=0;
    for(int i=0; i<m_Swarm.size(); i++)
    {
        Particle &particle = m_Swarm[i];
        for(int j=0; j<particle.dimension(); j++) // and y components
        {
            prob = QRandomGenerator::global()->bounded(1.0);
            if(prob<s_ProbMutation)
            {
                randomvariation = distribution(generator);
                newgene = particle.pos(j) + randomvariation;
                particle.setPos(j, newgene);
            }
        }

        checkBounds(particle);
    }
}

void GATask::evaluatePopulation()
{
    m_iBest = -1;
    m_Error=LARGEVALUE;

    if(s_bMultiThreaded)
    {
        QFutureSynchronizer<void> futureSync;
        for (int isw=0; isw<m_Swarm.size(); isw++)
        {
            Particle &particle = m_Swarm[isw];
            futureSync.addFuture(QtConcurrent::run(this, &GATask::evaluateParticle, &particle));
        }
        futureSync.waitForFinished();
    }
    else
    {
        for(int i=0; i<m_Swarm.size(); i++)
        {
            Particle &particle = m_Swarm[i];
            evaluateParticle(&particle);
        }
    }

    for(int i=0; i<m_Swarm.size(); i++)
    {
        Particle &particle = m_Swarm[i];
        if(particle.error(0)<m_Error)
        {
            m_Error = particle.error(0);
            m_BestPosition = particle.position();
            m_iBest = i;
        }
    }
}


// The lengthiest task
// Note: PSO is parallelized at swarm level and GA at particle level
// Note: QFutureSync requires that the parameters be passed by pointer and not by reference
void GATask::evaluateParticle(Particle *pParticle) const
{
    double Cl = foilFunc(pParticle);

    while(Cl>LARGEVALUE/10.0)
    {
        // XFoil has failed to converge: make a new random particle
        makeRandomParticle(pParticle);
        Cl = foilFunc(pParticle);
    }

    pParticle->setFitness(0, Cl);
    pParticle->setError(0, fabs(Cl-m_Objective.m_Target));
}


void GATask::makeFoil(Particle const*pParticle, Foil *pFoil) const
{
    pFoil->copyFoil(m_pFoil, false);

    double t1=0, hh=0, x=0;

    // for this demo case, modifies only the top surface, i.e. from node 0 to node m_iLE
    for(int i=0; i<m_iLE; i++)
    {
        x = pFoil->m_x[i];
        for(int j=0; j<pParticle->dimension(); j++)
        {
            t1 = double(j+1)/double(pParticle->dimension()+1); // HH undefined for t1=0
            hh = HH(x, t1, m_HHt2) * pParticle->pos(j);
            pFoil->m_xb[i] += pFoil->m_nx[i] *hh;
            pFoil->m_yb[i] += pFoil->m_ny[i] *hh;
        }
    }

    memcpy(pFoil->m_x, pFoil->m_xb, IBX*sizeof(double));
    memcpy(pFoil->m_y, pFoil->m_yb, IBX*sizeof(double));
    pFoil->normalizeGeometry();
}


double GATask::foilFunc(Particle const*pParticle) const
{
    Foil tempfoil;
    makeFoil(pParticle, &tempfoil);

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
    if(xfoil.lvconv) Cl = xfoil.cl;

    delete task;

    return Cl;
}

/** Hicks-Henne bump function
 * parameter t1 controls the bump's position and t2 its width
 * @todo make global
 */
double GATask::HH(double x, double t1, double t2) const
{
    if(x<=0.0 || x>=1.0) return 0.0;
    return pow(sin(PI*pow(x, log(0.5)/log(t1))), t2);
}


double GATask::error(const Particle *pParticle, int iObjective) const
{
    double err = fabs(pParticle->fitness(iObjective) - m_Objective.m_Target); // could also maximize or minimize etc.
    return err;
}


void GATask::calcFitness(Particle *pParticle) const
{
    Foil tempfoil;
    makeFoil(pParticle, &tempfoil);

    bool bViscous  = true;
    bool bInitBL = true;

    QString strange;

    XFoilTask *task = new XFoilTask; // watch the stack
    XFoil const &xfoil = task->m_XFoilInstance;
    task->m_OutStream.setString(&strange);
    task->setSequence(true, m_Alpha, m_Alpha, 0.0);
    task->initializeXFoilTask(&tempfoil, m_pPolar, bViscous, bInitBL, false);
    task->run();

    if(xfoil.lvconv)
    {
        pParticle->setFitness(0, xfoil.cl);
    }
    else pParticle->setFitness(0, LARGEVALUE); // set and unlikely value

    delete task;


}
