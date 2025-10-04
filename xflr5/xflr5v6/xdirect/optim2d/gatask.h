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

#pragma once

#include <QWidget>
#include <QEvent>

#include <xdirect/optim2d/optimtask.h>
#include <xflcore/core_enums.h>
#include <xflcore/xflevents.h>

#include "particle.h"
#include "optstructures.h"


/**
 * @brief This class implements the methods of the Genetic Algorithm
 * Run the task using either a worker thread or a timer https://doc.qt.io/qt-5/thread-basics.html
 * @todo make abstract and force reimplementation of foil specific methods in the derived class
 */
class GATask : public OptimTask
{
    Q_OBJECT

    public:
        GATask();

        void setHHParams(int n, double t2, double amp) {m_HHn=n; m_HHt2=t2; m_HHmax=amp;}

        void setFoil(Foil const*pFoil, int iLE) {m_pFoil=pFoil; m_iLE=iLE;}
        void setPolar(Polar *pPolar) {m_pPolar=pPolar;}
        void setDimension(int n) override;
        void setObjective(OptObjective const &obj) {m_Objective=obj;}

        void setAlpha(double aoa) {m_Alpha=aoa;}
        double alpha() const {return m_Alpha;}

        OptObjective const &objective() const {return m_Objective;}

        void makeSwarm() override;
        void makeFoil(Particle const*pParticle, Foil *pFoil) const;


        Particle bestParticle() const {if(m_iBest>=0&&m_iBest<m_Swarm.size()) return m_Swarm.at(m_iBest); else return Particle();}

        static void restoreDefaults();
        static void setParams(double xover, double pmut, double sigmut) {s_ProbXOver=xover; s_ProbMutation=pmut; s_SigmaMutation=sigmut;}

    private:
        void crossOver();
        void evaluateParticle(Particle *pParticle) const;
        void evaluatePopulation();
        void makeNewGen();
        void makeRandomParticle(Particle *pParticle) const override;

        void makeSelection();
        void mutateGaussian();
        void postIterEvent(int iBest) override;

        /** @todo foil or case specific methods to deport in a derived class */
        void calcFitness(Particle*pParticle) const override;
        double error(Particle const *pParticle, int iObjective) const override;
        double HH(double x, double t1, double t2) const;
        double foilFunc(Particle const*pParticle) const;


    private slots:
        void onIteration() override; // in case the iteration is triggered by a timer

    protected:

    private:
        Foil const*m_pFoil;
        Polar *m_pPolar;

        int m_iBest;
        double m_Error;

        OptObjective m_Objective;

        QVector<double> m_BestPosition; // population best

        double m_HHt2;     /**< t2 parameter of the HH functions */
        int    m_HHn;      /**< number of HH functions to use */
        double m_HHmax;    /**< the max amplitude of the HH functions */

        double m_Alpha;

        int m_iLE;   /** index of the foil's leading edge node */

    public:
        static double s_ProbXOver;       /** probability of crossover */
        static double s_ProbMutation;    /** probability of mutation */
        static double s_SigmaMutation;   /** standard deviation of the gaussian mutation */
};





