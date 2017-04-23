/****************************************************************************

	Miarex    Copyright (C) 2008-2016 Andre Deperrois adeperrois@xflr5.com

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
 *@file
 *
 * This file implementsthe QMiarex class associated to the GUI of 3D analysis
 *
 */


#ifndef QMIAREX_H
#define QMIAREX_H

class MainFrame;

#include "gl3dmiarexview.h"
#include <QWidget>
#include <QPixmap>
#include <QLabel>
#include <QCheckBox>
#include <QSlider>
#include <QStackedWidget>
#include <QToolButton>
#include <QPushButton>
#include <QRadioButton>
#include <QList>
#include <QDialog>
#include <QSettings>
#include <QXmlStreamWriter>
#include <gui_params.h>
#include "./analysis/PanelAnalysisDlg.h"
#include "./analysis/LLTAnalysisDlg.h"
#include <plane_analysis/planeanalysistask.h>
#include <misc/LineBtn.h>
#include <misc/DoubleEdit.h>
#include <misc/MinTextEdit.h>
#include <misc/LineCbBox.h>
#include <misc/LineDelegate.h>
#include <objects3d/Body.h>
#include <objects3d/Wing.h>
#include <objects3d/Plane.h>
#include <objects3d/WPolar.h>
#include <objects3d/WingOpp.h>
#include <objects3d/PlaneOpp.h>
#include "view/GLLightDlg.h"
#include <QGraph.h>
#include "graphtilewidget.h"

/**
 *@class QMiarex
 *@brief This is the class associated to the 3D calculations

  It is the handling class for the Miarex toolbar and it dispatches user commands towards
  object definition, analysis and post-processing.
  The main methods of this class are:
	- Management of wings, planes, polars, and operating points
	- Construction of the panels for 3D calculations
	- Management of the display
	- Management of the LLT and Panel Analysis,
	- Mapping of the analyis results to the operating point and polar objects
*/
class QMiarex : public QWidget
{
	friend class MainFrame;
	friend class TwoDWidget;
	friend class GL3DScales;
	friend class GL3dBodyDlg;
	friend class WingDlg;
	friend class Wing;
	friend class LLTAnalysisDlg;
	friend class StabPolarDlg;
	friend class StabViewDlg;
	friend class PanelAnalysisDlg;
	friend class Plane;
	friend class PlaneDlg;
	friend class ManageBodiesDlg;
	friend class ManageUFOsDlg;
	friend class GL3dWingDlg;
	friend class Settings;
	friend class UFOTableDelegate;
	friend class WPolarDlg;
	friend class WPolar;

	Q_OBJECT


public:

