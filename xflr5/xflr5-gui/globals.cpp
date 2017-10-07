/****************************************************************************

	Global functions

	Copyright (C) 2008-2017 Andre Deperrois adeperrois@xflr5.com

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

//Global functions

/**@file This file contains the definitions of methods used throughout the program and not specific to one application. */

#include <globals.h>
#include <mainframe.h>
#include <QtDebug>
#include <QPen>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QByteArray>
#include <math.h>
#include <qopengl.h>
#include <objects3d/WPolar.h>
#include <misc/Units.h>
#include <graph_globals.h>


/** 
* Returns a double number as its root and its base 10 exponent
* @param f the double number to reformat; is returned as f = f/pow(10.0,exp);
* @param exp the base 10 exponent of f.
*/
void ExpFormat(double &f, int &exp)
{
	if (f==0.0)
	{
		exp = 0;
		f = 0.0;
		return;
	}
	double f1 = qAbs(f);
//	int sgn = int(f/f1);
	if(f1<1)
		exp = (int)log10(f1)-1;
	else
		exp = (int)log10(f1);

	f = f/pow(10.0,exp);
}





/**
* Outputs in a debug file the current time and the value of the integer passed as an input parameter.
* The file is in the user's default temporary directory with the name Trace.log
* Used for debugging.
*@param n the integer to output
*/
void Trace(int n)
{
	if(!MainFrame::s_bTrace) return;

	if(MainFrame::s_pTraceFile && MainFrame::s_pTraceFile->isOpen())
	{
		QTextStream ts(MainFrame::s_pTraceFile);
		ts << "Int value=" << n << "\n";
	}
	qDebug()<<"Int value="<<n;
}


void Trace(QString msg, bool b)
{
	if(!MainFrame::s_bTrace) return;
	QString str;
	if(b) str += msg + "=true";
	else  str += msg + "=false";

	if(MainFrame::s_pTraceFile && MainFrame::s_pTraceFile->isOpen())
	{
		QTextStream ts(MainFrame::s_pTraceFile);
		ts << str << "\n";
	}
	qDebug()<<str;
}

/**
* Outputs in a debug file the current time and a string message passed as an input parameter.
* The file is in the user's default temporary directory with the name Trace.log. 
* Used for debugging.
*@param msg the message to output
*/
void Trace(QString msg)
{
	if(!MainFrame::s_bTrace) return;

	if(MainFrame::s_pTraceFile && MainFrame::s_pTraceFile->isOpen())
	{
		QTextStream ts(MainFrame::s_pTraceFile);
		ts<<msg<<"\n";
		ts.flush();
	}
	qDebug()<<msg;
}


/**
* Outputs in a debug file the current time, a string message and the value of the integer passed as an input parameter.
* The file is in the user's default temporary directory with the name Trace.log. 
* Used for debugging.
*@param msg the message to output
*@param n the integer to output
*/
void Trace(QString msg, int n)
{
	if(!MainFrame::s_bTrace) return;

	QString strong;
	strong = QString("  %1").arg(n);
	strong = msg + strong;



	if(MainFrame::s_pTraceFile && MainFrame::s_pTraceFile->isOpen())
	{
		QTextStream ts(MainFrame::s_pTraceFile);
		ts << strong << "\n";
	}
	qDebug()<<strong;
}


/**
* Outputs in a debug file the current time, a string message and the value of the floating number passed as an input parameter.
* The file is in the user's default temporary directory with the name Trace.log. 
* Used for debugging.
*@param msg the message to output
*@param f the float number to output
*/
void Trace(QString msg, double f)
{
	if(!MainFrame::s_bTrace) return;

	QString strong;
	strong = QString("  %1").arg(f);
	strong = msg + strong;


	if(MainFrame::s_pTraceFile && MainFrame::s_pTraceFile->isOpen())
	{
		QTextStream ts(MainFrame::s_pTraceFile);
		ts << strong << "\n";
	}
	qDebug()<<strong;
}


/**
* Returns the red component of a color scale depending on an input parameter with value between 0 and 1.
* Used to draw a color scale between 0=blue and 1=red
*@param tau the input parameter between 0 and 1.
*@return the red component of the color
*/
float GLGetRed(double tau)
{
	if(tau>2.0/3.0)      return 1.0;
	else if(tau>1.0/3.0) return (3.0*(tau-1.0/3.0));
	else                 return 0.0;
}


/**
* Returns the green component of a color scale depending on an input parameter with value between 0 and 1.
* Used to draw a color scale between 0=blue and 1=red
*@param tau the input parameter between 0 and 1.
*@return the green component of the color
*/
float GLGetGreen(double tau)
{
	if(tau<0.f || tau>1.0) 	return 0.0;
	else if(tau<1.0/4.0) 	return (4.0*tau);
	else if(tau>3.0/4.0) 	return (1.0-4.0*(tau-3.0/4.0));
	else                    return 1.0;
}


/**
* Returns the blue component of a color scale depending on an input parameter with value between 0 and 1.
* Used to draw a color scale between 0=blue and 1=red
*@param tau the input parameter between 0 and 1.
*@return the blue component of the color
*/
float GLGetBlue(double tau)
{
	if(tau>2.0/3.0)      return 0.0;
	else if(tau>1.0/3.0) return (1.0-3.0*(tau-1.0/3.0));
	else                 return 1.0;
}


/**
* Extracts three double values from a QString, and returns the number of extracted values.
*/
int readValues(QString line, double &x, double &y, double &z)
{
/*	char *sx = new char[30];
	char *sy = new char[30];
	char *text;*/
	int res=0;

	QString str;
	bool bOK;

	line = line.simplified();
	int pos = line.indexOf(" ");


	if(pos>0)
	{
		str = line.left(pos);
		line = line.right(line.length()-pos);
	}
	else
	{
		str = line;
		line = "";
	}
	x = str.toDouble(&bOK);
	if(bOK) res++;
	else
	{
		y=z=0.0;
		return res;
	}

	line = line.trimmed();
	pos = line.indexOf(" ");
	if(pos>0)
	{
		str = line.left(pos);
		line = line.right(line.length()-pos);
	}
	else
	{
		str = line;
		line = "";
	}
	y = str.toDouble(&bOK);
	if(bOK) res++;
	else
	{
		z=0.0;
		return res;
	}

	line = line.trimmed();
	if(!line.isEmpty())
	{
		z = line.toDouble(&bOK);
		if(bOK) res++;
	}
	else z=0.0;

	return res;
}




