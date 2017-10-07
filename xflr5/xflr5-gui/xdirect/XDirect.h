/****************************************************************************

	QXDirect Class
	Copyright (C) 2008-2016 Andre Deperrois adeperrois@xflr5.com

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
 * This file implements the QXDirect class which defines the GUI for foil analysis.
 *
*/

#ifndef QXDIRECT_H
#define QXDIRECT_H

#include <QPixmap>
#include <QSettings>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QStackedWidget>
#include <misc/LineBtn.h>
#include <misc/LineCbBox.h>
#include <misc/LineDelegate.h>
#include <misc/DoubleEdit.h>
#include <objects2d/Polar.h>
#include <objects2d/Foil.h>
#include <xdirect/analysis/XFoilAnalysisDlg.h>
#include <viewwidgets/oppointwidget.h>
#include <gui_enums.h>
#include <objects2d/OpPoint.h>
#include <QGraph.h>
#include <XFoil.h>


class MainFrame; // to shut the compiler up

/**
* @class QXDirect
* @brief This class is the general interface for Foil direct analysis.
* This is the handling class for the QXDirect right toolbar.
* It provides the methods to modify the foil geometry, define the Polar analysis, perform the analysis, and post-process the results.
* One of the very first class of this project.
*/
class QXDirect : public QWidget
{
	friend class MainFrame;
	friend class TwoDWidget;
	friend class XFoilAnalysisDlg;
	friend class BatchDlg;
	friend class BatchThreadDlg;
	friend class FoilPolarDlg;
	friend class EditPlrDlg;
	friend class XDirectStyleDlg;
	friend class Settings;
	friend class XFoilAdvancedDlg;
	friend class XDirectTileWidget;


    Q_OBJECT

public:
	QXDirect(QWidget *parent = NULL);
	~QXDirect();


signals:
	void projectModified();

public slots:
	void updateView();



public slots:
	void onXFoilAdvanced();

	void onPolarFilter();
	void onInputChanged();
	void onAnalyze();
	void onAnimate(bool bChecked);
	void onAnimateSpeed(int val);
	void onBatchAnalysis();
	void onMultiThreadedBatchAnalysis();
	void onCpi();
	void onCurOppOnly();
	void onCurveStyle(int index);
	void onCurveWidth(int index);
	void onCurvePoints(int index);
	void onCurveColor();
	void onDeleteCurFoil();
	void onDelCurOpp();
	void onDeleteCurPolar();
	void onDeleteFoilPolars();
	void onEditCurPolar();
	void onExportCurFoil();
	void onExportCurOpp();
	void onExportPolarOpps() ;
	void onExportCurPolar();
	void onExportAllPolars();
	void onExportXMLAnalysis();
	void onHideAllOpps();
	void onHideAllPolars();
	void onHideFoilPolars();
	void onImportJavaFoilPolar();
	void onImportXFoilPolars();
	void onImportXMLAnalysis();
	void onOpPointView();
	void onPolarView();
	void onResetAllPolarGraphsScales();
	void onResetCurPolar();
	void onSavePolars();
	void onSequence();
	void onShowFoilPolarsOnly();
	void onShowFoilPolars();
	void onShowCurve();
	void onShowAllOpps();
	void onShowAllPolars();
	void onDefinePolar();
	void onSpec();
	void onStoreOpp();
	void onViscous();
	void onDuplicateFoil();
	void onShowPolarOpps();
	void onHidePolarOpps();
	void onDeletePolarOpps();
	void onShowFoilOpps();
	void onHideFoilOpps();
	void onDeleteFoilOpps();
	void onDerotateFoil();
	void onNormalizeFoil();
	void onCadd();
	void onRefinePanelsGlobally();
	void onFoilCoordinates();
	void onFoilGeom();
	void onSetTEGap();
	void onSetLERadius();
	void onSetFlap();
	void onInterpolateFoils();
	void onNacaFoils();
	void onRenameCurPolar();
	void onAnimateSingle();

	void onQGraph();
	void onCpGraph();
	void onExportCurXFoilResults();