	QMiarex(QWidget *parent = NULL);
	~QMiarex();

signals:
	void projectModified();

private slots:
	void on3DCp();
	void on3DPrefs();
	void on3DResetScale();
	void on3DView();
	void onAdjustToWing();
	void onAdvancedSettings();
	void onAnalyze();
	void onAnimateWOpp();
	void onAnimateWOppSingle();
	void onAnimateWOppSpeed(int val);
	void onAnimateModeSingle(bool bStep=true);
	void onCheckViewIcons();
	void onCpSectionSlider(int pos);
	void onCpPosition();
	void onCpView();
	void onCurveColor();
	void onCurveStyle(int index);
	void onCurveWidth(int index);
	void onCurvePoints(int index);
	void onCurWOppOnly();
	void onDefineStabPolar();
	void onDefineWPolar();
	void onDefineWPolarObject();
	void onDeleteAllWOpps();
	void onDeleteAllWPlrOpps();
	void onDeleteCurPlane();
	void onDeleteCurWOpp();
	void onDeleteCurWPolar();
	void onDeletePlanePOpps();
	void onDeletePlaneWPolars();
	void onDownwash();
	void onDuplicateCurPlane();
	void onEditCurWPolar();
	void onEditCurWPolarPts();
	void onEditCurWing();
	void onEditCurBody();
	void onEditCurBodyObject();
	void onEditCurPlane();
	void onEditCurObject();
	void onEditCurWPolarObject();
	void onExporttoAVL();
	void onExporttoSTL();
	void onExportAnalysisToXML();
	void onImportSTLFile();
	void onExportCurPOpp();
	void onExportCurWPolar();
	void onExportPlanetoXML();
	void onExportWPolars();
	void onFinCurve();
	void onGL3DScale();
	void onHideAllWOpps();
	void onHideAllWPlrOpps();
	void onHideAllWPolars();
	void onHidePlaneOpps();
	void onHidePlaneWPolars();
	void onImportFromXml();
	void onImportPlanesfromXML();
	void onImportAnalysisFromXML();
	void onImportWPolars();
	void onInitLLTCalc();
	void onKeepCpSection();
	void onManagePlanes();
	void onModalView();
	void onMoment();
	void onNewPlane();
	void onNewPlaneObject();
	void onPanelForce();
	void onPlaneOppProperties();
	void onPlaneInertia();
	void onPolarFilter();
	void onReadAnalysisData();
	void onRenameCurPlane();
	void onRenameCurWPolar();
	void onResetCpSection();
	void onRootLocusView();
	void onResetCurWPolar();
	void onSetupLight();
	void onScaleWing();
	void onSequence();
	void onShowAllWOpps();
	void onShowAllWPlrOpps();
	void onShowAllWPolars();
	void onShowCurve();
	void onShowFlapMoments();
	void onShowTargetCurve();
	void onShowLift();
	void onShowIDrag();
	void onShowTransitions();
	void onShowPlaneWPolars();
	void onShowPlaneWPolarsOnly();
	void onShowPlaneOpps();
	void onShowVDrag();
	void onShowWPolarOppsOnly();
	void onShowXCmRef();
	void onStabCurve();
	void onStabilityDirection();
	void onStoreWOpp();
	void onStreamlines();
	void onSurfaces();
	void onSurfaceSpeeds();
	void onStabTimeView();
	void onWing2Curve();
	void onWOppView();
	void onWPolarView();
	void onWPolarProperties();

public:
	//overrides
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
	void showEvent(QShowEvent *event);

public:
	//class methods
	WPolar* addWPolar(WPolar* pWPolar);
	void connectSignals();
	void clearCpCurves();
	void createCpCurves();
	void createWPolarCurves();
	void createWOppCurves();
	void createStabilityCurves();
	void createStabRLCurves();
	void createStabTimeCurves();
	void createStabRungeKuttaCurves();
	void drawTextLegend();
	void exportAVLWing_Old(Wing *pWing, QTextStream &out, int index, double y, double Thetay);
	void exportAVLWing(Wing *pWing, QTextStream &out, int index, double y, double Thetay);
	void exportToTextStream(WPolar *pWPolar, QTextStream &out, XFLR5::enumTextFileType FileType, bool bDataOnly=false);
	void fillComboBoxes(bool bEnable = true);
	void fillWPlrCurve(Curve *pCurve, WPolar *pWPolar, int XVar, int YVar);
	void fillWOppCurve(WingOpp *pWOpp, Graph *pGraph, Curve *pCurve);
	void fillStabCurve(Curve *pCurve, WPolar *pWPolar, int iMode);
	void getPolarProperties(WPolar *pWPolar, QString &polarProps, bool bData=false);
	void glMake3DObjects();
	void importPlaneFromXML(QFile &xmlFile);
	void importWPolarFromXML(QFile &xmlFile);
	bool intersectObject(Vector3d O,  Vector3d U, Vector3d &I);
	void LLTAnalyze(double V0, double VMax, double VDelta, bool bSequence, bool bInitCalc);	
	bool loadSettings(QSettings *pSettings);
	int  matSize() {return m_theTask.m_MatSize;}
	void paintCpLegendText(QPainter &painter);
	void paintPanelForceLegendText(QPainter &painter);
	void panelAnalyze(double V0, double VMax, double VDelta, bool bSequence);
	void paintPlaneLegend(QPainter &painter, Plane *pPlane, WPolar *pWPolar, QRect drawRect);
	void paintPlaneOppLegend(QPainter &painter, QRect drawRect);
	QString POppTitle(PlaneOpp *pPOpp);
	void renamePlane(QString PlaneName);
	bool saveSettings(QSettings *pSettings);
	void setAnalysisParams();
	void setControls();
	void setCurveParams();
	void setGraphTiles();
	bool setPlaneOpp(bool bCurrent, double x = 0.0);
	PlaneOpp* setPlaneOppObject(Plane *pPlane, WPolar *pWPolar, PlaneOpp *pCurPOpp, bool bCurrent, double x);
	void setScale();
	void setStabGraphTitles();
	void setPlane(QString PlaneName="");
	void setupLayout();
	void setViewControls();
	void setView(XFLR5::enumGraphView eView);
	void setWGraphScale();
	void setWGraphTitles(Graph* pGraph);
	void setWPolar(bool bCurrent = true, QString WPlrName = "");
	void snapClient(QString const &FileName);
	void stopAnimate();
	void updateCurve();
	void updateUnits();
	void updateView();
	static QString WPolarVariableName(int iVar);


