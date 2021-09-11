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



/**
 *@file
 *
 * This file defines the LLTAnalysisDlg class, which is used to perform LLT analysis
 *
 */


#ifndef LLTANALYSIS_H
#define LLTANALYSIS_H


#include <xflanalysis/analysis3d_params.h>
#include <xflanalysis/analysis3d_globals.h>

#include <QVector>

/**
 *@class LLTAnalysis
 *@brief The class is used to perform the LLT analysis of one operating point

    The analysis is managed from the instance of the LLTAnalysisDlg class.
    All the data is in International Standard units kg, m, s.
 */

class Plane;
class WPolar;
class PlaneOpp;
class PlaneTaskEvent;
class Wing;
class Polar;

class LLTAnalysis : QObject
{
    Q_OBJECT

    friend class Miarex;
    friend class PlaneTask;
    friend class MainFrame;
    friend class LLTAnalysisDlg;
    friend class XflScriptExec;

public:
    LLTAnalysis();
    void clearPOppList();
    void initializeAnalysis();
    void setPlane(Plane *pPlane);
    void setWPolar(WPolar *pWPolar);
    void setLLTRange(double AlphaMin, double AlphaMax, double AlphaDelta, bool bSequence);
    void setLLTData(Plane *pPlane, WPolar *pWPolar);

    void setCurvePointers(QVector<double> *x, QVector<double> *y)
    {
        m_pX = x;
        m_pY = y;
    }

    bool isCancelled() const;
    bool hasWarnings() const;

    static void setMaxIter(int maxIter){s_IterLim = maxIter;}
    static void setConvergencePrecision(double precision) {s_CvPrec = precision;}
    static void setNSpanStations(int nStations){s_NLLTStations=nStations;}
    static void setRelaxationFactor(double relax){s_RelaxMax = relax;}

    static int maxIter(){return s_IterLim;}
    static double convergencePrecision() {return s_CvPrec;}
    static int nSpanStations(){return s_NLLTStations;}
    static double relaxationFactor(){return s_RelaxMax;}


private:
    double AlphaInduced(int k);
    double Beta(int m, int k);
    double Eta(int m);
    void computeWing(double QInf, double Alpha, QString &ErrorMessage);
    void setVelocity(double &QInf);
    void initializeGeom();
    int iterate(double &QInf, double const Alpha);
    void setBending(double QInf);
    bool setLinearSolution(double Alpha);
    void resetVariables();
    double Sigma(int m);

    PlaneOpp *createPlaneOpp(double QInf, double Alpha, bool bWingOut);
    bool loop();
    bool alphaLoop();
    bool QInfLoop();
    void traceLog(QString str);

    double getCl(Foil const*pFoil0, Foil const*pFoil1, double Re, double Alpha, double Tau, bool &bOutRe, bool &bError);
    double getZeroLiftAngle(Foil const*pFoil0, Foil const*pFoil1, double Re, double Tau);
    double getCm(Foil const *pFoil0, Foil const*pFoil1, double Re, double Alpha, double Tau, bool &bOutRe, bool &bError);
    double getCm0(Foil const*pFoil0, Foil const*pFoil1, double Re, double Tau, bool &bOutRe, bool &bError);
    double getCd(Foil const*pFoil0, Foil const*pFoil1, double Re, double Alpha, double Tau, double AR, bool &bOutRe, bool &bError);
    double getXCp(Foil const*pFoil0, Foil const*pFoil1, double Re, double Alpha, double Tau,  bool &bOutRe, bool &bError);
    double getXTr(Foil const*pFoil0, Foil const*pFoil1, double Re, double Alpha, double Tau, bool bTop, bool &bOutRe, bool &bError);
    double getPlrPointFromAlpha(const Foil *pFoil, double Re, double Alpha, int PlrVar, bool &bOutRe, bool &bError);
    void getLinearizedPolar(Foil *pFoil0, Foil *pFoil1, double Re, double Tau, double &Alpha0, double &Slope);


signals:
    void outputMsg(QString msg);

public slots:
    void onCancel();

private:

    Plane *m_pPlane;                           /**< A pointer to the Plane object for which the main wing calculation shall be performed >*/
    Wing *m_pWing;                             /**< A pointer to the Wing object for which the calculation shall be performed >*/
    WPolar *m_pWPolar;                          /**< A pointer to the WPolar object for which the calculation shall be performed >*/

