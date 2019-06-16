/****************************************************************************

	Body Class
	Copyright (C) 2007-2016 Andre Deperrois 

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
 * This file implements the definition of the Body class.
 */



#ifndef BODY_H
#define BODY_H



#include <QTextStream>
#include <QVarLengthArray>

#include <objects/objectcolor.h>
#include <objects/objects3d/Panel.h>
#include <objects/objects3d/NURBSSurface.h>
#include <objects/objects3d/PointMass.h>


#include <analysis3d/analysis3d_enums.h>

#define NHOOPPOINTS 67  //used for display and to export the geometry
#define NXPOINTS 97     //used for display and to export the geometry


/**
 * This class :
 *	 - defines the body object,
 * 	 - provides the methods for the calculation of the plane's geometric properties,
 *   - porvides methods for the panel calculations.
 * The data is stored in International Standard Units, i.e. meters, kg, and seconds.
 * Angular data is stored in degrees.
 */
class XFLR5ENGINELIBSHARED_EXPORT Body
{
public:
	Body();
	~Body();

	bool isInNURBSBody(double x, double z);
	bool isInNURBSBodyOld(Vector3d Pt);
	bool intersect(Vector3d A, Vector3d B, Vector3d &I, bool bRight);
    bool intersectFlatPanels(const Vector3d &A, const Vector3d &B, Vector3d &I) const;
	bool intersectNURBS(Vector3d A, Vector3d B, Vector3d &I, bool bRight);

	bool importDefinition(QTextStream &inStream, double mtoUnit, QString &errorMessage);

	int insertFrame(Vector3d Real);
	int insertFrameBefore(int iFrame);
	int insertFrameAfter(int iFrame);
	int insertPoint(Vector3d Real);
	int isFramePos(Vector3d Real, double ZoomFactor);
	int removeFrame(int n);
	int readFrame(QTextStream &in, int &Line, Frame *pFrame, double const &Unit);

	double length();

	double getu(double x);
	double getv(double u, Vector3d r, bool bRight);
	double getSectionArcLength(double x);

	Vector3d centerPoint(double u);
	Vector3d leadingPoint();

	void clearPointMasses();
	void computeAero(double *Cp, double &XCP, double &YCP, double &ZCP,
				  double &GCm, double &GRm, double &GYm, double &Alpha, Vector3d &CoG);
    void duplicate(const Body *pBody);
	void getPoint(double u, double v, bool bRight, Vector3d &Pt);
	Vector3d Point(double u, double v, bool bRight);
	void removeActiveFrame();
	void removeSideLine(int SideLine);
	void scale(double XFactor, double YFactor, double ZFactor, bool bFrameOnly=false, int FrameID=0);
	void translate(double XTrans, double, double ZTrans, bool bFrameOnly=false, int FrameID=0);
	void translate(Vector3d T, bool bFrameOnly=false, int FrameID=0);
	void setNURBSKnots();
	void setPanelPos();
	void setEdgeWeight(double uw, double vw);

	Frame *frame(int iFrame);
	Frame *activeFrame();

	int setActiveFrame(Frame *pFrame);
	Frame *setActiveFrame(int iFrame);


	double framePosition(int iFrame);
    int frameCount()      const {return m_SplineSurface.frameCount();}
    int framePointCount() const {return m_SplineSurface.framePointCount();}
    int sideLineCount()   const {return m_SplineSurface.framePointCount();}// same as FramePointCount();

	void computeBodyAxisInertia();
	void computeVolumeInertia(Vector3d &CoG, double &CoGIxx, double &CoGIyy, double &CoGIzz, double &CoGIxz);
	double totalMass();
	double &volumeMass(){return m_VolumeMass;}

	Vector3d CoG() {return m_CoG;}


	QString &bodyName(){return m_BodyName;}
	QString &bodyDescription() {return m_BodyDescription;}
	bool &textures(){return m_bTextures;}