	void onOpPointProps();
	void onPolarProps();
	void onRenameCurFoil();


public:
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);

	void setControls();
	void connectSignals();
	void createOppCurves(OpPoint *pOpp= NULL);
	void createPolarCurves();
	void fillComboBoxes(bool bEnable = true);
	void fillPolarCurve(Curve *pCurve, Polar *pPolar, int XVar, int YVar);
	void fillOppCurve(OpPoint *pOpp, Graph *pGraph, Curve *pCurve, bool bInviscid=false);

	void getPolarProperties(Polar *pPolar, QString polarProps);

	void importAnalysisFromXML(QFile &xmlFile);
	Polar *importXFoilPolar(QFile &txtFile);

	void loadSettings(QSettings *pSettings);
	void readParams();
	Foil *addNewFoil(Foil *pFoil);
	void renameFoil(Foil *pFoil);
	void saveSettings(QSettings *pSettings);
	void setCurveParams();
	void setFoilScale();
	void setGraphTiles();
	void setOpPointSequence();
	void setAnalysisParams();
	void setGraphTitles(Graph* pGraph, int iX, int iY);
	void setGraphTitles(Graph* pGraph);
	void setupLayout();
	void setView(XFLR5::enumGraphView eView);
	void stopAnimate();
	void updateCurveStyle();

	void * getVariable(Polar *pPolar, int iVar);

	Foil* setFoil(Foil* pFoil=NULL);
	Polar *setPolar(Polar *pPolar=NULL);
	OpPoint *setOpp(double Alpha=-123456789.0);

	bool bPolarView() {return m_bPolarView;}
	QGraph *CpGraph(){return &m_CpGraph;}
	QGraph *PlrGraph(int iPlrGraph){return m_PlrGraph.at(iPlrGraph);}
	int PlrGraphSize(){return m_PlrGraph.count();}

	static void setCurFoil(Foil*pFoil)    {m_pCurFoil = pFoil;}
	static void setCurPolar(Polar*pPolar) {m_pCurPolar = pPolar;}
	static void setCurOpp(OpPoint* pOpp)  {m_pCurOpp = pOpp;}

	static Foil *   curFoil()  {return m_pCurFoil;}
	static Polar*   curPolar() {return m_pCurPolar;}
	static OpPoint* curOpp()   {return m_pCurOpp;}


private:
	XFoilAnalysisDlg* m_pXFADlg ;
	OpPointWidget *m_pOpPointWidget;

	QStackedWidget *m_pctrlMiddleControls;

	QTimer *m_pAnimateTimer;

	QLabel *m_pctrlUnit1, *m_pctrlUnit2, *m_pctrlUnit3;

	QRadioButton *m_pctrlSpec1;
	QRadioButton *m_pctrlSpec2;
	QRadioButton *m_pctrlSpec3;

	QCheckBox *m_pctrlSequence;
	DoubleEdit *m_pctrlAlphaMin;
	DoubleEdit *m_pctrlAlphaMax;
	DoubleEdit *m_pctrlAlphaDelta;

	QCheckBox *m_pctrlViscous;
	QCheckBox *m_pctrlInitBL;
	QCheckBox *m_pctrlStoreOpp;
	QPushButton *m_pctrlAnalyze;

	QCheckBox *m_pctrlShowBL, *m_pctrlShowPressure;
	QCheckBox* m_pctrlAnimate;
	QSlider* m_pctrlAnimateSpeed;

	QLabel *m_pctrlPolarProps;

	QCheckBox *m_pctrlShowCurve;
//	QCheckBox *m_pctrlShowPoints;
	LineCbBox *m_pctrlCurveStyle;
	LineCbBox *m_pctrlCurveWidth;
	LineCbBox *m_pctrlPointStyle;
	LineBtn *m_pctrlCurveColor;

	LineDelegate *m_pStyleDelegate, *m_pWidthDelegate, *m_pPointDelegate;

	static MainFrame *s_pMainFrame;  /**< a static pointer to the instance of the application's MainFrame object */

	static bool s_bViscous;           /**< true if performing a viscous calculation, false if inviscid */
	static bool s_bAlpha;             /**< true if performing an analysis based on aoa, false if based on Cl */
	static bool s_bInitBL;            /**< true if the boundary layer should be initialized for the next xfoil calculation */
	static bool s_bFromZero;          /**< true if the batch analysis should start from Alpha=0 */
	static bool s_bKeepOpenErrors;    /**< true if the XfoilAnalysisDlg should be kept open if errors occured in the XFoil calculation */
	static bool s_bStoreOpp;          /**< true if the operating points should be stored */

	bool m_bPolarView;         /**< true if the polar view is selected, false if the operating point view is selected */
	bool m_bShowUserGraph;     /**< true if the 5th polar graph should be displayed */
	bool m_bAnimate;           /**< true if a result animation is underway */
	bool m_bAnimatePlus;       /**< true if the animation is going from lower to higher alpha, false if decreasing */
