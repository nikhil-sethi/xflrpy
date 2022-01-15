/****************************************************************************

    XFoilTask Class
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

/** @file This file implements the management task of an XFoil calculation. Used in multithreaded analysis. */

#pragma once

#include <QRunnable>

#include "xfoil.h"

#include <xflobjects/objects2d/polar.h>
#include <xflobjects/objects2d/foil.h>



/**
 * @struct Analysis a pair of foil and polar used by a thread to perform one foil polar analysis.
 */
struct FoilAnalysis
{
    Foil const*pFoil=nullptr;            /**< a pointer to the Foil to be analyzed by the thread */
    Polar *pPolar=nullptr;          /**< a pointer to the polar to be analyzed by the thread */
    double vMin=0, vMax=0, vInc=0;
};

// this class runs an XFoil analysis in a thread separate from the main thread

/**
*@class XFoilTask
* This file implements the management task of an XFoil calculation. Used in multithreaded analysis. 
*/
class XFoilTask : public QRunnable
{
    public:
        XFoilTask(QObject *pParent=nullptr);

    public:
        void run() override; // run in a thread started from the QThreadPool

        bool alphaSequence();
        bool ReSequence();
        bool isFinished() const {return m_bIsFinished;}

        bool initializeTask(FoilAnalysis &pFoilAnalysis, bool bViscous, bool bInitBL, bool bFromZero);
        bool initializeXFoilTask(const Foil *pFoil, Polar *pPolar, bool bViscous, bool bInitBL, bool bFromZero);
        bool iterate();

        void setSequence(bool bAlpha, double SpMin, double SpMax, double SpInc);
        void setReRange(double ReMin, double ReMax, double ReInc);
        void traceLog(QString const &str);

        void addXFoilData(OpPoint *pOpp, XFoil *pXFoil, const Foil *pFoil);

        static void cancelTask() {s_bCancel=true;}
        static void setCancelled(bool bCancelled) {s_bCancel=bCancelled;}

        static bool s_bSkipPolar;
        static bool s_bCancel;          /**< true if the user has asked to cancel the analysis */
        static bool s_bAutoInitBL;      /**< true if the BL initialization is left to the code's decision */
        static int  s_IterLim;
        static bool s_bSkipOpp;

    public:
        int m_Iterations;          /**< The number of iterations already performed */
        bool m_bIsFinished;        /**< true if the calculation is over */
        XFoil m_XFoilInstance;     /**< An instance of the XFoil class specific for this object */

        QTextStream m_OutStream;
        QString m_OutMessage;
        QString m_XFoilLog;
        QTextStream m_XFoilStream;


        double m_AlphaMin, m_AlphaMax, m_AlphaInc;
        double m_ClMin, m_ClMax, m_ClInc;
        double m_ReMin, m_ReMax, m_ReInc;

        bool m_bAlpha, m_bFromZero, m_bInitBL, m_bErrors;

        QObject *m_pParent;

    private:
        Foil const*m_pFoil;       /**< A pointer to the instance of the Foil object for which the calculation is performed */
        Polar *m_pPolar;         /**< A pointer to the instance of the Polar object for which the calculation is performed */
};

