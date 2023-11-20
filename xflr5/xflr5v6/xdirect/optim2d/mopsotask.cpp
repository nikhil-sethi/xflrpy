/****************************************************************************

    MOPSOTask Class
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
#include <QtConcurrent/QtConcurrentRun>
#include <QFutureSynchronizer>
#include <QRandomGenerator>

#include "mopsotask.h"
#include <xflcore/constants.h>



#include "mopsotask.h"
#include <xflcore/constants.h>


int    MOPSOTask::s_ArchiveSize       = 11;
double MOPSOTask::s_InertiaWeight     = 0.3;
double MOPSOTask::s_CognitiveWeight   = 0.7;
double MOPSOTask::s_SocialWeight      = 0.7;
double MOPSOTask::s_ProbRegenerate    = 0.07;


MOPSOTask::MOPSOTask()
{
}


void MOPSOTask::restoreDefaults()
{
    s_PopSize           = 11;
    s_ArchiveSize       = 10;
    s_MaxIter           = 100;
    s_InertiaWeight     = 0.3;
    s_CognitiveWeight   = 0.7;
    s_SocialWeight      = 0.7;
    s_ProbRegenerate    = 0.07;
}


void MOPSOTask::makeSwarm()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_Iter = 0;

    m_Swarm.resize(s_PopSize);

    for (int i=0; i<m_Swarm.size(); ++i)
    {
        Particle &particle = m_Swarm[i];
        makeRandomParticle(&particle);
    }

    updateFitnesses();

    for (int i=0; i<m_Swarm.size(); ++i)
    {
        if(!m_Swarm.at(i).isConverged()) outputMsg(QString::asprintf("    Particle %d is unconverged\n", i));
    }

    outputMsg(QString::asprintf("Made %d random particles\n", int(m_Swarm.size())));

    QApplication::restoreOverrideCursor();
}


void MOPSOTask::onIterate()
{
    if(m_Swarm.size()==0 || m_Swarm.size()!=s_PopSize)
    {
        m_Status = xfl::PENDING;
        outputMsg("Invalid swarm size\n");
        postPSOEvent(-1); // notifiy finished
        moveToThread(QApplication::instance()->thread());
        return;
    }

    m_Status = xfl::RUNNING;
    m_Iter=0;

    do
    {
        onIteration();
    }
    while(m_Status==xfl::RUNNING);
}


void MOPSOTask::onIteration()
{
    m_Iter++;
    if(m_Iter>s_MaxIter) return;  // failsafe because events are async

    if(s_bMultiThreaded)
    {
        QFutureSynchronizer<void> futureSync;
        for (int isw=0; isw<m_Swarm.size(); ++isw)
        {
           Particle &particle = m_Swarm[isw];
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
           futureSync.addFuture(QtConcurrent::run(&MOPSOTask::moveParticle, this, &particle));
#else
           futureSync.addFuture(QtConcurrent::run(this, &MOPSOTask::moveParticle, &particle));
#endif
        }
        futureSync.waitForFinished();
    }
    else
    {
        for (int isw=0; isw<m_Swarm.size(); ++isw)
        {
            Particle &particle = m_Swarm[isw];
            moveParticle(&particle);
            if(isCancelled()) break;
        }
    }


    bool bIsConverged = false;
    int iBest0 = 0;

    if(!isCancelled())
    {

        for (int isw=0; isw<m_Swarm.size(); ++isw)
        {
            if(!m_Swarm.at(isw).isConverged())        outputMsg(QString::asprintf("Particle %d is unconverged\n", isw));
        }

        makeParetoFront();

        // select the best first objective
        iBest0 = 0; // pareto first, if non better other

        Particle bestparticle;
        for(int i=0; i<m_Pareto.size(); i++)
        {
            Particle const &particle = m_Pareto.at(i);
            if(particle.error(0)<m_Objective.at(0).m_MaxError)
            {
                iBest0 = i;
                bestparticle = particle;
            }

            // check if the particle meets all criteria
            bIsConverged = true;
            for(int io=0; io<particle.nObjectives(); io++)
            {
                double err = PRECISION;
                if(m_Objective.at(io).m_Type==xfl::EQUALIZE)  err = m_Objective.at(io).m_MaxError;

                if(particle.error(io)>err)
                {
                    bIsConverged = false;
                    break;
                }
            }
            if(bIsConverged)  break;
        }

        postIterEvent(iBest0);
    }

    if(m_Iter>=s_MaxIter || bIsConverged || m_Status==xfl::CANCELLED)
    {
        if  (bIsConverged) outputMsg("   ---Converged---\n");
        else if(m_Status==xfl::CANCELLED) outputMsg("The task has been cancelled\n");

        m_Status = xfl::FINISHED;

        postPSOEvent(iBest0); // tell the GUI that the task is done

        // this task may be resumed, so move it back to the main GUI thread
        moveToThread(QApplication::instance()->thread());
    }
}



void MOPSOTask::moveParticle(Particle *pParticle) const
{
    double newpos(0), vel(0);
    double r1(0), r2(0);
    int igbest(0), ipbest(0);

    // regenerate particles with random probability
    double regen = QRandomGenerator::global()->bounded(1.0);

    // do not regenerate porticles in the Pareto front.
    bool bRegen = regen<s_ProbRegenerate;
    if(pParticle->isInParetoFront()) bRegen = false;
    if(!pParticle->isConverged()) bRegen = true;
    if(bRegen)
    {
        makeRandomParticle(pParticle);
    }
    else
    {
        // select a random best in the pareto front
        igbest = QRandomGenerator::global()->bounded(m_Pareto.size());
        Particle const &globalbest = m_Pareto.at(igbest);

        // select a random best in the personal bests
        ipbest = QRandomGenerator::global()->bounded(pParticle->nBest());

        // update the velocity
        for(int j=0; j<pParticle->dimension(); j++)
        {
            r1 = QRandomGenerator::global()->bounded(1.0);
            r2 = QRandomGenerator::global()->bounded(1.0);

            vel = s_InertiaWeight * pParticle->vel(j) +
                  s_CognitiveWeight * r1 * (pParticle->bestPos(ipbest, j) - pParticle->pos(j)) +
                  s_SocialWeight    * r2 * (globalbest.pos(j)             - pParticle->pos(j));

            //test if the velocity moves the particle off-bound and if true bounce it in the opposite direction
            newpos = pParticle->pos(j) + vel;
            if(newpos<m_Variable.at(j).m_Min || newpos>m_Variable.at(j).m_Max)
                vel = -vel;

            pParticle->setVel(j, vel);

            // update the position
            pParticle->setPos(j, pParticle->pos(j) + pParticle->vel(j));
        }
    }

    checkBounds(*pParticle);

    calcFitness(pParticle); // note: do not parallelize in derived class

    for(int i=0; i<m_Objective.size(); i++)
    {
        pParticle->setError(i, error(pParticle, i));
    }

    pParticle->updateBest();
}


/** Posted when an iteration has ended */
void MOPSOTask::postIterEvent(int iBest)
{
    // to be used with Qt::BlockingQueuedConnection to ensure ibjject is not deleted before call returns
    OptimEvent pOptEvent(OPTIM_ITER_EVENT, m_Iter, iBest, m_Pareto.at(iBest));
//    qApp->postEvent(m_pParent, pIterEvent);
    emit iterEvent(&pOptEvent);
}


