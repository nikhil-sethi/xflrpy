/****************************************************************************

	Wing Class
	Copyright (C) 2005-2016 Andre Deperrois 

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

/** @file This file implements the Wing class.
 */


#ifndef WING_H
#define WING_H



#include <QVarLengthArray>
#include <objects/objectcolor.h>
#include <objects/objects3d/vector3d.h>
#include <objects/objects3d/Panel.h>
#include <objects/objects3d/WingSection.h>
#include <objects/objects2d/Foil.h>
#include <objects/objects2d/Polar.h>

/**
 * @class Wing
 * @brief  This class defines the wing object, provides the methods for the calculation of the wing geometric properties, and
		   provides methods for LLT, VLM and Panel methods.
 *
 * A wing is identified and reference by its name, which is used  by the parent Plane objects and by child Polar and WingOPpp objects.
 *
 * The wing is defined by geometric data at a finite number of span-stations : chord, chord position, foil, dihedral and washout.
 *
 * The panel between two span-stations is a WingSection object. The term "panel" is reserved for the mesh panels used to perform a VLM or Panel analysis. 
 * @todo check variable names for consistency with this rule.
 *
 * The term stations is reserved for the spanwise points used by the LLT, or by the spanwise position of the panels in a VLM or panel analysis.
 * During an LLT or a Panel analysis, the value of the aerodynamic parameters of interest are stored in the member variables of this Wing object.
 * The data is retrieved and stored in an operating point at the end of the analysis.
 * The choice to include the wing-specific methods used in the analysis in this wing class is arbitrary. It simplifies somewhat the implementation
 * of the LLTAnalysis and PanelAnalysisDlg classes, but makes this Wing class more complex.
 *
 * The data is stored in International Standard Units, i.e. meters, kg, and seconds. Angular data is stored in degrees.
*/

class PointMass;
class WPolar;
class Surface;
class Panel;

class XFLR5ENGINELIBSHARED_EXPORT Wing
{
	friend class Objects3D;
	friend class Miarex;
	friend class MainFrame;
	friend class WPolar;
	friend class PlaneOpp;
	friend class WingOpp;
	friend class Plane;
	friend class Surface;
	friend class WingDlg;
	friend class PlaneAnalysisTask;
	friend class LLTAnalysisDlg;
	friend class LLTAnalysis;
	friend class PanelAnalysisDlg;
	friend class PanelAnalysis;
	friend class PlaneDlg;
	friend class ImportWingDlg;
	friend class WPolarDlg;
	friend class StabPolarDlg;
	friend class GL3dWingDlg;
	friend class ManageUFOsDlg;
	friend class InertiaDlg;
	friend class StabViewDlg;
	friend class EditPlaneDlg;
	friend class EditBodyDlg;
	friend class gl3dView;
	friend class XMLPlaneReader;


public:

	Wing();
	~Wing();

	bool importDefinition(QString path_to_file, QString errorMessage);
	bool exportDefinition(QString path_to_file, QString errorMessage);

	void createSurfaces(Vector3d const &T, double XTilt, double YTilt);//generic surface, LLT, VLM or Panel

	int  VLMPanelTotal(bool bThinSurface);
	void VLMSetBending();

	void panelComputeOnBody(double QInf, double Alpha, double *Cp, double *Gamma, double &XCP, double &YCP, double &ZCP,
						 double &GCm, double &VCm, double &ICm, double &GRm, double &GYm, double &VYm,double &IYm,
						 WPolar *pWPolar, Vector3d CoG);


	void panelComputeViscous(double QInf, WPolar *pWPolar, double &WingVDrag, bool bViscous, QString &OutString);
	void panelComputeBending(bool bThinSurface);

	bool isWingPanel(int nPanel);
	bool isWingNode(int nNode);

