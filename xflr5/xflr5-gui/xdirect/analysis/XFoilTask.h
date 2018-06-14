/****************************************************************************

	XFoilTask Class
	   Copyright (C) 2011-2017 Andre Deperrois 

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

/** @file This file implements the management task of an XFoil calculation. Used in multithreaded analysis. */

#ifndef XFOILTASK_H
#define XFOILTASK_H

#include <QRunnable>

#include "XFoil.h"

#include <objects/objects2d/Polar.h>
#include <objects/objects2d/Foil.h>



/**
 * @struct Analysis a pair of foil and polar used by a thread to perform one foil polar analysis.
 */
struct FoilAnalysis
{
	Foil *pFoil;            /**< a pointer to the Foil to be analyzed by the thread */
	Polar *pPolar;          /**< a pointer to the polar to be analyzed by the thread */
	double vMin, vMax, vInc;
};

// this class runs an XFoil analysis in a thread separate from the main thread

/**
*@class XFoilTask
* This file implements the management task of an XFoil calculation. Used in multithreaded analysis. 
*/
class XFoilTask : public QRunnable
{
public:
	XFoilTask(void *pParent = NULL);


public:
	void run();
	bool alphaSequence();
	bool ReSequence();
    bool isFinished(){return m_bIsFinished;}

	bool initializeTask(FoilAnalysis *pFoilAnalysis, bool bStoreOpp, bool bViscous=true, bool bInitBL=true, bool bFromZero=false);
	bool initializeTask(Foil *pFoil, Polar *pPolar, bool bStoreOpp, bool bViscous=true, bool bInitBL=true, bool bFromZero=false);
	bool iterate();

	void setSequence(double bAlpha, double SpMin, double SpMax, double SpInc);
	void setReRange(double ReMin, double ReMax, double ReInc);
	void traceLog(QString str);

	void setGraphPointers(QVarLengthArray<double, 1024> *x0, QVarLengthArray<double, 1024> *y0, QVarLengthArray<double, 1024> *x1,QVarLengthArray<double, 1024> *y1)
	{
		m_x0 = x0;
		m_y0 = y0;
		m_x1 = x1;
		m_y1 = y1;
	}

	static bool s_bSkipPolar;
	static bool s_bCancel;          /**< true if the user has asked to cancel the analysis */
	static bool s_bAutoInitBL;      /**< true if the BL initialization is left to the code's decision */
	static int s_IterLim;
	static bool s_bSkipOpp;
	
	int m_Iterations;        /**< The number of iterations already performed */
	bool m_bIsFinished;      /**< true if the calculation is over */
	XFoil XFoilInstance;     /**< An instance of the XFoil class specific for this object */

	QTextStream m_OutStream;
	QString m_OutMessage;
	QString m_XFoilLog;
	QTextStream m_XFoilStream;

	//	Curve Data
	QVarLengthArray<double, 1024> *m_x0, *m_x1, *m_y0,*m_y1;


	double m_AlphaMin, m_AlphaMax, m_AlphaInc;
	double m_ClMin, m_ClMax, m_ClInc;
	double m_ReMin, m_ReMax, m_ReInc;

	bool m_bAlpha, m_bFromZero, m_bInitBL, m_bErrors;

	bool m_bStoreOpp;

	QList<OpPoint*> m_OppList;

	void *m_pParent;

private:
	Foil *m_pFoil;           /**< A pointer to the instance of the Foil object for which the calculation is performed */
	Polar *m_pPolar;         /**< A pointer to the instance of the Polar object for which the calculation is performed */
};

#endif // XFOILTASK_H