/** Posted when the iteration loop has ended */
void MOPSOTask::postPSOEvent(int iBest)
{
    OptimEvent *pPSOEvent = new OptimEvent(OPTIM_END_EVENT, m_Iter, iBest, m_Pareto.at(iBest));
    qApp->postEvent(m_pParent, pPSOEvent);
}


#define PARTICLEBESTSIZE 3
void MOPSOTask::makeRandomParticle(Particle *pParticle) const
{
    int nBest = std::min(int(m_Objective.size()), PARTICLEBESTSIZE);
    pParticle->resizeArrays(m_Variable.size(), m_Objective.size(), nBest);

    for(int i=0; i<pParticle->dimension(); i++)
    {
        double deltap = m_Variable.at(i).m_Max - m_Variable.at(i).m_Min;
        double pos = m_Variable.at(i).m_Min + QRandomGenerator::global()->bounded(deltap);

        pParticle->setPos(i, pos);
        pParticle->initializeBest();

        double deltav = deltap/2.0;
        double vel = -deltav/2.0 + QRandomGenerator::global()->bounded(deltav);
        pParticle->setVel(i, vel);
    }
}


void MOPSOTask::updateFitnesses()
{
    if(s_bMultiThreaded)
    {
        QFutureSynchronizer<void> futureSync;
        for (int isw=0; isw<m_Swarm.size(); ++isw)
        {
           Particle &particle = m_Swarm[isw];

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
           futureSync.addFuture(QtConcurrent::run(&MOPSOTask::calcFitness, this, &particle));
#else
           futureSync.addFuture(QtConcurrent::run(this, &MOPSOTask::calcFitness, &particle));
#endif
        }
        futureSync.waitForFinished();
    }
    else
    {
        for(int i=0; i<m_Swarm.size(); i++)
            calcFitness(&m_Swarm[i]);
    }

    for(int i=0; i<m_Swarm.size(); i++)
    {
        for(int iobj=0; iobj<m_Objective.size(); iobj++)
            m_Swarm[i].setError(iobj, error(&m_Swarm.at(i), iobj));
    }
}


void MOPSOTask::updateErrors()
{
    double err=LARGEVALUE;
    for(int j=0; j<m_Swarm.size(); j++)
    {
        Particle &particle = m_Swarm[j];
        for(int iobj=0; iobj<m_Objective.size(); iobj++)
        {
            err = error(&particle, iobj);
            particle.setError(iobj, err);
        }
        particle.initializeBest();
    }
}


void MOPSOTask::makeParetoFront()
{
    for(int j=0; j<m_Swarm.size(); j++)
    {
        Particle &pj = m_Swarm[j];
        pj.setInParetoFront(false);

        if(!pj.isConverged())
        {
        }
        else
        {
            bool bIsDominated=false;
            for(int ip=0; ip<m_Pareto.size(); ip++)
            {
                Particle const &ppar = m_Pareto.at(ip);
                if(ppar.dominates(&pj))
                {
                    // this particle does not belong to the Pareto front, discard it
                    bIsDominated=true;
                    break;
                }
            }

            if(!bIsDominated)
            {
                // this particle is worthy
                // check if it dominates any of the existing particles in the Pareto frontier
                for(int ip=m_Pareto.size()-1; ip>=0; ip--)
                {
                    Particle const &ppar = m_Pareto.at(ip);
                    if(pj.dominates(&ppar))
                    {
                        m_Pareto.removeAt(ip);
                    }
                }
                m_Pareto.append(pj);
                pj.setInParetoFront(true);
            }
        }
    }

    /** @todo target removal to ensure max dispersion of the Pareto front */
    while(m_Pareto.size()>s_ArchiveSize)
    {
        m_Pareto.removeAt(QRandomGenerator::global()->bounded(m_Pareto.size()));
    }
}

