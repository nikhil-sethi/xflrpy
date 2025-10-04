/****************************************************************************

    Graph class
        Copyright (C) 2003-2019 André Deperrois

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

#include <QSettings>
#include <QFile>
#include <QPoint>
#include <QRect>
#include <QColor>
#include <QVector>
#include <QFont>

#include <xflcore/linestyle.h>
#include <xflwidgets/view/grid.h>

class Curve;

#define MAXTIMEGRAPHS  4  /**< The max number of graphs available for display in the stability time view. */
#define MAXWINGGRAPHS  5  /**< The max number of graphs available for display in XDirect. */
#define MAXPOLARGRAPHS 5  /**< The max number of graphs available for display in XDirect. */
#define MAXGRAPHS      6  /**< The max number of graphs available for display at one time. */

namespace GRAPH
{
    typedef enum{OPPGRAPH, POLARGRAPH, POPPGRAPH, WPOLARGRAPH, STABTIMEGRAPH, CPGRAPH, INVERSEGRAPH, OTHERGRAPH} enumGraphType;
}

class MainFrame;

class Graph
{
    friend class GraphWidget;
    friend class GraphDlg;

    public:
        void drawGraph(QRect const & rect, QPainter &painter);
        void drawGraph(QPainter &painter);
        void drawAxes(QPainter &painter);
        void drawCurve(int nIndex, QPainter &painter);
        void drawLegend(QPainter &painter, QPoint &Place, const QFont &LegendFont, const QColor &LegendColor, const QColor &backColor);
        void drawTitles(QPainter &painter);
        void drawXMinGrid(QPainter &painter);
        void drawYMinGrid(QPainter &painter);
        void drawXMajGrid(QPainter &painter);
        void drawYMajGrid(QPainter &painter);
        void drawXTicks(QPainter &painter);
        void drawYTicks(QPainter &painter);

        void expFormat(double &f, int &exp) const;
        void exportToFile(QFile &XFile, bool bCSV);

        void loadSettings(QSettings &settings);
        void saveSettings(QSettings &settings);

        QPoint offset() const {return m_ptoffset;}

        QFont const &labelFont() const {return m_LabelFont;}
        QFont const &titleFont() const {return m_TitleFont;}
        void getLabelFont(QFont &labelFont) const {labelFont = m_LabelFont;}
        void getTitleFont(QFont &titleFont) const {titleFont = m_TitleFont;}
        void setLabelFont(QFont const&font) {m_LabelFont = font;}
        void setTitleFont(QFont const&font) {m_TitleFont = font;}

        void highlight(QPainter &painter, Curve *pCurve, int ref);
        static void setOppHighlighting(bool bHighLight){s_bHighlightPoint = bHighLight;}
        static bool isHighLighting(){return s_bHighlightPoint;}

        void setGraphType(GRAPH::enumGraphType type) {m_GraphType=type;}
        GRAPH::enumGraphType graphType() const {return m_GraphType;}

        bool bAutoX() const {return m_bAutoX;}
        bool bAutoY() const {return m_bAutoY;}

        Grid const &grid()      const {return m_Grid;}
        Grid &grid()            {return m_Grid;}
        bool bXMajGrid()        const {return m_Grid.bXMajGrid();}
        bool bXMinGrid()        const {return m_Grid.bXMinGrid();}
        bool bYMajGrid(int iy)  const {return m_Grid.bYMajGrid(iy);}
        bool bYMinGrid(int iy)  const {return m_Grid.bYMinGrid(iy);}
        bool bAutoXMin()        const {return m_Grid.bXAutoMinGrid();}
        bool bAutoYMin(int iy)  const {return m_Grid.bYAutoMinGrid(iy);}
        void showXMajGrid(bool bShow) {m_Grid.showXMajGrid(bShow);}
        void showXMinGrid(bool bShow) {m_Grid.showXMinGrid(bShow);}
        void showYMajGrid(int iy, bool bShow) {m_Grid.showYMajGrid(iy, bShow);}
        void showYMinGrid(int iy, bool bShow) {m_Grid.showYMinGrid(iy, bShow);}
        LineStyle const &xMajGridStyle()       const {return m_Grid.xMajStyle();}
        LineStyle const &yMajGridStyle(int iy) const {return m_Grid.yMajStyle(iy);}
        LineStyle const &xMinGridStyle()       const {return m_Grid.xMinStyle();}
        LineStyle const &yMinGridStyle(int iy) const {return m_Grid.yMinStyle(iy);}

        bool hasBorder() const {return m_BorderStyle.m_bIsVisible;}

        bool isInDrawRect(int const &x, int const &y);
        bool isInDrawRect(QPointF const &pt);
        bool isInDrawRect(QPoint const &pt);
        bool initializeGraph(int width=0, int height=0);

        double clientTox(int x) const;
        double clientToy(int y) const;

        double clientTox(double x) const;
        double clientToy(double y) const;

        int xToClient(double x) const;
        int yToClient(double y) const;


        void copySettings(Graph* pGraph, bool bScales=true);
        void deselectPoint();
        Curve * getCurvePoint(const int &xClt, const int &yClt, int &nSel);
        Curve * getClosestPoint(double const &x, double const &y, double &xSel, double &ySel, int &nSel);
        void resetLimits();
        void resetCurves();
        void scaleAxes(double zoom);
        void scaleXAxis(double zoom);
        void scaleYAxis(double zoom);

        void setAutoXUnit();
        void setAutoYUnit();

