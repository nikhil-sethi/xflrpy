/****************************************************************************

        PlaneOpp Class
    Copyright (C) 2006-2019 André Deperrois

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
 * This class defines the operating point object for the 3D analysis of planes
 *
 */


#pragma once


#include <QDataStream>

#include <xflobjects/objects3d/wingopp.h>
#include <xflobjects/xflobject.h>

class Plane;
class WPolar;

//using namespace std;

/**
*@brief
*    This class defines the operating point object which stores the data of plane analysis
*
        Each instance of this class is uniquely associated to an instance of a WPolar, which is itself uniquely
        associated to a Wing or a Plane object.
        The results associated to each of the plane's wing is stored in WingOpp objects, declared as member variables.
        The data is stored in International Standard Units, i.e. meters, seconds, kg, and Newtons.
        Angular data is stored in degrees.
*/
class PlaneOpp : public XflObject
{
    friend class Objects3D;
    friend class Miarex;
    friend class MainFrame;
    friend class WPolar;
    friend class PanelAnalysis;
    friend class LLTAnalysis;

    public:
        PlaneOpp(Plane *pPlane=nullptr, WPolar *pWPolar=nullptr, int PanelArraySize=0);
        ~PlaneOpp();

        QString name() const override;

        void addWingOpp(int iw, int PanelArraySize);
        WingOpp const *wingOpp(int index) const {if(index<0||index>=MAXWINGS) return nullptr; else return m_pWOpp[index];}
        void allocateMemory(int PanelArraySize);
        void releaseMemory();


        double alpha() const {return m_Alpha;}
        double beta()  const {return m_Beta;}
        double phi()   const {return m_Bank;}
        double ctrl()  const {return m_Ctrl;}
        double QInf()  const {return m_QInf;}
        void setAlpha(double a) {m_Alpha=a;}
        void setBeta(double b) {m_Beta=b;}
        void setPhi(double f) {m_Bank=f;}
        void setCtrl(double c) {m_Ctrl=c;}
        void setQInf(double q) {m_QInf=q;}

        xfl::enumAnalysisMethod analysisMethod() const {return m_AnalysisMethod;}
        bool isLLTMethod() const {return m_AnalysisMethod==xfl::LLTMETHOD;}

        QString const &planeName() const {return m_PlaneName;}
        QString const &polarName()const {return m_WPlrName;}
        void setPlaneName(QString const &name) {m_PlaneName=name;}
        void setPolarName(QString const &name) {m_WPlrName=name;}

        xfl::enumPolarType const& polarType() const {return m_WPolarType;}
        void setPolarType(xfl::enumPolarType type) {m_WPolarType=type;}

        bool isOut() const {return m_bOut;}

        bool isT1Polar() const {return m_WPolarType==xfl::FIXEDSPEEDPOLAR;}   /**< returns true if the polar is of the FIXEDSPEEDPOLAR type, false otherwise >*/
        bool isT2Polar() const {return m_WPolarType==xfl::FIXEDLIFTPOLAR;}   /**< returns true if the polar is of the FIXEDLIFTPOLAR type, false otherwise >*/
        bool isT4Polar() const {return m_WPolarType==xfl::FIXEDAOAPOLAR;}   /**< returns true if the polar is of the FIXEDLIFTPOLAR type, false otherwise >*/
        bool isT5Polar() const {return m_WPolarType==xfl::BETAPOLAR;}   /**< returns true if the polar is of the STABILITYPOLAR type, false otherwise >*/
        bool isT7Polar() const {return m_WPolarType==xfl::STABILITYPOLAR;}   /**< returns true if the polar is of the STABILITYPOLAR type, false otherwise >*/

        bool serializePOppWPA(QDataStream &ar, bool bIsStoring);
        bool serializePOppXFL(QDataStream &ar, bool bIsStoring);

        void getProperties(QString &PlaneOppProperties, QString lengthUnitLabel, QString massUnitLabel, QString speedUnitLabel,
                           double mtoUnit, double kgtoUnit, double mstoUnit);


        bool isStabilityPOpp() const {return m_WPolarType==xfl::STABILITYPOLAR;}

        static bool storePOpps() {return s_bStoreOpps;}
        static bool keepOutPOpps() {return s_bKeepOutOpps;}

