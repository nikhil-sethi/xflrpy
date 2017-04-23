/****************************************************************************

    OpPoint Class
	Copyright (C) 2003 Andre Deperrois adeperrois@xflr5.com

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


#include "OpPoint.h"
#include "Foil.h"
#include "Polar.h"
#include "objects_global.h"
#include <QtDebug>



/**
 * The public constructor
 */
OpPoint::OpPoint()
{
	m_bViscResults = false;//not a  viscous point a priori
	m_bBL          = false;// no boundary layer surface either
	m_bTEFlap      = false;
	m_bLEFlap      = false;

	Xtr1   = 0.0;
	Xtr2   = 0.0;
	XForce = 0.0;
	YForce = 0.0;
	Cpmn   = 0.0;
	m_XCP  = 0.0;
	m_LEHMom   = 0.0; m_TEHMom = 0.0;

	memset(Qi,  0, sizeof(Qi));
	memset(Qv,  0, sizeof(Qv));
	memset(Cpi,  0, sizeof(Cpi));
	memset(Cpv,  0, sizeof(Cpv));
//	memset(x,  0, sizeof(x));
//	memset(y,  0, sizeof(y));

	memset(xd1,  0, sizeof(xd1));
	memset(xd2,  0, sizeof(xd2));
	memset(xd3,  0, sizeof(xd3));
	memset(yd1,  0, sizeof(yd1));
	memset(yd2,  0, sizeof(yd2));
	memset(yd3,  0, sizeof(yd3));
	nd1 = 0;
	nd2 = 0;
	nd3 = 0;
//	Format = 1;

	m_bIsVisible = true;
	m_PointStyle = 0;
	m_Style = 0;
	m_Width = 1;

	m_red   = (int)(((double)rand()/(double)RAND_MAX)*200);
	m_green = (int)(((double)rand()/(double)RAND_MAX)*200);
	m_blue  = (int)(((double)rand()/(double)RAND_MAX)*200);
	m_alphaChannel = 255;
}



/**
 * Calculates the moments acting on the flap hinges
 * @param pOpPoint
 */
void OpPoint::setHingeMoments(Foil *pFoil)
{
//	bool bFound;
	int i;
	double hmom, hfx, hfy;
	double dx, dy, xmid, ymid, pmid;
	double xof, yof;
	double ymin, ymax;
	xof = pFoil->m_TEXHinge/100.0;
	ymin = pFoil->baseLowerY(xof);
	ymax = pFoil->baseUpperY(xof);
	yof = ymin + (ymax-ymin) * pFoil->m_TEYHinge/100.0;

	if(pFoil->m_bTEFlap)
	{
		hmom = 0.0;
		hfx  = 0.0;
		hfy  = 0.0;

		//---- integrate pressures on top and bottom sides of flap
		for (i=0;i<pFoil->n-1;i++)
		{
			if (pFoil->x[i]>xof &&	pFoil->x[i+1]>xof)
			{
				dx = pFoil->x[i+1] - pFoil->x[i];
				dy = pFoil->y[i+1] - pFoil->y[i];
				xmid = 0.5*(pFoil->x[i+1]+pFoil->x[i]) - xof;
				ymid = 0.5*(pFoil->y[i+1]+pFoil->y[i]) - yof;

				if(m_bViscResults) pmid = 0.5*(Cpv[i+1] + Cpv[i]);
				else               pmid = 0.5*(Cpi[i+1] + Cpi[i]);

				hmom += pmid * (xmid*dx + ymid*dy);
				hfx  -= pmid * dy;
				hfy  += pmid * dx;
			}
		}
		//Next add top chunk left out in the previous loop
/*		bFound = false;
		for (i=0;i<MainFrame::g_ppCurFoil->n-1;i++){
			if(MainFrame::g_ppCurFoil->x[i]>xof && MainFrame::g_ppCurFoil->x[i+1]<xof){
				bFound =true;
				break;
			}
		}
		if(bFound) {
			dx = topx - x[i-1];
			dy = topy - y[i-1];
			xmid = 0.5*(topx+x[i-1]) - xof;
			ymid = 0.5*(topy+y[i-1]) - yof;
			if(s[i] != s[i-1]) frac = (tops-s[i-1])/(s[i]-s[i-1]);
			else	 frac = 0.0;

			if(lvisc) {
				topp = cpv[i]*frac + cpv[i-1]*(1.0-frac);
				pmid = 0.5*(topp+cpv[i-1]);
			}
			else{
				topp = cpi[i]*frac + cpi[i-1]*(1.0-frac);
				pmid = 0.5*(topp+cpi[i-1]);
			}
			hmom = hmom + pmid*(xmid*dx + ymid*dy);
			hfx  = hfx  - pmid* dy;
			hfy  = hfy  + pmid* dx;
		}*/
		//Then add bottom chunk left out


		//Next add internal hinge to top surface contribution

		//Next add internal hinge to bottom surface contribution

		//store the results
		m_TEHMom = hmom;
		XForce   = hfx;
		YForce   = hfy;
	}
}