        void setBkColor(QColor cr){m_BkColor = cr;}
    //    void setBorderColor(QColor crBorder){m_BorderStyle.m_Color = crBorder;}
        void setBorder(bool bBorder) {m_BorderStyle.m_bIsVisible = bBorder;}
    //    void setBorderStipple(int s) {m_BorderStyle.setStipple(s);}
    //    void setBorderWidth(int w) {m_BorderStyle.m_Width = w;}
        void setBorderStyle(LineStyle const &ls2) {m_BorderStyle=ls2;}
        void setDrawRect(QRect Rect) {m_rCltRect = Rect;}
        void setMargin(int m);
        void setInverted(bool bInverted);
        void setScaleType(int scaleType){ m_AutoScaleType = scaleType;}

        void setX0(double f){    xo = f;}

        void setXMax(double f) {xmax = f;}
        void setXMin(double f) {xmin = f;}

        void setXTitle(const QString &str);
        void setYTitle(const QString &str);

        void setXUnit(double f);
        void setY0(double f);
        void setYMax(double f);
        void setYMin(double f);
        void setYMinorUnit(double f);
        void setYUnit(double f);

        void setXMajGrid(bool const &state, QColor const &clr, int const &style, int const &width);
        void setYMajGrid(bool const &state, QColor const &clr, int const &style, int const &width);


        void setAuto(bool bAuto);
        void setAutoX(bool bAuto);
        void setAutoY(bool bAuto);

        void setTitleColor(QColor const &crColor) {m_TitleColor = crColor;}
        void setLabelColor(QColor const &crColor);
        void setWindow(double x1, double x2, double y1, double y2);


        QColor const &titleColor() const {return m_TitleColor;}
        QColor const &labelColor() const {return m_LabelColor;}

        bool bInverted() const{    return m_bYInverted;}

        double xOrigin() const{    return xo;}


        double xMin()    const {return xmin;}
        double xMax()    const {return xmax;}
        double xScale()  const {return m_scalex;}
        double xUnit()   const {return xunit;}
        int xVariable()  const {return m_X;}
        double yOrigin() const {return yo;}
        double yMin()    const {return ymin;}
        double yMax()    const {return ymax;}
        double yUnit()   const {return yunit;}
        double yScale()  const {return m_scaley;}
        int yVariable()  const {return m_Y;}

        int margin() const {return m_iMargin;}

        int scaleType() const {return m_AutoScaleType;}

        void setVariables(int const & X, int const & Y);
        void setXVariable(int const & X);
        void setYVariable(int const & Y);


        bool bYMajGrid() const;
        bool bYMinGrid() const;
        bool selectPoint(QString const &CurveName, int sel);
        bool setXScale();
        bool setYScale();


        QString xTitle() const {return m_XTitle;}
        QString yTitle() const {return m_YTitle;}

        QRect *clientRect() {return &m_rCltRect;}

        void setGraphDefaults(bool bDark=true);
        void setGraphName(QString const &GraphName) {m_GraphName = GraphName;}

        void graphName(QString &GraphName){ GraphName = m_GraphName;}
        QString const &graphName() const{return m_GraphName;}
        Curve* curve(int nIndex);
        Curve const* curveAt(int nIndex) const;
        Curve* curve(QString const &CurveTitle);
        Curve* addCurve();
        Curve* addCurve(QString const &name);
        Curve* addCurve(Curve *pCurve);
        void deleteCurve(int index);
        void deleteCurve(Curve *pCurve);
        void deleteCurve(QString CurveTitle);
        void deleteCurves();
        void resetXLimits();
        void resetYLimits();

        int curveCount() const {return m_oaCurves.size();}
        QVector<Curve*> *getCurves() {return &m_oaCurves;}

        QPointF toClient(double x, double y) const {return QPointF(xToClient(x), yToClient(y));}


        QColor backgroundColor() const {return m_BkColor;}
        QColor borderColor() const {return m_BorderStyle.m_Color;}
        LineStyle borderStyle() const {return m_BorderStyle;}
        int borderStipple() const {return m_BorderStyle.m_Stipple;}
        int borderWidth() const {return m_BorderStyle.m_Width;}


        Graph();
        virtual ~Graph();

        static QColor s_CurveColors[10];

    private:

        QString m_GraphName;        /** The graph's name, used for little else than to identify it in the settings file */

        int m_AutoScaleType;

        QRect m_rCltRect;         //in screen coordinates


        bool m_bAutoX, m_bAutoY;
        bool m_bYInverted;


        LineStyle m_BorderStyle;

        Grid m_Grid;

        QString m_XTitle;
        QString m_YTitle;
        QVector<Curve*> m_oaCurves;

        QPoint m_ptoffset; //in screen coordinates, w.r.t. the client area

        int exp_x, exp_y;
        double xo, yo;
        double xunit, yunit;
        double xmin, ymin, xmax, ymax;
        double Cxmin, Cxmax, Cymin, Cymax;
        double m_scalex, m_scaley;
        double m_h, m_w; //graph width and height
        int m_iMargin;

        QColor m_TitleColor;
        QColor m_LabelColor;

        QColor m_BkColor;


        int m_X, m_Y; //index of X and Y variables


    private:
        QFont m_TitleFont;
        QFont m_LabelFont;
        GRAPH::enumGraphType m_GraphType;

        static bool s_bHighlightPoint;       /**< true if the active OpPoint should be highlighted on the polar curve. */

};