    private:
        xfl::enumAnalysisMethod m_AnalysisMethod;   /**< defines by which type of method (LLT, VLM, PANEL), this WingOpp was calculated */

        QString m_PlaneName;       /**< the pPane's name to which the PlaneOpp is attached */
        QString m_WPlrName;         /**< the WPolar's name to which the PlaneOpp is attached */


        double m_Alpha;            /**< the angle of attack*/
        double m_Beta;             /**< the sideslip angle */
        double m_Bank;             /**< the bank angle */
        double m_Ctrl;             /**< the value of the control variable */

        double m_Span;             /**< the parent's Wing span */
        double m_MAChord;          /**< the parent's Wing mean aerodynamic chord*/
        int m_NStation;            /**< unused */


        //    bool m_bWing[MAXWINGS];    /**< true if respectively a main wing, 2nd wing, elevator, fin are part of the parent Plane object */
        bool m_bVLM1;              /**<  true if the PlaneOpp is the result of a horseshoe VLM analysis */
        bool m_bOut;               /**<  true if the interpolation of viscous properties was outside the Foil Polar mesh */

    public:
        xfl::enumPolarType m_WPolarType;   /**< defines the type of the parent WPolar */
        WingOpp *m_pWOpp[MAXWINGS];      /**< An array of pointers to the four WingOpp objects associated to the four wings */
        double m_QInf;                        /**< the freestream velocity */
        double *m_dG;                         /**< the VLM vortex strengths, or the panel's doublet's strengths */
        double *m_dSigma;                     /**< the panel's source strengths */
        double *m_dCp;                        /**< the array of Cp coefficients */
        int m_NPanels;                        /**<  the number of VLM or 3D-panels */
        int m_nControls;                      /**< the number of control surfaces associated to the WingOpp */

        std::complex<double> m_EigenValue[8];      /**< the eigenvalues of the four longitudinal and four lateral modes */
        std::complex<double> m_EigenVector[8][4];  /**< the longitudinal and lateral eigenvectors (4 longitudinal + 4 lateral) x 4 components */

        std::complex<double> m_phiPH;      /**< complex phugoid eigenvalue computed using Phillip's formula */
        std::complex<double> m_phiDR;      /**< complex Dutch roll eigenvalue computed using Phillip's formula */

        //non dimensional stability derivatives
        double CXu, CZu, Cmu, CXa, CLa, Cma, CXq, CLq, Cmq, CYb, CYp, CYr, Clb, Clp, Clr, Cnb, Cnp, Cnr;
        //non-dimensional control derivatives
        double CXe,CYe,CZe;
        double CLe,CMe,CNe;

        double m_ALong[4][4];    /**< the longitudinal state matrix */
        double m_ALat[4][4];    /**< the lateral state matrix */
        double m_BLong[4];      /**< the longitudinal control vector */
        double m_BLat[4];       /**< the lateral control vector */

        double m_XNP;         /**< the neutral point position resulting from a stability calculations  */

        bool m_bThinSurface;        /**< true if the WingOpp is the results of a calculation on the middle surface */
        bool m_bTiltedGeom;         /**< true if the WingOpp is the results of a calculation on the tilted geometry */

        double m_Weight;

        double m_CL;          /**< the wing lift coefficient */
        double m_CX;          /**< the total drag coefficient */
        double m_CY;          /**< the side force coefficient */

        double m_VCD;         /**< the wing viscous drag coefficient */
        double m_ICD;         /**< the wing induced drag coefficient */

        double m_GCm;         /**< the wing pitching moment */
        double m_VCm;         /**< the pitching moment induced by the viscous drag forces */
        double m_ICm;         /**< the pitching moment induced by the pressure forces */

        double m_GRm;         /**< the wing rolling moment */

        double m_GYm;         /**< the total yawing moment */
        double m_VYm;         /**< the wing viscous yawing moment */
        double m_IYm;         /**< the wing induced yawing moment */

        Vector3d m_CP;         /**< the position of the centre of pressure */

        static bool s_bStoreOpps;       /**< true if the OpPoints should be added to the array at the end of the analysis*/
        static bool s_bKeepOutOpps;     /**< true if points with viscous propertiesinterpolated out of the polar mesh should be kept */

};
