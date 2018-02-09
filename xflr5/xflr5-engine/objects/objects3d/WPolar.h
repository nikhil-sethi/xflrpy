/****************************************************************************

	WPolar Class
	Copyright (C) 2005-2016 Andre Deperrois adeperrois@xflr5.com

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
 * This class defines the polar object for the 3D analysis of wings and planes
 *
 */



#ifndef WPOLAR_H
#define WPOLAR_H




#include <objects3d/Plane.h>

/**
*@brief
*	This class defines the polar object used in 2D and 3D calculations
*
	The class stores both the analysis parameters and the analysis results.

	Each instance of this class is uniquely associated to an instance of a Wing or a Plane object.
	The data is stored in International Standard Units, i.e. meters, seconds, kg, and Newtons.
	Angular data is stored in degrees
*/
#include <QVarLengthArray>


#include <analysis3d_enums.h>
#include "WingOpp.h"
#include "PlaneOpp.h"

#define MAXPOLARPOINTS   5000     /**< The max number of points on a polar. */
#define MAXEXTRADRAG 4
#define MAXCONTROLS 100


class WPolar
{
//	friend class LLTAnalysis;


public:
	WPolar();

	void addPlaneOpPoint(PlaneOpp* pPOpp);
	void replacePOppDataAt(int pos, PlaneOpp *pPOpp);
	void insertPOppDataAt(int pos, PlaneOpp *pPOpp);
	void insertDataAt(int pos, double Alpha, double Beta, double QInf, double Ctrl, double Cl, double CY, double ICd, double PCd, double GCm,
						  double ICm, double VCm, double GRm, double GYm, double IYm, double VYm, double XCP, double YCP,
						  double ZCP, double Cb, double XNP);
	void calculatePoint(int iPt);
	void copy(WPolar *pWPolar);
	void duplicateSpec(WPolar *pWPolar);
	void *getWPlrVariable(int iVar);
	void remove(int i);
	void remove(double alpha);
	void clearData();
	void retrieveInertia(void *ptr);

	bool serializeWPlrWPA(QDataStream &ar, bool bIsStoring);
	bool serializeWPlrXFL(QDataStream &ar, bool bIsStoring);

	XFLR5::enumPolarType &polarType()           {return m_WPolarType;}       /**< returns the type of the polar as an index in the enumeration. */
	XFLR5::enumAnalysisMethod &analysisMethod() {return m_AnalysisMethod;}   /**< returns the analysis method of the polar as an index in the enumeration. */
	bool isLLTMethod()         {return m_AnalysisMethod==XFLR5::LLTMETHOD;}
	bool isVLMMethod()         {return m_AnalysisMethod==XFLR5::VLMMETHOD;}
	bool isPanel4Method()      {return m_AnalysisMethod==XFLR5::PANEL4METHOD;}
	bool isQuadMethod()        {return isPanel4Method() || isVLMMethod();}
	bool isTriCstMethod()      {return m_AnalysisMethod==XFLR5::TRICSTMETHOD;}
	bool isTriLinearMethod()   {return m_AnalysisMethod==XFLR5::TRILINMETHOD;}
	bool isTriangleMethod()    {return isTriCstMethod() || isTriLinearMethod();}



	QString &polarName()                 {return m_WPlrName;}       /**< returns the polar's name as a QString object. */
	QString &planeName()                 {return m_PlaneName;}      /**< returns the name of the polar's parent object as a QString object. */

	double &density()                    {return m_Density;}        /**< returns the fluid's density, in IS units. */
	double &viscosity()                  {return m_Viscosity;}      /**< returns the fluid's kinematic viscosity, in IS units. */

