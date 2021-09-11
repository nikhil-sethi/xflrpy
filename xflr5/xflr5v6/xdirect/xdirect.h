/****************************************************************************

    XDirect Class
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

/** @file
 *
 * This file implements the QXDirect class which defines the GUI for foil analysis.
 *
*/

#pragma once

#include <QSettings>
#include <QVector>
#include <QStackedWidget>
#include <QLabel>
#include <QRadioButton>
#include <QSlider>
#include <QGroupBox>

#include <xflobjects/objects2d/polar.h>
#include <xflobjects/objects2d/foil.h>
#include <xdirect/analysis/xfoilanalysisdlg.h>
#include <xdirect/oppointwt.h>
#include <xflcore/core_enums.h>
#include <xflobjects/objects2d/oppoint.h>
#include <xflgraph/graph.h>

#include <xfoil.h>

class FoilTreeView;
class LineBtn;
class LineCbBox;
class LineDelegate;
class MainFrame; // to shut the compiler up
class DoubleEdit;
class MinTextEdit;
class LinePickerWt;

/**
* @class XDirect
* @brief This class is the general interface for Foil direct analysis.
* This is the handling class for the QXDirect right toolbar.
* It provides the methods to modify the foil geometry, define the Polar analysis, perform the analysis, and post-process the results.
* One of the very first class of this project.
*/
class XDirect : public QWidget
{
    friend class BatchAbstractDlg;
    friend class BatchCtrlDlg;
    friend class BatchGraphDlg;
    friend class BatchThreadDlg;
    friend class EditPlrDlg;
    friend class FoilPolarDlg;
    friend class MainFrame;
    friend class Settings;
    friend class TwoDWidget;
    friend class XDirectStyleDlg;
    friend class XDirectTileWidget;
    friend class XFoilAnalysisDlg;
    friend class FoilTreeView;

    Q_OBJECT

    public:
        XDirect(QWidget *parent = nullptr);
        ~XDirect();

        bool isOppView()   const {return !m_bPolarView;}
        bool isPolarView() const {return  m_bPolarView;}

        void setView(xfl::enumGraphView eView);

        bool bPolarView() const {return m_bPolarView;}
        Graph *CpGraph() {return &m_CpGraph;}
        Graph *PlrGraph(int iPlrGraph) {return m_PlrGraph.at(iPlrGraph);}
        int PlrGraphSize() const{return m_PlrGraph.count();}

        void resetCurves() {m_bResetCurves=true;}

        static void setCurFoil(Foil*pFoil);
        static void setCurPolar(Polar*pPolar);
        static void setCurOpp(OpPoint* pOpp);

        static Foil *   curFoil();
        static Polar*   curPolar();
        static OpPoint* curOpp();

        static int timeUpdateInterval() {return s_TimeUpdateInterval;}
        static void setTimeUpdateInterval(int t) {s_TimeUpdateInterval=t;}

        static bool bKeepOpenOnErrors() {return s_bKeepOpenErrors;}
        static void setKeepOpenOnErrors(bool b) {s_bKeepOpenErrors=b;}

    signals:
        void projectModified();

    public slots:
        void updateView();

    public slots:
        void onAnalyze();
        void onAnimate(bool bChecked);
        void onAnimateSingle();
        void onAnimateSpeed(int val);
        void onBatchCtrlAnalysis();
        void onCadd();
        void onCpGraph();
        void onCpi();
        void onCurOppOnly();
        void onDefinePolar();
        void onDeleteCurOpp();
        void onDeleteCurFoil();
        void onDeleteCurPolar();
        void onDeleteFoilOpps();
        void onDeleteFoilPolars();
        void onDeletePolarOpps();
        void onDerotateFoil();
        void onDuplicateFoil();
        void onEditCurPolar();
        void onExportAllFoilPolars();
        void onExportAllPolarsTxt();
        void onExportAllPolarsTxt(QString DirName, xfl::enumTextFileType);
        void onExportBLData();
        void onExportCurFoil();
        void onExportCurOpp();
        void onExportCurPolar();
        void onExportPolarOpps() ;
        void onExportXMLAnalysis();
        void onFoilCoordinates();
        void onFoilGeom();
        void onHideAllOpps();
        void onHideAllPolars();
        void onHideFoilOpps();
        void onHideFoilPolars();
        void onHidePolarOpps();
        void onImportJavaFoilPolar();
        void onImportXFoilPolars();
        void onImportXMLAnalysis();
        void onInputChanged();
        void onInterpolateFoils();
        void onMultiThreadedBatchAnalysis();
        void onNacaFoils();
        void onNormalizeFoil();
        void onOpPointView();
        void onOptim2d();
        void onPolarFilter();
        void onPolarView();
        void onQGraph();
        void onRefinePanelsGlobally();
        void onRenameCurFoil();
        void onRenameCurPolar();
        void onResetAllPolarGraphsScales();
        void onResetCurPolar();
        void onSaveFoilPolars();
        void onSequence();
        void onSetFlap();
        void onSetLERadius();
        void onSetTEGap();
        void onShowAllOpps();
        void onShowAllPolars();
        void onShowFoilOpps();
        void onShowFoilPolars();
        void onShowFoilPolarsOnly();
        void onShowPolarOpps();
        void onSpec();
        void onStoreOpp();
        void onViscous();
        void onXFoilAdvanced();