QColor randomColor(bool bLightColor)
{
	QColor clr;
	if(bLightColor)
	{
		clr.setHsv((int)(((double)qrand()/(double)RAND_MAX)*360),
				   (int)(((double)qrand()/(double)RAND_MAX)*255),
				   (int)(((double)qrand()/(double)RAND_MAX)*100)+155,
					255);
	}
	else
	{
		clr.setHsv((int)(((double)qrand()/(double)RAND_MAX)*360),
				   (int)(((double)qrand()/(double)RAND_MAX)*101)+ 99,
				   (int)(((double)qrand()/(double)RAND_MAX)*100)+50,
					255);
	}
	return clr;
}




void GLLineStipple(int style)
{
	if     (style == Qt::DashLine)       glLineStipple (1, 0xCFCF);
	else if(style == Qt::DotLine)        glLineStipple (1, 0x6666);
	else if(style == Qt::DashDotLine)    glLineStipple (1, 0xFF18);
	else if(style == Qt::DashDotDotLine) glLineStipple (1, 0x7E66);
	else                                 glLineStipple (1, 0xFFFF);
}

//-----------------------------------------------------------------------------------------------



XFLR5::enumPanelDistribution distributionType(QString strDist)
{
	if(strDist.compare("COSINE",           Qt::CaseInsensitive)==0) return XFLR5::COSINE;
	else if(strDist.compare("UNIFORM",     Qt::CaseInsensitive)==0) return XFLR5::UNIFORM;
	else if(strDist.compare("SINE",        Qt::CaseInsensitive)==0) return XFLR5::SINE;
	else if(strDist.compare("INVERSESINE", Qt::CaseInsensitive)==0) return XFLR5::INVERSESINE;
	else return XFLR5::UNIFORM;
}


QString distributionType(XFLR5::enumPanelDistribution dist)
{
	switch(dist)
	{
		case XFLR5::COSINE: return "COSINE"; break;
		case XFLR5::UNIFORM: return "UNIFORM"; break;
		case XFLR5::SINE: return "SINE"; break;
		case XFLR5::INVERSESINE: return "INVERSE SINE"; break;
		default: return "";
	}
}



XFLR5::enumBodyLineType bodyPanelType(QString strPanelType)
{
	if(strPanelType.compare("FLATPANELS", Qt::CaseInsensitive)==0) return XFLR5::BODYPANELTYPE;
	else                                                           return XFLR5::BODYSPLINETYPE;
}

QString bodyPanelType(XFLR5::enumBodyLineType panelType)
{
	switch(panelType)
	{
		case XFLR5::BODYPANELTYPE:  return "FLATPANELS"; break;
		case XFLR5::BODYSPLINETYPE: return "NURBS"; break;
		default: return "";
	}
}


XFOIL::enumPolarType polarType(QString strPolarType)
{
	if     (strPolarType.compare("FIXEDSPEEDPOLAR",   Qt::CaseInsensitive)==0) return XFOIL::FIXEDSPEEDPOLAR;
	else if(strPolarType.compare("FIXEDLIFTPOLAR",    Qt::CaseInsensitive)==0) return XFOIL::FIXEDLIFTPOLAR;
	else if(strPolarType.compare("RUBBERCHORDPOLAR",  Qt::CaseInsensitive)==0) return XFOIL::RUBBERCHORDPOLAR;
	else if(strPolarType.compare("FIXEDAOAPOLAR",     Qt::CaseInsensitive)==0) return XFOIL::FIXEDAOAPOLAR;
	else return XFOIL::FIXEDSPEEDPOLAR;
}

QString polarType(XFOIL::enumPolarType polarType)
{
	switch(polarType)
	{
		case XFOIL::FIXEDSPEEDPOLAR:  return "FIXEDSPEEDPOLAR";   break;
		case XFOIL::FIXEDLIFTPOLAR:   return "FIXEDLIFTPOLAR";    break;
		case XFOIL::RUBBERCHORDPOLAR: return "RUBBERCHORDPOLAR";  break;
		case XFOIL::FIXEDAOAPOLAR:    return "FIXEDAOAPOLAR";     break;
		default: return "";
	}
}



XFLR5::enumPolarType WPolarType(QString strPolarType)
{
	if     (strPolarType.compare("FIXEDSPEEDPOLAR", Qt::CaseInsensitive)==0) return XFLR5::FIXEDSPEEDPOLAR;
	else if(strPolarType.compare("FIXEDLIFTPOLAR",  Qt::CaseInsensitive)==0) return XFLR5::FIXEDLIFTPOLAR;
	else if(strPolarType.compare("FIXEDAOAPOLAR",   Qt::CaseInsensitive)==0) return XFLR5::FIXEDAOAPOLAR;
	else if(strPolarType.compare("STABILITYPOLAR",  Qt::CaseInsensitive)==0) return XFLR5::STABILITYPOLAR;
	else if(strPolarType.compare("BETAPOLAR",       Qt::CaseInsensitive)==0) return XFLR5::BETAPOLAR;
	else return XFLR5::FIXEDSPEEDPOLAR;
}

QString WPolarType(XFLR5::enumPolarType polarType)
{
	switch(polarType)
	{
		case XFLR5::FIXEDSPEEDPOLAR:  return "FIXEDSPEEDPOLAR"; break;
		case XFLR5::FIXEDLIFTPOLAR:   return "FIXEDLIFTPOLAR";  break;
		case XFLR5::FIXEDAOAPOLAR:    return "FIXEDAOAPOLAR";   break;
		case XFLR5::STABILITYPOLAR:   return "STABILITYPOLAR";  break;
		case XFLR5::BETAPOLAR:        return "BETAPOLAR";       break;
		default: return "";
	}
}


XFLR5::enumAnalysisMethod analysisMethod(QString strAnalysisMethod)
{
	if     (strAnalysisMethod.compare("LLTMETHOD",   Qt::CaseInsensitive)==0) return XFLR5::LLTMETHOD;
	else if(strAnalysisMethod.compare("VLMMETHOD",   Qt::CaseInsensitive)==0) return XFLR5::VLMMETHOD;
	else if(strAnalysisMethod.compare("PANELMETHOD", Qt::CaseInsensitive)==0) return XFLR5::PANELMETHOD;
	else return XFLR5::VLMMETHOD;
}