//	bool m_bShowPanels;        /**< true if the panels should be displayed on the foil surface */
	bool m_bType1;             /**< true if the type 1 polars are to be displayed in the graphs */
	bool m_bType2;             /**< true if the type 2 polars are to be displayed in the graphs */
	bool m_bType3;             /**< true if the type 3 polars are to be displayed in the graphs */
	bool m_bType4;             /**< true if the type 4 polars are to be displayed in the graphs */
	bool m_bTrans;             /**< true if the user is dragging a view */
	bool m_bFromList;          /**< true if the batch analysis is based on a list of Re values */
	bool m_bShowTextOutput;    /**< true if the batch analysis should display text result output */
	bool m_bNeutralLine;       /**< true if the neutral line should be displayed */
	bool m_bCurOppOnly;        /**< true if only the current operating point should be displayed */
	bool m_bShowInviscid;      /**< true if the inviscid results should be displayed */
	bool m_bCpGraph;           /**< true if the Cp graph should be displayed */
	bool m_bSequence;          /**< true if a sequential analysis is to be performed */
	bool m_bXPressed;          /**< true if the 'X' key is pressed */
	bool m_bYPressed;          /**< true if the 'Y' key is pressed */
	bool m_bResetCurves;       /**< true if the graph curves need to be redrawn before the next view update */

	int m_posAnimate;          /**< the current aoa in the animation */

	static int s_TimeUpdateInterval;  /**< time interval in ms between two output display updates during an XFoil analysis */

	int m_iPlrGraph;           /**< defines whch polar graph is selected if m_iPlrView=1 */
	XFLR5::enumGraphView m_iPlrView;  /**< defines the number of graphs to be displayed in the polar view */
	int m_FoilYPos;            /**< y position for the foil display, in pixels from the bottom of the screen */

	double m_fFoilScale;        /**< the scale for foil display*/

	static QList<double> s_ReList;        /**< the user-defined list of Re numbers, used for batch analysis */
	static QList<double> s_MachList;      /**< the user-defined list of Mach numbers, used for batch analysis */
	static QList<double> s_NCritList;     /**< the user-defined list of NCrit numbers, used for batch analysis */

	double m_Alpha;             /**< the min value of the aoa for a sequential analysis of Type 1, 2, or 3*/
	double m_AlphaMax;          /**< the max value of the aoa for a sequential analysis of Type 1, 2, or 3*/
	double m_AlphaDelta;        /**< the increment value of the aoa for a sequential analysis of Type 1, 2, or 3*/
	double m_Cl;                /**< the min value of the lift coefficient for a sequential analysis of Type 1, 2, or 3*/
	double m_ClMax;             /**< the max value of the aoa for a sequential analysis of Type 1, 2, or 3*/
	double m_ClDelta;           /**< the increment value of the aoa for a sequential analysis of Type 1, 2, or 3*/
	double m_Reynolds;          /**< the min value of the Reynolds number for a sequential analysis of Type 4*/
	double m_ReynoldsMax;       /**< the max value of the Reynolds number for a sequential analysis of Type 4*/
	double m_ReynoldsDelta;     /**< the increment value of the Reynolds number for a sequential analysis of Type 4*/

//	double m_Mach;              /**< Stores the Mach number defined in the last Polar creation */
//	double m_ASpec;             /**< Stores the aoa defined in the last Type 4 Polar creation */
//	double m_NCrit;             /**< Stores the NCrit number defined in the last Polar creation */
//	double m_XTopTr;            /**< Stores the position of the top transition point defined in the last Polar creation */
//	double m_XBotTr;            /**< Stores the position of the bottom transition point defined in the last Polar creation */
//	enumPolarType m_PolarType;  /**< Stores the type of the last polar which has been created. */

	static Polar s_refPolar;    /**< Used a static reference to store default data */


	QList<Foil*> *m_poaFoil;    /**< pointer to the foil object array */
	QList<Polar*> *m_poaPolar;  /**< pointer to the polar object array */
	QList<OpPoint*> *m_poaOpp;  /**< pointer to the OpPoint object array */

	QGraph m_CpGraph;           /**< the Cp graph for the OpPoint view */
	QList<QGraph*> m_PlrGraph;  /**< the array of pointer to the 5 Polar graphs */

	LineStyle m_LineStyle;      /**< the style of the lines displayed in the comboboxes*/

	QRect m_rCltRect;		    /**< the client rectangle of the central TwoDWidget */
	QPoint m_PointDown;		    /**< the client coordinated of the last mouse left-click */

	XFoil m_XFoil;		        /**< the unique instance of the XFoil object */

	static Foil *m_pCurFoil;
	static Polar *m_pCurPolar;
	static OpPoint *m_pCurOpp;
};


#endif // QXDIRECT_H
