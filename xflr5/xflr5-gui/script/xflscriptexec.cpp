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
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QtDebug>

#include <globals/globals.h>
#include "xflscriptexec.h"
#include <misc/options/displayoptions.h>
#include <miarex/mgt/XmlPlaneReader.h>
#include <miarex/mgt/xmlwpolarreader.h>
#include <xdirect/xmlpolarreader.h>

XflScriptExec::XflScriptExec()
{
	m_pXFile = NULL;

	setLogFile();
}


XflScriptExec::~XflScriptExec()
{
	for(int ip=0; ip<m_PlaneExecList.count(); ip++)
		delete m_PlaneExecList.at(ip);

	for(int ip=0; ip<m_oaPlane.count(); ip++)
		delete m_oaPlane.at(ip);

	for(int ip=0; ip<m_oaWPolar.count(); ip++)
		delete m_oaWPolar.at(ip);

	for(int ip=0; ip<m_oaPOpp.count(); ip++)
		delete m_oaPOpp.at(ip);
	if(m_pXFile)
	{
		m_pXFile->close();
		delete m_pXFile;
	}
}


void XflScriptExec::makePlaneAnalysisList()
{
	m_PlaneExecList.clear();

	for(int ip=0; ip<m_oaPlane.count(); ip++)
	{
		Plane *pPlane = m_oaPlane.at(ip);
		if(pPlane)
		{
			for(int iwp=0; iwp<m_scriptReader.m_wPolarList.count(); iwp++)
			{
				WPolar *pWPolar = makeWPolar(m_scriptReader.m_wPolarList.at(iwp));
				if(pWPolar)
				{
					PlaneAnalysis *pAnalysis = new PlaneAnalysis;
					pAnalysis->pPlane = pPlane;
					pAnalysis->pWPolar = pWPolar;
					switch(pWPolar->polarType())
					{
						case XFLR5::FIXEDSPEEDPOLAR:
						case XFLR5::FIXEDLIFTPOLAR:
						{
							pAnalysis->vMin = m_scriptReader.aoaMin;
							pAnalysis->vMax = m_scriptReader.aoaMax;
							pAnalysis->vInc = m_scriptReader.aoaInc;
							break;
						}
						case XFLR5::FIXEDAOAPOLAR:
						{
							pAnalysis->vMin = m_scriptReader.qinfMin;
							pAnalysis->vMax = m_scriptReader.qinfMax;
							pAnalysis->vInc = m_scriptReader.qinfInc;
							break;
						}
						case XFLR5::STABILITYPOLAR:
						{
							pAnalysis->vMin = m_scriptReader.ctrlMin;
							pAnalysis->vMax = m_scriptReader.ctrlMax;
							pAnalysis->vInc = m_scriptReader.ctrlInc;
							break;
						}
						case XFLR5::BETAPOLAR:
						{
							pAnalysis->vMin = m_scriptReader.betaMin;
							pAnalysis->vMax = m_scriptReader.betaMax;
							pAnalysis->vInc = m_scriptReader.betaInc;
							break;
						}
						default:
						{
							break;
						}
					}
					m_PlaneExecList.append(pAnalysis);
					traceLog("   added analysis for plane "+pPlane->planeName()+" and "+pWPolar->polarName());
				}
			}
		}
	}
}


void XflScriptExec::makeFoilAnalysisList()
{
	m_PlaneExecList.clear();

	for(int ip=0; ip<m_oaFoil.count(); ip++)
	{
		Foil *pFoil = m_oaFoil.at(ip);
		if(pFoil)
		{
			for(int ip=0; ip<m_scriptReader.m_polarList.count(); ip++)
			{
				Polar *pPolar = makePolar(m_scriptReader.m_wPolarList.at(ip));
				if(pPolar)
				{
					FoilAnalysis *pFoilAnalysis = new FoilAnalysis;
					pFoilAnalysis->pFoil = pFoil;
					pFoilAnalysis->pPolar = pPolar;
					switch(pPolar->polarType())
					{
						case XFLR5::FIXEDSPEEDPOLAR:
						case XFLR5::FIXEDLIFTPOLAR:
						{
							pFoilAnalysis->vMin = m_scriptReader.alphaMin;
							pFoilAnalysis->vMax = m_scriptReader.alphaMax;
							pFoilAnalysis->vInc = m_scriptReader.alphaInc;
							break;
						}
						case XFLR5::FIXEDAOAPOLAR:
						{
							pFoilAnalysis->vMin = m_scriptReader.qinfMin;
							pFoilAnalysis->vMax = m_scriptReader.qinfMax;
							pFoilAnalysis->vInc = m_scriptReader.qinfInc;
							break;
						}
						default:
						{
							break;
						}
					}
					m_FoilExecList.append(pFoilAnalysis);
					traceLog("   added analysis for foil "+pFoil->foilName()+" and "+pPolar->polarName());
				}
			}
		}
	}
}



void XflScriptExec::loadFoilPolarFiles()
{
	for(int ifo=0; ifo<m_scriptReader.m_polarList.count(); ifo++)
	{
		QString polarPathName = m_scriptReader.m_InputPolarDirectoryPath+QDir::separator()+m_scriptReader.m_polarList.at(ifo);
		QFile plrFile(polarPathName);
		if (!plrFile.open(QIODevice::ReadOnly))
		{
			QString strange = "Could not read the file"+polarPathName;
			traceLog(strange);
		}
		else
		{
			QList<Polar*> polarList;

			Foil *pFoil = (Foil*)readPolarFile(plrFile, polarList);
			if(pFoil)
			{
				traceLog("   adding foil: "+pFoil->foilName());
				m_oaFoil.append(pFoil);
				for(int iplr=0; iplr<polarList.count(); iplr++)
				{
					m_oaPolar.append(polarList.at(iplr));
					traceLog("      adding polar: "+m_oaPolar.at(iplr)->polarName());
				}
			}
			else
			{
				traceLog("   failed to add the foil from: "+polarPathName);
			}
		}
	}
}