	bool isFixedSpeedPolar() const    {return m_WPolarType==XFLR5::FIXEDSPEEDPOLAR;}      /**< returns true if the polar is of the FIXEDSPEEDPOLAR type, false otherwise >*/
	bool isFixedLiftPolar()  const    {return m_WPolarType==XFLR5::FIXEDLIFTPOLAR;}       /**< returns true if the polar is of the FIXEDLIFTPOLAR type, false otherwise >*/
	bool isFixedaoaPolar()   const    {return m_WPolarType==XFLR5::FIXEDAOAPOLAR;}        /**< returns true if the polar is of the FIXEDAOAPOLAR type, false otherwise >*/
	bool isStabilityPolar()  const    {return m_WPolarType==XFLR5::STABILITYPOLAR;}       /**< returns true if the polar is of the STABILITYPOLAR type, false otherwise >*/
	bool isBetaPolar()       const    {return m_WPolarType==XFLR5::BETAPOLAR;}            /**< returns true if the polar is of the BETAPOLAR type, false otherwise >*/

	bool &bThinSurfaces() {return m_bThinSurfaces;}  /**< returns true if the analysis if using thin surfaces, i.e. VLM, false if 3D Panels for the Wing objects. */
	bool &bWakeRollUp()  {return m_bWakeRollUp;}
	bool &bTilted() {return m_bTiltedGeom; }
	bool &bGround() {return m_bGround;}
	bool &bIgnoreBodyPanels() {return m_bIgnoreBodyPanels;}
	bool &bViscous() {return m_bViscous;}
	bool &bVLM1() {return m_bVLM1;}
	bool &bAutoInertia() {return m_bAutoInertia;}
	bool bDirichlet() {return m_BoundaryCondition==XFLR5::DIRICHLET;}

	int &polarFormat() {return m_PolarFormat;}

	XFLR5::enumBC &boundaryCondition() {return m_BoundaryCondition;}
	XFLR5::enumRefDimension  &referenceDim(){return m_ReferenceDim;}
	double &referenceArea()  {return m_referenceArea;}
	double &referenceSpanLength()  {return m_referenceSpanLength;}
	double &referenceChordLength() {return m_referenceChordLength;}

	double &velocity() {return m_QInfSpec;}
	double &Alpha()    {return m_AlphaSpec;}
	double &Beta()     {return m_BetaSpec;}
	double &sideSlip() {return m_BetaSpec;}       /**< returns the sideslip angle, in degrees. */
	double &Phi()      {return m_BankAngle;}
	double &mass()     {return m_Mass;}
	double &groundHeight() {return m_Height;}

	Vector3d &CoG() {return m_CoG;}
	double &CoGIxx() {return m_CoGIxx;}
	double &CoGIyy() {return m_CoGIyy;}
	double &CoGIzz() {return m_CoGIzz;}
	double &CoGIxz() {return m_CoGIxz;}


	bool &isVisible()      {return m_bIsVisible;}
	int &points()          {return m_PointStyle;}
	int &curveStyle()      {return m_Style;}
	int &curveWidth()      {return m_Width;}
	QColor &curveColor()   {return m_Color;}

	int dataSize(){return m_Alpha.size();}

private:

	int m_Style, m_Width, m_PointStyle;
	bool m_bIsVisible;
	QColor m_Color;

	bool     m_bVLM1;              /**< true if the analysis is performed with horseshoe vortices, flase if quad rings */
//	bool     m_bDirichlet;         /**< true if Dirichlet boundary conditions should be applied, false if Neumann */
	bool     m_bGround;            /**< true if ground effect should be taken into account in the analysis */
	bool     m_bIgnoreBodyPanels;  /**< true if the body panels should be ignored in the analysis */
	bool     m_bThinSurfaces;      /**< true if VLM, false if 3D-panels */
	bool     m_bTiltedGeom;        /**< true if the analysis should be performed on the tilted geometry */
	bool     m_bViscous;           /**< true if the analysis is viscous */
	bool     m_bWakeRollUp;        /**< true if wake roll-up  should be taken into account in the analysis */
	int      m_PolarFormat;        /**< the identification number which references the format used to serialize the data */

