/****************************************************************************

    QXInverse Class
    Copyright (C) 2009-2016 André Deperrois

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
 * @file
 * This implements the QXInverse class which provides the interface for inverse design of Foil objects
 */

#pragma once

#include <QWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QStackedWidget>
#include <QSettings>

#include <xflgraph/graph.h>

#include <gui_objects/spline5.h>

class MainFrame;
class inverseviewwt;
class Foil;
class MinTextEdit;
class IntEdit;
class DoubleEdit;
class XFoil;

/**
 * @brief This class implements the interface for the inverse Foil design.
 *
 *Note: this interface was written without a good understanding of the XFoil methodology, which is a potential source of errors.
 */
class XInverse : public QWidget
{
    Q_OBJECT

    friend class MainFrame;
    friend class inverseviewwt;
    friend class InverseOptionsDlg;
    friend class FoilSelectionDlg;
    friend class Settings;

    public:
        XInverse(QWidget *parent = nullptr);
        ~XInverse();

        void setupLayout();


    private slots:
        void onCpxx();
        void onInverseApp();
        void onMarkSegment();
        void onInverseStyles();
        void onTangentSpline();
        void onShowSpline();
        void onNewSpline();
        void onApplySpline();
        void onSpecal();
        void onQReset();
        void onOverlayFoil();
        void onClearOverlayFoil();
        void onFilter();
        void onSmooth();
        void onSymm() ;
        void onExecute();
        void onStoreFoil();
        void onExtractFoil();
        void onResetFoilScale();
        void onInsertCtrlPt();
        void onPertubate();
        void onQGraphSettings();
        void onQInitial();
        void onQSpec();
        void onQViscous();
        void onQPoints();
        void onQReflected();
        void onRemoveCtrlPt();
        void onSpecInv() ;
        void onZoomIn();


    private:
        void updateView();

        void mouseMoveEvent(QMouseEvent *event) override;
        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void keyPressEvent(QKeyEvent *event) override;
        void keyReleaseEvent(QKeyEvent *event) override;

        void doubleClickEvent(QPoint pos);
        void zoomEvent(QPointF pos, double zoomFactor);

        void checkActions();
        void drawGrid(QPainter &painter, double scale);
        void paintView(QPainter &painter);
        void paintGraph(QPainter &painter);
        void paintFoil(QPainter &painter);
        void setXInverseScale(QRect CltRect);
        void resetQ();
        void resetScale();
        void resetMixedQ();
        void releaseZoom();
        void smooth(int Pos1 = -1, int Pos2 = -1);
        void clear();
        void connectSignals();
        void createMCurve();
        void createQCurve();
        void cancelMark();
        void cancelSpline();
        void cancelSmooth();
        void setFoil();
        void setTAngle(double a);
        void setTGap(double tr, double ti);

        void execMDES();
        void loadSettings(QSettings &settings);
        void saveSettings(QSettings &settings);

        bool execQDES();
        bool setParams();
        bool initXFoil(Foil * pFoil);


        double qincom(double qc, double qinf, double tklam);

        Vector3d mousetoReal(QPoint point);

    signals:
        void projectModified();

    private:

        MinTextEdit *m_pmteOutput;
        QLabel *m_plabSpecif;
        QRadioButton *m_prbSpecAlpha, *m_prbSpecCl;
        QPushButton *m_ppbExec, *m_ppbFilter, *m_ppbPert, *m_ppbApplySpline, *m_ppbNewSpline, *m_ppbResetQSpec, *m_ppbSmooth;
        QCheckBox *m_pchShowSpline, *m_pchTangentSpline, *m_pchSymm;
        DoubleEdit *m_pdeSpec, *m_pdeFilterParam, *m_pdeTGapy, *m_pdeTGapx, *m_pdeTAngle;

        QLineEdit *m_pleMAlphaSpec, *m_pleMClSpec;
        IntEdit *m_pieIter;
        QCheckBox *m_pchCpxx;

        QCheckBox *m_pchMShowSpline, *m_pchMTangentSpline;
        QPushButton *m_ppbMExec, *m_ppbMNewSpline, *m_ppbMApplySpline, *m_ppbMSmooth, *m_ppbMResetQSpec;
        QPushButton *m_ppbMark;