QString analysisMethod(XFLR5::enumAnalysisMethod analysisMethod)
{
	switch(analysisMethod)
	{
		case XFLR5::LLTMETHOD:   return "LLTMETHOD";   break;
		case XFLR5::VLMMETHOD:   return "VLMMETHOD";   break;
		case XFLR5::PANELMETHOD: return "PANELMETHOD"; break;
		default: return "";
	}
}



XFLR5::enumBC boundaryCondition(QString strBC)
{
    if   (strBC.compare("DIRICHLET", Qt::CaseInsensitive)==0) return XFLR5::DIRICHLET;
    else                                                      return XFLR5::NEUMANN;
}

QString boundaryCondition(XFLR5::enumBC boundaryCondition)
{
    switch(boundaryCondition)
    {
        case XFLR5::DIRICHLET: return "DIRICHLET";   break;
        case XFLR5::NEUMANN:   return "NEUMANN";     break;
        default: return "";
    }
}

bool stringToBool(QString str)
{
	return str.compare("true", Qt::CaseInsensitive)==0 ? true : false;
}

QString referenceDimension(XFLR5::enumRefDimension refDimension)
{
	switch(refDimension)
	{
		case XFLR5::PLANFORMREFDIM:  return "PLANFORMREFDIM";
		case XFLR5::PROJECTEDREFDIM: return "PROJECTEDREFDIM";
		case XFLR5::MANUALREFDIM:    return "MANUALREFDIM";
		default: return "";
	}
}


XFLR5::enumRefDimension referenceDimension(QString strRefDimension)
{
	if     (strRefDimension.compare("PLANFORMREFDIM",  Qt::CaseInsensitive)==0) return XFLR5::PLANFORMREFDIM;
	else if(strRefDimension.compare("PROJECTEDREFDIM", Qt::CaseInsensitive)==0) return XFLR5::PROJECTEDREFDIM;
	else if(strRefDimension.compare("MANUALREFDIM",    Qt::CaseInsensitive)==0) return XFLR5::MANUALREFDIM;
	else return XFLR5::PLANFORMREFDIM;
}



void * readFoilFile(QFile &xFoilFile)
{
	QString strong;
	QString tempStr;
	QString FoilName;

	Foil* pFoil = NULL;
	int pos, i, ip;
	pos = 0;
	double x, y, z, area;
	double xp, yp;
	bool bRead;
	QTextStream in(&xFoilFile);

	QString fileName = xFoilFile.fileName();
	int idx = fileName.lastIndexOf(QDir::separator());
	if(idx>0)
	{
		fileName = fileName.right(fileName.length()-idx-1);
		if(fileName.endsWith(".dat", Qt::CaseInsensitive))
		{
			fileName = fileName.left(fileName.length()-4);
		}
	}

	pFoil = new Foil();
	if(!pFoil)	return NULL;

	while(tempStr.length()==0 && !in.atEnd())
	{
		strong = in.readLine();
		pos = strong.indexOf("#",0);
		// ignore everything after # (including #)
		if(pos>0)strong.truncate(pos);
		tempStr = strong;
		tempStr.remove(" ");
		FoilName = strong;
	}

	if(!in.atEnd())
	{
		// FoilName contains the last comment

		if(readValues(strong,x,y,z)==2)
		{
			//there isn't a name on the first line, use the file's name
			FoilName = fileName;
			{
				pFoil->xb[0] = x;
				pFoil->yb[0] = y;
				pFoil->nb=1;
				xp = x;
				yp = y;
			}
		}
		else FoilName = strong;
		// remove fore and aft spaces
		FoilName = FoilName.trimmed();
	}

	bRead = true;
	xp=-9999.0;
	yp=-9999.0;
	do
	{
		strong = in.readLine();
		pos = strong.indexOf("#",0);
		// ignore everything after # (including #)
		if(pos>0)strong.truncate(pos);
		tempStr = strong;
		tempStr.remove(" ");
		if (!strong.isNull() && bRead && tempStr.length())
		{
			if(readValues(strong, x,y,z)==2)
			{
				//add values only if the point is not coincident with the previous one
				double dist = sqrt((x-xp)*(x-xp) + (y-yp)*(y-yp));
				if(dist>0.000001)
				{
					pFoil->xb[pFoil->nb] = x;
					pFoil->yb[pFoil->nb] = y;
					pFoil->nb++;
					if(pFoil->nb>IQX)
					{
						delete pFoil;
						return NULL;
					}
					xp = x;
					yp = y;
				}
			}
			else bRead = false;
		}
	}while (bRead && !strong.isNull());

	pFoil->setFoilName(FoilName);

	// Check if the foil was written clockwise or counter-clockwise

	area = 0.0;
	for (i=0; i<pFoil->nb; i++)
	{
		if(i==pFoil->nb-1)	ip = 0;
		else				ip = i+1;
		area +=  0.5*(pFoil->yb[i]+pFoil->yb[ip])*(pFoil->xb[i]-pFoil->xb[ip]);
	}

	if(area < 0.0)
	{
		//reverse the points order
		double xtmp, ytmp;
		for (int i=0; i<pFoil->nb/2; i++)
		{
			xtmp         = pFoil->xb[i];
			ytmp         = pFoil->yb[i];
			pFoil->xb[i] = pFoil->xb[pFoil->nb-i-1];
			pFoil->yb[i] = pFoil->yb[pFoil->nb-i-1];
			pFoil->xb[pFoil->nb-i-1] = xtmp;
			pFoil->yb[pFoil->nb-i-1] = ytmp;
		}
	}

	memcpy(pFoil->x, pFoil->xb, sizeof(pFoil->xb));
	memcpy(pFoil->y, pFoil->yb, sizeof(pFoil->yb));
	pFoil->n = pFoil->nb;

	QColor clr = randomColor(false);
	pFoil->setColor(clr.red(), clr.green(), clr.blue(), clr.alpha());
	pFoil->initFoil();

	return pFoil;
}





/**
 *Reads a Foil and its related Polar objects from a binary stream associated to a .plr file.
 * @param ar the binary stream
 * @return the pointer to the Foil object which has been created, or NULL if failure.
 */