	void getFoils(Foil **pFoil0, Foil **pFoil1, double y, double &t);
	void duplicate(Wing *pWing);
	void computeChords(int NStation=0);
	void computeChords(int NStation, double *chord, double *offset, double *twist);
	void computeGeometry();
	void computeVolumeInertia(Vector3d &CoG, double &CoGIxx, double &CoGIyy, double &CoGIzz, double &CoGIxz);
	void computeBodyAxisInertia();

	bool intersectWing(Vector3d O,  Vector3d U, Vector3d &I);

	void scaleSweep(double NewSweep);
	void scaleTwist(double NewTwist);
	void scaleSpan(double NewSpan);
	void scaleChord(double NewChord);
	void scaleArea(double newArea);
	void scaleAR(double newAR);
	void scaleTR(double newTR);

	void surfacePoint(double xRel, double ypos, enumPanelPosition pos, Vector3d &Point, Vector3d &PtNormal);

	bool isWingOut()      {return m_bWingOut;}
	bool &isFin()          {return m_bIsFin;}
	bool &isDoubleFin()    {return m_bDoubleFin;}
	bool &isSymFin()       {return m_bSymFin;}

	void insertSection(int iSection);
	bool appendWingSection();
	bool appendWingSection(double Chord, double Twist, double Pos, double Dihedral, double Offset, int NXPanels, int NYPanels,
						   XFLR5::enumPanelDistribution XPanelDist, XFLR5::enumPanelDistribution YPanelDist, QString RightFoilName, QString LeftFoilName);
	void removeWingSection(int const iSection);
	
	void clearWingSections();
	void clearPointMasses();
	void clearSurfaces();

	//access methods
	int NWingSection() {return m_WingSection.count();}
	int &NXPanels(const int &iSection);
	int &NYPanels(const int &iSection);
	int NYPanels();

	XFLR5::enumPanelDistribution &XPanelDist(const int &iSection);
	XFLR5::enumPanelDistribution &YPanelDist(const int &iSection);

	bool isWingFoil(Foil *pFoil);
	double rootChord()     {return m_WingSection.first()->m_Chord;}
	double rootOffset()    {return m_WingSection.first()->m_Offset;}
	double tipChord()      {return m_WingSection.last()->m_Chord;}
	double tipTwist()      {return m_WingSection.last()->m_Twist;}
	double tipOffset()     {return m_WingSection.last()->m_Offset;}
	double tipPos()        {return m_WingSection.last()->m_YPosition;}
	double planformSpan()  {return m_PlanformSpan;}
	double projectedSpan() {return m_ProjectedSpan;}

	double &Chord(const int &iSection);
	double &Twist(const int &iSection);
	double &YPosition(const int &iSection);
	double &Dihedral(const int &iSection);
	double &Offset(const int &iSection);
	double &Length(const int &iSection);
	double &YProj(const int &iSection);
	double &ZPosition(const int &iSection);

	double yrel(double SpanPos);
	double getChord(double yob);
	double getOffset(double yob);
	double getDihedral(double yob);
	double getTwist(double y);
	double averageSweep();
	double &volumeMass() {return m_VolumeMass;}
	double totalMass();
	double C4(double yob, double xRef);
	double ZPosition(double y);
	double Beta(int m, int k);

	bool &isSymetric() {return m_bSymetric;}

	int &nFlaps() {return m_nFlaps;}

	static double minPanelSize(){return s_MinPanelSize;}

	Vector3d CoG() {return m_CoG;}
	const QString& wingName() const {return m_WingName;}
	QString& rWingName() {return m_WingName;}
	const QString& WingDescription() const {return m_WingDescription;}
	QString &rightFoil(const int &iSection);
	QString &leftFoil(const int &iSection);
	QString& rWingDescription() {return m_WingDescription;}

	bool &textures(){return m_bTextures;}

	ObjectColor wingColor() {return m_WingColor;}
	void setWingColor(ObjectColor colour){m_WingColor= colour;}

	XFLR5::enumWingType &wingType() {return m_WingType;}

	void getTextureUV(int iSurf, double *leftV, double *rightV, double &leftU, double &rightU, int nPoints);

