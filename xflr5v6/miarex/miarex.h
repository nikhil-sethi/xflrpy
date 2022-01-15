/****************************************************************************

    Miarex
    Copyright (C) André Deperrois

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


#pragma once

class MainFrame;

#include <QCheckBox>
#include <QDialog>
#include <QGroupBox>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <QSlider>
#include <QStackedWidget>
#include <QToolButton>
#include <QVector>
#include <QWidget>

#include <xflcore/core_enums.h>

#include "./analysis/panelanalysisdlg.h"
#include "./analysis/lltanalysisdlg.h"
#include <xflanalysis/plane_analysis/planetask.h>
#include <xflgraph/graph.h>
#include <xflcore/linestyle.h>


//forward declarations
class Body;
class Curve;
class DoubleEdit;
class GLLightDlg;
class GraphTileWidget;
class LLTAnalysisDlg;
class LineBtn;
class LineCbBox;
class LineDelegate;
class LinePickerWt;
class MinTextEdit;
class PanelAnalysisDlg;
class Plane;
class PlaneOpp;
class PlaneTask;
class PlaneTreeView;
class WPolar;
class Wing;
class WingOpp;
class gl3dMiarexView;

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
class Miarex : public QWidget
{
    friend class GL3DScales;
    friend class GL3dBodyDlg;
    friend class GL3dWingDlg;
    friend class LLTAnalysisDlg;
    friend class MainFrame;
    friend class ManageUFOsDlg;
    friend class PanelAnalysisDlg;
    friend class Plane;
    friend class PlaneDlg;
    friend class PlaneTreeView;
    friend class Settings;
    friend class StabPolarDlg;
    friend class StabViewDlg;
    friend class TwoDWidget;
    friend class WPolar;
    friend class WPolarDlg;
    friend class Wing;
    friend class WingDlg;

    Q_OBJECT


    public:

        Miarex(QWidget *parent = nullptr);
        ~Miarex();

        bool isPOppView()      const {return m_iView==xfl::WOPPVIEW;}
        bool isPolarView()     const {return m_iView==xfl::WPOLARVIEW;}
        bool isCpView()        const {return m_iView==xfl::WCPVIEW;}
        bool is3dView()        const {return m_iView==xfl::W3DVIEW;}
        bool isStabPolarView() const {return m_iView==xfl::STABPOLARVIEW;}
        bool isStabTimeView()  const {return m_iView==xfl::STABTIMEVIEW;}
        bool isStabilityView() const {return isStabPolarView() || isStabTimeView();}

        static void resetCurves() {s_bResetCurves = true;}


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
        void onMoment();
        void onNewPlane();
        void onNewPlaneObject();
        void onPanelForce();
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
        void onTaskFinished();
        void onWing2Curve();
        void onWOppView();
        void onWPolarView();

    public:
        //overrides
        void keyPressEvent(QKeyEvent *event) override;
        void keyReleaseEvent(QKeyEvent *event) override;
        void showEvent(QShowEvent *event) override;

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
        void exportToTextStream(const WPolar *pWPolar, QTextStream &out, xfl::enumTextFileType FileType, bool bDataOnly=false);
        void fillWPlrCurve(Curve *pCurve, const WPolar *pWPolar, int XVar, int YVar);
        void fillWOppCurve(WingOpp const*pWOpp, Graph *pGraph, Curve *pCurve);
        void fillStabCurve(Curve *pCurve, WPolar const *pWPolar, int iMode);
        void getPolarProperties(WPolar const *pWPolar, QString &polarProps, bool bData=false);
        void importPlaneFromXML(QFile &xmlFile);
        void importWPolarFromXML(QFile &xmlFile);
        bool intersectObject(Vector3d O,  Vector3d U, Vector3d &I);
        void LLTAnalyze(double V0, double VMax, double VDelta, bool bSequence, bool bInitCalc);
        bool loadSettings(QSettings &settings);
        int  matSize() {return m_theTask.matSize();}
        void drawColorGradient(QPainter &painter, QRect const & gradientRect);
        void paintCpLegendText(QPainter &painter);
        void paintPanelForceLegendText(QPainter &painter);
        void panelAnalyze(double V0, double VMax, double VDelta, bool bSequence);
        void paintPlaneLegend(QPainter &painter, const Plane *pPlane, const WPolar *pWPolar, const QRect &drawRect, float devicePixelRatio);
        void paintPlaneOppLegend(QPainter &painter, QRect drawRect);
        QString POppTitle(PlaneOpp *pPOpp);
        void renamePlane(QString PlaneName);
        bool saveSettings(QSettings &settings);
        void setAnalysisParams();
        void setControls();
        void setGraphTiles();
        bool setPlaneOpp(PlaneOpp *pPOpp);
        bool setPlaneOpp(bool bCurrent, double x);
        void setScale();
        void setStabGraphTitles();
        void setPlane(Plane*pPlane);
        void setPlane(const QString &PlaneName="");
        void setupLayout();
        void setViewControls();
        void setView(xfl::enumGraphView eView);
        void setWGraphScale();
        void setWGraphTitles(Graph* pGraph);
        void setWPolar(WPolar*pWPolar=nullptr);
        void setWPolar(bool bCurrent, QString const&WPlrName);
        void snapClient(QString const &FileName);
        void stopAnimate();
        void updateCurve();
        void updateUnits();
        void updateView();
        void updateTreeView();

        static QString WPolarVariableName(int iVar);

        Plane* curPlane() {return m_pCurPlane;}
        WPolar *curWPolar() {return m_pCurWPolar;}
        PlaneOpp *curPOpp(){return m_pCurPOpp;}

        bool curPOppOnly() const {return m_bCurPOppOnly;}

        Wing *pWing(int iw);

        //____________________Variables______________________________________
        //

    public:
        LLTAnalysisDlg *m_pLLTDlg;                    /**< the dialog class which manages the LLT calculations */
        LLTAnalysis m_theLLTAnalysis;

        PanelAnalysisDlg *m_pPanelAnalysisDlg;        /**< the dialog class which manages the 3D VLM and Panel calculations */
        PanelAnalysis m_thePanelAnalysis;

        PlaneTask m_theTask;

        PlaneTreeView *m_pPlaneTreeView;

        // Widget variables ... self explicit, not documented
        QPushButton *m_ppbKeepCpSection, *m_ppbResetCpSection;
        QSlider *m_pslCpSectionSlider;
        DoubleEdit *m_pdeSpanPos;
        QCheckBox *m_pchSequence;
        DoubleEdit *m_pdeAlphaMin;
        DoubleEdit *m_pdeAlphaMax;
        DoubleEdit *m_pdeAlphaDelta;
        QCheckBox *m_pchInitLLTCalc;
        QCheckBox *m_pchStoreWOpp;
        QPushButton *m_ppbAnalyze;

        QCheckBox *m_pchPanelForce, *m_pchLift, *m_pchIDrag, *m_pchVDrag, *m_pchTrans, *m_pchWOppAnimate;
        QSlider *m_pslAnimateWOppSpeed;
        QCheckBox *m_pchMoment,  *m_pchDownwash, *m_pchCp,*m_pchSurfVel, *m_pchStream;

        QCheckBox *m_pchAxes, *m_pchLight, *m_pchSurfaces, *m_pchOutline, *m_pchPanels;
        QCheckBox *m_pchFoilNames, *m_pchMasses;

        QAction *m_pXView, *m_pYView, *m_pZView, *m_pIsoView, *m_pFlipView;
        QToolButton *m_ptbX, *m_ptbY, *m_ptbZ, *m_ptbIso, *m_ptbFlip;

        QPushButton *m_ppb3DResetScale;
        QSlider *m_pslClipPlanePos;

        QLabel *m_plabUnit1, *m_plabUnit2, *m_plabUnit3;
        QLabel *m_plabParameterName;

        //stability widgets
        QFrame *m_pswBottomControls;
        QGroupBox *m_pThreeDViewBox, *m_pCpBox;

    public:

        // Class variables

        bool m_b3DCp;                      /**< true if the Cp Colors are to be displayed on the 3D openGl view */
        bool m_bICd;                       /**< true if the induced drag forces should be displayed in the operating point or 3D view*/
        bool m_bVCd;                       /**< true if the viscous drag forces should be displayed in the operating point or 3D view*/
        bool m_bAnimateWOpp;               /**< true if there is an animation going on for an operating point */
        bool m_bAnimateMode;               /**< true if there is an animation going on for a Mode */
        bool m_bAnimateWOppPlus;           /**< true if the animation is going in aoa crescending order */
        bool m_bCurPOppOnly;               /**< true if only the current WOpp is to be displayed */
        bool m_bCurFrameOnly;              /**< true if only the currently selected body frame is to be displayed */
        bool m_bDirichlet;                 /**< true if Dirichlet BC are applied in 3D panel analysis, false if Neumann */
        bool m_bDownwash;                  /**< true if the arrows represeting downwash are to be displayed on the 3D openGl view */
        bool m_bInitLLTCalc;               /**< true if the LLT parameters should be set to default prior to the analysis. Otherwise, the iterations will start at the results of the previous calculation */
        bool m_bIs2DScaleSet;              /**< true if the 2D scale has been set, false if needs to be reset */
        bool m_bLongitudinal;              /**< true if longitudinal stability results are to be displayed, false if lateral */
        bool m_bMoments;                   /**< true if the arrows representing moments are to be displayed on the 3D openGl view */
        bool m_bPanelForce;                /**< true if the forces acting on the panels are to be displayed in the 3D view */
        bool m_bSequence;                  /**< true if a sequential analysis is to be performed */
        bool m_bShowCp;                    /**< true if the active curve should be displayed in Cp view */
        bool m_bShowEllipticCurve;         /**< true if the elliptic loading should be displayed in the local lift graph */
        bool m_bShowBellCurve;             /**< true if the bell distribution loading should be displayed in the local lift graph */
        bool m_bShowWingCurve[MAXWINGS];   /**< true if various plane's wing curves shoud be displayed*/
        bool m_bShowFlapMoments;           /**< true if the flap moment values should be display together with the operating point results*/
        bool m_bTrans;                     /**< true if the view is being dragged */
        bool m_bTransGraph;                   /**< true if a graph is being dragged */
        bool m_bType1;                     /**< true if polars of type 1 are to be displayed */
        bool m_bType2;                     /**< true if polars of type 2 are to be displayed */
        bool m_bType4;                     /**< true if polars of type 4 are to be displayed */
        bool m_bType7;                     /**< true if polars of type 71 are to be displayed */
        bool m_bXCmRef;                    /**< true if the position of the reference point for the moments should be displayed in the operating point view*/
        bool m_bXBot;                      /**< true if the transition on the bottom surface should be displayed in the operating point or in 3D view*/
        bool m_bXCP;                       /**< true if the lift curve should be displayed in the operating point or in the 3D view*/
        bool m_bXTop;                      /**< true if the transition on the top surface should be displayed in the operating point or in 3D view */
        bool m_bXPressed;                  /**< true if the X key is pressed */
        bool m_bYPressed;                  /**< true if the Y key is pressed */

        static bool s_bLogFile;                   /**< true if the log file warning is turned on */



        static bool s_bResetCurves;               /**< true if the curves of the active view should be regenerated before the next view update >*/


        PlaneOpp * m_pCurPOpp;                    /**< a pointer to the active Plane Operating Point, or NULL if none is active*/


        bool m_bCurveVisible;                     /**< true if the active curve is to be displayed */

        LineStyle m_LineStyle;                    /**< the style of the lines displayed in the comboboxes*/
        LineStyle m_CpLineStyle;                    /**< the style of the lines displayed in the comboboxes*/


        QTimer *m_pTimerWOpp;         /**< A pointer to the timer which signals the animation in the operating point and 3D view */
        QTimer *m_pTimerMode;         /**< A pointer to the timer which signals the animation of the modes in the 3D view */


        WingOpp *m_pWOpp[MAXWINGS];   /**< an array of pointers to the operating points of the four wings of the currently selected plane */


        Plane * m_pCurPlane;          /**< the currently active Plane */
        WPolar * m_pCurWPolar;        /**< the currently active WPolar */

        int m_StabilityResponseType;   /**< 0 = initial conditions, 1=forced response, 2=modal response */

        xfl::enumGraphView m_iWPlrView;              /**< defines how many graphs will be displayed in WPolar view */
        xfl::enumGraphView m_iWingView;              /**< defines how many graphs will be displayed in the operating point view */
        xfl::enumGraphView m_iRootLocusView;         /**< defines how many graphs will be displayed in the root locus view */
        xfl::enumGraphView m_iStabTimeView;          /**< defines how many graphs will be displayed in the stability time view */

        int m_InducedDragPoint;     /**< 0 if downwash is at panel's centroid, 1 if averaged over panel length; used in CWing::VLMTrefftz */
        int m_LLTMaxIterations;     /**< the number of iterations for LLT */
        int m_posAnimateWOpp;       /**< the current animation aoa ind ex for WOpp animation */
        int m_posAnimateMode;       /**< the current animation aoa index for Mode animation */
        int m_WakeInterNodes;        /**< number of intermediate nodes between wake panels */

        xfl::enumMiarexViews m_iView;    /**< defines the currently active view */


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

        Graph m_CpGraph;                       /**< the Cp Graph in 3D panel analysis */
        QVector<Graph*> m_WingGraph;             /**< the array of pointer to the OpPoint graphs */
        QVector<Graph*> m_WPlrGraph;             /**< the array of pointers to the WPolar graphs */
        QVector<Graph*> m_StabPlrGraph;          /**< the array of pointers to the two root locus graphs */
        QVector<Graph*> m_TimeGraph;             /**< the array of pointers to the time response graphs in stability view */

        bool m_bResetTextLegend;
        QPixmap m_PixText;

        double m_LastAlpha;          /**< last angle of attack selected in the top list box>*/
        double m_LastBeta;           /**< last sideslip angle selected in the top list box>*/

    public:
        static MainFrame *s_pMainFrame;       /**< a pointer to the frame class */
        gl3dMiarexView *m_pgl3dMiarexView;              /**< a pointer to the openGL 3.0 widget where 3d calculations and rendering are performed */
};