    double m_vMin;          /**< The starting aoa for the analysis of type 1 & 2 polars */
    double m_vMax;          /**< The ending aoa for the analysis of type 1 & 2 polars */
    double m_vDelta;        /**< The aoa increment for the analysis of type 1 & 2 polars */
    bool m_bSequence;           /**< true if the analysis should be performed for a range of input values, false if only a single OpPoint calculation is requested */


    bool m_bError;              /**< true if the analysis couldn't converge within the max number of iterations */
    bool m_bWarning;            /**< true if one the OpPoints could not be properly interpolated */
    bool m_bCancel;                             /**< true if the user has cancelled the analysis */
    bool m_bConverged;                          /**< true if the analysis has converged  */
    bool m_bWingOut;                            /**< true if the interpolation of viscous properties falls outside the polar mesh */

    double m_Ai[MAXSPANSTATIONS+1];                /**< Induced Angle coefficient at the span stations */
    double m_BendingMoment[MAXSPANSTATIONS+1];    /**< bending moment at the span stations */
    double m_CDi;                               /**< The wing's induced drag coefficient */
    double m_CDv;                               /**< The wing's viscous drag coefficient */
    double m_Cl[MAXSPANSTATIONS+1];                /**< Local lift coefficient at the span stations */
    double m_Chord[MAXSPANSTATIONS+1];          /**< chord at the span stations */
    double m_CL;                                /**< The wing's lift coefficient */
    double m_Cm[MAXSPANSTATIONS+1];                /**< Total pitching moment coefficient at the span stations */
    double m_CmAirf[MAXSPANSTATIONS+1];            /**< Airfoil part of the pitching moment coefficient at the span stations */
    double m_GCm;                               /**< The wing's total pitching moment */
    double m_GRm;                               /**< The wing's total rolling moment */
    double m_GYm;                               /**< The wing's total yawing moment */
    double m_ICd[MAXSPANSTATIONS+1];            /**< Induced Drag coefficient at the span stations */
    double m_ICm;                               /**< The wing's induced pitching moment */
    double m_IYm;                               /**< The wing's induced yawing moment */
    QString m_LengthUnit;                       /**< Name of the user-defined length unit */
    double m_Maxa;                              /**< The max value of the difference of induced angle at any span station between two iterations */
    double m_mtoUnit;                           /**< Conversion factor for the display of results in the user-defined length unit*/
    double m_Offset[MAXSPANSTATIONS+1];         /**< offset at  the span stations */
    double m_PCd[MAXSPANSTATIONS+1];            /**< Viscous Drag coefficient at the span stations */
    double m_QInf0;                             /**< The freestream velocity */
    double m_Re[MAXSPANSTATIONS+1];                /**< Reynolds number at the span stations */
    double m_SpanPos[MAXSPANSTATIONS+1];        /**< Span position of the span stations */
    double m_StripArea[MAXSPANSTATIONS+1];        /** <Local strip area at the span stations */
    double m_Twist[MAXSPANSTATIONS+1];          /**< twist at the span stations */
    double m_VCm;                               /**< The wing's viscous pitching moment */
    double m_VYm;                               /**< The wing's viscous yawing moment */
    double m_XCPSpanAbs[MAXSPANSTATIONS+1];        /**< Center of Pressure pos at the span stations */
    double m_XCPSpanRel[MAXSPANSTATIONS+1];        /**< Center of Pressure pos at the span stations */
    double m_XTrTop[MAXSPANSTATIONS+1];            /**< Upper transition location at the span stations */
    double m_XTrBot[MAXSPANSTATIONS+1];            /**< Lower transition location at the span stations */

    Vector3d m_CP;                               /**< The position of the center of pressure */

    int m_nPoints;                              /**< the number of points to calculate in the sequence */

    //    Curve Data
    QVector<double> *m_pX, *m_pY;


    static int s_IterLim;                       /**< The maximum number of iterations in the calculation */
    static int s_NLLTStations;                  /**< The number of LLT stations in the spanwise direction */
    static double s_RelaxMax;                   /**< The relaxation factor for the iterations */
    static double s_CvPrec;                     /**< Precision criterion to stop the iterations. The difference in induced angle at any span point between two iterations should be less than the criterion */
    static bool s_bInitCalc;                    /**< true if the iterations analysis should be intialized with the linear solution at each new a.o.a. calculation, false otherwise */

    QVector<PlaneOpp*> m_PlaneOppList;
    QVector<Polar*> const *m_poaPolar;
};

#endif // LLTANALYSIS_H