	bool serializeWingWPA(QDataStream &ar, bool bIsStoring);
	bool serializeWingXFL(QDataStream &ar, bool bIsStoring);

	void exportSTLBinary(QDataStream &outStream, int CHORDPANELS, int SPANPANELS);
	void exportSTLText(QTextStream &outStream, int CHORDPANELS, int SPANPANELS);

	Foil* foil(QString strFoilName);

    double IntegralC2(double y1, double y2, double c1, double c2);
    double IntegralCy(double y1, double y2, double c1, double c2);


	static double getInterpolatedVariable(int nVar, Foil *pFoil0, Foil *pFoil1, double Re, double Cl, double Tau, bool &bOutRe, bool &bError);
	static double getPlrPointFromCl(Foil *pFoil, double Re, double Cl, int PlrVar, bool &bOutRe, bool &bError);

//__________________________Variables_______________________
private:
	QString m_WingName;	                       /**< the Wing's name; this name is used to identify the wing and as a reference for child Polar and WingOpp objects. */
	QString m_WingDescription;                 /**< a text field for the description of the Wing */
	ObjectColor m_WingColor;                        /**< the Wing's display color */

	XFLR5::enumWingType m_WingType;  /** Defines the type of wing on the plane : main, second, elevator, fin, other */

	static double s_MinPanelSize;      /**< wing minimum panel size ; panels of less length are ignored */

	bool m_bTextures;
	bool m_bWingOut;	             /**< true if the wing OpPoint is outside the flight envelope of the available Type 1 polar mesh */
	bool m_bSymetric;	             /**< true if the wing's geometry is symetric */
	bool m_bIsFin;                   /**< true if this wing describes a fin */

	bool m_bDoubleFin;               /**< true if the wing describes a double fin symetric about the y=0 plane */
	bool m_bSymFin;                  /**< true if the wing describes a double fin symetric about the z=0 plane */
//	double m_bDoubleSymFin;          /**< true if the fin is both double and symetric */

	int m_NStation;                  /**< the number of stations for wing calculation; either the number of points of LLT, or the number of spanwise panels  */
//	int m_nNodes;                    /**< the number of nodes of the panel mesh */
//	int m_AVLIndex;                  /**< a random identification number needed to export to AVL */

	int m_nFlaps;                    /**< the number of T.E. flaps, numbered from left wing to right wing; for a main wing this number is even*/
	QList<double> m_FlapMoment;      /**< the flap moments resulting from the panel of VLM analysis */

	double m_QInf0;                  /**< the freestream velocity */

	double m_VolumeMass;             /**< the mass of the Wing's structure, excluding point masses */
	double m_TotalMass;              /**< the Wing's total mass, i.e. the sum of the volume mass and of the point masses */

	double m_GChord;                 /**< the mean geometric chord */
	double m_yMac;                   /**< the mean aerodynamic chord span position  -  @todo meaning and calculation are unclear*/
	double m_CDi;                    /**< the wing's induced drag coefficient for the current calculation */
	double m_CDv;                    /**< the wing's viscous drag coefficient for the current calculation */
	double m_WingCL;                     /**< the lift coefficient */
	double m_Maxa;                   /**< the convergence crtierion on the difference of induced angle at any span bewteen two LLT iterations*/
	double m_ICm;                    /**< the induced par of the pitching moment coefficient */
	double m_GCm;                    /**< the total pitching moment coefficient */
	double m_VCm;                    /**< the viscous part of the pitching moment coefficient */
	double m_GYm;                    /**< the total yawing moment coefficient */
	double m_IYm;                    /**< the induced part of the yawing moment coefficient */
	double m_VYm;                    /**< the viscous part of the yawing moment coefficient */
	double m_GRm;                    /**< the geometric rolling moment coefficient */

	Vector3d m_CP;                    /**< the centre of pressure's position */