	PlaneOpp *curPOpp(){return m_pCurPOpp;}
	bool curPOppOnly(){return m_bCurPOppOnly;}

	Wing *pWing(int iw);

//____________________Variables______________________________________
//

public:
	LLTAnalysisDlg *m_pLLTDlg;                    /**< the dialog class which manages the LLT calculations */
	LLTAnalysis m_theLLTAnalysis;

	PanelAnalysisDlg *m_pPanelAnalysisDlg;        /**< the dialog class which manages the 3D VLM and Panel calculations */
	PanelAnalysis m_thePanelAnalysis;

	PlaneAnalysisTask m_theTask;


	// Widget variables ... self explicit, not documented
	QPushButton *m_pctrlKeepCpSection, *m_pctrlResetCpSection;
	QSlider *m_pctrlCpSectionSlider;
	DoubleEdit *m_pctrlSpanPos;
	QCheckBox *m_pctrlSequence;
	DoubleEdit *m_pctrlAlphaMin;
	DoubleEdit *m_pctrlAlphaMax;
	DoubleEdit *m_pctrlAlphaDelta;
	QCheckBox *m_pctrlInitLLTCalc;
	QCheckBox *m_pctrlStoreWOpp;
	QPushButton *m_pctrlAnalyze;

	QCheckBox *m_pctrlPanelForce, *m_pctrlLift, *m_pctrlIDrag, *m_pctrlVDrag, *m_pctrlTrans, *m_pctrlWOppAnimate;
	QSlider *m_pctrlAnimateWOppSpeed;
	QCheckBox *m_pctrlMoment,  *m_pctrlDownwash, *m_pctrlCp,*m_pctrlSurfVel, *m_pctrlStream;

	QCheckBox *m_pctrlShowCurve;
	LineCbBox *m_pctrlCurveStyle, *m_pctrlCurveWidth, *m_pctrlCurvePoints;
	LineBtn *m_pctrlCurveColor;
	LineDelegate *m_pStyleDelegate, *m_pWidthDelegate, *m_pPointDelegate;

	QCheckBox *m_pctrlAxes, *m_pctrlLight, *m_pctrlSurfaces, *m_pctrlOutline, *m_pctrlPanels;
	QCheckBox *m_pctrlFoilNames, *m_pctrlMasses;

	QAction *m_pXView, *m_pYView, *m_pZView, *m_pIsoView, *m_pFlipView;
	QToolButton *m_pctrlX, *m_pctrlY, *m_pctrlZ, *m_pctrlIso, *m_pctrlFlip;

	QPushButton *m_pctrl3DResetScale;
	QSlider *m_pctrlClipPlanePos;

