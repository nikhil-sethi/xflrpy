/****************************************************************************

    Graph class
        Copyright (C) 2003-2019 Andre Deperrois

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

class Curve;

#define MAXTIMEGRAPHS  4  /**< The max number of graphs available for display in the stability time view. */
#define MAXWINGGRAPHS  5  /**< The max number of graphs available for display in QXDirect. */
#define MAXPOLARGRAPHS 5  /**< The max number of graphs available for display in QXDirect. */
#define MAXGRAPHS      6  /**< The max number of graphs available for display at one time. */

namespace GRAPH
{
typedef enum{OPPGRAPH, POLARGRAPH, POPPGRAPH, WPOLARGRAPH, STABTIMEGRAPH, CPGRAPH, INVERSEGRAPH, OTHERGRAPH} enumGraphType;
}

class MainFrame;

class Graph
{
    friend class GraphWidget;

public:
    void drawGraph(QRect const & rect, QPainter &painter);
    void drawGraph(QPainter &painter);
    void drawAxes(QPainter &painter);
    void drawCurve(int nIndex, QPainter &painter);
    void drawLegend(QPainter &painter, QPoint &Place, QFont &LegendFont, QColor &LegendColor, QColor &backColor);
    void drawTitles(QPainter &painter);
    void drawXMinGrid(QPainter &painter);
    void drawYMinGrid(QPainter &painter);
    void drawXMajGrid(QPainter &painter);
    void drawYMajGrid(QPainter &painter);
    void drawXTicks(QPainter &painter);
    void drawYTicks(QPainter &painter);
    void expFormat(double &f, int &exp);
    void exportToFile(QFile &XFile, bool bCSV);
    void highlight(QPainter &painter, Curve *pCurve, int ref);

    void loadSettings(QSettings &settings);
    void saveSettings(QSettings &settings);
    QPoint getOffset();

    void getLabelFont(QFont &labelFont);
    void getTitleFont(QFont &titleFont);
    void setLabelFont(QFont &font);
    void setTitleFont(QFont &font);


    static void setOppHighlighting(bool bHighLight){s_bHighlightPoint = bHighLight;}
    static bool isHighLighting(){return s_bHighlightPoint;}

    void setGraphType(GRAPH::enumGraphType type) {m_GraphType=type;}
    GRAPH::enumGraphType graphType() const {return m_GraphType;}
    bool bAutoX() const;
    bool bAutoY() const;
    bool bAutoXMin() const;
    bool bAutoYMin() const;
    bool hasBorder() const;
    bool bInverted() const;
    bool isInDrawRect(int const &x, int const &y);
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
    void setAutoXMinUnit(bool bAuto);
    void setAutoYMinUnit(bool bAuto);
    void setAutoXUnit();
    void setAutoYUnit();
    void setAxisData(int s, int w, QColor clr);
    void setBkColor(QColor cr);
    void setBorderColor(QColor crBorder);
    void setBorder(bool bBorder);
    void setBorderStyle(int s);
    void setBorderWidth(int w);
    void setDrawRect(QRect Rect);
    void setMargin(int m);
    void setInverted(bool bInverted);
    void setScaleType(int scaleType){ m_AutoScaleType = scaleType;}

    void setXTitle(const QString &str);
    void setYTitle(const QString &str);
    void setX0(double f);
    void setXMax(double f);
    void setXMin(double f);
    void setXMinorUnit(double f);
    void setXUnit(double f);
    void setY0(double f);
    void setYMax(double f);
    void setYMin(double f);
    void setYMinorUnit(double f);
    void setYUnit(double f);
    void setXMajGrid(bool const &state, QColor const &clr, int const &style, int const &width);
    void setYMajGrid(bool const &state, QColor const &clr, int const &style, int const &width);
    void setXMinGrid(bool state, bool bAuto, QColor clr, int style, int width, double unit = -1.0);
    void setYMinGrid(bool state, bool bAuto, QColor clr, int style, int width, double unit = -1.0);
    void setAuto(bool bAuto);
    void setAutoX(bool bAuto);
    void setAutoY(bool bAuto);
    void setAxisColor(QColor crColor);
    void setAxisStyle(int nStyle);
    void setAxisWidth(int Width);
    void setTitleColor(QColor crColor) {m_TitleColor = crColor;}
    void setLabelColor(QColor crColor);
    void setWindow(double x1, double x2, double y1, double y2);