	/**<  Span Coefficients  resulting from VLM or LLT calculation @todo replace with QVarLenArray<double>*/
	double m_Ai[MAXSPANSTATIONS+1];            /**< the induced angles, in degrees */
	double m_Cl[MAXSPANSTATIONS+1];            /**< the lift coefficient at stations */
	double m_ICd[MAXSPANSTATIONS+1];           /**< the induced drag coefficient at stations */
	double m_PCd[MAXSPANSTATIONS+1];           /**< the viscous drag coefficient at stations */
	double m_Re[MAXSPANSTATIONS+1];            /**< the Reynolds number at stations */
	double m_XTrTop[MAXSPANSTATIONS+1];        /**< the upper transition location at stations */
	double m_XTrBot[MAXSPANSTATIONS+1];        /**< the lower transition location at stations */
	double m_Cm[MAXSPANSTATIONS+1];            /**< the total pitching moment coefficient at stations */
	double m_CmAirfoil[MAXSPANSTATIONS+1];     /**< the pitching moment coefficient at stations w.r.t. the chord's quarter point */
	double m_XCPSpanRel[MAXSPANSTATIONS+1];    /**< the relative position of the strip's center of pressure at stations as a % of the local chord length*/
	double m_XCPSpanAbs[MAXSPANSTATIONS+1];    /**< the absolute position of the strip's center of pressure pos at stations */
	double m_Chord[MAXSPANSTATIONS+1];         /**< the chord at stations */
	double m_Offset[MAXSPANSTATIONS+1];        /**< the offset at LLT stations */
	double m_Twist[MAXSPANSTATIONS+1];         /**< the twist at LLT stations */
	double m_StripArea[MAXSPANSTATIONS+1];     /**< the area of each chordwise strip */
	double m_BendingMoment[MAXSPANSTATIONS+1]; /**< the bending moment at stations */
	double m_SpanPos[MAXSPANSTATIONS+1];       /**< the span positions of LLT stations */

	QVarLengthArray<double> m_xHinge;           /**< the chorwise position of flap hinges */
	QVarLengthArray<double> m_xPanel;           /**< the chorwise distribution of VLM panels */

	Vector3d m_Vd[MAXSPANSTATIONS];             /**< the downwash vector at span stations */
	Vector3d m_F[MAXSPANSTATIONS];              /**< the lift vector at span stations */

public:	


	QList<WingSection*> m_WingSection;         /**< the array of wing sections. A WingSection extends between a foil and the next. */
	QList<PointMass*> m_PointMass;             /**< the array of PointMass objects associated to this Wing object*/

	QList<Surface*> m_Surface;                 /**< the array of Surface objects associated to the wing */
	
	double m_MAChord;                          /**< the wing's mean aerodynamic chord */
	double m_PlanformSpan;                     /**< the planform span, i.e. if the dihedral was 0 at each junction */
	double m_ProjectedSpan;                    /**< the span projected on the xy plane defined by z=0 */
	double m_PlanformArea;                     /**< the planform wing area, i.e. if the dihedral was 0 at each junction */
	double m_ProjectedArea;                    /**< the wing area projected on the xy plane defined by z=0; */
	double m_AR;                               /**< the wing's aspect ratio */
	double m_TR;                               /**< the wing's taper ratio */
	double m_CoGIxx;                           /**< the Ixx component of the inertia tensor, calculated at the CoG */
	double m_CoGIyy;                           /**< the Ixx component of the inertia tensor, calculated at the CoG */
	double m_CoGIzz;                           /**< the Ixx component of the inertia tensor, calculated at the CoG */
	double m_CoGIxz;                           /**< the Ixx component of the inertia tensor, calculated at the CoG */
	Vector3d m_CoG;                             /**< the position of the CoG */

	int m_MatSize;                             /**< the number of mesh panels on this Wing; dependant on the polar type */
	Panel *m_pWingPanel;                       /**< a pointer to the first panel of this wing in the array of panels */

	static QList<Foil*> *s_poaFoil;
	static QList<Polar*> *s_poaPolar;
};

#endif