	QLabel *m_pctrlUnit1, *m_pctrlUnit2, *m_pctrlUnit3;
	QLabel *m_pctrlParameterName;

	//stability widgets
	MinTextEdit *m_pctrlPolarProps;
	QStackedWidget *m_pctrlBottomControls, *m_pctrlMiddleControls;


public:

// Class variables

	bool m_b3DCp;                      /**< true if the Cp Colors are to be displayed on the 3D openGl view */
	bool m_bICd;                       /**< true if the induced drag forces should be displayed in the operating point or 3D view*/
	bool m_bVCd;                       /**< true if the viscous drag forces should be displayed in the operating point or 3D view*/
	bool m_bAnimateWOpp;               /**< true if there is an animation going on for an operating point */
	bool m_bAnimateMode;               /**< true if there is an animation going on for a Mode */
	bool m_bAnimateWOppPlus;           /**< true if the animation is going in aoa crescending order */
	bool m_bCrossPoint;                /**< true if the control point on the arcball is to be displayed */
	bool m_bCurPOppOnly;               /**< true if only the current WOpp is to be displayed */
	bool m_bCurFrameOnly;              /**< true if only the currently selected body frame is to be displayed */
	bool m_bDirichlet;                 /**< true if Dirichlet BC are applied in 3D panel analysis, false if Neumann */
	bool m_bDownwash;                  /**< true if the arrows represeting downwash are to be displayed on the 3D openGl view */
	bool m_bInitLLTCalc;               /**< true if the LLT parameters should be set to default prior to the analysis. Otherwise, the iterations will start at the results of the previous calculation */
	bool m_bIs2DScaleSet;              /**< true if the 2D scale has been set, false if needs to be reset */
	bool m_bLongitudinal;              /**< true if longitudinal stability results are to be displayed, false if lateral */
	bool m_bMoments;                   /**< true if the arrows representing moments are to be displayed on the 3D openGl view */
	bool m_bPanelForce;                /**< true if the forces acting on the panels are to be displayed in the 3D view */
	bool m_bPickCenter;                /**< true if the user is in the process of picking a new center for OpenGL display */
	bool m_bResetWake;                 /**< true if the wake geometry should be reset to its default shape prior to the analysis */
	bool m_bSequence;                  /**< true if a sequential analysis is to be performed */
	bool m_bShowCp;                    /**< true if the active curve should be displayed in Cp view */
	bool m_bShowCpScale;               /**< true if the Cp Scale in Miarex is to be displayed */
	bool m_bShowEllipticCurve;         /**< true if the elliptic loading should be displayed in the local lift graph */
	bool m_bShowBellCurve;             /**< true if the bell distribution loading should be displayed in the local lift graph */
	bool m_bShowWingCurve[MAXWINGS];   /**< true if various plane's wing curves shoud be displayed*/
	bool m_bShowFlapMoments;           /**< true if the flap moment values should be display together with the operating point results*/
	bool m_bSurfVelocities;            /**< true if the velocities should be displayed in the operating point or 3D view*/
	bool m_bStream;                    /**< true if the streamlines should be displayed in the operating point or 3D view*/
	bool m_bTrans;                     /**< true if the view is being dragged */
	bool m_bTransGraph;	               /**< true if a graph is being dragged */
	bool m_bType1;                     /**< true if polars of type 1 are to be displayed */
	bool m_bType2;                     /**< true if polars of type 2 are to be displayed */
	bool m_bType4;                     /**< true if polars of type 4 are to be displayed */
	bool m_bType7;                     /**< true if polars of type 71 are to be displayed */
	bool m_bPanelNormals;              /**< true if the panel normals should be displayed */
	bool m_bXCmRef; 	               /**< true if the position of the reference point for the moments should be displayed in the operating point view*/
	bool m_bXBot;                      /**< true if the transition on the bottom surface should be displayed in the operating point or in 3D view*/
	bool m_bXCP;                       /**< true if the lift curve should be displayed in the operating point or in the 3D view*/
	bool m_bXTop;                      /**< true if the transition on the top surface should be displayed in the operating point or in 3D view */
	bool m_bXPressed;                  /**< true if the X key is pressed */
	bool m_bYPressed;                  /**< true if the Y key is pressed */

