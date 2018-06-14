/****************************************************************************

	PlaneOpp Class
	Copyright (C) 2006-2016 Andre Deperrois 

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


#ifndef PLANEOPP_H
#define PLANEOPP_H


#include <QDataStream>

#include <objects/objects3d/WingOpp.h>
#include <objects/objectcolor.h>



//using namespace std;

/**
*@brief
*	This class defines the operating point object which stores the data of plane analysis
*
	Each instance of this class is uniquely associated to an instance of a WPolar, which is itself uniquely
	associated to a Wing or a Plane object.
	The results associated to each of the plane's wing is stored in WingOpp objects, declared as member variables.
	The data is stored in International Standard Units, i.e. meters, seconds, kg, and Newtons.
	Angular data is stored in degrees.
*/
class XFLR5ENGINELIBSHARED_EXPORT PlaneOpp
{
	friend class Objects3D;
	friend class Miarex;
	friend class MainFrame;
	friend class WPolar;
	friend class PanelAnalysis;
	friend class LLTAnalysis;

public:
	PlaneOpp(void *pPlanePtr=NULL, void *pWPolarPtr=NULL, int PanelArraySize=0);
	~PlaneOpp();


	void addWingOpp(int iw, int PanelArraySize);

	void allocateMemory(int PanelArraySize);
	void releaseMemory();


	double &alpha() {return m_Alpha;}
	double &beta()  {return m_Beta;}
	double &phi()   {return m_Bank;}
	double &ctrl()  {return m_Ctrl;}
	double &QInf()  {return m_QInf;}


	XFLR5::enumAnalysisMethod analysisMethod() {return m_AnalysisMethod;}
	bool isLLTMethod(){return m_AnalysisMethod==XFLR5::LLTMETHOD;}

	QString &planeName(){return m_PlaneName;}
	QString &polarName(){return m_WPlrName;}
	XFLR5::enumPolarType polarType(){return m_WPolarType;}

	bool isOut(){return m_bOut;}

	bool &isVisible()      {return m_bIsVisible;}
	int &points()          {return m_PointStyle;}
	int &style()           {return m_Style;}
	int &width()           {return m_Width;}
	ObjectColor color()  const  {return m_Color;}
	void setPlaneOppColor(ObjectColor colour)  {m_Color = colour;}

	bool serializePOppWPA(QDataStream &ar, bool bIsStoring);
	bool serializePOppXFL(QDataStream &ar, bool bIsStoring);

	void getPlaneOppProperties(QString &PlaneOppProperties, QString lengthUnitLabel, QString massUnitLabel, QString speedUnitLabel,
										 double mtoUnit, double kgtoUnit, double mstoUnit);


	bool isStabilityPOpp(){return m_WPolarType==XFLR5::STABILITYPOLAR;}

    static bool storePOpps() {return s_bStoreOpps;}
    static bool keepOutPOpps() {return s_bKeepOutOpps;}

private:
	XFLR5::enumAnalysisMethod m_AnalysisMethod;   /**< defines by which type of method (LLT, VLM, PANEL), this WingOpp was calculated */

	QString m_PlaneName;       /**< the pPane's name to which the PlaneOpp is attached */
	QString m_WPlrName;         /**< the WPolar's name to which the PlaneOpp is attached */

	int m_Style, m_Width, m_PointStyle;
	bool m_bIsVisible;
	ObjectColor m_Color;

	double m_Alpha;            /**< the angle of attack*/
	double m_Beta;             /**< the sideslip angle */
	double m_Bank;             /**< the bank angle */
	double m_Ctrl;             /**< the value of the control variable */
	
	double m_Span;             /**< the parent's Wing span */
	double m_MAChord;          /**< the parent's Wing mean aerodynamic chord*/
	int m_NStation;            /**< unused */


//	bool m_bWing[MAXWINGS];    /**< true if respectively a main wing, 2nd wing, elevator, fin are part of the parent Plane object */
	bool m_bVLM1;              /**<  true if the PlaneOpp is the result of a horseshoe VLM analysis */
	bool m_bOut;               /**<  true if the interpolation of viscous properties was outside the Foil Polar mesh */

public:
	XFLR5::enumPolarType m_WPolarType;   /**< defines the type of the parent WPolar */
	WingOpp *m_pPlaneWOpp[MAXWINGS];      /**< An array of pointers to the four WingOpp objects associated to the four wings */
	double m_QInf;                        /**< the freestream velocity */
	double *m_dG;                         /**< the VLM vortex strengths, or the panel's doublet's strengths */
	double *m_dSigma;                     /**< the panel's source strengths */
	double *m_dCp;                        /**< the array of Cp coefficients */
	int m_NPanels;                        /**<  the number of VLM or 3D-panels */
	int m_nControls;	                  /**< the number of control surfaces associated to the WingOpp */

	complex<double> m_EigenValue[8];      /**< the eigenvalues of the four longitudinal and four lateral modes */
	complex<double> m_EigenVector[8][4];  /**< the longitudinal and lateral eigenvectors (4 longitudinal + 4 lateral) x 4 components */

	complex<double> m_phiPH;      /**< complex phugoid eigenvalue computed using Phillip's formula */
	complex<double> m_phiDR;      /**< complex Dutch roll eigenvalue computed using Phillip's formula */

	//non dimensional stability derivatives
	double CXu, CZu, Cmu, CXa, CLa, Cma, CXq, CLq, Cmq, CYb, CYp, CYr, Clb, Clp, Clr, Cnb, Cnp, Cnr;
	//non-dimensional control derivatives
	double CXe,CYe,CZe;
	double CLe,CMe,CNe;

	double m_ALong[4][4];	/**< the longitudinal state matrix */
	double m_ALat[4][4];	/**< the lateral state matrix */
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
#endif