        QWidget *m_pwtMInv,*m_pwtFInv;
        QStackedWidget *m_pswInv;


        static MainFrame *s_pMainFrame;  /**< a static pointer to the instance of the application's MainFrame object */
        static inverseviewwt *s_p2dWidget;   /**< a static pointer to the instance of the application's central widget used for 2D drawings */

        XFoil *m_pXFoil;             /**< a void pointer to the unique instance of the XFoil object */

        Foil *m_pRefFoil;           /**< a pointer to the reference foil geometry used for inverse design */
        Foil *m_pModFoil;           /**< a pointer to the resulting Foil modified by inverse design operations */
        Foil const *m_pOverlayFoil;

        Spline5 m_Spline;            /**< the spline oject to modify the velocity curve */

        bool m_bXPressed;           /**< true if the 'X' key is pressed */
        bool m_bYPressed;           /**< true if the 'Y' key is pressed */
        bool m_bLoaded;             /**< true if a Foil has been loaded from the database and copied to the reference Foil */
        bool m_bTrans;              /**< true if the Foil representation is in the process of being translated */
        bool m_bTransGraph;         /**< true if the curves in the graph are in the process of being translated */
        bool m_bRefFoil;            /**< true if the reference Foil should be displayed */
        bool m_bModFoil;            /**< true if the modified Foil should be displayed */
        bool m_bGetPos;             /**< true if the program is waiting for the user to click on a point curve */
        bool m_bMark;               /**< true if the user is in the process of marking a curve segment for modifiction */
        bool m_bMarked;             /**< true if a curve segment has been marked for modifiction */
        bool m_bSpline;             /**< true if the user is in the process of selecting points segments for the spline */
        bool m_bSplined;            /**< true if the velocity curve has been modified by application of the spline */
        bool m_bSmooth;             /**< true if the user is in the process of smoothing the curve */
        bool m_bZoomPlus;           /**< true if the user is in the process of zooming in by drawing a rectangle */
        bool m_bFullInverse;        /**< true if the full inverse method is selected, false if mixed-inverse */
        bool m_bReflected;          /**< true if the reflected curve should be displayed */

        bool m_bShowPoints;         /**< true if the curve points are visible in the graph */
        bool m_bTangentSpline;      /**< true if the spline should be tangent to the velocity curve at its end points */


        int m_Mk1;                 /** the index of the first marked point on the graph */
        int m_Mk2;                 /** the index of the second marked point on the graph */

        double m_fRefScale;        /**< the default scale for the display of the Foil, for the current window size */
        double m_fScale;           /**< the current scale for the Foil display */
        double m_fYScale;

        LineStyle m_ReflectedStyle;      /**< the reflected curve's style */

        Graph m_QGraph;           /**< the velocity QGraph object */
        Graph *m_pCurGraph;       /**< = &m_QGraph if the mouse hivers over the graph, false otherwise */
        Curve* m_pQCurve;          /**< a pointer to the inviscid velocity reference Curve */
        Curve* m_pQVCurve;         /**< a pointer to the viscous velocity reference Curve */
        Curve* m_pMCurve;          /**< a pointer to the modified specification curve */
        Curve* m_pReflectedCurve;  /**< a pointer to the reflected curve */

        QRect m_rCltRect;          /**< the view's client area, in pixels */
        QRect m_rGraphRect;        /**< the graph's client are, in pixels */
        QRect m_ZoomRect;          /**< the rectangle drawn by the user to define the area to zoom in */

        QPoint m_PointDown;        /**< the screen point where the left mouse button was last pressed */
        QPoint m_ptPopUp;          /**< the screen point where the right mouse button was last pressed */
        QPoint m_ptOffset;         /**< the offset position for the display of the Foil */

        // temporary allocations
        int m_tmpPos, m_Pos1, m_Pos2, m_nPos, m_SplineLeftPos, m_SplineRightPos;
        double xd, yd;
        double xu, yu;

        QPoint tanpt;
        QPoint P0, P1, P2;

};