	static bool m_bLogFile;			       /**< true if the log file warning is turned on */


	static bool m_bResetglGeom;               /**< true if the geometry OpenGL list needs to be re-generated */
	static bool m_bResetglMesh;               /**< true if the mesh OpenGL list needs to be re-generated */
	static bool m_bResetglWake;               /**< true if the wake OpenGL list needs to be re-generated */
	static bool m_bResetglOpp;                /**< true if the OpenGL lists need to be re-generated */
	static bool m_bResetglLift;               /**< true if the OpenGL lists need to be re-generated */
	static bool m_bResetglDrag;               /**< true if the OpenGL lists need to be re-generated */
	static bool m_bResetglDownwash;           /**< true if the OpenGL lists need to be re-generated */
	static bool m_bResetglPanelForce;         /**< true if the OpenGL lists need to be re-generated */
	static bool m_bResetglPanelCp;            /**< true if the OpenGL lists need to be re-generated */
	static bool m_bResetglStream;             /**< true if the streamlines OpenGL list needs to be re-generated */
	static bool m_bResetglLegend;             /**< true if the legend needs to be reset if the window has been resized */
	static bool m_bResetglBody;               /**< true if the openGL list for the body needs to be re-generated */
	static bool m_bResetglBodyMesh;           /**< true if the openGL list for panel mesh needs to be re-generated */
	static bool m_bResetglSurfVelocities;     /**< true if the crossflow OpenGL list needs to be refreshed */

	static bool s_bResetCurves;               /**< true if the curves of the active view should be regenerated before the next view update >*/

	static bool s_bAutoCpScale;		          /**< true if the Cp scale should be set automatically */
	static double s_LegendMin;                /**< minimum value of the Cp scale in 3D view */
	static double s_LegendMax;                /**< maximum value of the Cp scale in 3D view */

	static double s_LiftScale;                /**< scaling factor for the lift display in 3D view */
	static double s_VelocityScale;            /**< scaling factor for the velocity display in 3D view */
	static double s_DragScale;                /**< scaling factor for the drag display in 3D view */

	PlaneOpp * m_pCurPOpp;                    /**< a pointer to the active Plane Operating Point, or NULL if none is active*/



	bool m_bCurveVisible;                     /**< true if the active curve is to be displayed */

	LineStyle m_LineStyle;                    /**< the style of the lines displayed in the comboboxes*/
	LineStyle m_CpLineStyle;                    /**< the style of the lines displayed in the comboboxes*/


	QTimer *m_pTimerWOpp;         /**< A pointer to the timer which signals the animation in the operating point and 3D view */
	QTimer *m_pTimerMode;         /**< A pointer to the timer which signals the animation of the modes in the 3D view */


	WingOpp *m_pWOpp[MAXWINGS];   /**< an array of pointers to the operating points of the four wings of the currently selected plane */


	static QList<void *> *m_poaPlane;			/**< for convenienece, a pointer to the array of pointers to plane objects */
	static QList<void *> *m_poaWPolar;			/**< for convenienece, a pointer to the array of UFO polar objects */
	static QList<void *> *m_poaPOpp;			/**< for convenienece, a pointer to the array of Plane OpPoint objects */


	Plane * m_pCurPlane;          /**< the currently active Plane */
	WPolar * m_pCurWPolar;        /**< the currently active WPolar */

	int m_StabilityResponseType;   /**< 0 = initial conditions, 1=forced response, 2=modal response */