	XFLR5::enumBC m_BoundaryCondition;
	XFLR5::enumRefDimension  m_ReferenceDim;        /**< Describes the origin of the refernce area : 1 if planform area, else projected area */

	QString  m_WPlrName;            /**< the polar's name */
	QString  m_PlaneName;          /**< the name of the parent wing or plane */


	double m_referenceArea;          /**< The reference area for the calculation of aero coefficients */
	double m_referenceChordLength;   /**< The reference length = the mean aero chord, for the calculation of aero coefficients */
	double m_referenceSpanLength;    /**< The reference span for the calculation of aero coefficients */

	Vector3d       m_CoG;                  /**< the position of the CoG */
	double        m_Density;              /**< The fluid's density */
	double        m_Mass;                 /**< The mass for type 2 and type 7 polars */

	XFLR5::enumAnalysisMethod m_AnalysisMethod;  /**< The method used for the analysis. May be one of the following types : LLTMETHOD, VLMMETHOD, PANELMETHOD */
	XFLR5::enumPolarType      m_WPolarType;      /**< The type of analysis. May be one of the following types :FIXEDSPEEDPOLAR, FIXEDLIFTPOLAR, FIXEDAOAPOLAR, STABILITYPOLAR */

public:
	bool     m_bRelaxWake;         /**< true if wake relaxation is implemented */
	bool     m_bAutoInertia;       /**< true if the inertia to be taken into account is the one of the parent plane */
	double   m_CoGIxx;             /**< The Ixx component of the inertia tensor, w.r.t. the CoG origin */
	double   m_CoGIxz;             /**< The Ixz component of the inertia tensor, w.r.t. the CoG origin */
	double   m_CoGIyy;             /**< The Iyy component of the inertia tensor, w.r.t. the CoG origin */
	double   m_CoGIzz;             /**< The Izz component of the inertia tensor, w.r.t. the CoG origin */

	double   m_inertiaGain[7];

	int      m_nControls;          /**< the number of control surfaces for this wing or plane */
	int      m_NXWakePanels;       /**< the number of wake panels in each streamwise column */
	double   m_AlphaSpec;          /**< the angle of attack for type 4 & 5 polars */
	double   m_BetaSpec;           /**< The sideslip angle for type 1,2, 4 polars */
	double   m_BankAngle;          /**< The bank angle */
	double   m_QInfSpec;           /**< the freestream velocity for type 1 & 5 polars */
	double   m_Height;             /**< The plane flight altitude, used if ground effect is to be taken into account*/
	double   m_Viscosity;          /**< The fluid's kinematic viscosity */

	double   m_WakePanelFactor;    /**< the ratio between the length of two wake panels in the x direction */
	double   m_TotalWakeLength;    /**< the wake's length x MAC; defines the position of the Trefftz plane */

	QVarLengthArray<double> m_ControlGain;      /**< the scaling factor for each of the control surfaces */

	QList <double>  m_1Cl;        /**< 1/Cl, special for Matthieu */
	QList <double>  m_Alpha;      /**< the angle of attack */
	QList <double>  m_Beta;       /**< the sideslip angle */
	QList <double>  m_QInfinite;  /**< the free stream speed - type 2 WPolars */
	QList <double>  m_Cl32Cd;     /**< the power factor */
	QList <double>  m_ClCd;       /**< the glide ratio */
	QList <double>  m_CL;         /**< lift coef. */

	QList <double>  m_Ctrl;       /**< Ctrl variable */
	QList <double>  m_CY;         /**< Side Force */
	QList <double>  m_FX;         /**< the total drag */


	QList <double>  m_FY;         /**< the total side force */
	QList <double>  m_FZ;         /**< the total wing lift */
	QList <double>  m_Gamma;      /**< glide angle = Atan(Cx/Cz), in degrees */
	QList <double>  m_GCm;        /**< Total Pitching Moment coefficient */
	QList <double>  m_GRm;        /**< Total rolling moment */