    private:
        void keyPressEvent(QKeyEvent *event);
        void keyReleaseEvent(QKeyEvent *event);

        void setControls();
        void connectSignals();
        void createOppCurves(OpPoint *pOpp=nullptr);
        void createPolarCurves();
        void fillPolarCurve(Curve *pCurve, Polar *pPolar, int XVar, int YVar);
        void fillOppCurve(OpPoint *pOpp, Graph *pGraph, Curve *pCurve, bool bInviscid=false);

        void importAnalysisFromXML(QFile &xmlFile);
        Polar *importXFoilPolar(QFile &txtFile);

        void loadSettings(QSettings &settings);
        void readParams();
        Foil *addNewFoil(Foil *pFoil);
        void renameFoil(Foil *pFoil);
        void saveSettings(QSettings &settings);
        void setFoilScale();
        void setGraphTiles();
        void setOpPointSequence();
        void setAnalysisParams();
        void setGraphTitles(Graph* pGraph, int iX, int iY);
        void setGraphTitles(Graph* pGraph);
        void setupLayout();
        void stopAnimate();
        void updateCurveStyle(const LineStyle &ls);

        QVector<double> * getVariable(Polar *pPolar, int iVar);

        Foil *setFoil(Foil* pFoil=nullptr);
        Polar *setPolar(Polar *pPolar=nullptr);
        OpPoint *setOpp(OpPoint *pOpp);
        OpPoint *setOpp(double Alpha=-123456789.0);



    private:

        FoilTreeView *m_pFoilTreeView;

        XFoilAnalysisDlg* m_pXFADlg;
        OpPointWt *m_pOpPointWidget;

        QTimer *m_pAnimateTimer;

        QLabel *m_plabUnit1, *m_plabUnit2, *m_plabUnit3;

        QRadioButton *m_prbSpec1, *m_prbSpec2, *m_prbSpec3;

        QCheckBox *m_pchSequence;
        DoubleEdit *m_pdeAlphaMin, *m_pdeAlphaMax, *m_pdeAlphaDelta;

        QCheckBox *m_pchViscous;
        QCheckBox *m_pchInitBL;
        QCheckBox *m_pchStoreOpp;
        QPushButton *m_ppbAnalyze;

        QCheckBox *m_pchActiveOppOnly;
        QCheckBox *m_pchShowBL, *m_pchShowPressure;
        QCheckBox* m_pchAnimate;
        QSlider* m_pslAnimateSpeed;

        QGroupBox *m_pDisplayBox;

        static MainFrame *s_pMainFrame;  /**< a static pointer to the instance of the application's MainFrame object */


        bool m_bPolarView;         /**< true if the polar view is selected, false if the operating point view is selected */
        bool m_bAnimate;           /**< true if a result animation is underway */
        bool m_bAnimatePlus;       /**< true if the animation is going from lower to higher alpha, false if decreasing */

        bool m_bType1;             /**< true if the type 1 polars are to be displayed in the graphs */
        bool m_bType2;             /**< true if the type 2 polars are to be displayed in the graphs */
        bool m_bType3;             /**< true if the type 3 polars are to be displayed in the graphs */
        bool m_bType4;             /**< true if the type 4 polars are to be displayed in the graphs */
        bool m_bTrans;             /**< true if the user is dragging a view */

        bool m_bNeutralLine;       /**< true if the neutral line should be displayed */
        bool m_bCurOppOnly;        /**< true if only the current operating point should be displayed */
        bool m_bShowInviscid;      /**< true if the inviscid results should be displayed */
        bool m_bCpGraph;           /**< true if the Cp graph should be displayed */
        bool m_bXPressed;          /**< true if the 'X' key is pressed */
        bool m_bYPressed;          /**< true if the 'Y' key is pressed */
        bool m_bResetCurves;       /**< true if the graph curves need to be redrawn before the next view update */

        int m_posAnimate;          /**< the current aoa in the animation */

        int m_iPlrGraph;           /**< defines whch polar graph is selected if m_iPlrView=1 */
        xfl::enumGraphView m_iPlrView;  /**< defines the number of graphs to be displayed in the polar view */

        QVector<Foil*> *m_poaFoil;    /**< pointer to the foil object array */
        QVector<Polar*> *m_poaPolar;  /**< pointer to the polar object array */
        QVector<OpPoint*> *m_poaOpp;  /**< pointer to the OpPoint object array */

        Graph m_CpGraph;           /**< the Cp graph for the OpPoint view */
        QVector<Graph*> m_PlrGraph;  /**< the array of pointer to the 5 Polar graphs */

        LineStyle m_LineStyle;      /**< the style of the lines displayed in the comboboxes*/

        XFoil m_XFoil;                /**< the unique instance of the XFoil object */

        static double s_Re, s_ReMax, s_ReDelta;
        static bool s_bViscous;           /**< true if performing a viscous calculation, false if inviscid */
        static bool s_bAlpha;             /**< true if performing an analysis based on aoa, false if based on Cl */
        static bool s_bInitBL;            /**< true if the boundary layer should be initialized for the next xfoil calculation */
        static bool s_bKeepOpenErrors;    /**< true if the XfoilAnalysisDlg should be kept open if errors occured in the XFoil calculation */
        static int s_TimeUpdateInterval;  /**< time interval in ms between two output display updates during an XFoil analysis */
};