    QColor axisColor()  const {return m_AxisColor;}
    QColor titleColor() const {return m_TitleColor;}
    QColor labelColor() const {return m_LabelColor;}

    int margin() const {return m_iMargin;}
    int axisStyle() const;
    int axisWidth() const;
    int scaleType() const {return m_AutoScaleType;}
    int xVariable() const;
    int yVariable() const;
    void setVariables(int const & X, int const & Y);
    void setXVariable(int const & X);
    void setYVariable(int const & Y);
    double xOrigin() const;
    double xMin() const;
    double xMax() const;
    double xUnit() const;
    double yOrigin() const;
    double yMin() const;
    double yMax() const;
    double yUnit() const;
    double xScale() const;
    double yScale() const;

    bool bXMajGrid() const;
    bool yMajGrid() const;
    bool bXMinGrid() const;
    bool bYMinGrid() const;
    bool selectPoint(QString const &CurveName, int sel);
    bool setXScale();
    bool setYScale();

    void setXMajGrid(bool const &bGrid);
    void setYMajGrid(bool const &bGrid);
    void setXMinGrid(bool const &bGrid);
    void setYMinGrid(bool const &bGrid);
    void bXMajGrid(bool &bstate, QColor &clr, int &style, int &width);
    void yMajGrid(bool &bstate, QColor &clr, int &style, int &width);
    void bXMinGrid(bool &bstate, bool &bAuto, QColor &clr, int &style, int &width, double &unit);
    void bYMinGrid(bool &bstate, bool &bAuto, QColor &clr, int &style, int &width, double &unit);
    QString xTitle() const {return m_XTitle;}
    QString yTitle() const {return m_YTitle;}

    QRect *clientRect();

    void setGraphDefaults(bool bDark=true);
    void setGraphName(QString GraphName);
    void graphName(QString &GraphName);

    QString graphName(){return m_GraphName;}
    Curve* curve(int nIndex);
    Curve* curve(QString CurveTitle);
    Curve* addCurve();
    Curve* addCurve(Curve *pCurve);
    void deleteCurve(int index);
    void deleteCurve(Curve *pCurve);
    void deleteCurve(QString CurveTitle);
    void deleteCurves();
    void resetXLimits();
    void resetYLimits();

    int curveCount() const {return m_oaCurves.size();}
    QVector<Curve*> *getCurves() {return &m_oaCurves;}


    QColor backgroundColor() const {return m_BkColor;}
    QColor borderColor() const {return m_BorderColor;}
    int borderStyle() const {return m_BorderStyle;}
    int borderWidth() const {return m_BorderWidth;}


    Graph();
    virtual ~Graph();

    static QColor s_CurveColors[10];

private:

    QString m_GraphName;        /** The graph's name, used for little else than to identify it in the settings file */

    int m_AutoScaleType;

    QRect m_rCltRect;         //in screen coordinates

    bool m_bXMajGrid, m_bXMinGrid;
    bool m_bYMajGrid, m_bYMinGrid;

    bool m_bXAutoMinGrid, m_bYAutoMinGrid;

    bool m_bYInverted;
    bool m_bAutoX, m_bAutoY;
    bool m_bBorder;

    int m_AxisStyle;// axis style
    int m_AxisWidth;// axis width

    int m_XMajStyle, m_YMajStyle;
    int m_XMajWidth, m_YMajWidth;
    QColor m_XMajClr,   m_YMajClr;
    int m_XMinStyle, m_YMinStyle;
    int m_XMinWidth, m_YMinWidth;
    QColor m_XMinClr,   m_YMinClr;
    double m_XMinorUnit,m_YMinorUnit;

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
    QColor m_AxisColor;


    QColor m_TitleColor;
    QColor m_LabelColor;

    QColor m_BkColor;
    QColor m_BorderColor;
    int m_BorderStyle;
    int m_BorderWidth;


    int m_X, m_Y; //index of X and Y variables



private:
    QFont m_TitleFont;
    QFont m_LabelFont;
    GRAPH::enumGraphType m_GraphType;

    static bool s_bHighlightPoint;       /**< true if the active OpPoint should be highlighted on the polar curve. */

};