	QList <double>  m_GYm;        /**< Total yawing moment coefficient */
	QList <double>  m_ICd;        /**< induced drag coef. */
	QList <double>  m_ICm;        /**< Induced Pitching Moment coefficient */
	QList <double>  m_IYm;        /**< induced yawing moment coefficient */
	QList <double>  m_Rm;         /**< the total rolling moment */
	QList <double>  m_Pm;         /**< the total pitching moment */
	QList <double>  m_MaxBending; /**< the max bending moment at the root chord */

	QList <double>  m_Oswald;     /**< Oswald's efficiency factor */
	QList <double>  m_PCd;        /**< profile drag coef. */

	complex<double> m_EigenValue[8][MAXPOLARPOINTS]; /**< until we have a QList<complex<double>> ? */
	QList <double>  m_PhugoidFrequency;        /**< the phugoid's frequency, as a result of stability analysis only */
	QList <double>  m_PhugoidDamping;          /**< the phugoid's damping factor, as a result of stability analysis only */
	QList <double>  m_RollDampingT2;           /**< the time to double or half for the damping of the roll-damping mode, as a result of stability analysis only */
	QList <double>  m_ShortPeriodDamping;      /**< the damping of the short period mode, as a result of stability analysis only */
	QList <double>  m_ShortPeriodFrequency;    /**< the frequency of the short period mode, as a result of stability analysis only */
	QList <double>  m_DutchRollDamping;        /**< the damping of the Dutch roll mode, as a result of stability analysis only */
	QList <double>  m_DutchRollFrequency;      /**< the frequency of the Dutch roll mode, as a result of stability analysis only */
	QList <double>  m_SpiralDampingT2;         /**< the time to double or half for the damping of the spiral mode, as a result of stability analysis only >*/

	QList <double>  m_XCpCl;                   /**< XCp.Cl, used in calculation of neutral point position >*/
	QList <double>  m_SM;                      /**< (XCP-XCmRef)/m.a.c; >*/
	QList <double>  m_TCd;                     /**< the total drag coeficient >*/
	QList <double>  m_VCm;                     /**< the viscous Pitching Moment coefficient >*/
	QList <double>  m_VertPower;               /**< the power for steady horizontal flight = m.g.Vz >*/
	QList <double>  m_HorizontalPower;         /**< the power for steady horizontal flight = Fx.Vx >*/

	QList <double>  m_Vx;         /**< the horizontal component of the velocity */
	QList <double>  m_VYm;        /**< Profile yawing Moment coefficient */
	QList <double>  m_Vz;         /**< the sink speed = sqrt(2mg/rho/S)/powerfactor */
	QList <double>  m_XCP;        /**< the centre of pressure X-position relative to the wing's root LE */
	QList <double>  m_XNP;        /**< the position of the neutral point, as a result of stability analysis only */

	QList <double>  m_YCP;        /**< the centre of pressure Y-position relative to the wing's root LE */
	QList <double>  m_Ym;         /**< the total yawing moment */
	QList <double>  m_ZCP;        /**< the centre of pressure Z-position relative to the wing's root LE */

	QList <double>  m_ExtraDrag;  /**< the custom extra drag in addition to the induced and viscous drag parts */

	QList <double>  m_Mass_var;   /**< the mass calculated as ref_mass + gain*control */
	QList <double>  m_CoG_x;      /**< the CoG position calculated as ref_CoG_x + gain*control */
	QList <double>  m_CoG_z;      /**< the CoG position calculated as ref_CoG_z + gain*control */

	double m_ExtraDragArea[MAXEXTRADRAG], m_ExtraDragCoef[MAXEXTRADRAG];

	double m_XNeutralPoint;       /**< Neutral point position, calculated from d(XCp.Cl)/dCl >*/
};

#endif