void XflScriptExec::makeFoils()
{
	for(int ifo=0; ifo<m_scriptReader.m_foilList.count(); ifo++)
	{
		QString foilPathName = m_scriptReader.m_InputFoilDirectoryPath+QDir::separator()+m_scriptReader.m_foilList.at(ifo);
		QFile datFile(foilPathName);
		if (!datFile.open(QIODevice::ReadOnly))
		{
			QString strange = "Could not read the file"+foilPathName;
			traceLog(strange);

		}
		else
		{
			Foil *pFoil = (Foil*)readFoilFile(datFile);
			if(pFoil)
			{
				traceLog("   adding foil: "+pFoil->foilName());
				m_oaFoil.append(pFoil);
			}
			else
			{
				traceLog("   failed to add the foil from "+foilPathName);
			}
		}
	}
}


void XflScriptExec::makePlanes()
{
	QString planeDirName = m_scriptReader.m_InputPlaneDirectoryPath;

	for(int ip=0; ip<m_scriptReader.m_planeList.count(); ip++)
	{
		QString planePathName = planeDirName+QDir::separator()+m_scriptReader.m_planeList.at(ip);
		QFile xmlFile(planePathName);
		if (!xmlFile.open(QIODevice::ReadOnly))
		{
			QString strange = "Could not read the file"+planePathName;
			traceLog(strange);
		}
		else
		{
			Plane *pPlane = new Plane;
			XMLPlaneReader xplReader(xmlFile, pPlane);
			if(xplReader.readXMLPlaneFile())
			{
				traceLog("   adding plane: "+pPlane->planeName());
				m_oaPlane.append(pPlane);
			}
			else
			{
				QString errorMsg;
				errorMsg.sprintf("line %d column %d",(int)xplReader.lineNumber(),(int)xplReader.columnNumber());
				traceLog("   failed to add the plane "+pPlane->planeName() +xplReader.errorString() + errorMsg);
			}
		}
	}
}


Polar* XflScriptExec::makePolar(QString fileName)
{
	QString pathName = m_scriptReader.m_InputWPolarDirectoryPath+QDir::separator()+fileName;
	QFile xmlFile(pathName);
	if (!xmlFile.open(QIODevice::ReadOnly))
	{
		QString strange = "Could not read the file"+pathName;
		traceLog(strange);
		return NULL;
	}
	else
	{
		Polar *pPolar = new Polar;
		XmlPolarReader xpReader(xmlFile, pPolar);
		xpReader.readXMLPolarFile();
		m_oaPolar.append(pPolar);
		return pPolar;
	}
	return NULL;
}


WPolar* XflScriptExec::makeWPolar(QString fileName)
{
	QString pathName = m_scriptReader.m_InputWPolarDirectoryPath+QDir::separator()+fileName;
	QFile xmlFile(pathName);
	if (!xmlFile.open(QIODevice::ReadOnly))
	{
		QString strange = "Could not read the file"+pathName;
		traceLog(strange);
		return NULL;
	}
	else
	{
		WPolar *pWPolar = new WPolar;
		XmlWPolarReader xwpReader(xmlFile, pWPolar);
		xwpReader.readXMLPolarFile();
		m_oaWPolar.append(pWPolar);
		return pWPolar;
	}
	return NULL;
}


void XflScriptExec::setLogFile()
{
	QString FileName = QDir::tempPath() + "/XFLR5.log";
	m_pXFile = new QFile(FileName);
	if(m_pXFile->open(QIODevice::WriteOnly | QIODevice::Text))
	{
		m_outLogStream.setDevice(m_pXFile);
		m_outLogStream << "\n";
		m_outLogStream << VERSIONNAME;
		m_outLogStream << "\n";
		QDateTime dt = QDateTime::currentDateTime();
		QString str = dt.toString("dd.MM.yyyy  hh:mm:ss");
		m_outLogStream << str<<"\n";

		m_outLogStream.flush();
	}
}

void XflScriptExec::traceLog(QString strMsg)
{
	m_outLogStream << strMsg;
	m_outLogStream.flush();
}



void XflScriptExec::readScript()
{
	QString PathName = ":/src/script/example_script.xml";
/*	PathName = QFileDialog::getOpenFileName(NULL, "Open XML File",
											Settings::s_LastDirName,
											"XML file (*.xml)");
	if(!PathName.length())		return ;*/

	int pos = PathName.lastIndexOf("/");
	if(pos>0) Settings::s_LastDirName = PathName.left(pos);

	QFile xmlFile(PathName);
	if (!xmlFile.open(QIODevice::ReadOnly))
	{
		QString strange = "Could not read the file"+PathName;
		traceLog(strange);
		return;
	}

	m_scriptReader.setDevice(&xmlFile);
	m_scriptReader.readScript();
	if(m_scriptReader.hasError())
	{
		QString strange;
		strange.sprintf("\nline %d column %d", (int)m_scriptReader.lineNumber(), (int)m_scriptReader.columnNumber());
		QString errorMsg = m_scriptReader.errorString() + strange;
		traceLog(errorMsg);
	}
	else
	{
	}
}




void XflScriptExec::runScript()
{
	for(int ia=0; ia<m_PlaneExecList.count(); ia++)
	{
		PlaneAnalysis *pAnalysis = m_PlaneExecList.at(ia);
		PlaneAnalysisTask aTask;
		aTask.initializeTask(pAnalysis);
	}
}















