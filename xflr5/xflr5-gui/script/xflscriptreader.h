/****************************************************************************

	XFLScriptReader Class
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

#ifndef XFLSCRIPTREADER_H
#define XFLSCRIPTREADER_H
#include <QFile>
#include <QXmlStreamReader>
#include <QList>

class XFLScriptReader : public QXmlStreamReader
{
public:
	XFLScriptReader();
	bool readScript();

	bool readInput();
	bool readOutput();
	bool readOptions();
	bool readFoils();
	bool readFoilPolars();
	bool readFoilAnalysis();
	bool readPlanes();
	bool readPlanePolars();
	bool readPlaneAnalysis();

	QStringList m_planeList, m_wPolarList;
	QStringList m_foilList, m_polarList;

	double alphaMin, alphaMax, alphaInc;
	double qinfMin, qinfMax, qinfInc;

	QList<double> m_Reynolds, m_NCrit, m_Mach;
	double m_XtrTop, m_XtrBot;

	double aoaMin, aoaMax, aoaInc;
	double betaMin, betaMax, betaInc;
	double VInfMin, VInfMax, VInfInc;
	double ctrlMin, ctrlMax, ctrlInc;
	QString m_InputPlaneDirectoryPath, m_InputWPolarDirectoryPath;
	QString m_InputFoilDirectoryPath, m_InputPolarDirectoryPath;
	QString m_OutputDirectoryPath, m_projectFileName;
	bool m_bMakeXfl;
	bool m_bMakePOpps;
	bool m_bMultiThreading;
	int m_nMaxThreads;
};

#endif // XFLSCRIPTREADER_H