void * readPolarFile(QFile &plrFile, QList<Polar*> &polarList)
{
	Foil* pFoil = NULL;
	Polar *pPolar = NULL;
	Polar * pOldPolar;
	int i, n, l;

	QDataStream ar(&plrFile);
	ar.setVersion(QDataStream::Qt_4_5);
	ar.setByteOrder(QDataStream::LittleEndian);

	ar >> n;

	if(n<100000)
	{
		//old format
		return NULL;
	}
	else if (n >=100000)
	{
		//new format XFLR5 v1.99+
		//first read all available foils
		ar>>n;
		for (i=0;i<n; i++)
		{
			pFoil = new Foil();
			if (!serializeFoil(pFoil, ar, false))
			{
				delete pFoil;
				return NULL;
			}
		}

		//next read all available polars

		ar>>n;
		for (i=0;i<n; i++)
		{
			pPolar = new Polar();

			if (!serializePolar(pPolar, ar, false))
			{
				delete pPolar;
				return NULL;
			}
			for (l=0; l<polarList.size(); l++)
			{
				pOldPolar = polarList.at(l);
				if (pOldPolar->foilName()  == pPolar->foilName() &&
					pOldPolar->polarName() == pPolar->polarName())
				{
					//just overwrite...
					polarList.removeAt(l);
					delete pOldPolar;
					//... and continue to add
				}
			}
			polarList.append(pPolar);
		}
	}
	return pFoil;
}







/**
 * Draws the foil in the client area.
 * @param painter a reference to the QPainter object on which the foil will be drawn
 * @param alpha the rotation angle in degrees of the foil
 * @param scalex the scaling factor in the x-direction
 * @param scaley the scaling factor in the y-direction
 * @param Offset the foil offset in the client area
 */
void drawFoil(QPainter &painter, Foil*pFoil, double const &alpha, double const &scalex, double const &scaley, QPointF const &Offset)
{
	double xa, ya, sina, cosa;
	QPointF From, To;

	int k;
	QPen FoilPen, HighPen;

	FoilPen.setColor(colour(pFoil));
	FoilPen.setWidth(pFoil->foilLineWidth());
	FoilPen.setStyle(getStyle(pFoil->foilLineStyle()));
	painter.setPen(FoilPen);

	HighPen.setColor(QColor(255,0,0));

	cosa = cos(alpha*PI/180.0);
	sina = sin(alpha*PI/180.0);

	xa = (pFoil->x[0]-0.5)*cosa - pFoil->y[0]*sina + 0.5;
	ya = (pFoil->x[0]-0.5)*sina + pFoil->y[0]*cosa;
	From.rx() = ( xa*scalex + Offset.x());
	From.ry() = (-ya*scaley + Offset.y());

	for (k=1; k<pFoil->n; k++)
	{
		xa = (pFoil->x[k]-0.5)*cosa - pFoil->y[k]*sina+ 0.5;
		ya = (pFoil->x[k]-0.5)*sina + pFoil->y[k]*cosa;
		To.rx() =  xa*scalex+Offset.x();
		To.ry() = -ya*scaley+Offset.y();

		painter.drawLine(From,To);

		From = To;
	}
}


/**
 * Draws the foil's mid line in the client area.
 * @param painter a refernce to the QPainter object on which the foil will be drawn
 * @param alpha the rotation angle in degrees of the foil
 * @param scalex the scaling factor in the x-direction
 * @param scaley the scaling factor in the y-direction
 * @param Offset the foil offset in the client area
 */
void drawMidLine(QPainter &painter, Foil*pFoil, double const &scalex, double const &scaley, QPointF const &Offset)
{
	QPointF From, To;
	int k;
	QPen FoilPen;

	FoilPen.setColor(colour(pFoil));
	FoilPen.setWidth(pFoil->foilLineWidth());
	FoilPen.setStyle(Qt::DashLine);
	painter.setPen(FoilPen);


	From.rx() = ( pFoil->m_rpMid[0].x*scalex)  +Offset.x();
	From.ry() = (-pFoil->m_rpMid[0].y*scaley)  +Offset.y();


	for (k=0; k<MIDPOINTCOUNT; k++)
	{
		To.rx() = ( pFoil->m_rpMid[k].x*scalex)+Offset.x();
		To.ry() = (-pFoil->m_rpMid[k].y*scaley)+Offset.y();

		painter.drawLine(From, To);
		From = To;
	}
}





/**
 * Draws the foil's points in the client area.
 * @param painter a refernce to the QPainter object on which the foil will be drawn
 * @param alpha the rotation angle in degrees of the foil
 * @param scalex the scaling factor in the x-direction
 * @param scaley the scaling factor in the y-direction
 * @param Offset the foil offset in the client area
 */
void drawPoints(QPainter &painter, Foil*pFoil, double alpha, double const &scalex, double const &scaley,
				QPointF const &Offset, QColor backColor)
{
	QPen FoilPen, HighPen;
	FoilPen.setColor(colour(pFoil));
	FoilPen.setWidth(pFoil->foilLineWidth());
	FoilPen.setStyle(Qt::SolidLine);
	painter.setPen(FoilPen);


	HighPen.setColor(QColor(255,0,0));

/*	for (int i=0; i<pFoil->n;i++)
	{
		pt1.rx() = ( pFoil->x[i]*scalex + Offset.x() - width);
		pt1.ry() = (-pFoil->y[i]*scaley + Offset.y() - width);

		painter.drawRect(pt1.x(), pt1.y(), 4, 4) ;
	}*/

	double xa, ya, cosa, sina;
	cosa = cos(alpha*PI/180.0);
	sina = sin(alpha*PI/180.0);

	for (int i=0; i<pFoil->n;i++)
	{
		xa = (pFoil->x[i]-0.5)*cosa - pFoil->y[i]*sina + 0.5;
		ya = (pFoil->x[i]-0.5)*sina + pFoil->y[i]*cosa;

		QPoint pt( xa*scalex + Offset.x(), -ya*scaley + Offset.y());

		drawPoint(painter, pFoil->foilPointStyle(), backColor, pt);
	}

	if(pFoil->iHighLight()>=0)
	{
		HighPen.setWidth(2);
		painter.setPen(HighPen);

		int ih = pFoil->iHighLight();
		xa = (pFoil->x[ih]-0.5)*cosa - pFoil->y[ih]*sina + 0.5;
		ya = (pFoil->x[ih]-0.5)*sina + pFoil->y[ih]*cosa;

		QPoint pt( xa*scalex + Offset.x(), -ya*scaley + Offset.y());

		drawPoint(painter, pFoil->foilPointStyle(), backColor, pt);
	}
}


