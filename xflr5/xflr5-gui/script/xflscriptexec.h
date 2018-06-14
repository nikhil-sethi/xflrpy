/****************************************************************************

	xflScriptExec Class
	Copyright (C) 2016-2016 Andre Deperrois 

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

#ifndef XFLSCRIPTEXEC_H
#define XFLSCRIPTEXEC_H
#include <script/xflscriptreader.h>
#include <QTextStream>

#include <objects/objects3d/Plane.h>
#include <objects/objects3d/WPolar.h>

#include <analysis3d/plane_analysis/planeanalysistask.h>
#include <xdirect/analysis/XFoilTask.h>

class XflScriptExec
{
public:
	XflScriptExec();
	~XflScriptExec();
	void readScript();
	void runScript();
	void makeFoils();
	void makePlanes();
	void loadFoilPolarFiles();
	void makeFoilAnalysisList();
	void makePlaneAnalysisList();

private:
	void setLogFile();
	void traceLog(QString strMsg);
	Polar *makePolar(QString pathName);
	WPolar *makeWPolar(QString pathName);

private:
	XFLScriptReader m_scriptReader;
	QFile *m_pXFile;
	QTextStream m_outLogStream;

	QList<FoilAnalysis*> m_FoilExecList;
	QList<PlaneAnalysis*> m_PlaneExecList;

	QList <Plane *> m_oaPlane;   /**< The array of void pointers to the Plane objects. */
	QList <WPolar *> m_oaWPolar;  /**< The array of void pointers to the WPolar objects. */
	QList <PlaneOpp *> m_oaPOpp;    /**< The array of void pointers to the PlaneOpp objects. */
	QList <Foil*>  m_oaFoil;
	QList <Polar*> m_oaPolar;
};

#endif // XFLSCRIPTEXEC_H
