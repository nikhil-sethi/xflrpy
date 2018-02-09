/****************************************************************************

	BatchDlg Class
	Copyright (C) 2003-2017 Andre Deperrois adeperrois@xflr5.com

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
/**
* @file BatchDlg
* This file implements the interface to manage a batch analysis of Foils.
*/
#ifndef BATCHDLG_H
#define BATCHDLG_H

#include <QDialog>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QRadioButton>
#include <QTextEdit>
#include <QGraph.h>
#include "graphwidget.h"
#include <objects2d/Foil.h>
#include <objects2d/Polar.h>
#include <misc/text/IntEdit.h>
#include <misc/text/DoubleEdit.h>
#include <xdirect/analysis/XFoilTask.h>
#include <xdirect/analysis/xfoiltaskevent.h>



/**
* @class XFoilAnalysisDlg
* This class provides an interface to manage a batch analysis of Foils.
*/
class BatchDlg : public QDialog
{
	Q_OBJECT
	friend class QXDirect;
	friend class MainFrame;
	friend class QReListDlg;
public:
	BatchDlg(QWidget *pParent=NULL);
	~BatchDlg();


	void handleXFoilTaskEvent(const XFoilTaskEvent *event);
	void customEvent(QEvent * event);

private:
	void keyPressEvent(QKeyEvent *event);
	void showEvent(QShowEvent *event);
	void hideEvent(QHideEvent *event);
	void reject();

	void alphaLoop();
	void cleanUp();
	Polar* createPolar(Foil *pFoil, double Spec, double Mach, double NCrit);
	void setupLayout();
	void initDialog();
	void readParams();
	void ReLoop();
	void resetCurves();
	void setFileHeader();
	void setPlrName(Polar *pPolar);
	void analyze();
	void outputMsg(QString &msg);


private slots:
	void onAnalyze();
	void onRange();
	void onAcl();
	void onEditReList();
	void onPolarType();
	void onClose();
	void onSkipPoint();
	void onSkipPolar();
	void onFoilList();
	void onFoilSelectionType();
	void onProgress();
	void onAnalysisSettings();

private:
    static QPoint s_Position;   /**< the position on the client area of the dialog's topleft corner */

	QRadioButton * m_pctrlFoil1, * m_pctrlFoil2;
	QPushButton *m_pctrlFoilList;
	QRadioButton *m_rbtype1, *m_rbtype2, *m_rbtype3, *m_rbtype4;
	QRadioButton *m_rbRange1, *m_rbRange2;
	QRadioButton *m_rbspec1, *m_rbspec2;
	QPushButton *m_pctrlEditList;
	IntEdit *m_pctrlMaxIter;
	DoubleEdit *m_pctrlReMin, *m_pctrlReMax, *m_pctrlReDelta, *m_pctrlMach;
	DoubleEdit *m_pctrlSpecMin, *m_pctrlSpecMax, *m_pctrlSpecDelta;
	DoubleEdit *m_pctrlNCrit, *m_pctrlXTopTr, *m_pctrlXBotTr;
	QLabel *m_pctrlSpecVar;
	QLabel *m_pctrlMaType, *m_pctrlReType;
	QCheckBox *m_pctrlInitBLOpp, *m_pctrlInitBLPolar;
	QCheckBox *m_pctrlFromZero, *m_pctrlStoreOpp;
	QPushButton *m_pctrlSkipOpp, *m_pctrlSkipPolar;
	QPushButton *m_pctrlClose, *m_pctrlAnalyze;
	QTextEdit *m_pctrlTextOutput;
	GraphWidget *m_pctrlGraphOutput;

	static void* s_pXDirect;     /**< a void pointer to the instance of the QXDirect object >*/
	static bool s_bCurrentFoil;  /**< true if the analysis should be performed on the active Foil, false if on a list of Foils>*/
	XFOIL::enumPolarType m_PolarType;   /**< the type of Polar to be created for the analysis>*/

	double m_Mach;               /**< the Mach number for the Polars and the analysis>*/

	double m_ReMin, m_ReMax, m_ReInc;  /**< The range of Re values to analyze>*/
	double m_SpMin, m_SpMax, m_SpInc;  /**< The range of specified aoa, Cl or Re values, depending on the type of Polar and on the user-specified input method.>*/

	double m_AlphaMin, m_AlphaMax, m_AlphaInc;  /**< The range of aoa for a Type 1/2/3 Polar >*/
	double m_ClMin, m_ClMax, m_ClInc;           /**< The range of lift coefficient for a Type 1/2/3 Polar>*/

	double m_ACrit;                /**< The free transition parameter to be used in the Polar creation>*/
	double m_XTop;                 /**< The forced transition point on the upper surface to be used in the Polar creation>*/
	double m_XBot;                 /**< The forced transition point on the lower surface to be used in the Polar creation>*/

	bool m_bAlpha;                 /**< true if the analysis should be performed for a range of aoa, false if for a range of licf coefficient.>*/
	bool m_bFromList;              /**< true if the analysis should be performed on a list of (Re, Ma, Ncrit) sets, false if for a range of Reynolds numbers.>*/
	bool m_bFromZero;              /**< true if the analysis should start at alpha=0 or Cl=0, false if from min to max.>*/
	bool m_bInitBL;                /**< true if the boundary layer should be initialized after an unconverged iteration>*/
	bool m_bCancel;                /**< true if the analysis is in the process of being cancelled>*/
	bool m_bIsRunning;             /**< true if the analysis is running>*/
	bool m_bErrors;                /**< true if the analysis has generated errors>*/

	QFile *m_pXFile;               /**< a pointer to the log file>*/

	Foil *m_pFoil;                 /**< a pointer to the Foil for which the analysis will be run>*/

	QStringList m_FoilList;        /**< the list of Foil objects to analyze>*/

	QGraph *m_pRmsGraph;           /**< a pointer to the output graph >*/

	XFoilTask *m_pXFoilTask;       /**< A pointer to the instance of the XFoilTask associated to this batch analysis. 
	                                    The task is unique to the instance of this class, and re-used each time a new analysis is launched.>*/
};

#endif // BATCHDLG_H