void drawPoint(QPainter &painter, int pointStyle, QColor bkColor, QPoint pt)
{
	painter.save();
	QBrush backBrush(bkColor);
	painter.setBrush(backBrush);
	switch(pointStyle)
	{
		case 0: break;
		case 1:
		{
			int ptSide = 2;
			painter.drawEllipse(pt.x()-ptSide, pt.y()-ptSide, 2*ptSide, 2*ptSide );
			break;
		}
		case 2:
		{
			int ptSide = 4;
			painter.drawEllipse(pt.x()-ptSide, pt.y()-ptSide, 2*ptSide, 2*ptSide );
			break;
		}
		case 3:
		{
			int ptSide = 2;
			painter.drawRect(pt.x()-ptSide, pt.y()-ptSide, 2*ptSide, 2*ptSide );
			break;
		}
		case 4:
		{
			int ptSide = 4;
			painter.drawRect(pt.x()-ptSide, pt.y()-ptSide, 2*ptSide, 2*ptSide );
			break;
		}
		default: break;
	}
	painter.restore();
}


void setAutoWPolarName(void * ptrWPolar, void *ptrPlane)
{
	if(!ptrPlane) return;
	QString str, strong;
	QString strSpeedUnit;
	Units::getSpeedUnitLabel(strSpeedUnit);

	int i, nCtrl;

	WPolar *pWPolar =(WPolar*)ptrWPolar;
	Plane *pPlane = (Plane*)ptrPlane;

	Units::getSpeedUnitLabel(str);

	switch(pWPolar->polarType())
	{
		case XFLR5::FIXEDSPEEDPOLAR:
		{
			pWPolar->polarName() = QString("T1-%1 ").arg(pWPolar->velocity() * Units::mstoUnit(),0,'f',1);
			pWPolar->polarName() += strSpeedUnit;
			break;
		}
		case XFLR5::FIXEDLIFTPOLAR:
		{
			pWPolar->polarName() = QString("T2");
			break;
		}
		case XFLR5::FIXEDAOAPOLAR:
		{
			pWPolar->polarName() = QString(QString::fromUtf8("T4-%1°")).arg(pWPolar->Alpha(),0,'f',1);
			break;
		}
		case XFLR5::BETAPOLAR:
		{
			pWPolar->polarName() = QString(QString::fromUtf8("T5-a%1°-%2"))
							  .arg(pWPolar->Alpha(),0,'f',1)
							  .arg(pWPolar->velocity() * Units::mstoUnit(),0,'f',1);
			pWPolar->polarName() += strSpeedUnit;
			break;
		}
		case XFLR5::STABILITYPOLAR:
		{
			pWPolar->polarName() = QString("T7");
			break;
		}
		default:
		{
			pWPolar->polarName() = "Tx";
			break;
		}
	}

	switch(pWPolar->analysisMethod())
	{
		case XFLR5::LLTMETHOD:
		{
			pWPolar->polarName() += "-LLT";
			break;
		}
		case XFLR5::VLMMETHOD:
		{
			if(pWPolar->bVLM1()) pWPolar->polarName() += "-VLM1";
			else                 pWPolar->polarName() += "-VLM2";
			break;
		}
		case XFLR5::PANELMETHOD:
		{
			if(!pWPolar->bThinSurfaces()) pWPolar->polarName() += "-Panel";
			else
			{
				if(pWPolar->bVLM1()) pWPolar->polarName() += "-VLM1";
				else                 pWPolar->polarName() += "-VLM2";
			}
			break;
		}
	}

	nCtrl = 0;



	if(pWPolar->isStabilityPolar())
	{
		if(!pPlane->isWing())
		{
			if(pWPolar->m_ControlGain.size()>0 && qAbs(pWPolar->m_ControlGain[0])>PRECISION)
			{
				strong = QString::fromUtf8("-Wing(g%1)")
								   .arg(pWPolar->m_ControlGain[0],0,'f',1);
				pWPolar->polarName() += strong;
			}
			nCtrl++;
		}

		if(pPlane->stab())
		{
			if(pWPolar->m_ControlGain.size()>1 && qAbs(pWPolar->m_ControlGain[1])>PRECISION)
			{
				strong = QString::fromUtf8("-Elev(g%1)").arg(pWPolar->m_ControlGain[1],0,'f',1);
				pWPolar->polarName() += strong;
			}
			nCtrl++;
		}

		for(i=0; i<pPlane->wing()->nFlaps(); i++)
		{
			if(pWPolar->m_ControlGain.size()>i+nCtrl && qAbs(pWPolar->m_ControlGain[i+nCtrl])>PRECISION)
			{
				strong = QString::fromUtf8("-WF%1(g%2)")
						 .arg(i+1)
						 .arg(pWPolar->m_ControlGain[i+nCtrl],0,'f',1);
				pWPolar->polarName() += strong;
			}
		}
		nCtrl += pPlane->wing()->nFlaps();

		if(pPlane->stab())
		{
			for(i=0; i<pPlane->stab()->nFlaps(); i++)
			{
				if(pWPolar->m_ControlGain.size()>i+nCtrl && qAbs(pWPolar->m_ControlGain[i+nCtrl])>PRECISION)
				{
					strong = QString::fromUtf8("-EF%1(g%2)").arg(i+1).arg(pWPolar->m_ControlGain[i+nCtrl]);
					pWPolar->polarName() += strong;
				}
			}
			nCtrl += pPlane->stab()->nFlaps();
		}

		if(pPlane->fin())
		{
			for(i=0; i<pPlane->fin()->nFlaps(); i++)
			{
				if(pWPolar->m_ControlGain.size()>i+nCtrl && qAbs(pWPolar->m_ControlGain[i+nCtrl])>PRECISION)
				{
					strong = QString::fromUtf8("-FF%1(g%2)").arg(i+1).arg(pWPolar->m_ControlGain[i+nCtrl]);
					pWPolar->polarName() += strong;
				}
			}
		}
	}


	if(qAbs(pWPolar->Beta()) > .001  && pWPolar->polarType()!=XFLR5::BETAPOLAR)
	{
		strong = QString(QString::fromUtf8("-b%1°")).arg(pWPolar->Beta(),0,'f',1);
		pWPolar->polarName() += strong;
	}

	if(qAbs(pWPolar->Phi()) > .001)
	{
		strong = QString(QString::fromUtf8("-B%1°")).arg(pWPolar->Phi(),0,'f',1);
		pWPolar->polarName() += strong;
	}

	if(!pWPolar->bAutoInertia())
	{
		strong.sprintf("-%.1f", pWPolar->mass()*Units::kgtoUnit());
		if(pWPolar->isStabilityPolar()&&fabs(pWPolar->m_inertiaGain[0])>PRECISION)
			str.sprintf("/%0.2f", pWPolar->m_inertiaGain[0]*Units::kgtoUnit());
		else str.clear();
		pWPolar->polarName() += strong + str + Units::weightUnitLabel();

		strong.sprintf("-x%.1f", pWPolar->CoG().x*Units::mtoUnit());
		if(pWPolar->isStabilityPolar()&&fabs(pWPolar->m_inertiaGain[1])>PRECISION)
			str.sprintf("/%0.2f", pWPolar->m_inertiaGain[1]*Units::mtoUnit());
		else str.clear();
		pWPolar->polarName() += strong + str + Units::lengthUnitLabel();

		if(qAbs(pWPolar->CoG().z)>=.000001)
		{
			strong.sprintf("-z%.1f", pWPolar->CoG().z*Units::mtoUnit());
			if(pWPolar->isStabilityPolar()&&fabs(pWPolar->m_inertiaGain[2])>PRECISION)
				str.sprintf("/%0.2f", pWPolar->m_inertiaGain[2]*Units::mtoUnit());
			else str.clear();
			pWPolar->polarName() += strong + str + Units::lengthUnitLabel();
		}
	}

	if(!pWPolar->bViscous())
	{
		pWPolar->polarName() += "-Inviscid";
	}
	if(pWPolar->bIgnoreBodyPanels())
	{
		pWPolar->polarName() += "-NoBodyPanels";
	}
	if(pWPolar->referenceDim()==XFLR5::PROJECTEDREFDIM) pWPolar->polarName() += "-proj_area";

	if(pWPolar->bTilted()) pWPolar->polarName() += "-TG";

	for(int i=0; i<MAXEXTRADRAG; i++)
	{
		if(fabs(pWPolar->m_ExtraDragCoef[i])>PRECISION && fabs(pWPolar->m_ExtraDragArea[i])>PRECISION)
		{
			pWPolar->polarName()+="+Drag";
			break;
		}
	}
}