/**
 * Exports the data of the polar to a text file
 * @param out the instance of output QtextStream
 * @param Version the version name of the program
 * @param FileType TXT if the data is separated by spaces, CSV for a comma separator
 * @param bDataOnly true if the analysis parameters should not be output
 */

void OpPoint::exportOpp(QTextStream &out, QString Version, bool bCSV, Foil*pFoil, bool bDataOnly)
{
	int k;
	QString strong;

	if(!bDataOnly)
	{
		out << Version+"\n";

		strong = m_FoilName + "\n";
		out<< strong;
		strong = m_PlrName + "\n";
		out<< strong;
		if(!bCSV) strong=QString("Alpha = %1,  Re = %2,  Ma = %3,  ACrit =%4 \n\n")
									   .arg(m_Alpha,5,'f',1).arg(m_Reynolds,8,'f',0).arg(m_Mach,6,'f',4).arg(ACrit,4,'f',1);
		else      strong=QString("Alpha =, %1,  Re =, %2,  Ma =, %3,  ACrit =, %4 \n\n")
									   .arg(m_Alpha,5,'f',1).arg(m_Reynolds,8,'f',0).arg(m_Mach,6,'f',4).arg(ACrit,4,'f',1);
		out<< strong;
	}

	if(!bCSV) out << "   x        Cpi      Cpv        Qi        Qv\n";
	else      out << "x,Cpi,Cpv,Qi,Qv\n";

	for (k=0; k<n; k++)
	{
		if(!bCSV) strong=QString("%1  %2   %3   %4   %5\n")
									   .arg(pFoil->x[k],7,'f',4).arg(Cpi[k],7,'f',3).arg(Cpv[k],7,'f',3).arg(Qi[k],7,'f',3).arg(Qv[k],7,'f',3);
		else      strong=QString("%1,%2,%3,%4,%5\n")
									   .arg(pFoil->x[k],7,'f',4).arg(Cpi[k],7,'f',3).arg(Cpv[k],7,'f',3).arg(Qi[k],7,'f',3).arg(Qv[k],7,'f',3);
		out<< strong;
	}

	out << "\n\n";
}


/**
 * Returns a QString object holding the description and value of the OpPoint's parameters
 * @param &OpPointProperties the reference of the QString object to be filled with the description
 * @param bData true if the analysis data should be appended to the string
 */
