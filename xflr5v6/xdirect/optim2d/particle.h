/****************************************************************************

    Particle Class
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

#include <QVector>

/**
 * @class Multi-Objective Particle
 * To use in single objective PSO or GA, set NObjectives=1 and NBest=1
 */
class Particle
{
    public:
        Particle();

        int dimension() const {return m_Position.size();}
        int nObjectives() const {return m_Fitness.size();}
        int nBest() const {return m_nBest;}

        void resetBestError();
        double error(int iobj) const {return m_Error.at(iobj);}
        void setError(int iobj, double err) {m_Error[iobj]=err;}

        double bestError(int iFront, int iobj) const {return m_BestError.at(iFront).at(iobj);}
        void setBestError(int iFront, int iobj, double err) {m_BestError[iFront][iobj]=err;}
        void setBestPosition(int iFront, int idim, double pos) {m_BestPosition[iFront][idim]=pos;}
        void storeBestPosition(int ifront) {m_BestPosition[ifront]=m_Position;}

        QVector<double> const &velocity() const {return m_Velocity;}
        QVector<double> const &position() const {return m_Position;}

        void setPos(int i, double dble) {m_Position[i]=dble;}
        void setVel(int i, double dble) {m_Velocity[i]=dble;}

        double pos(int i) const {return m_Position.at(i);}
        double bestPos(int iFront, int iComponent) const {return m_BestPosition.at(iFront).at(iComponent);}
        double vel(int i) const {return m_Velocity.at(i);}


        void setFitness(int i, double f) {m_Fitness[i]=f;}
        double fitness(int i) const {return m_Fitness.at(i);}

        void initializeBest();
        void updateBest();

        void resizeArrays(int dim, int nobj, int nbest);

        bool dominates(Particle const* pOther) const;

    private:
        // size = dimension = nVariables
        QVector<double> m_Position;
        QVector<double> m_Velocity;

        //size = NObjectives
        QVector<double> m_Fitness; /** the value of each objective funnction for this particle */
        QVector<double> m_Error;   /** the error associated to each objective */

        //size = PARETOSIZE
        int m_nBest;
        QVector<QVector<double>> m_BestError;    /** the particle's personal best errors achieved so far; size=nObjectives*/
        QVector<QVector<double>> m_BestPosition; /** the particle's personal best positions achieved so far; size=dimension */
};