XFLR5::enumWingType wingType(QString strWingType)
{
	if     (strWingType.compare("MAINWING",   Qt::CaseInsensitive)==0) return XFLR5::MAINWING;
	else if(strWingType.compare("SECONDWING", Qt::CaseInsensitive)==0) return XFLR5::SECONDWING;
	else if(strWingType.compare("ELEVATOR",   Qt::CaseInsensitive)==0) return XFLR5::ELEVATOR;
	else if(strWingType.compare("FIN",        Qt::CaseInsensitive)==0) return XFLR5::FIN;
	else                                                               return XFLR5::OTHERWING;
}

QString wingType(XFLR5::enumWingType wingType)
{
	switch(wingType)
	{
		case XFLR5::MAINWING:   return "MAINWING";
		case XFLR5::SECONDWING: return "SECONDWING";
		case XFLR5::ELEVATOR:   return "ELEVATOR";
		case XFLR5::FIN:        return "FIN";
		case XFLR5::OTHERWING:  return "OTHERWING";
	}
	return "OTHERWING";
}




/**
* Takes a double number holding the value of a Reynolds number and returns a string.
*@param str the return string  with the formatted number
*@param f the Reynolds number to be formatted
*/
void ReynoldsFormat(QString &str, double f)
{
	int i, q, r, exp;
	f = (int(f/1000.0))*1000.0;

	exp = (int)log10(f);
	r = exp%3;
	q = (exp-r)/3;

	QString strong;
	strong = QString("%1").arg(f,0,'f',0);

	int l = strong.length();

	for (i=0; i<q; i++){
		strong.insert(l-3*(i+1)-i," ");
		l++;
	}

	for (i=strong.length(); i<9; i++){
		strong = " "+strong;
	}

	str = strong;
}


QColor getColor(int r, int g, int b, int a)
{
	r = min(r, 255);
	r = max(r, 0);
	g = min(g, 255);
	g = max(g, 0);
	b = min(b, 255);
	b = max(b, 0);
	a = min(a, 255);
	a = max(a, 0);
	return QColor(r,g,b,a);
}

QColor colour(OpPoint *pOpp)
{
	return QColor(pOpp->red(), pOpp->green(), pOpp->blue(), pOpp->alphaChannel());
}

QColor colour(Polar *pPolar)
{
	return QColor(pPolar->red(), pPolar->green(), pPolar->blue(), pPolar->alphaChannel());
}

QColor colour(Foil *pFoil)
{
	return QColor(pFoil->red(), pFoil->green(), pFoil->blue(), pFoil->alphaChannel());
}


void setRandomFoilColor(Foil *pFoil, bool bLightTheme)
{
	QColor clr = randomColor(!bLightTheme);
	pFoil->setColor(clr.red(), clr.green(), clr.blue(), clr.alpha());
}


/**
* Reads the RGB int values of a color from binary datastream and returns a QColor. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param r the red component
*@param g the green component
*@param b the blue component
*/
void readColor(QDataStream &ar, int &r, int &g, int &b)
{
	qint32 colorref;

	ar >> colorref;
	b = (int)(colorref/256/256);
	colorref -= b*256*256;
	g = (int)(colorref/256);
	r = colorref - g*256;
}


/**
* Writes the RGB int values of a color to a binary datastream. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param r the red component
*@param g the green component
*@param b the blue component

*/
void writeColor(QDataStream &ar, int r, int g, int b)
{
	qint32 colorref;

	colorref = b*256*256+g*256+r;
	ar << colorref;
}