void OpPoint::getOppProperties(QString &OpPointProperties, Foil *pFoil, bool bData)
{
	QString strong;
	OpPointProperties.clear();

	strong  = QString(QObject::tr("Re")+"    = %1 ").arg(m_Reynolds,7,'f',0);
	OpPointProperties += strong +"\n";

	strong  = QString(QObject::tr("Alpha")+" = %1").arg(m_Alpha,6,'f',2);
	OpPointProperties += strong +QString::fromUtf8("°")+"\n";

	strong  = QString(QObject::tr("Mach")+"  = %1 ").arg(m_Mach,7,'f',3);
	OpPointProperties += strong + "\n";

	strong  = QString(QObject::tr("NCrit")+" = %1 ").arg(ACrit,5,'f',1);
	OpPointProperties += strong + "\n";

	strong  = QString(QObject::tr("CL")+"    = %1 ").arg(Cl,9,'f',5);
	OpPointProperties += strong + "\n";

	strong  = QString(QObject::tr("CD")+"    = %1 ").arg(Cd,9,'f',5);
	OpPointProperties += strong + "\n";

	strong  = QString(QObject::tr("Cm")+"    = %1 ").arg(Cm,9,'f',5);
	OpPointProperties += strong + "\n";

	strong  = QString(QObject::tr("Cdp")+"   = %1 ").arg(Cdp,9,'f',5);
	OpPointProperties += strong + "\n";

	strong  = QString(QObject::tr("Cpmn")+"  = %1 ").arg(Cpmn,9,'f',5);
	OpPointProperties += strong + "\n";

	strong  = QString(QObject::tr("XCP")+"   = %1 ").arg(m_XCP,9,'f',5);
	OpPointProperties += strong + "\n";

	strong  = QString(QObject::tr("Top Transition")+" = %1 ").arg(Xtr1,9,'f',5);
	OpPointProperties += strong + "\n";

	strong  = QString(QObject::tr("Bot Transition")+" = %1 ").arg(Xtr2,9,'f',5);
	OpPointProperties += strong + "\n";

	if(m_bTEFlap)
	{
		strong  = QString(QObject::tr("T.E. Flap moment")+" = %1 ").arg(m_TEHMom,9,'f',5);
		OpPointProperties += strong + "\n";
	}
	if(m_bLEFlap)
	{
		strong  = QString(QObject::tr("L.E. Flap moment")+" = %1 ").arg(m_LEHMom,9,'f',5);
		OpPointProperties += strong + "\n";
	}

	if(!bData) return;
	QTextStream out;
	strong.clear();
	out.setString(&strong);
	exportOpp(out, "", false, pFoil, true);
	OpPointProperties += "\n"+strong;
}


void OpPoint::getColor(int &r, int &g, int &b, int &a)
{
	r = m_red;
	g = m_green;
	b = m_blue;
	a = m_alphaChannel;
}


void OpPoint::setColor(int r, int g, int b, int a)
{
	m_red = r;
	m_green = g;
	m_blue = b;
	m_alphaChannel = a;
}


QString OpPoint::opPointName()
{
	QString name;
	name = QString("-Re=%1-Alpha=%2-NCrit=%3-XTrTop=%4-XtrBot=%5").arg(Reynolds(),8,'f',0).arg(aoa(),5,'f',2)
			.arg(ACrit, 5, 'f', 1).arg(Xtr1, 5, 'f', 3).arg(Xtr2, 5, 'f', 3);
	name = foilName()+name;
	return name;
}





/**
 * Loads or saves the data of this OpPoint to a binary file
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool OpPoint::serializeOppWPA(QDataStream &ar, bool bIsStoring, int ArchiveFormat)
{
	int a, b, k, Format;
	float f,gg;

	if(bIsStoring)
	{
		/** deprecated, we dont't store in .wpa format anymore */
	}
	else
	{

		if(ArchiveFormat>=100002) ar>>Format;
		else Format = 0;
		//read variables
		readCString(ar, m_FoilName);
		readCString(ar, m_PlrName);

		ar >> f; m_Reynolds =f;
		ar >> f; m_Mach = f;
		ar >> f; m_Alpha = f;
		ar >> n >> nd1 >> nd2 >> nd3;
		ar >> a >> b;
		if(a) m_bViscResults = true; else m_bViscResults = false;
		if(a!=0 && a!=1) return false;

		if(b) m_bBL = true; else m_bBL = false;
		if(b!=0 && b!=1) return false;

		ar >> f; Cl = f;
		ar >> f; Cm = f;
		ar >> f; Cd = f;
		ar >> f; Cdp = f;
		ar >> f; Xtr1 = f;
		ar >> f; Xtr2 = f;
		ar >> f; ACrit =f;
		ar >> f; m_TEHMom = f;
		ar >> f; Cpmn = f;
		for (k=0; k<n; k++)	{
			ar >> f; Cpv[k] = f;
			ar >> f; Cpi[k] = f;
		}

//			if (Format ==2) {
		for (k=0; k<n; k++)
		{
			if(Format<=100002)	ar >> f; //s[k]  = f;
			ar >> f; Qv[k] = f;
			ar >> f; Qi[k] = f;
		}
//			}
		for (k=0; k<=nd1; k++)
		{
			ar >> f >> gg;
			xd1[k] = f;
			yd1[k] = gg;
		}
		for (k=0; k<nd2; k++)
		{
			ar >> f >> gg;
			xd2[k] = f;
			yd2[k] = gg;
		}
		for (k=0; k<nd3; k++)
		{
			ar >> f >> gg;
			xd3[k] = f;
			yd3[k] = gg;
		}
		if(ArchiveFormat>=100002)
		{
			ar>>m_Style>>m_Width;
			readCOLORREF(ar, m_red,m_green,m_blue);

			ar >> a ;
			if(a!=0 && a!=1) return false;
			if(a) m_bIsVisible = true; else m_bIsVisible = false;

			ar >> a ;
			if(a!=0 && a!=1) return false;
			m_PointStyle = a;
		}
	}
	return true;
}


