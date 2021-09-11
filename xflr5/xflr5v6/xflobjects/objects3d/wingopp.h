/****************************************************************************

    WingOpp Class
    Copyright (C) 2005-2016 André Deperrois

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



/** @file
 *
 * This class defines the operating point object for the 3D analysis of wings.
 *
 */


#ifndef WINGOPP_H
#define WINGOPP_H



/**
*@brief
*    This class implements the operating point object which stores the data of plane analysis
*
    In the case of an analysis of an independant wing, the instance of this WingOpp class is
    uniquely associated to an instance of a WPolar, which is itself uniquely associated to the Wing object.
    Alternatively, the WingOpp may be a member variable of a PlaneOpp object.
    The data is stored in International Standard Units, i.e. meters, seconds, kg, and Newtons.
    Angular data is stored in degrees.
*/

#include <QString>
#include <QVector>
#include <QTextStream>

#include <complex>


#include <xflanalysis/analysis3d_params.h>
#include <xflcore/core_enums.h>
#include <xflgeom/geom3d/vector3d.h>

class Wing;
class WPolar;

class WingOpp
{
    friend class Miarex;
    friend class MainFrame;
    friend class WPolar;
    friend class PlaneOpp;
    friend class StabViewDlg;
    friend class PanelAnalysisDlg;
    friend class PanelAnalysis;
    friend class LLTAnalysis;
    friend class ObjectPropsDlg;

public:
    WingOpp(int PanelArraySize=0);
    ~WingOpp();

private:
    //________________METHODS____________________________________

    bool serializeWingOppWPA(QDataStream &ar, bool bIsStoring);
    bool serializeWingOppXFL(QDataStream &ar, bool bIsStoring);
    bool exportWOpp(QTextStream &out, bool bCSV);

    double maxLift() const;
    void createWOpp(Wing const *pWing, WPolar const*pWPolar);

private:
    QString m_WingName;    // the wing name to which the WingOpp belongs
    QString m_PlrName;     // the polar name to which the WingOpp belongs

    int m_nWakeNodes;            /**< the number of wake nodes */
    int m_NXWakePanels;            /**< the number of wake panels */
    int m_nControls;            /**< the number of control surfaces associated to the WingOpp */

    double m_FirstWakePanel;    /**< the size of the first wake panel in the array. */
    double m_WakeFactor;        /**< the geometric progression factor of the panel lengths in a column of wake panels */
    double m_Weight;



public:
    bool m_bOut;        /**< true if there was an interpolation error of the viscous properties for this WingOpp */

    int m_NVLMPanels;    /**< the number of panels */
    int m_NStation;        /**< the number of stations along the span */
    int m_nFlaps;        /**< the number of trailing edge flaps */

    xfl::enumAnalysisMethod m_AnalysisMethod;   /**< defines by which type of method (LLT, VLM, PANEL), this WingOpp was calculated */

    double *m_dCp;                           /**< a pointer to the array of pressure coefficient for each panel */
    double *m_dG;                            /**< a pointer to the array of vortice or doublet strengths */
    double *m_dSigma;                        /**< a pointer to the array of source strengths */


    double m_Span;                          /**< the parent's Wing span */
    double m_MAChord;                       /**< the parent's Wing mean aerodynamic chord*/
    double m_QInf;                          /**< the freestream velocity */
    double m_Alpha;                         /**< the aoa */
    double m_Beta;                          /**< the sideslip angle */
    double m_Phi;                           /**< the bank angle */

    double m_CL;          /**< the wing lift coefficient */
    double m_CX;          /**< the total drag coefficient */
    double m_CY;          /**< the side force coefficient */
    double m_VCD;         /**< the wing's viscous drag coefficient */
    double m_ICD;         /**< the wing's induced drag coefficient */
    double m_GCm;         /**< the wing's pitching moment */
    double m_GRm;         /**< the wing's rolling moment */
    double m_VCm;         /**< the pitching moment induced by the viscous drag forces */
    double m_ICm;         /**< the pitching moment induced by the pressure forces */
    double m_GYm;         /**< the total yawing moment */
    double m_VYm;         /**< the wing's viscous yawing moment */
    double m_IYm;         /**< the wing's induced yawing moment */
    double m_MaxBending;  /**< the bending moment at the root chord */

    double m_oldCtrl;        /**< the value of the control parameterfor stability polars. @deprecated, kept for compatibility with former .wpa files */

    Vector3d m_CP;         /**< the position of the centre of pressure */

    double m_Chord[MAXSPANSTATIONS+1];      /**< the chord lengths at stations */
    double m_Twist[MAXSPANSTATIONS+1];        /**< the twist at span stations */

    double m_SpanPos[MAXSPANSTATIONS+1];    /**< the spanwise position of the stations */
    double m_Ai[MAXSPANSTATIONS+1];         /**< the calculated induced angles, in degrees */
    double m_ICd[MAXSPANSTATIONS+1];        /**< the induced drag coefficient at stations */
    double m_PCd[MAXSPANSTATIONS+1];        /**< the viscous drag coefficient at stations */
    double m_Cl[MAXSPANSTATIONS+1];         /**< the lift coefficient at stations */
    double m_XCPSpanRel[MAXSPANSTATIONS+1]; /**< the relative position of the centre of pressure on each chordwise strip, in chord % */
    double m_XCPSpanAbs[MAXSPANSTATIONS+1]; /**< the absolute position of the centre of pressure on each chordwise strip */
    double m_StripArea[MAXSPANSTATIONS+1];  /**< the area of the chordwise strips */
    double m_XTrTop[MAXSPANSTATIONS+1];     /**< the transition location on the top surface*/
    double m_XTrBot[MAXSPANSTATIONS+1];     /**< the transition location on the bottom suface */

    QVector<double> m_FlapMoment;   /**< the flap hinge moments */


    double m_Re[MAXSPANSTATIONS+1];              /**< the calculated Reynolds number at span stations */
    double m_Cm[MAXSPANSTATIONS+1];              /**< the total pitching moment coefficient at span stations */
    double m_CmAirf[MAXSPANSTATIONS+1];          /**< the airfoil pitching moment coefficient about 1/4 chord point, at span stations */
    double m_BendingMoment[MAXSPANSTATIONS+1];   /**< the bending moment at span stations */

    QVector<Vector3d> m_Vd;          /**< the downwash at the trailing edge */
    QVector<Vector3d> m_F;           /**< the force acting on the chordwise = sum of the panel forces on this strip*/

    std::complex<double> m_oldEigenValue[8];      /**< the eigenvalues of the four longitudinal and four lateral modes. @deprecated, kept for compatibility with former .wpa files */
    std::complex<double> m_oldEigenVector[8][4];  /**< the longitudinal and lateral eigenvectors (4 longitudinal + 4 lateral) x 4 components. @deprecated, kept for compatibility with former .wpa files */

};
#endif