/**
* Reads the RGB int values of a color from binary datastream and returns a QColor. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param r the red component
*@param g the green component
*@param b the blue component
*@param a the alpha component
*/
void readColor(QDataStream &ar, int &r, int &g, int &b, int &a)
{
	uchar byte=0;

	ar>>byte;//probably a format identificator
	ar>>byte>>byte;
	a = (int)byte;
	ar>>byte>>byte;
	r = (int)byte;
	ar>>byte>>byte;
	g = (int)byte;
	ar>>byte>>byte;
	b = (int)byte;
	ar>>byte>>byte; //
}

/**
* Writes the RGB int values of a color to a binary datastream. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param r the red component
*@param g the green component
*@param b the blue component
*@param a the alpha component
*/
void writeColor(QDataStream &ar, int r, int g, int b, int a)
{
	uchar byte;

	byte = 1;
	ar<<byte;
	byte = a & 0xFF;
	ar << byte<<byte;
	byte = r & 0xFF;
	ar << byte<<byte;
	byte = g & 0xFF;
	ar << byte<<byte;
	byte = b & 0xFF;
	ar << byte<<byte;
	byte = 0;
	ar << byte<<byte;
}


/**
* Reads a sequence of characters from a binary stream and returns a QString. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param strong the QString read from the stream
*/
void readString(QDataStream &ar, QString &strong)
{
	qint8 qi, ch;
	char c;

	ar >> qi;
	strong.clear();
	for(int j=0; j<qi;j++)
	{
		strong += " ";
		ar >> ch;
		c = char(ch);
		strong[j] = c;
	}
}

/**
* Writes a sequence of characters from a QStrinf to a binary stream. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param strong the QString to output to the stream
*/
void writeString(QDataStream &ar, QString const &strong)
{
	qint8 qi = strong.length();

	QByteArray textline;
	char *text;
	textline = strong.toLatin1();
	text = textline.data();
	ar << qi;
	ar.writeRawData(text, qi);
}