	ObjectColor bodyColor() const {return m_BodyColor;}
	void setBodyColor(ObjectColor color) {m_BodyColor=color;}

	NURBSSurface& nurbs() {return m_SplineSurface;}

	XFLR5::enumBodyLineType &bodyType(){return m_LineType;}
    bool isFlatPanelType() const {return m_LineType==XFLR5::BODYPANELTYPE;}
    bool isSplineType()    const {return m_LineType==XFLR5::BODYSPLINETYPE;}

	NURBSSurface *splineSurface() {return &m_SplineSurface;}

	int & nxPanels() {return m_nxPanels;}
	int & nhPanels() {return m_nhPanels;}

	void exportGeometry(QTextStream &outStream, int type, double mtoUnit, int nx, int nh);
	void exportSTLBinary(QDataStream &outStream, int nXPanels, int nHoopPanels);
	void exportSTLBinarySplines(QDataStream &outStream, int nXPanels, int nHoopPanels);
	void exportSTLBinaryFlatPanels(QDataStream &outStream);

	bool exportBodyDefinition(QTextStream &outStream, double mtoUnit);

	bool serializeBodyWPA(QDataStream &ar, bool bIsStoring);
	bool serializeBodyXFL(QDataStream &ar, bool bIsStoring);

	int readValues(QString line, double &x, double &y, double &z);
	bool Rewind1Line(QTextStream &in, int &Line, QString &strong);


	//____________________VARIABLES_____________________________________________

	QString m_BodyName;                       /**< the Body's name, used as its reference */
	QString m_BodyDescription;                /**< a free description for the Body */

	NURBSSurface m_SplineSurface;             /**< the spline surface which defines the left (port) side of the body */

	XFLR5::enumBodyLineType m_LineType;              /**< the type of body surfaces 1=PANELS  2=NURBS */

	int m_iActiveFrame;		                  /**< the currently selected Frame for display */
	int m_iHighlightFrame;                    /**< the currently selected Frame to highlight */
	int m_NElements;                          /**< the number of mesh elements for this Body object = m_nxPanels * m_nhPanels *2 */
	int m_nxPanels;                           /**< For a NURBS body, the number of mesh elements in the direction of the x-axis */
	int m_nhPanels;                           /**< For a NURBS body, the number of mesh elements in the hoop direction */

	int m_BodyStyle;                          /**< the index of the spline's style */
	int m_BodyWidth;                          /**< the width of the spline */
	ObjectColor m_BodyColor;                       /**< the Body's display color */

	double m_Bunch;                            /**< a bunch parameter to set the density of the points of the NURBS surface; unused */

	double m_VolumeMass;                       /**< the mass of the Body's structure, excluding point masses */
	double m_TotalMass;                        /**< the wing's total mass, i.e. the sum of the volume mass and of the point masses */
	QList<PointMass*> m_PointMass;             /**< the array of PointMass objects */

	double m_CoGIxx;                           /**< the Ixx component of the inertia tensor, calculated at the CoG */
	double m_CoGIyy;                           /**< the Ixx component of the inertia tensor, calculated at the CoG */
	double m_CoGIzz;                           /**< the Ixx component of the inertia tensor, calculated at the CoG */
	double m_CoGIxz;                           /**< the Ixx component of the inertia tensor, calculated at the CoG */
	Vector3d m_CoG;                             /**< the position of the CoG */


	QVarLengthArray<int> m_xPanels;              /**< the number of mesh panels between two frames */
	QVarLengthArray<int> m_hPanels;              /**< the number of mesh panels in the hoop direction between two sidelines */
	QVarLengthArray<double> m_XPanelPos;


	Panel *m_pBodyPanel;                       /** A pointer to the first body panel in the array */

	//allocate temporary variables to
	//avoid lengthy memory allocation times on the stack
    mutable double value, bs, cs;
    mutable Vector3d t_R, t_Prod, t_Q, t_r, t_N;
//	mutable Vector3d P0, P1, P2, PI;

	bool m_bTextures;

};
#endif