	XFLR5::enumGraphView m_iWPlrView;              /**< defines how many graphs will be displayed in WPolar view */
	XFLR5::enumGraphView m_iWingView;              /**< defines how many graphs will be displayed in the operating point view */
	XFLR5::enumGraphView m_iRootLocusView;         /**< defines how many graphs will be displayed in the root locus view */
	XFLR5::enumGraphView m_iStabTimeView;          /**< defines how many graphs will be displayed in the stability time view */

	int m_InducedDragPoint;     /**< 0 if downwash is at panel's centroid, 1 if averaged over panel length; used in CWing::VLMTrefftz */
	int m_LLTMaxIterations;     /**< the number of iterations for LLT */
	int m_posAnimateWOpp;       /**< the current animation aoa ind ex for WOpp animation */
	int m_posAnimateMode;       /**< the current animation aoa index for Mode animation */
	int m_WakeInterNodes;		/**< number of intermediate nodes between wake panels */

	XFLR5::enumMiarexViews m_iView;    /**< defines the currently active view */


	double m_CurSpanPos;        /**< Span position for Cp Graph  */

	double m_AlphaMin;          /**< the min value of the aoa for sequential analysis*/
	double m_AlphaMax;          /**< the max value of the aoa for sequential analysis*/
	double m_AlphaDelta;        /**< the increment value of the aoa for sequential analysis*/
	double m_BetaMin;           /**< the min value of the sideslip for sequential analysis*/
	double m_BetaMax;           /**< the max value of the sideslip for sequential analysis*/
	double m_BetaDelta;         /**< the increment value of the sideslip for sequential analysis*/
	double m_ControlMin;        /**< the min value of the control parameter for sequential stability analysis*/
	double m_ControlMax;        /**< the max value of the control parameter for sequential stability analysis*/
	double m_ControlDelta;      /**< the increment value of the control parameter for sequential stability analysis*/
	double m_QInfMin;           /**< the min value of the velocity for sequential analysis */
	double m_QInfMax;           /**< the max value of the velocity for sequential analysis */
	double m_QInfDelta;         /**< the increment value of the velocity for sequential analysis */

	// mode animation data
	double m_ModeState[6];      /**< defines the value of the 6 dofs (x, y, z, rx, ry, rz) to display the position and orientation of the geometry */
	double m_ModeNorm;          /**< defines the amplitude of the modal animation */
	double m_ModeTime;          /**< defines the time moment for the modal animation */
	double m_Modedt;            /**< defines the time increment for the modal animation */
	
	// time curve data
	double m_TimeInput[4];      /**< defines the initial state for the display of the dynamic response in stability time graphs */
	double m_TotalTime;         /**< defines the total time for the display of the dynamic response in stability time graphs */
	double m_Deltat;            /**< defines the time increment for the display of the dynamic response in stability time graphs */
	double m_RampTime;          /**< defines the ramp time for the display of the dynamic response in stability time graphs */
	double m_RampAmplitude;     /**< defines the ramp amplitude for the display of the dynamic response in stability time graphs */

	double m_BellCurveExp;
	bool m_bMaxCL;

	QGraph m_CpGraph;                       /**< the Cp Graph in 3D panel analysis */
	QList<QGraph*> m_WingGraph;             /**< the array of pointer to the OpPoint graphs */
	QList<QGraph*> m_WPlrGraph;             /**< the array of pointers to the WPolar graphs */
	QList<QGraph*> m_StabPlrGraph;          /**< the array of pointers to the two root locus graphs */
	QList<QGraph*> m_TimeGraph;             /**< the array of pointers to the time response graphs in stability view */

	bool m_bResetTextLegend;
	QPixmap m_PixText;

	double m_LastAlpha;          /**< last angle of attack selected in the top list box>*/
	double m_LastBeta;           /**< last sideslip angle selected in the top list box>*/


public:
	static MainFrame *s_pMainFrame;       /**< a pointer to the frame class */
	gl3dMiarexView *m_pGL3dView;              /**< a pointer to the openGL 3.0 widget where 3d calculations and rendering are performed */



};

#endif // QMIAREX_H