/**
 * Loads or Saves the data of this foil to a binary file.
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool serializeFoil(Foil *pFoil, QDataStream &ar, bool bIsStoring)
{
	// saves or loads the foil to the archive ar

	int ArchiveFormat = 1006;
	// 1006 : QFLR5 v0.02 : added Foil description
	// 1005 : added LE Flap data
	// 1004 : added Points and Centerline property
	// 1003 : added Visible property
	// 1002 : added color and style save
	// 1001 : initial format
	int p, j;
	float f,ff;

	if(bIsStoring)
	{
		ar << ArchiveFormat;
		writeString(ar, pFoil->m_FoilName);
		writeString(ar, pFoil->m_FoilDescription);
		ar << pFoil->m_FoilStyle << pFoil->m_FoilWidth;
		writeColor(ar, pFoil->m_red, pFoil->m_green, pFoil->m_blue);

		if (pFoil->m_bIsFoilVisible) ar << 1; else ar << 0;
		if (pFoil->m_PointStyle>0)   ar << 1; else ar << 0;//1004
		if (pFoil->m_bCenterLine)    ar << 1; else ar << 0;//1004
		if (pFoil->m_bLEFlap)        ar << 1; else ar << 0;
		ar << (float)pFoil->m_LEFlapAngle << (float)pFoil->m_LEXHinge << (float)pFoil->m_LEYHinge;
		if (pFoil->m_bTEFlap)        ar << 1; else ar << 0;
		ar << (float)pFoil->m_TEFlapAngle << (float)pFoil->m_TEXHinge << (float)pFoil->m_TEYHinge;
		ar << 1.f << 1.f << 9.f;//formerly transition parameters
		ar << pFoil->nb;
		for (j=0; j<pFoil->nb; j++)
		{
			ar << (float)pFoil->xb[j] << (float)pFoil->yb[j];
		}
		ar << pFoil->n;
		for (j=0; j<pFoil->n; j++)
		{
			ar << (float)pFoil->x[j] << (float)pFoil->y[j];
		}
		return true;
	}
	else
	{
		ar >> ArchiveFormat;
		if(ArchiveFormat<1000||ArchiveFormat>1010)
			return false;

		readString(ar, pFoil->m_FoilName);
		if(ArchiveFormat>=1006)
		{
			readString(ar, pFoil->m_FoilDescription);
		}
		if(ArchiveFormat>=1002)
		{
			ar >> pFoil->m_FoilStyle >> pFoil->m_FoilWidth;
			readColor(ar, pFoil->m_red, pFoil->m_green, pFoil->m_blue);
		}
		if(ArchiveFormat>=1003)
		{
			ar >> p;
			if(p) pFoil->m_bIsFoilVisible = true; else pFoil->m_bIsFoilVisible = false;
		}
		if(ArchiveFormat>=1004)
		{
			ar >> p;
			pFoil->m_PointStyle = p;
			ar >> p;
			if(p) pFoil->m_bCenterLine = true; else pFoil->m_bCenterLine = false;
		}

		if(ArchiveFormat>=1005)
		{
			ar >> p;
			if (p) pFoil->m_bLEFlap = true; else pFoil->m_bLEFlap = false;
			ar >> f; pFoil->m_LEFlapAngle =f;
			ar >> f; pFoil->m_LEXHinge = f;
			ar >> f; pFoil->m_LEYHinge = f;
		}
		ar >> p;
		if (p) pFoil->m_bTEFlap = true; else pFoil->m_bTEFlap = false;
		ar >> f; pFoil->m_TEFlapAngle =f;
		ar >> f; pFoil->m_TEXHinge = f;
		ar >> f; pFoil->m_TEYHinge = f;

		ar >> f >> f >> f; //formerly transition parameters
		ar >> pFoil->nb;
		if(pFoil->nb>IBX) return false;

		for (j=0; j<pFoil->nb; j++)
		{
			ar >> f >> ff;
			pFoil->xb[j]  = f;  pFoil->yb[j]=ff;
		}
		if(ArchiveFormat>=1001)
		{
			ar >> pFoil->n;
			if(pFoil->n>IBX) return false;

			for (j=0; j<pFoil->n; j++)
			{
				ar >> f >> ff;
				pFoil->x[j]=f; pFoil->y[j]=ff;
			}
			if(pFoil->nb==0 && pFoil->n!=0)
			{
				pFoil->nb = pFoil->n;
				memcpy(pFoil->xb, pFoil->x, sizeof(pFoil->xb));
				memcpy(pFoil->yb, pFoil->y, sizeof(pFoil->yb));
			}
		}
		else
		{
			memcpy(pFoil->x, pFoil->xb, sizeof(pFoil->xb));
			memcpy(pFoil->y, pFoil->yb, sizeof(pFoil->yb));
			pFoil->n=pFoil->nb;
		}


		pFoil->initFoil();
		pFoil->setFlap();

		return true;
	}
}



/**
 * Loads or saves the data of this polar to a binary file
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool serializePolar(Polar *pPolar, QDataStream &ar, bool bIsStoring)
{
	int i, j, n, l, k;
	int ArchiveFormat;// identifies the format of the file
	float f;

	if(bIsStoring)
	{
		//write variables
		n = (int)pPolar->m_Alpha.size();

		ar << 1004; // identifies the format of the file
		// 1004 : added XCp
		// 1003 : re-instated NCrit, XtopTr and XBotTr with polar
		writeString(ar, pPolar->m_FoilName);
		writeString(ar, pPolar->m_PlrName);

		if(pPolar->m_PolarType==XFOIL::FIXEDSPEEDPOLAR)       ar<<1;
		else if(pPolar->m_PolarType==XFOIL::FIXEDLIFTPOLAR)   ar<<2;
		else if(pPolar->m_PolarType==XFOIL::RUBBERCHORDPOLAR) ar<<3;
		else if(pPolar->m_PolarType==XFOIL::FIXEDAOAPOLAR)    ar<<4;
		else                                   ar<<1;

		ar << pPolar->m_MaType << pPolar->m_ReType  ;
		ar << (int)pPolar->m_Reynolds << (float)pPolar->m_Mach ;
		ar << (float)pPolar->m_ASpec;
		ar << n << (float)pPolar->m_ACrit;
		ar << (float)pPolar->m_XTop << (float)pPolar->m_XBot;
		writeColor(ar, pPolar->m_red, pPolar->m_green, pPolar->m_blue);

		ar << pPolar->m_Style << pPolar->m_Width;
		if (pPolar->m_bIsVisible)  ar<<1; else ar<<0;
		ar<<pPolar->m_PointStyle;

		for (i=0; i< pPolar->m_Alpha.size(); i++){
			ar << (float)pPolar->m_Alpha[i] << (float)pPolar->m_Cd[i] ;
			ar << (float)pPolar->m_Cdp[i]   << (float)pPolar->m_Cl[i] << (float)pPolar->m_Cm[i];
			ar << (float)pPolar->m_XTr1[i]  << (float)pPolar->m_XTr2[i];
			ar << (float)pPolar->m_HMom[i]  << (float)pPolar->m_Cpmn[i];
			ar << (float)pPolar->m_Re[i];
			ar << (float)pPolar->m_XCp[i];
		}

		ar << pPolar->m_ACrit << pPolar->m_XTop << pPolar->m_XBot;

		return true;
	}
	else
	{
		//read variables
		float Alpha, Cd, Cdp, Cl, Cm, XTr1, XTr2, HMom, Cpmn, Re, XCp;
		int iRe;
		ar >> ArchiveFormat;
		if (ArchiveFormat <1001 ||ArchiveFormat>1100)
		{
			return false;
		}

		readString(ar, pPolar->m_FoilName);
		readString(ar, pPolar->m_PlrName);

		if(pPolar->m_FoilName =="" || pPolar->m_PlrName =="" )
		{
			return false;
		}

		ar >>k;
		if(k==1)      pPolar->m_PolarType = XFOIL::FIXEDSPEEDPOLAR;
		else if(k==2) pPolar->m_PolarType = XFOIL::FIXEDLIFTPOLAR;
		else if(k==3) pPolar->m_PolarType = XFOIL::RUBBERCHORDPOLAR;
		else if(k==4) pPolar->m_PolarType = XFOIL::FIXEDAOAPOLAR;
		else          pPolar->m_PolarType = XFOIL::FIXEDSPEEDPOLAR;


		ar >> pPolar->m_MaType >> pPolar->m_ReType;

		if(pPolar->m_MaType!=1 && pPolar->m_MaType!=2 && pPolar->m_MaType!=3)
		{
			return false;
		}
		if(pPolar->m_ReType!=1 && pPolar->m_ReType!=2 && pPolar->m_ReType!=3)
		{
			return false;
		}

		ar >> iRe;
		pPolar->m_Reynolds = (double) iRe;
		ar >> f; pPolar->m_Mach =f;

		ar >> f; pPolar->m_ASpec =f;

		ar >> n;
		ar >> f; pPolar->m_ACrit =f;
		ar >> f; pPolar->m_XTop =f;
		ar >> f; pPolar->m_XBot =f;

		readColor(ar, pPolar->m_red, pPolar->m_green, pPolar->m_blue);

		ar >> pPolar->m_Style >> pPolar->m_Width;

		if(ArchiveFormat>=1002)
		{
			ar >> l;
			if(l!=0 && l!=1 )
			{
				return false;
			}
			if (l) pPolar->m_bIsVisible =true; else pPolar->m_bIsVisible = false;
		}

		ar >> l;  pPolar->m_PointStyle =l;

		bool bExists;
		for (i=0; i< n; i++)
		{
			ar >> Alpha >> Cd >> Cdp >> Cl >> Cm;
			ar >> XTr1 >> XTr2;
			ar >> HMom >> Cpmn;

			if(ArchiveFormat >=4) ar >> Re;
			else                  Re = (float)pPolar->m_Reynolds;

			if(ArchiveFormat>=1004) ar>> XCp;
			else                    XCp = 0.0;

			bExists = false;
			if(pPolar->m_PolarType!=XFOIL::FIXEDAOAPOLAR)
			{
				for (j=0; j<pPolar->m_Alpha.size(); j++)
				{
					if(qAbs(Alpha-pPolar->m_Alpha[j])<0.001)
					{
						bExists = true;
						break;
					}
				}
			}
			else
			{
				for (j=0; j<pPolar->m_Re.size(); j++)
				{
					if(qAbs(Re-pPolar->m_Re[j])<0.1)
					{
						bExists = true;
						break;
					}
				}
			}
			if(!bExists)
			{
				pPolar->addPoint(Alpha, Cd, Cdp, Cl, Cm, XTr1, XTr2, HMom, Cpmn, Re, XCp);
			}
		}
		if(ArchiveFormat>=1003)
			ar >>pPolar->m_ACrit >> pPolar->m_XTop >> pPolar->m_XBot;
	}
	return true;
}