/**
 * Loads or saves the data of this OpPoint to a binary file
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool OpPoint::serializeOppXFL(QDataStream &ar, bool bIsStoring, int ArchiveFormat)
{
	bool boolean;
	int k;
	float f0,f1;
	double dble;

	if(bIsStoring)
	{
		ar << 200003;
		//200003 : first xfl format
		//write variables
		ar << m_FoilName;
		ar << m_PlrName;

		ar << m_Style << m_Width;
		writeqColor(ar, m_red, m_green, m_blue, m_alphaChannel);
		ar << m_bIsVisible << false;

		ar << m_Reynolds << m_Mach << m_Alpha;
		ar << n << nd1 << nd2 << nd3;

		ar << m_bViscResults;
		ar << m_bBL;

		ar << Cl << Cm << Cd << Cdp;
		ar << Xtr1 << Xtr2 << m_XCP;
		ar << ACrit << m_TEHMom << Cpmn;

		for (k=0; k<n; k++)	     ar << (float)Cpv[k] << (float)Cpi[k];
		for (k=0; k<n; k++)	     ar << (float)Qv[k]  << (float)Qi[k];
		for (k=0; k<=nd1; k++)   ar << (float)xd1[k] << (float)yd1[k];
		for (k=0; k<nd2; k++)	ar << (float)xd2[k] << (float)yd2[k];
		for (k=0; k<nd3; k++)	ar << (float)xd3[k] << (float)yd3[k];

		// space allocation for the future storage of more data, without need to change the format
		for (int i=0; i<20; i++) ar << 0;
		for (int i=0; i<50; i++) ar << (double)0.0;
	}
	else
	{

		ar >> ArchiveFormat;
		//write variables
		ar >> m_FoilName;
		ar >> m_PlrName;

		ar >> m_Style >> m_Width;
		readqColor(ar, m_red, m_green, m_blue, m_alphaChannel);
		ar >> m_bIsVisible >> boolean;

		ar >> m_Reynolds >> m_Mach >> m_Alpha;
		ar >> n >> nd1 >> nd2 >> nd3;

		ar >> m_bViscResults;
		ar >> m_bBL;

		ar >> Cl >> Cm >> Cd >> Cdp;
		ar >> Xtr1 >> Xtr2 >> m_XCP;
		ar >> ACrit >> m_TEHMom >> Cpmn;

		for (k=0; k<n; k++)
		{
			ar >> f0 >> f1;
			Cpv[k] = f0;
			Cpi[k] = f1;
		}
		for (k=0; k<n; k++)
		{
			ar >> f0 >> f1;
			Qv[k] = f0;
			Qi[k] = f1;
		}
		for (k=0; k<=nd1; k++)
		{
			ar >> f0 >> f1;
			xd1[k] = f0;
			yd1[k] = f1;
		}
		for (k=0; k<nd2; k++)
		{
			ar >> f0 >> f1;
			xd2[k] = f0;
			yd2[k] = f1;
		}
		for (k=0; k<nd3; k++)
		{
			ar >> f0 >> f1;
			xd3[k] = f0;
			yd3[k] = f1;
		}

		// space allocation
		for (int i=0; i<20; i++) ar >> k;
		for (int i=0; i<50; i++) ar >> dble;
	}
	return true;
}





