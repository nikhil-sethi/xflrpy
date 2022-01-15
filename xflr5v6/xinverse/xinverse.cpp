/****************************************************************************

    XInverse Class
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

#include <QGroupBox>
#include <QGridLayout>
#include <QAction>
#include <QMessageBox>
#include <QDebug>

#include "xinverse.h"

#include <globals/mainframe.h>

#include <xflobjects/editors/renamedlg.h>
#include <twodwidgets/inverseviewwt.h>
#include <xflobjects/objects2d/objects2d.h>
#include <xdirect/xdirect.h>
#include <xflanalysis/analysis3d_params.h>
#include <xflcore/constants.h>
#include <xflcore/displayoptions.h>
#include <xflcore/xflcore.h>
#include <xflgraph/controls/graphdlg.h>
#include <xflgraph/curve.h>
#include <xflobjects/objects2d/foil.h>
#include <xflobjects/objects_global.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/customwts/mintextedit.h>
#include <xfoil.h>
#include <xinverse/foilselectiondlg.h>
#include <xinverse/inverseoptionsdlg.h>
#include <xinverse/pertdlg.h>

MainFrame *XInverse::s_pMainFrame;
inverseviewwt *XInverse::s_p2dWidget;

/** The public contructor */
XInverse::XInverse(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    m_bFullInverse = false;

    m_pXFoil = nullptr;
    m_pCurGraph = nullptr;

    m_bTransGraph    = false;
    m_bLoaded        = false;
    m_bZoomPlus      = false;
    m_bShowPoints    = false;
    m_bTangentSpline = false;
    m_bReflected     = false;
    m_bMarked        = false;
    m_bTrans   = false;
    m_bSpline  = false;
    m_bSplined = true;
    m_bRefFoil = true;
    m_bModFoil = false;
    m_bGetPos  = false;
    m_bMark    = false;
    m_bMarked  = false;
    m_bSmooth  = false;
    m_bXPressed = m_bYPressed = false;

    m_fRefScale = m_fScale = m_fYScale = 1.0;

    m_pRefFoil = new Foil();
    m_pModFoil = new Foil();
    m_pRefFoil->setColor(255,100,100);
    m_pRefFoil->setLineStipple(Line::SOLID);
    m_pRefFoil->setLineWidth(1);
    m_pModFoil->setColor(100,100,255);
    m_pModFoil->setLineStipple(Line::SOLID);
    m_pModFoil->setLineWidth(1);

    m_pOverlayFoil = nullptr;

    m_Spline.insertPoint(0.0,  0.0);
    m_Spline.insertPoint(0.25, 0.0);
    m_Spline.insertPoint(0.5,  0.0);
    m_Spline.insertPoint(0.75, 0.0);
    m_Spline.insertPoint(1.0,  0.0);
    m_Spline.splineKnots();
    m_Spline.splineCurve();

    m_Spline.setStipple(Line::SOLID);
    m_Spline.setWidth(1);
    m_Spline.setColor(QColor(170,120, 0));

    m_ReflectedStyle.m_Stipple = Line::DASH;
    m_ReflectedStyle.m_Width = 1;
    m_ReflectedStyle.m_Color = QColor(195,155, 35);

    m_nPos    = 0;
    m_tmpPos  = -1;
    m_Pos1    = -1;
    m_Pos2    = -1;
    m_SplineLeftPos   = -1;
    m_SplineRightPos  = -1;

    m_QGraph.setGraphType(GRAPH::INVERSEGRAPH);
    m_QGraph.setScaleType(2);
    m_QGraph.setGraphDefaults();
    m_QGraph.setXTitle(tr("x/c"));
    m_QGraph.setYTitle(tr("Q/Vinf"));
    m_QGraph.setXMin(0.0);
    m_QGraph.setXMax(1.0);
    m_QGraph.setYMin(-0.1);
    m_QGraph.setYMax(0.1);
    m_QGraph.setGraphName(tr("Q Graph"));
    m_pQCurve  = m_QGraph.addCurve();
    m_pMCurve  = m_QGraph.addCurve();
    m_pQVCurve = m_QGraph.addCurve();
    m_pReflectedCurve = m_QGraph.addCurve();
    m_pReflectedCurve->setVisible(m_bReflected);

    setupLayout();
    if(m_bFullInverse)
    {
        m_pswInv->setCurrentIndex(0);
    }
    else
    {
        m_pswInv->setCurrentIndex(1);
    }
}


XInverse::~XInverse()
{
    delete m_pModFoil;
    delete m_pRefFoil;
    //    delete m_pPertDlg;
    //    delete m_pGraphDlg;
    //    delete m_pXInverseStyleDlg;
}


/**
 * Cancels the existing modification limits to the velocity curve
 */
void XInverse::cancelMark()
{
    m_ppbMark->setChecked(false);
    m_bGetPos = false;
    m_bMark   = false;
}


/**
 * Cancels the existing smoothing limits on the velocity curve
 */
void XInverse::cancelSmooth()
{
    m_bSmooth = false;
    m_bGetPos = false;
    m_ppbSmooth->setChecked(false);
    m_ppbMSmooth->setChecked(false);
}

/**
 * Cancels the spline definition process
 */
void XInverse::cancelSpline()
{
    //    m_bSpline  = false;
    m_bSplined = false;
    m_ppbNewSpline->setChecked(false);
    m_bSmooth = false;
    m_bGetPos = false;
    m_nPos    = 0;
    m_tmpPos  = -1;
    m_Pos1    = -1;
    m_Pos2    = -1;
}


/**
 * Initializes the widgets and actions with the current data
 */
void XInverse::checkActions()
{
    s_pMainFrame->m_pInvQInitial->setChecked(m_pQCurve->isVisible());
    s_pMainFrame->m_pInvQSpec->setChecked(m_pMCurve->isVisible());
    s_pMainFrame->m_pInvQViscous->setChecked(m_pQVCurve->isVisible());
    s_pMainFrame->m_pInvQPoints->setChecked(m_bShowPoints);
    s_pMainFrame->m_pInvQReflected->setChecked(m_bReflected);

    if(m_bFullInverse)
    {
        m_pchShowSpline->setChecked(m_bSpline);
        m_pchTangentSpline->setChecked(m_bTangentSpline);
    }
    else
    {
        m_pchMShowSpline->setChecked(m_bSpline);
        m_pchMTangentSpline->setChecked(m_bTangentSpline);
        m_pchCpxx->setChecked(m_pXFoil->lcpxx);
    }
}

/**
 * Clears the data associated to the loaded Foil
 */
void XInverse::clear()
{
    m_pRefFoil->m_n = 0;
    m_pRefFoil->setName(QString());
    m_pModFoil->setName(QString());

    m_bLoaded = false;
    m_pReflectedCurve->clear();
    m_pMCurve->clear();
    m_pQCurve->clear();
    m_pQVCurve->clear();
}


/**
 * Performs the connections between SIGNALS and SLOTS
 */
void XInverse::connectSignals()
{
    connect(m_prbSpecAlpha,     SIGNAL(clicked()), this, SLOT(onSpecal()));
    connect(m_prbSpecCl,        SIGNAL(clicked()), this, SLOT(onSpecal()));
    connect(m_pdeSpec,          SIGNAL(editingFinished()), this, SLOT(onSpecInv()));
    connect(m_pchShowSpline,    SIGNAL(clicked()), this, SLOT(onShowSpline()));
    connect(m_ppbNewSpline,     SIGNAL(clicked()), this, SLOT(onNewSpline()));
    connect(m_ppbApplySpline,   SIGNAL(clicked()), this, SLOT(onApplySpline()));
    connect(m_pchTangentSpline, SIGNAL(clicked()), this, SLOT(onTangentSpline()));
    connect(m_ppbResetQSpec,    SIGNAL(clicked()), this, SLOT(onQReset()));
    connect(m_ppbSmooth,        SIGNAL(clicked()), this, SLOT(onSmooth()));
    connect(m_ppbMSmooth,        SIGNAL(clicked()), this, SLOT(onSmooth()));
    connect(m_ppbPert,          SIGNAL(clicked()), this, SLOT(onPertubate()));
    connect(m_ppbFilter,        SIGNAL(clicked()), this, SLOT(onFilter()));
    connect(m_pchSymm,          SIGNAL(clicked()), this, SLOT(onSymm()));
    connect(m_ppbExec,          SIGNAL(clicked()), this, SLOT(onExecute()));

    connect(m_ppbMNewSpline,     SIGNAL(clicked()), this, SLOT(onNewSpline()));
    connect(m_ppbMark,           SIGNAL(clicked()), this, SLOT(onMarkSegment()));
    connect(m_ppbMApplySpline,   SIGNAL(clicked()), this, SLOT(onApplySpline()));
    connect(m_pchMTangentSpline, SIGNAL(clicked()), this, SLOT(onTangentSpline()));
    connect(m_pchMShowSpline,    SIGNAL(clicked()), this, SLOT(onShowSpline()));
    connect(m_ppbMResetQSpec,    SIGNAL(clicked()), this, SLOT(onQReset()));
    connect(m_pchCpxx,           SIGNAL(clicked()), this, SLOT(onCpxx()));
    connect(m_ppbMExec,          SIGNAL(clicked()), this, SLOT(onExecute()));
}


/**
 * Creates the velocity curve
 */
void XInverse::createQCurve()
{
    double x=0, y=0;
    m_pQCurve->clear();

    int points;
    if(m_bFullInverse) points = 257;
    else points  = m_pXFoil->n;

    for (int i=1; i<=points; i++)
    {
        x = 1.0 - m_pXFoil->sspec[i];
        y = m_pXFoil->qcomp(m_pXFoil->qspec[1][i])/m_pXFoil->qinf;
        m_pQCurve->appendPoint(x,y);
    }
}

/**
 * Creates the modified velocity specification curve
 */
void XInverse::createMCurve()
{
    int i, points;
    double x,y;
    m_pMCurve->clear();
    m_pReflectedCurve->clear();

    if(m_bFullInverse) points = 257;
    else               points = m_pXFoil->n;

    for (i=1; i<=points; i++)
    {
        x = 1.0 - m_pXFoil->sspec[i];
        y = m_pXFoil->qcomp(m_pXFoil->qspec[1][i])/m_pXFoil->qinf;
        m_pMCurve->appendPoint(x,y);
        m_pReflectedCurve->appendPoint(m_pXFoil->sspec[i],-y);
    }
}

/**
 * Draws the grid underneath the Foil display
 * @param painter the instance of the QPainter object on which to draw
 * @param scale the scaling factor for drawing
 */
void XInverse::drawGrid(QPainter &painter, double scale)
{
    painter.save();
    double scalex;
    int TickSize;

    TickSize = 5;
    scalex= scale;

    QPen TextPen(DisplayOptions::textColor());
    painter.setPen(TextPen);

    //neutral line first
    //    QPen LinePen(MainFrame::m_TextColor);
    //    painter.setPen(LinePen);

    painter.drawLine(0, m_ptOffset.y(), m_rCltRect.right(), m_ptOffset.y());

    double xo            =  0.0;
    double xmin          =  0.0;
    double xmax          =  1.0;
    //    double ymin          = -0.2;
    //    double ymax          =  0.2;
    double XGridUnit     =  0.1;
    double XHalfGridUnit =  0.05;
    double XMinGridUnit  =  0.01;

    double xt  = xo-int((xo-xmin)*1.0001/XGridUnit)*XGridUnit;//one tick at the origin
    double xht = xo-int((xo-xmin)*1.0001/XHalfGridUnit)*XHalfGridUnit;//one tick at the origin
    double xmt = xo-int((xo-xmin)*1.0001/XMinGridUnit)*XMinGridUnit;//one tick at the origin


    QString strLabel;
    while(xt<=xmax*1.001)
    {
        //Draw  ticks
        painter.drawLine(int(xt*scalex) + m_ptOffset.x(), m_ptOffset.y(),
                         int(xt*scalex) + m_ptOffset.x(), m_ptOffset.y()+TickSize);
        strLabel = QString("%1").arg(xt,0,'f',1);
        painter.drawText(int(xt*scalex)+m_ptOffset.x()-5, m_ptOffset.y()+int(TickSize*5), strLabel);
        xt += XGridUnit ;
    }

    while(xht<=xmax*1.001)
    {
        //Draw  ticks
        painter.drawLine(int(xht*scalex) + m_ptOffset.x(), m_ptOffset.y(),
                         int(xht*scalex) + m_ptOffset.x(), m_ptOffset.y()+TickSize*2);
        xht += XHalfGridUnit ;
    }

    while(xmt<=xmax*1.001)
    {
        //Draw  ticks
        painter.drawLine(int(xmt*scalex) + m_ptOffset.x(), m_ptOffset.y(),
                         int(xmt*scalex) + m_ptOffset.x(), m_ptOffset.y()+TickSize);
        xmt += XMinGridUnit ;
    }

    painter.restore();

}


/**
 * Executes a full inverse design analysis
 * Updates the geometry of the modified Foil
 */
void XInverse::execMDES()
{
    //----- put modified info back into global arrays
    m_pmteOutput->append(tr("executing..."));

    double qscom;
    for (int i=1; i<= m_pXFoil->nsp; i++)
    {
        //        isp = pXFoil->nsp - i + 1;
        qscom =  m_pXFoil->qinf*m_pMCurve->m_y[i-1];
        m_pXFoil->qspec[1][i] = qincom(qscom, m_pXFoil->qinf, m_pXFoil->tklam);
    }
    m_pXFoil->ExecMDES();

    for(int i=1; i<=m_pXFoil->nsp; i++)
    {
        m_pModFoil->m_x[i-1] = m_pXFoil->xb[i];
        m_pModFoil->m_y[i-1] = m_pXFoil->yb[i];
    }
    for(int i=1; i<=m_pXFoil->nsp; i++)
    {
        m_pModFoil->m_xb[i-1] = m_pXFoil->xb[i];
        m_pModFoil->m_yb[i-1] = m_pXFoil->yb[i];
    }
    m_pModFoil->m_n  = m_pXFoil->nsp;
    m_pModFoil->m_nb = m_pXFoil->nsp;
    m_pModFoil->initFoil();

    m_bModFoil = true;
}


/**
 * Executes a mixed inverse design analysis
 * Updates the geometry of the modified Foil
 *@return true unless the modifications points were not marked
 */
bool XInverse::execQDES()
{
    if(!m_bMarked)
    {
        // || !pXFoil->liqset
        m_pmteOutput->setTextColor(Qt::red);
        m_pmteOutput->append(tr("Must mark off target segment first"));
        m_pmteOutput->setTextColor(Qt::black);
        return false;
    }
    m_pmteOutput->append(tr("executing..."));

    //----- put modified info back into global arrays
    //    int isp;
    double qscom;
    for (int i=1; i<= m_pXFoil->nsp; i++)
    {
        //        isp = pXFoil->nsp - i + 1;
        qscom =  m_pXFoil->qinf*m_pMCurve->m_y[i-1];
        m_pXFoil->qspec[1][i] = qincom(qscom, m_pXFoil->qinf, m_pXFoil->tklam);
    }
    bool bRes =  m_pXFoil->ExecQDES();

    QString str;
    QString strong = "";
    strong = "   dNMax       dGMax\n";
    for(int l=1; l<=m_pXFoil->QMax; l++)
    {
        str = QString("%1e  %2").arg(m_pXFoil->dnTrace[l],7,'e',3).arg(m_pXFoil->dgTrace[l],7,'e',3);
        m_pmteOutput->append(str);
    }

    if(bRes)
    {
        m_pmteOutput->setTextColor(QColor(Qt::green).darker(175));
        strong = tr("Converged");
    }
    else
    {
        m_pmteOutput->setTextColor(Qt::red);
        strong = tr("Unconverged");
    }
    m_pmteOutput->append(strong);
    m_pmteOutput->append("\n");
    m_pmteOutput->setTextColor(Qt::black);

    for (int i=1; i<=m_pXFoil->n; i++)
    {
        m_pModFoil->m_x[i-1] = m_pXFoil->x[i];
        m_pModFoil->m_y[i-1] = m_pXFoil->y[i];
    }
    for (int i=1; i<=m_pXFoil->nb; i++)
    {
        m_pModFoil->m_xb[i-1] = m_pXFoil->x[i];
        m_pModFoil->m_yb[i-1] = m_pXFoil->y[i];
    }
    m_pModFoil->m_n  = m_pXFoil->n;
    m_pModFoil->m_nb = m_pXFoil->nb;

    m_pModFoil->initFoil();
    m_bModFoil = true;

    return true;
}


/**
 * Initializes XFoil with the data from the input Foil object
 * @param pFoil a pointer to the Foil object with which to initialize the XFoil object
 * @true if the initialization has been sucessful
 */
bool XInverse::initXFoil(Foil * pFoil)
{
    //loads pFoil in XFoil, calculates normal vectors, and sets results in current foil
    if(!pFoil) return  false;

    m_pModFoil->setName(pFoil->name() + tr(" Modified"));

    m_pXFoil->initialize();
    for(int i =0; i<pFoil->m_n; i++)
    {
        m_pXFoil->xb[i+1] = pFoil->m_x[i];
        m_pXFoil->yb[i+1] = pFoil->m_y[i];
    }
    m_pXFoil->nb     = pFoil->m_n;
    m_pXFoil->lflap  = false;
    m_pXFoil->lbflap = false;
    m_pXFoil->ddef   = 0.0;
    m_pXFoil->xbf    = 1.0;
    m_pXFoil->ybf    = 0.0;

    m_pXFoil->lqspec = false;
    m_pXFoil->lscini = false;

    if(m_pXFoil->Preprocess())
    {
        m_pXFoil->CheckAngles();

        for (int k=0; k<m_pXFoil->n;k++)
        {
            pFoil->m_nx[k] = m_pXFoil->nx[k+1];
            pFoil->m_ny[k] = m_pXFoil->ny[k+1];
        }
        pFoil->m_n = m_pXFoil->n;
        return true;
    }
    else
    {
        QMessageBox::warning(s_pMainFrame,tr("Warning"),tr("Unrecognized foil format"));
        return false;
    }
}


/**
 * Overrides the QWidget's keyPressEvent method.
 * Dispatches the key press event
 * @param event the QKeyEvent
 */
void XInverse::keyPressEvent(QKeyEvent *event)
{
//    bool bCtrl = false;
//    if(event->modifiers() & Qt::ControlModifier)   bCtrl =true;
    switch (event->key())
    {
        case Qt::Key_X:
            m_bXPressed = true;
            break;
        case Qt::Key_Y:
            m_bYPressed = true;
            break;

        case Qt::Key_Escape:
        {
            if(m_bZoomPlus)
            {
                releaseZoom();
            }
            else if(m_bGetPos)
            {
                m_ppbMark->setChecked(false);
                m_ppbNewSpline->setChecked(false);
                m_ppbMNewSpline->setChecked(false);
                m_bGetPos = false;
                m_bSpline = false;
                m_bSmooth = false;
                if(m_bFullInverse)
                {
                    cancelSpline();
                    cancelSmooth();
                }
                else
                {
                    cancelSpline();
                    cancelSmooth();
                }
                updateView();
            }
            break;
        }
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if (m_bSmooth)
            {
                smooth(-1);
                m_bGetPos = false;
            }
            else
            {
                if(m_bFullInverse)
                {
                    m_ppbExec->setFocus();
                }
                else
                {
                    m_ppbExec->setFocus();
                }
                return;
            }
            break;
        }
        case Qt::Key_G:
        {
            onQGraphSettings();
            return;
        }
        case Qt::Key_R:
        {
            if(m_pCurGraph)
            {
                m_QGraph.setAuto(true);
                updateView();
            }
            else onResetFoilScale();
            break;
        }

/*        case Qt::Key_1:
            if(bCtrl)
            {
                s_pMainFrame->onAFoil();
                return;
            }
            break;
        case Qt::Key_2:
            if(bCtrl)
            {
                s_pMainFrame->onAFoil();
                return;
            }
            break;
        case Qt::Key_3:
            if(bCtrl)
            {
                s_pMainFrame->onXInverse();
                return;
            }
            break;

        case Qt::Key_4:
            if(bCtrl)
            {
                s_pMainFrame->onXInverseMixed();
                return;
            }
            break;
        case Qt::Key_5:
            if(bCtrl)
            {
                s_pMainFrame->onXDirect();
                return;
            }
            break;
        case Qt::Key_6:
        {
            if(bCtrl)
            {
                s_pMainFrame->onMiarex();
                return;
            }
            break;
        }*/
        default:
            event->ignore();
    }
}


/**
 * Overrides the QWidget's keyReleaseEvent method.
 * Dispatches the key release event
 * @param event the QKeyEvent
 */
void XInverse::keyReleaseEvent(QKeyEvent *event)
{
    m_bXPressed = m_bYPressed = false;
    switch (event->key())
    {
        case Qt::Key_Escape:
        {
            if(m_bZoomPlus) releaseZoom();
            break;
        }

        default:
            QWidget::keyReleaseEvent(event);
    }
}


/**
 * Loads the user's default settings from the application QSettings object
 * @param pSettings a pointer to the QSettings object
 */
void XInverse::loadSettings(QSettings &settings)
{
    settings.beginGroup("XInverse");
    {
        m_bFullInverse = settings.value("FullInverse").toBool();

        m_Spline.theStyle().loadSettings(settings, "InverseSpline");

        m_pRefFoil->theStyle().loadSettings(settings, "BaseFoilStyle");
        m_pModFoil->theStyle().loadSettings(settings, "ModFoilStyle");
    }
    settings.endGroup();
    m_QGraph.loadSettings(settings);
}


/**
 * Saves the user settings
 * @param pSettings a pointer to the QSetting object.
 */
void XInverse::saveSettings(QSettings &settings)
{
    settings.beginGroup("XInverse");
    {
        settings.setValue("FullInverse", m_bFullInverse);
        m_Spline.theStyle().saveSettings(settings, "InverseSpline");
        m_pRefFoil->theStyle().saveSettings(settings, "BaseFoilStyle");
        m_pModFoil->theStyle().saveSettings(settings, "ModFoilStyle");
    }
    settings.endGroup();

    m_QGraph.saveSettings(settings);
}


/**
 * Overrides the QWidget's mouseDoubleClickEvent method.
 * Dispatches the event
 * @param event the QMouseEvent
 */
void XInverse::doubleClickEvent(QPoint pos)
{
    if (!m_QGraph.isInDrawRect(pos)) return;

    onQGraphSettings();
}


/**
 * Overrides the QWidget's mouseMoveEvent method.
 * Dispatches the event
 * @param event the QMouseEvent
 */
void XInverse::mouseMoveEvent(QMouseEvent *event)
{
    double x1,y1, xmin, xmax, ymin,  ymax, xpt, ypt, scale, ux, uy, unorm, vx, vy, vnorm, scal;
    double xx0,xx1,xx2,yy0,yy1,yy2, dist;
    int a, n, ipt;
    QPoint point;
    point = event->pos();

    if(m_bGetPos)
    {
        m_tmpPos = m_pMCurve->closestPoint(m_QGraph.clientTox(point.x()), m_QGraph.clientToy(point.y()), dist);
    }
    else if(m_bZoomPlus && (event->buttons() & Qt::LeftButton))
    {
        m_ZoomRect.setRight(point.x());
        m_ZoomRect.setBottom(point.y());
    }
    else if(m_rCltRect.contains(point) && (event->buttons() & Qt::LeftButton) && m_bTrans)
    {
        if(m_bTransGraph)
        {
            // we're dragging the graph
            x1 =  m_QGraph.clientTox(m_PointDown.x()) ;
            y1 =  m_QGraph.clientToy(m_PointDown.y()) ;

            xu = m_QGraph.clientTox(point.x());
            yu = m_QGraph.clientToy(point.y());

            xmin = m_QGraph.xMin() - xu+x1;
            xmax = m_QGraph.xMax() - xu+x1;
            ymin = m_QGraph.yMin() - yu+y1;
            ymax = m_QGraph.yMax() - yu+y1;

            m_QGraph.setWindow(xmin, xmax, ymin, ymax);
        }
        else
        {
            //we're dragging the foil
            m_ptOffset.rx() += point.x() - m_PointDown.x();
            m_ptOffset.ry() += point.y() - m_PointDown.y();
        }
    }
    else if ((event->buttons() & Qt::LeftButton)  && !m_bZoomPlus && m_bSpline && m_Spline.m_iSelect>=0)
    {
        // user is dragging the point
        x1 =  m_QGraph.clientTox(point.x()) ;
        y1 =  m_QGraph.clientToy(point.y()) ;
        if(m_rGraphRect.contains(point))
        {
            n = m_Spline.m_iSelect;

            if(n==0)
            {
                // user is dragging end point
                // find closest graph point
                ipt = m_pMCurve->closestPoint(m_QGraph.clientTox(point.x()), m_QGraph.clientToy(point.y()), dist);
                m_SplineLeftPos = ipt;
                xpt = m_pMCurve->m_x[ipt];
                ypt = m_pMCurve->m_y[ipt];
                // check for inversion
                if(xpt> m_Spline.m_CtrlPt.last().x)
                {
                    m_Spline.m_CtrlPt[n].x = m_Spline.m_CtrlPt.last().x;
                    m_Spline.m_CtrlPt[n].y = m_Spline.m_CtrlPt.last().y;
                    m_Spline.m_CtrlPt.last().x = xpt;
                    m_Spline.m_CtrlPt.last().y = ypt;
                }
                else
                {
                    m_Spline.m_CtrlPt[n].x = xpt;
                    m_Spline.m_CtrlPt[n].y = ypt;
                }
                m_bSplined = false;
                m_Spline.splineCurve();
            }
            else if(n == m_Spline.m_CtrlPt.size()-1)
            {
                // user is dragging end point
                // find closest graph point
                ipt = m_pMCurve->closestPoint(m_QGraph.clientTox(point.x()), m_QGraph.clientToy(point.y()), dist);
                m_SplineRightPos = ipt;
                xpt = m_pMCurve->m_x[ipt];
                ypt = m_pMCurve->m_y[ipt];
                // check for inversion
                if(xpt< m_Spline.m_CtrlPt[0].x)
                {
                    m_Spline.m_CtrlPt[n].x = m_Spline.m_CtrlPt[0].x;
                    m_Spline.m_CtrlPt[n].y = m_Spline.m_CtrlPt[0].y;
                    m_Spline.m_CtrlPt[0].x = xpt;
                    m_Spline.m_CtrlPt[0].y = ypt;
                }
                else
                {
                    m_Spline.m_CtrlPt[n].x = xpt;
                    m_Spline.m_CtrlPt[n].y = ypt;
                }
                m_Spline.splineCurve();
                m_bSplined = false;
            }
            else if (n==1 && m_bTangentSpline)
            {
                // Second point must remain on tangent to curve
                // difficulty is that we are working in non-normal coordinates

                tanpt = point;
                P0 = QPoint(m_QGraph.xToClient(m_pMCurve->m_x[m_SplineLeftPos-1]), m_QGraph.yToClient(m_pMCurve->m_y[m_SplineLeftPos-1]));
                P1 = QPoint(m_QGraph.xToClient(m_pMCurve->m_x[m_SplineLeftPos]),   m_QGraph.yToClient(m_pMCurve->m_y[m_SplineLeftPos]));
                P2 = QPoint(m_QGraph.xToClient(m_pMCurve->m_x[m_SplineLeftPos+1]), m_QGraph.yToClient(m_pMCurve->m_y[m_SplineLeftPos+1]));

                //v is the tangent to the curve in screen coordinates
                vx = double((P0.x()-P1.x())*(P0.x()-P2.x())*(P1.x()-P2.x())*(P2.x()-P0.x()));
                vy = double( P0.y() *(P1.x()-P2.x())            * (P1.x()-P2.x()) * (P2.x()-P0.x())
                               - P1.y() *(2.0*P1.x()-P0.x()-P2.x()) * (P0.x()-P2.x()) * (P2.x()-P0.x())
                               - P2.y() *(P1.x()-P0.x())            * (P0.x()-P1.x()) * (P0.x()-P2.x()));
                vnorm = sqrt(vx*vx+vy*vy);
                vx/=vnorm;
                vy/=vnorm;
                scal = double(point.x()-P1.x())*vx + double(point.y()-P1.y())*vy;
                tanpt.rx() = P1.x() + int(vx * scal);
                tanpt.ry() = P1.y() + int(vy * scal);

                x1 =  m_QGraph.clientTox(tanpt.x()) ;
                y1 =  m_QGraph.clientToy(tanpt.y()) ;

                xx0 = m_pMCurve->m_x[m_SplineLeftPos-1];
                xx1 = m_pMCurve->m_x[m_SplineLeftPos];
                xx2 = m_pMCurve->m_x[m_SplineLeftPos+1];
                yy0 = m_pMCurve->m_y[m_SplineLeftPos-1];
                yy1 = m_pMCurve->m_y[m_SplineLeftPos];
                yy2 = m_pMCurve->m_y[m_SplineLeftPos+1];

                ux = (xx0-xx1)*(xx0-xx2)*(xx1-xx2)*(xx2-xx0);
                uy =      yy0 *(xx1-xx2)         * (xx1-xx2) * (xx2-xx0)
                        - yy1 *(2.0*xx1-xx0-xx2) * (xx0-xx2) * (xx2-xx0)
                        - yy2 *(xx1-xx0)         * (xx0-xx1) * (xx0-xx2);

                //                unorm = sqrt(ux*ux*scx*scx+uy*uy*scy*scy)/scx/scy;
                unorm = sqrt(ux*ux+uy*uy);
                ux /= unorm;
                uy /= unorm;

                vx = x1-m_Spline.m_CtrlPt[n-1].x;
                vy = y1-m_Spline.m_CtrlPt[n-1].y;

                scal =  (ux*vx + uy*vy);
                m_Spline.m_CtrlPt[n].x = m_Spline.m_CtrlPt[0].x + scal * ux ;
                m_Spline.m_CtrlPt[n].y = m_Spline.m_CtrlPt[0].y + scal * uy ;
                m_Spline.splineCurve();
                m_bSplined = false;
            }
            else if (n==m_Spline.m_CtrlPt.size()-2 && m_bTangentSpline)
            {
                //penultimate point must remain on tangent to curve
                // difficulty is that we are working in non-normal coordinates
                tanpt = QPoint(point.x(), point.y());
                P0 = QPoint(m_QGraph.xToClient(m_pMCurve->m_x[m_SplineRightPos-1]), m_QGraph.yToClient(m_pMCurve->m_y[m_SplineRightPos-1]));
                P1 = QPoint(m_QGraph.xToClient(m_pMCurve->m_x[m_SplineRightPos]),   m_QGraph.yToClient(m_pMCurve->m_y[m_SplineRightPos]));
                P2 = QPoint(m_QGraph.xToClient(m_pMCurve->m_x[m_SplineRightPos+1]), m_QGraph.yToClient(m_pMCurve->m_y[m_SplineRightPos+1]));
                //v is the tangent to the curve in screen coordinates
                vx = double((P0.x()-P1.x())*(P0.x()-P2.x())*(P1.x()-P2.x())*(P2.x()-P0.x()));

                vy = double( P0.y() *(P1.x()-P2.x())            * (P1.x()-P2.x()) * (P2.x()-P0.x())
                               - P1.y() *(2.0*P1.x()-P0.x()-P2.x()) * (P0.x()-P2.x()) * (P2.x()-P0.x())
                               - P2.y() *(P1.x()-P0.x())            * (P0.x()-P1.x()) * (P0.x()-P2.x()));
                vnorm = sqrt(vx*vx+vy*vy);
                vx/=vnorm;
                vy/=vnorm;
                scal = double(point.x()-P1.x())*vx + double(point.y()-P1.y())*vy;
                tanpt.rx() = P1.x() + int(vx * scal);
                tanpt.ry() = P1.y() + int(vy * scal);

                x1 =  m_QGraph.clientTox(tanpt.x()) ;
                y1 =  m_QGraph.clientToy(tanpt.y()) ;

                xx0 = m_pMCurve->m_x[m_SplineRightPos-1];
                xx1 = m_pMCurve->m_x[m_SplineRightPos];
                xx2 = m_pMCurve->m_x[m_SplineRightPos+1];
                yy0 = m_pMCurve->m_y[m_SplineRightPos-1];
                yy1 = m_pMCurve->m_y[m_SplineRightPos];
                yy2 = m_pMCurve->m_y[m_SplineRightPos+1];

                ux = (xx0-xx1)*(xx0-xx2)*(xx1-xx2)*(xx2-xx0);
                uy =      yy0 *(xx1-xx2)         * (xx1-xx2) * (xx2-xx0)
                        - yy1 *(2.0*xx1-xx0-xx2) * (xx0-xx2) * (xx2-xx0)
                        - yy2 *(xx1-xx0)         * (xx0-xx1) * (xx0-xx2);

                unorm = sqrt(ux*ux+uy*uy);
                ux /= unorm;
                uy /= unorm;

                vx = x1-m_Spline.m_CtrlPt[n+1].x;
                vy = y1-m_Spline.m_CtrlPt[n+1].y;

                scal =  (ux*vx + uy*vy);
                m_Spline.m_CtrlPt[n].x = m_Spline.m_CtrlPt[n+1].x + scal * ux;
                m_Spline.m_CtrlPt[n].y = m_Spline.m_CtrlPt[n+1].y + scal * uy;
                m_Spline.splineCurve();
                m_bSplined = false;
            }
            else if (n>0 && n<m_Spline.m_CtrlPt.size()-1)
            {
                m_Spline.m_CtrlPt[n].x = x1;
                m_Spline.m_CtrlPt[n].y = y1;
                m_Spline.splineCurve();
                m_bSplined = false;
            }
        }
    }
    else if((event->buttons() & Qt::MidButton)  || event->modifiers().testFlag(Qt::AltModifier))
    {
        releaseZoom();
        QPoint pttmp(point.x(), point.y());
        if(m_QGraph.isInDrawRect(pttmp))
        {
            //zoom graph
            m_QGraph.setAuto(false);
            if(point.y()-m_PointDown.y()<0) m_QGraph.scaleAxes(1.02);
            else                            m_QGraph.scaleAxes(1.0/1.02);
        }
        else
        {
            scale = m_fScale;

            if(point.y()-m_PointDown.y()>0) m_fScale *= 1.02;
            else                            m_fScale /= 1.02;

            a = int((m_rCltRect.right()+m_rCltRect.left())/2);
            m_ptOffset.rx() = a + int((m_ptOffset.x()-a)*m_fScale/scale);
        }
    }
    else
    {
        if(m_QGraph.isInDrawRect(point))
        {
            //            s_pMainFrame->statusBar()->showMessage(QString("X = %1, Y = %2").arg(m_QGraph.clientTox(event->x())).arg(m_QGraph.clientToy(event->y())));
            m_pCurGraph = &m_QGraph;
        }
        else
        {
            m_pCurGraph = nullptr;
        }

        // highlight if mouse passe over a point
        if(m_bSpline)
        {
            x1 =  m_QGraph.clientTox(point.x());
            y1 =  m_QGraph.clientToy(point.y());
            n = m_Spline.isControlPoint(x1,y1, m_QGraph.xScale(), m_QGraph.yScale());
            if (n>=0 && n<m_Spline.m_CtrlPt.size())
            {
                m_Spline.m_iHighlight = n;
            }
            else m_Spline.m_iHighlight = -1;
        }
    }
    m_PointDown = point;
    updateView();
}





/**
 * Overrides the QWidget's mousePressEvent method.
 * Dispatches the event
 * @param event the QMouseEvent
 */
void XInverse::mousePressEvent(QMouseEvent *event)
{
    bool bCtrl, bShift;
    bCtrl = bShift = false;
    if(event->modifiers() & Qt::ControlModifier) bCtrl  = true;
    if(event->modifiers() & Qt::ShiftModifier)   bShift = true;

    int CtrlPt;
    QPoint pttmp;
    QPoint point = event->pos();
    if((event->buttons() & Qt::LeftButton))
    {
        if(!m_bGetPos)
        {
            m_PointDown.rx() = point.x();
            m_PointDown.ry() = point.y();
            pttmp = QPoint(point.x(), point.y());

            if(m_QGraph.isInDrawRect(pttmp))
            {
                m_bTransGraph = true;
                s_p2dWidget->setCursor(Qt::ClosedHandCursor);
                xd = m_QGraph.clientTox(point.x());
                yd = m_QGraph.clientToy(point.y());
                if(m_bSpline)
                {
                    CtrlPt = m_Spline.isControlPoint(xd, yd, m_QGraph.xScale(), m_QGraph.yScale());
                    if(CtrlPt<0) m_Spline.m_iSelect = -1;
                    else
                    {
                        m_Spline.m_iSelect = CtrlPt;
                        //                        return;
                    }
                    if (bCtrl)
                    {
                        if(CtrlPt>=0)
                        {
                            if (m_Spline.m_iSelect>=0)
                            {
                                if(!m_Spline.removePoint(m_Spline.m_iSelect))
                                {
                                    m_pmteOutput->append(tr("The minimum number of control points has been reached for this spline degree"));
                                    m_pmteOutput->append("\n");

                                    return;
                                }
                                m_Spline.splineKnots();
                                m_Spline.splineCurve();
                            }
                        }
                    }
                    else if (bShift)
                    {
                        m_Spline.insertPoint(xd,yd);
                        m_Spline.splineKnots();
                        m_Spline.splineCurve();
                    }
                    if(CtrlPt>=0) return;
                }
            }
            else m_bTransGraph = false;

            if(m_bZoomPlus && m_QGraph.isInDrawRect(point))
            {
                m_ZoomRect.setLeft(point.x());
                m_ZoomRect.setTop(point.y());
                m_ZoomRect.setRight(point.x());
                m_ZoomRect.setBottom(point.y());
                return;
            }
            else if(m_bZoomPlus && !m_QGraph.isInDrawRect(point))
            {
                releaseZoom();
            }
            else
            {
                s_p2dWidget->setCursor(Qt::ClosedHandCursor);
                m_bTrans = true;
                m_bZoomPlus = false;
            }
        }
    }
    else if((event->buttons() & Qt::RightButton))
    {
        m_ptPopUp = event->pos();
    }
}


/**
 * Overrides the QWidget's mouseReleaseEvent method.
 * Dispatches the event
 * @param event the QMouseEvent
 */
void XInverse::mouseReleaseEvent(QMouseEvent *event)
{
    m_bTrans = false;

    int tmp, width, height;
    double x1,x2,w,h,xw,yh,xm,ym, dist;
    double xmin, ymin, xmax, ymax;
    double ratio,x, y, ux, uy, xpt, ypt, norm;

    QPoint point = event->pos();

    if(m_bZoomPlus && m_rCltRect.contains(point))
    {
        QRect ZRect = m_ZoomRect.normalized();
        if (!ZRect.isEmpty() )
        {
            xu = m_QGraph.clientTox(point.x());
            yu = m_QGraph.clientToy(point.y());

            width  = abs(m_PointDown.x()-point.x());
            height = abs(m_PointDown.y()-point.y());
            //preserve ratio
            w = qAbs(xu-xd);
            h = qAbs(yu-yd);
            xw =     m_QGraph.xMax() - m_QGraph.xMin();
            yh =     m_QGraph.yMax() - m_QGraph.yMin();
            xm = (xu+xd)/2.0;
            ym = (yu+yd)/2.0;

            if(width>=height)
            {
                xmin  = xm - w/2.0;
                xmax  = xm + w/2.0;
                ratio = w/xw;

                ymin  = ym - ratio*yh/2.0;
                ymax  = ym + ratio*yh/2.0;
            }
            else
            {
                ymin  = ym - h/2.0;
                ymax  = ym + h/2.0;
                ratio = h/yh;
                xmin  = xm - ratio * xw/2.0;
                xmax  = xm + ratio * xw/2.0;
            }
            if (m_QGraph.isInDrawRect(ZRect.left(), ZRect.top()) &&
                    m_QGraph.isInDrawRect(ZRect.right(), ZRect.bottom()))
            {
                m_QGraph.setWindow(xmin, xmax, ymin, ymax);
            }
            m_ZoomRect.setRight(m_ZoomRect.left()-1);
            m_ZoomRect.setTop(m_ZoomRect.bottom()+1);
        }
        else
        {
            releaseZoom();
        }
    }
    else if(m_bZoomPlus && !m_rCltRect.contains(point))
    {
        releaseZoom();
    }
    else if(m_bGetPos && m_rCltRect.contains(point))
    {
        if(m_nPos == 0)
        {
            m_Pos1 = m_pMCurve->closestPoint(m_QGraph.clientTox(point.x()), m_QGraph.clientToy(point.y()), dist);
        }
        if(m_nPos == 1)
        {
            m_Pos2 = m_pMCurve->closestPoint(m_QGraph.clientTox(point.x()), m_QGraph.clientToy(point.y()), dist);
        }
        m_nPos++;
        if(m_nPos == 2)
        {
            if(m_bSmooth)
            {
                m_pmteOutput->append("\n");
                smooth(m_Pos1, m_Pos2);
            }
            else if(m_bSpline)
            {
                x1 = m_pMCurve->m_x[m_Pos1];
                x2 = m_pMCurve->m_x[m_Pos2];
                if(qAbs(x2-x1)<0.00001) return;
                if(x2<x1)
                {
                    tmp    = m_Pos2;
                    m_Pos2 = m_Pos1;
                    m_Pos1 = tmp;
                }

                m_SplineLeftPos  = m_Pos1;
                m_SplineRightPos = m_Pos2;

                m_Spline.m_CtrlPt.clear();
                m_Spline.insertPoint(m_pMCurve->m_x[m_Pos1], m_pMCurve->m_y[m_Pos1]);
                m_Spline.insertPoint(m_pMCurve->m_x[m_Pos2], m_pMCurve->m_y[m_Pos2]);

                x = (3.0*m_pMCurve->m_x[m_Pos1] + m_pMCurve->m_x[m_Pos2])/4.0;
                y = (3.0*m_pMCurve->m_y[m_Pos1] + m_pMCurve->m_y[m_Pos2])/4.0;
                m_Spline.insertPoint(x,y);

                x = (m_pMCurve->m_x[m_Pos1] + m_pMCurve->m_x[m_Pos2])/2.0;
                y = (m_pMCurve->m_y[m_Pos1] + m_pMCurve->m_y[m_Pos2])/2.0;
                m_Spline.insertPoint(x,y);

                x = (m_pMCurve->m_x[m_Pos1] + 3.0*m_pMCurve->m_x[m_Pos2])/4.0;
                y = (m_pMCurve->m_y[m_Pos1] + 3.0*m_pMCurve->m_y[m_Pos2])/4.0;
                m_Spline.insertPoint(x,y);


                if (m_bTangentSpline)
                {
                    //Second point must remain on tangent to curve
                    ux = m_pMCurve->m_x[m_Pos1+1] - m_pMCurve->m_x[m_Pos1];
                    uy = m_pMCurve->m_y[m_Pos1+1] - m_pMCurve->m_y[m_Pos1];
                    norm = sqrt(ux*ux+uy*uy);
                    ux /= norm;
                    uy /= norm;
                    xpt = m_Spline.m_CtrlPt[1].x - m_Spline.m_CtrlPt[0].x;
                    ypt = m_Spline.m_CtrlPt[1].y - m_Spline.m_CtrlPt[0].y;

                    m_Spline.m_CtrlPt[1].x = m_Spline.m_CtrlPt[0].x + (ux*xpt + uy*ypt) * ux;
                    m_Spline.m_CtrlPt[1].y = m_Spline.m_CtrlPt[0].y + (ux*xpt + uy*ypt) * uy;


                    //penultimate point must remain on tangent to curve
                    ux = m_pMCurve->m_x[m_Pos2] - m_pMCurve->m_x[m_Pos2-1];
                    uy = m_pMCurve->m_y[m_Pos2] - m_pMCurve->m_y[m_Pos2-1];
                    norm = sqrt(ux*ux+uy*uy);
                    ux /= norm;
                    uy /= norm;

                    xpt = m_Spline.m_CtrlPt[m_Spline.m_CtrlPt.size()-2].x - m_Spline.m_CtrlPt[m_Spline.m_CtrlPt.size()-1].x;
                    ypt = m_Spline.m_CtrlPt[m_Spline.m_CtrlPt.size()-2].y - m_Spline.m_CtrlPt[m_Spline.m_CtrlPt.size()-1].y;

                    m_Spline.m_CtrlPt[m_Spline.m_CtrlPt.size()-2].x = m_Spline.m_CtrlPt[m_Spline.m_CtrlPt.size()-1].x + (ux*xpt + uy*ypt) * ux;
                    m_Spline.m_CtrlPt[m_Spline.m_CtrlPt.size()-2].y = m_Spline.m_CtrlPt[m_Spline.m_CtrlPt.size()-1].y + (ux*xpt + uy*ypt) * uy;
                }

                m_Spline.splineKnots();
                m_Spline.splineCurve();
                if(m_bFullInverse)
                {
                    m_ppbNewSpline->setChecked(0);
                }
                else
                {
                    m_ppbMNewSpline->setChecked(0);
                }
                m_pmteOutput->append(
                            tr("Drag the points to modify the spline, Apply, and Execute to generate the new geometry"));
            }
            else if(m_bMark)
            {
                if (m_Pos1 == m_Pos2) return;

                m_Mk1 = m_Pos1;
                m_Mk2 = m_Pos2;
                m_pXFoil->iq1 = min(m_Pos1, m_Pos2)+1;
                m_pXFoil->iq2 = max(m_Pos1, m_Pos2)+1;
                if(m_pXFoil->iq1<=1) m_pXFoil->iq1 = 2;
                if(m_pXFoil->iq2>=m_pXFoil->n) m_pXFoil->iq2 = m_pXFoil->n-1;
                m_pXFoil->liqset = true;

                m_bMarked = true;
                m_bMark   = false;
                m_ppbMark->setChecked(false);
            }
            m_bGetPos = false;

        }
    }

    s_p2dWidget->setCursor(Qt::CrossCursor);
    updateView();
}



/**
 * Converts screen coordinates to viewport coordinates
 * @param point the screen coordinates
 * @return the viewport coordinates
 */
Vector3d XInverse::mousetoReal(QPoint point)
{
    Vector3d Real;

    Real.x =  (point.x() - m_ptOffset.x())/m_fScale;
    Real.y = -(point.y() - m_ptOffset.y())/m_fScale;
    Real.z = 0.0;

    return Real;
}


/**
 * The user has requested to apply the spline to the velocity curve
 */
void XInverse::onApplySpline()
{
    if(!m_bSplined)
    {
        int i;
        double qscom, xx;
        for (i=1; i<m_pMCurve->size()-1; i++)
        {
            xx = m_pMCurve->m_x[i];
            if (xx > m_Spline.m_CtrlPt.first().x &&
                    xx < m_Spline.m_CtrlPt.last().x )
            {
                //interpolate spline at xx
                m_pMCurve->m_y[i] = m_Spline.getY(xx);
            }
        }

        for (i=1; i<m_pMCurve->size()-1; i++)
        {
            m_pReflectedCurve->m_y[i] = -m_pMCurve->m_y[i];
        }

        m_bSplined = true;
        for (i=1; i<= m_pXFoil->nsp; i++)
        {
            //            isp = pXFoil->nsp - i + 1;
            qscom =  m_pXFoil->qinf*m_pMCurve->m_y[i-1];
            m_pXFoil->qspec[1][i] = qincom(qscom,m_pXFoil->qinf,m_pXFoil->tklam);
        }

        m_pXFoil->lqspec = false;

        updateView();
    }
    if(m_bZoomPlus) releaseZoom();
    //    m_bSpline = false;
    m_nPos    = 0;
    m_tmpPos  = -1;
    m_Pos1    = -1;
    m_Pos2    = -1;

    m_pmteOutput->append(tr("Spline is applied"));
    updateView();
    emit projectModified();
}


/**
 * Not sure - refer to XFoil documentation
 */
void XInverse::onCpxx()
{
    cancelSpline();
    cancelSmooth();
    cancelMark();
    if (m_bZoomPlus) releaseZoom();
    m_pXFoil->lcpxx = m_pchCpxx->isChecked();
}


/**
 * The user has requested the execution of the modification
 */
void XInverse::onExecute()
{
    if (m_bZoomPlus) releaseZoom();
    cancelSpline();
    cancelSmooth();
    cancelMark();

    if(m_bFullInverse)
    {
        setTAngle(m_pdeTAngle->value());
        setTGap(m_pdeTGapx->value(), m_pdeTGapy->value());
        execMDES();
    }
    else
    {
        m_pXFoil->niterq = m_pieIter->value();
        execQDES();
    }
    updateView();
    emit projectModified();
}


/**
 *     Extracts a Foil from the database for display and modification
*/
void XInverse::onExtractFoil()
{
    FoilSelectionDlg dlg(s_pMainFrame);

    dlg.initDialog(Objects2d::pOAFoil(), QStringList());

    if(m_bLoaded)
    {
        dlg.setFoilName(m_pRefFoil->name());
    }

    if(QDialog::Accepted == dlg.exec())
    {
        m_bMark = false;
        m_bMarked = false;
        m_bSpline = false;
        m_bSplined  = true;
        Foil *pFoil;
        pFoil = Objects2d::foil(dlg.selectedFoilName());
        XDirect::setCurFoil(pFoil);

        m_pRefFoil->copyFoil(pFoil);

        m_pModFoil->setName(m_pRefFoil->name() + tr(" Modified"));
        initXFoil(m_pRefFoil);
        setFoil();
        updateView();
    }
}


/** Selects a foil for overlay in the display */
void XInverse::onOverlayFoil()
{
    FoilSelectionDlg dlg(s_pMainFrame);

    dlg.initDialog(Objects2d::pOAFoil(), QStringList());

    if(QDialog::Accepted == dlg.exec())
    {
        m_pOverlayFoil = Objects2d::foil(dlg.selectedFoilName());
        updateView();
    }
}


void XInverse::onClearOverlayFoil()
{
    m_pOverlayFoil = nullptr;
    updateView();
}


/**
 * Applies a Hanning type filter to the velocity curve
 */
void XInverse::onFilter()
{
    cancelSpline();
    if (m_bZoomPlus) releaseZoom();

    double filt = m_pdeFilterParam->value();
    QString strong;
    QTextStream ts(&strong);
    m_pXFoil->HanningFilter(filt, ts);
    m_pmteOutput->append("Hanning filter:");
    m_pmteOutput->append(strong);
    m_pmteOutput->append("\n");
    createMCurve();

    updateView();
}


/**
 *The user has requested an edition of the graph settings
*/
void XInverse::onQGraphSettings()
{
    GraphDlg *m_pGraphDlg = new GraphDlg(s_pMainFrame);

    m_pGraphDlg->XSel() = 0;
    m_pGraphDlg->YSel() = 0;
    m_pGraphDlg->setGraph(&m_QGraph);

    m_pGraphDlg->exec();
    updateView();
}




/**
 * The user has requested the insertion of a new control point in the Spline object at the mouse position
 */
void XInverse::onInsertCtrlPt()
{
    double xd = m_QGraph.clientTox(m_ptPopUp.x());
    double yd = m_QGraph.clientToy(m_ptPopUp.y());

    if(xd < m_Spline.m_CtrlPt.first().x) return;
    if(xd > m_Spline.m_CtrlPt.last().x) return;

    m_Spline.insertPoint(xd,yd);
    m_Spline.splineKnots();
    m_Spline.splineCurve();
    updateView();
}


/**
 * The user has toggled between full-inverse and mixed-inverse applications
 */
void XInverse::onInverseApp()
{
    m_bFullInverse = s_pMainFrame->m_prbFullInverse->isChecked();

    if(m_bFullInverse)
    {
        m_pswInv->setCurrentIndex(0);
    }
    else
    {
        m_pswInv->setCurrentIndex(1);
    }
    setFoil();
    updateView();
}


/**
 * The user has requested a modification of the styles of the curves
 */
void XInverse::onInverseStyles()
{
    InverseOptionsDlg *m_pXInverseStyleDlg = new InverseOptionsDlg(s_pMainFrame);
    m_pXInverseStyleDlg->m_pXInverse = this;
    m_pXInverseStyleDlg->initDialog();
    m_pXInverseStyleDlg->exec();

    m_pQCurve->setStipple(m_pRefFoil->lineStipple());
    m_pQCurve->setWidth(m_pRefFoil->lineWidth());
    m_pQCurve->setColor(m_pRefFoil->color());

    m_pMCurve->setStipple(m_pModFoil->lineStipple());
    m_pMCurve->setWidth(m_pModFoil->lineWidth());
    m_pMCurve->setColor(m_pModFoil->color());

    updateView();
}


/**
 * The user has requested to mark a segment for modification on the curve
 */
void XInverse::onMarkSegment()
{
    cancelSpline();
    cancelSmooth();

    if (m_bZoomPlus) releaseZoom();

    if(m_ppbMark->isChecked())
    {
        m_pmteOutput->append(tr("Mark target segment for modification"));
    }

    m_tmpPos  = -1;
    m_bMark   = true;
    m_bMarked = false;
    m_bSpline = false;
    m_bGetPos = true;
    m_nPos    = 0;

    m_ppbMNewSpline->setChecked(false);
    m_pchMShowSpline->setChecked(false);

    updateView();
}


/**
 * The user has requested the creation of a new spline to define the modification of the velocity curve
 */
void XInverse::onNewSpline()
{
    releaseZoom();
    if((m_bFullInverse && m_ppbNewSpline->isChecked()) || (!m_bFullInverse && m_ppbMNewSpline->isChecked()))
    {
        cancelSmooth();
        cancelMark();

        m_pmteOutput->append(tr("Mark spline endpoints"));

        m_bSpline = true;
        m_bSplined = false;
        if(m_bFullInverse) m_pchShowSpline->setChecked(true);
        else               m_pchMShowSpline->setChecked(true);
        m_bSmooth = false;
        m_bGetPos = true;
        m_nPos    = 0;
        m_tmpPos  = -1;
        m_Pos1    = -1;
        m_Pos2    = -1;
    }
    else
    {
        cancelSpline();
    }
    updateView();
}


/**
 * @todo check The user has requested the launch of the interface to define the perturbation to the curve
 */
void XInverse::onPertubate()
{
    int m=0;
    m_pXFoil->pert_init(1);

    PertDlg PerturbDlg(s_pMainFrame);

    for (m=0; m<=qMin(32, m_pXFoil->nc); m++)
    {
        PerturbDlg.m_cnr[m] = real(m_pXFoil->cn[m]);
        PerturbDlg.m_cni[m] = imag(m_pXFoil->cn[m]);
    }
    PerturbDlg.m_nc = qMin(32, m_pXFoil->nc);
    PerturbDlg.initDialog();

    if(PerturbDlg.exec() == QDialog::Accepted)
    {
        for (m=0; m<=qMin(32, m_pXFoil->nc); m++)
        {
            m_pXFoil->cn[m] = complex<double>(PerturbDlg.m_cnr[m], PerturbDlg.m_cni[m]);
        }

        m_pXFoil->pert_process(1);
        createMCurve();
        m_pMCurve->setVisible(true);
        updateView();
    }
}


/** Toggles the visibility of the reference curve */
void XInverse::onQInitial()
{
    m_pQCurve->setVisible(!m_pQCurve->isVisible());
    checkActions();
    updateView();
}

/** Toggles the visibility of the specification curve */
void XInverse::onQSpec()
{
    m_pMCurve->setVisible(!m_pMCurve->isVisible());
    checkActions();
    updateView();
}

/** Toggles the visibility of the viscous curve */
void XInverse::onQViscous()
{
    if(m_pXFoil->lvisc)
    {
        m_pQVCurve->setVisible(!m_pQVCurve->isVisible());
        updateView();
    }
    checkActions();
}

/** Toggles the visibility of the curve's points */
void XInverse::onQPoints()
{
    m_bShowPoints = !m_bShowPoints;
    m_pQCurve->setPointStyle(m_bShowPoints);
    m_pMCurve->setPointStyle(m_bShowPoints);
    checkActions();
    updateView();
}

/** Toggles the visibility of the reflected curve */
void XInverse::onQReflected()
{
    m_bReflected = !m_bReflected;
    m_pReflectedCurve->setVisible(m_bReflected);
    checkActions();
    updateView();
}


/** Resets the specification curve */
void XInverse::onQReset()
{
    cancelSpline();
    cancelSmooth();
    cancelMark();
    releaseZoom();
    if(m_bFullInverse) resetQ();
    else               resetMixedQ();
    updateView();
}

/**
 * The user has requested to remove the selected control point from the spline
 */
void XInverse::onRemoveCtrlPt()
{
    if (m_Spline.m_iHighlight>=0)
    {
        if(!m_Spline.removePoint(m_Spline.m_iHighlight))
        {
            m_pmteOutput->append(tr("The minimum number of control points has been reached for this spline degree"));
            m_pmteOutput->append(tr("\n"));

            return;
        }
    }
    m_Spline.splineCurve();
    updateView();
}

/** The user has requested a reset of the Foil scale */
void XInverse::onResetFoilScale()
{
    releaseZoom();
    resetScale();
    updateView();
}


/**
 * The user has toggled between aoa and lift coefficient as the input parameter
 */
void XInverse::onSpecal()
{
    if(m_prbSpecAlpha->isChecked())
    {
        m_plabSpecif->setText(tr("Alpha = "));
        m_pdeSpec->setDigits(2);
        m_pdeSpec->setValue(m_pXFoil->alqsp[1]*180.0/PI);
        m_pXFoil->iacqsp = 1;
    }
    else
    {
        m_plabSpecif->setText(tr("Cl = "));
        m_pdeSpec->setDigits(3);
        m_pdeSpec->setValue(m_pXFoil->clqsp[1]);
        m_pXFoil->iacqsp = 2;
    }
}

/**
 * The user has modified the value of the aoa or lift coefficient
 */
void XInverse::onSpecInv()
{
    if (m_bZoomPlus) releaseZoom();

    if(m_prbSpecAlpha->isChecked())
    {
        m_pXFoil->alqsp[1] = m_pdeSpec->value()*PI/180.0;
        m_pXFoil->iacqsp = 1;
        m_pXFoil->qspcir();
    }
    else if(m_prbSpecCl->isChecked())
    {
        m_pXFoil->clqsp[1] = m_pdeSpec->value();
        m_pXFoil->iacqsp = 2;
        m_pXFoil->qspcir();
    }
    createQCurve();
    createMCurve();
    updateView();
}


/** The user has toggled the spline display */
void XInverse::onShowSpline()
{
    if(m_bFullInverse) m_bSpline = m_pchShowSpline->isChecked();
    else               m_bSpline = m_pchMShowSpline->isChecked();
    m_bSplined =   !m_bSpline;
    updateView();
}


/** The user has requested a smoothing operation on the curve */
void XInverse::onSmooth()
{
    cancelSpline();
    if(m_ppbSmooth->isChecked() || m_ppbMSmooth->isChecked())
    {
        m_pmteOutput->append(tr("Mark target segment for smoothing, or type 'Return' to smooth the entire distribution, then Execute"));
        m_pmteOutput->append("\n");

        m_bSpline = false;
        m_bSmooth = true;
        updateView();
        m_bGetPos = true;
        m_nPos    = 0;
    }
    else cancelSmooth();
}


/**
 * The user has requested the storage of the modified Foil in the database
 */
void XInverse::onStoreFoil()
{
    if(!m_bLoaded) return;

    Foil* pNewFoil = new Foil();
    pNewFoil->copyFoil(m_pModFoil);
    pNewFoil->setLineStipple(Line::SOLID);
    pNewFoil->setLineWidth(1);
    memcpy(pNewFoil->m_xb, m_pModFoil->m_x, sizeof(m_pModFoil->m_x));
    memcpy(pNewFoil->m_yb, m_pModFoil->m_y, sizeof(m_pModFoil->m_y));
    pNewFoil->m_nb = m_pModFoil->m_n;
    pNewFoil->setName(m_pRefFoil->name());

    QStringList NameList;
    for(int k=0; k<Objects2d::foilCount(); k++)
    {
        Foil *pOldFoil = Objects2d::foilAt(k);
        NameList.append(pOldFoil->name());
    }

    RenameDlg renDlg(s_pMainFrame);
    renDlg.initDialog(&NameList, m_pRefFoil->name() + " modified", tr("Enter the foil's new name"));
    if(renDlg.exec() !=QDialog::Rejected)
    {
        pNewFoil->setName(renDlg.newName());
        Objects2d::insertThisFoil(pNewFoil);
    }
    else
    {
        delete pNewFoil;
    }
}


/** The user has toggled the symetric requirement for the foil*/
void XInverse::onSymm()
{
    cancelSpline();

    if (m_bZoomPlus) releaseZoom();

    m_pXFoil->lqsym = m_pchSymm->isChecked();
    m_pXFoil->lqspec = false;
}


/** The user has requested to zoom in on a part of the display */
void XInverse::onZoomIn()
{
    if(!m_bZoomPlus)
    {
        if(m_fScale/m_fRefScale <32.0)
        {
            m_bZoomPlus = true;
            s_pMainFrame->m_pInverseZoomIn->setChecked(true);
        }
        else
        {
            releaseZoom();
        }
    }
    else {
        releaseZoom();
    }
}


/** The user has toggled the requirement for a spline tangent to the specification curve */
void XInverse::onTangentSpline()
{
    if(m_bFullInverse) m_bTangentSpline = m_pchTangentSpline->isChecked();
    else               m_bTangentSpline = m_pchMTangentSpline->isChecked();
    m_pchTangentSpline->setChecked(m_bTangentSpline);
    m_pchMTangentSpline->setChecked(m_bTangentSpline);
}


/**
 * Draws the graph
 * @param painter a reference to the QPainter object with which to draw
 */
void XInverse::paintGraph(QPainter &painter)
{
    painter.save();

    //  draw  the graph
    if(m_rGraphRect.width()>200 && m_rGraphRect.height()>150)
    {
        m_QGraph.drawGraph(painter);
        QPoint Place(int(m_rGraphRect.right()-300), m_rGraphRect.top()+12);
        m_QGraph.drawLegend(painter, Place, DisplayOptions::textFont(), DisplayOptions::textColor(), DisplayOptions::backgroundColor());
    }

    // draw the zoom rectangle, if relevant
    QRect ZRect = m_ZoomRect.normalized();

    if(m_bZoomPlus && !ZRect.isEmpty())
    {
        QPen ZoomPen(QColor(100,100,100));
        ZoomPen.setStyle(Qt::DashLine);
        painter.setPen(ZoomPen);
        painter.drawRect(ZRect);
    }

    //Draw spline, if any
    if(m_bSpline && !m_bGetPos)
    {

        QPoint pt = m_QGraph.offset();

        m_Spline.drawSpline(painter, 1.0/m_QGraph.xScale(), -1.0/m_QGraph.yScale(), pt);
        m_Spline.drawCtrlPoints(painter, 1.0/m_QGraph.xScale(), -1.0/m_QGraph.yScale(), pt);

    }

    // Highlight selected points, if any
    if(m_bGetPos)
    {
        //QRect r;
        m_QGraph.highlight(painter, m_pMCurve,m_tmpPos);
        if(m_nPos>=1) m_QGraph.highlight(painter, m_pMCurve, m_Pos1);
        if(m_nPos>=2) m_QGraph.highlight(painter, m_pMCurve, m_Pos2);

    }
    // Show marked segment if mixed-inverse design
    if(m_bMarked)
    {
        if(m_Mk1<m_pMCurve->count() && m_Mk2<m_pMCurve->count())
        {
            QPoint ptl, ptr;
            ptl.rx() = m_QGraph.xToClient(m_pMCurve->m_x[m_Mk1]);
            ptr.rx() = m_QGraph.xToClient(m_pMCurve->m_x[m_Mk2]);
            ptl.ry() = m_QGraph.yToClient(m_pMCurve->m_y[m_Mk1]);
            ptr.ry() = m_QGraph.yToClient(m_pMCurve->m_y[m_Mk2]);

            QPen MarkPen(QColor(175,30,30));
            MarkPen.setStyle(Qt::SolidLine);
            MarkPen.setWidth(2);
            painter.setPen(MarkPen);

            if(m_rGraphRect.contains(ptl))
            {
                painter.drawLine(ptl.x(), ptl.y()-20, ptl.x(), ptl.y()+20);
            }
            if(m_rGraphRect.contains(ptr))
            {
                painter.drawLine(ptr.x(), ptr.y()-20, ptr.x(), ptr.y()+20);
            }
        }
    }

    if(m_QGraph.isInDrawRect(m_PointDown) && DisplayOptions::bMousePos())
    {
        QPen textPen(DisplayOptions::textColor());
        QFontMetrics fm(DisplayOptions::textFont());

        int fmheight  = fm.height();

        painter.setPen(textPen);

        painter.drawText(s_p2dWidget->width()-14*fm.averageCharWidth(),  fmheight, QString("x = %1").arg(m_QGraph.clientTox(m_PointDown.x()),9,'f',5));
        painter.drawText(s_p2dWidget->width()-14*fm.averageCharWidth(),2*fmheight, QString("y = %1").arg(m_QGraph.clientToy(m_PointDown.y()),9,'f',5));
    }
    painter.restore();
}


/**
 * Draws the Foil
 * @param painter a reference to the QPainter object with which to draw
 */
void XInverse::paintFoil(QPainter &painter)
{
    painter.save();
    QString str,str1,str2;

    //    draw the scale/grid
    if(m_bModFoil || m_bRefFoil)
    {
        drawGrid(painter, m_fScale);
    }

    //draw the reference and modified foils
    double alpha = m_pXFoil->alqsp[1]*180./PI;

    QPen TextPen(DisplayOptions::textColor());

    if(m_bRefFoil && m_bLoaded)
    {
        QPen FoilPen(m_pRefFoil->color());
        FoilPen.setStyle(xfl::getStyle(m_pRefFoil->lineStipple()));
        FoilPen.setWidth(m_pRefFoil->lineWidth());
        painter.setPen(FoilPen);

        xfl::drawFoil(painter, m_pRefFoil, -alpha, m_fScale, m_fScale*m_fYScale, m_ptOffset);
        painter.drawLine(20, m_rGraphRect.bottom()+20, 40, m_rGraphRect.bottom()+20);
        painter.setPen(TextPen);
        painter.drawText(50, m_rGraphRect.bottom()+25, m_pRefFoil->name());
    }

    if(m_bModFoil && m_bLoaded)
    {
        QPen ModPen(m_pModFoil->color());
        ModPen.setStyle(xfl::getStyle(m_pModFoil->lineStipple()));
        ModPen.setWidth(m_pModFoil->lineWidth());
        painter.setPen(ModPen);

        xfl::drawFoil(painter, m_pModFoil, -alpha, m_fScale, m_fScale*m_fYScale, m_ptOffset);
        painter.drawLine(20, m_rGraphRect.bottom()+35, 40, m_rGraphRect.bottom()+35);
        painter.setPen(TextPen);
        painter.drawText(50, m_rGraphRect.bottom()+40, m_pModFoil->name());
    }

    if(m_pOverlayFoil)
    {
        QPen ModPen(m_pOverlayFoil->color());
        ModPen.setStyle(xfl::getStyle(m_pOverlayFoil->lineStipple()));
        ModPen.setWidth(m_pOverlayFoil->lineWidth());
        painter.setPen(ModPen);

        xfl::drawFoil(painter, m_pOverlayFoil, -alpha, m_fScale, m_fScale*m_fYScale, m_ptOffset);
        painter.drawLine(20, m_rGraphRect.bottom()+50, 40, m_rGraphRect.bottom()+50);
        painter.setPen(TextPen);
        painter.drawText(50, m_rGraphRect.bottom()+55, m_pOverlayFoil->name());
    }

    if (m_pRefFoil->pointStyle()>0)
    {
        QPen CtrlPen(m_pRefFoil->color());
        CtrlPen.setStyle(xfl::getStyle(m_pRefFoil->lineStipple()));
        CtrlPen.setWidth(m_pRefFoil->lineWidth());
        painter.setPen(CtrlPen);

        xfl::drawFoilPoints(painter, m_pRefFoil, -alpha, 1.0,  1.0, m_ptOffset, DisplayOptions::backgroundColor());
    }

    painter.setFont(DisplayOptions::textFont());
    QFontMetrics fm(DisplayOptions::textFont());
    int dD = fm.height();
    int Back = 4;
    int LeftPos = m_rCltRect.left()+10;
    int ZPos = m_rCltRect.bottom() - 10 - Back*dD;

    int D = 0;

    str = tr("                     Base");
    if(m_bModFoil && m_bLoaded)  str +=tr("       Mod.");

    painter.setPen(TextPen);

    painter.drawText(LeftPos,ZPos+D, str);
    D += dD;

    str1 = QString(tr("Thickness        = %1%")).arg(m_pRefFoil->thickness()*100.0, 6, 'f', 2);
    if(m_bModFoil && m_bLoaded)  str2 = QString("    %1%").arg(m_pModFoil->thickness()*100.0, 6, 'f', 2);
    else str2 = "";
    painter.drawText(LeftPos,ZPos+D, str1+str2);
    D += dD;

    str1 = QString(tr("Max.Thick.pos.   = %1%")).arg(m_pRefFoil->xThickness()*100.0, 6, 'f', 2);
    if(m_bModFoil && m_bLoaded)  str2 = QString("    %1%").arg(m_pModFoil->xThickness()*100.0, 6, 'f', 2);
    else str2 = "";
    painter.drawText(LeftPos,ZPos+D, str1+str2);
    D += dD;

    str1 = QString(tr("Max.Camber       = %1%")).arg( m_pRefFoil->camber()*100.0, 6, 'f', 2);
    if(m_bModFoil && m_bLoaded)  str2 = QString("    %1%").arg(m_pModFoil->camber()*100.0, 6, 'f', 2);
    else str2 = "";
    painter.drawText(LeftPos,ZPos+D, str1+str2);
    D += dD;

    str1 = QString(tr("Max.Camber.pos.  = %1%")).arg(m_pRefFoil->xCamber()*100.0, 6, 'f', 2);
    if(m_bModFoil && m_bLoaded)  str2 = QString("    %1%").arg(m_pModFoil->xCamber()*100.0, 6, 'f', 2);
    else str2 = "";
    painter.drawText(LeftPos,ZPos+D, str1+str2);
    D += dD;

    //convert screen coordinates to foil coordinates

    Vector3d real = mousetoReal(m_PointDown);
    painter.drawText(m_QGraph.clientRect()->width()-14*fm.averageCharWidth(),
                     m_QGraph.clientRect()->height() + dD,   QString("x = %1")
                     .arg(real.x,9,'f',5));
    painter.drawText(m_QGraph.clientRect()->width()-14*fm.averageCharWidth(),
                     m_QGraph.clientRect()->height() + 2*dD, QString("y = %1")
                     .arg(real.y,9,'f',5));

    painter.restore();
}


/** Paints the view
 * @param painter a reference to the QPainter object with which to draw
 */
void XInverse::paintView(QPainter &painter)
{
    painter.save();

    painter.fillRect(m_rCltRect, DisplayOptions::backgroundColor());
    paintGraph(painter);
    paintFoil(painter);

    painter.restore();
}

/**
 * -------------------------------------
 * XFoil source code: sets incompressible speed from karman-tsien compressible speed
 *-------------------------------------
 */
double XInverse::qincom(double qc, double qinf, double tklam)
{
    //-------------------------------------
    //     sets incompressible speed from
    //     karman-tsien compressible speed
    //-------------------------------------

    if(tklam<1.0e-4 || qAbs(qc)<1.0e-4)
    {
        //----- for nearly incompressible case or very small speed, use asymptotic
        //      expansion of singular quadratic formula to avoid numerical problems
        return( qc/(1.0 - tklam));
    }
    else
    {
        //----- use quadratic formula for typical case
        double tmp = 0.5*(1.0 - tklam)*qinf/(qc*tklam);
        return (qinf*tmp*(sqrt(1.0 + 1.0/(tklam*tmp*tmp)) - 1.0));
    }
}

/**
 * Ends the zoom-in process
 */
void XInverse::releaseZoom()
{
    m_bZoomPlus  = false;
    m_ZoomRect.setRight(m_ZoomRect.left()-1);
    m_ZoomRect.setTop(m_ZoomRect.bottom()+1);
    s_pMainFrame->m_pInverseZoomIn->setChecked(false);
}


/**
 * Resets the mixed inverse specification curve
 */
void XInverse::resetMixedQ()
{
    m_pMCurve->clear();
    for (int i=0; i<m_pQCurve->size(); i++)
    {
        m_pMCurve->appendPoint(m_pQCurve->m_x[i], m_pQCurve->m_y[i]);
    }

    //    m_pXFoil->gamqsp(1);
    //    CreateMCurve();
}


/**
 * Resets the reference velocity curve
 */
void XInverse::resetQ()
{
    m_pXFoil->cncalc(m_pXFoil->qgamm,false);
    m_pXFoil->qspcir();
    createMCurve();
    updateView();
}


/**
 * Resets the Foil scale
 */
void XInverse::resetScale()
{
    int h4 = m_rCltRect.height()/4;
    m_ptOffset.rx() = m_rGraphRect.left() +(1*m_QGraph.margin());
    m_fRefScale  = m_rGraphRect.width()-2.0*m_QGraph.margin();

    m_ptOffset.ry() = m_rCltRect.bottom()-h4/2;
    m_fScale = m_fRefScale;
    m_fYScale = 1.0;
}


/**
 * Initializes the interface with the selected foil
 */
void XInverse::setFoil()
{
    int i;
    QFile *pXFile=nullptr;

    QString strong;
    for(i=1; i<=m_pXFoil->n; i++)
    {
        m_pModFoil->m_x[i-1] = m_pXFoil->x[i];
        m_pModFoil->m_y[i-1] = m_pXFoil->y[i];
    }
    m_pModFoil->m_n = m_pXFoil->n;

    if(m_bFullInverse)
    {
        m_pXFoil->InitMDES();
        createQCurve();
        createMCurve();

        m_pdeSpec->setValue(m_pXFoil->alqsp[1]*180.0/PI);
        m_pdeTAngle->setValue(m_pXFoil->agte*180.0);//agte expressed in PI units:!?!?
        m_pdeTGapx->setValue(real(m_pXFoil->dzte));
        m_pdeTGapy->setValue(imag(m_pXFoil->dzte));
    }
    else
    {
        // Mixed Inverse
        QString FileName = QDir::tempPath() + "/XFLR5.log";
        pXFile = new QFile(FileName);
        if (!pXFile->open(QIODevice::WriteOnly | QIODevice::Text)) pXFile = nullptr;

        //        pXFoil->m_pOutStream->setDevice(pXFile);

        m_pXFoil->InitQDES();

        createQCurve();
        createMCurve();
        strong = QString::asprintf("Alpha = %.3f", m_pXFoil->algam/PI*180.0);
        m_pleMAlphaSpec->setText(strong);
        strong = QString(tr("Cl = %1")).arg(m_pXFoil->clgam,0,'f',3);
        m_pleMClSpec->setText(strong);
        m_pieIter->setValue(m_pXFoil->niterq);
    }

    if(m_pXFoil->lvisc)
    {
        //a previous xfoil calculation is still active, so add the associated viscous curve
        double x,y;
        double dsp, dqv, sp1, sp2, qv1, qv2;

        m_pQVCurve->clear();

        for(i=2; i<= m_pXFoil->n; i++)
        {
            dsp = m_pXFoil->s[i] - m_pXFoil->s[i-1];
            dqv = m_pXFoil->qcomp(m_pXFoil->qvis[i]) - m_pXFoil->qcomp(m_pXFoil->qvis[i-1]);
            sp1 = (m_pXFoil->s[i-1] + 0.25*dsp)/m_pXFoil->s[m_pXFoil->n];
            sp2 = (m_pXFoil->s[i]   - 0.25*dsp)/m_pXFoil->s[m_pXFoil->n];
            qv1 = m_pXFoil->qcomp(m_pXFoil->qvis[i-1]) + 0.25*dqv;
            qv2 = m_pXFoil->qcomp(m_pXFoil->qvis[i]  ) - 0.25*dqv;
            x = 1.0 - sp1;
            y = qv1/m_pXFoil->qinf;
            m_pQVCurve->appendPoint(x,y);
            x = 1.0 - sp2;
            y = qv2/m_pXFoil->qinf;
            m_pQVCurve->appendPoint(x,y);
        }
        m_pQVCurve->setVisible(true);
    }
    else
    {
        m_pQVCurve->setVisible(false);
    }

    m_bLoaded = true;

    if(pXFile)
    {
        pXFile->close();
        delete pXFile;
    }
}

/**
 * Initializes the widgets with the active data
 * @return
 */
bool XInverse::setParams()
{
    Foil *pFoil=nullptr;
    QString strFoilName;

    if(m_bFullInverse)
    {
        s_pMainFrame->m_prbFullInverse->setChecked(true);
        s_pMainFrame->m_prbMixedInverse->setChecked(false);
        m_pswInv->setCurrentIndex(0);
    }
    else
    {
        s_pMainFrame->m_prbFullInverse->setChecked(false);
        s_pMainFrame->m_prbMixedInverse->setChecked(true);
        m_pswInv->setCurrentIndex(1);
    }

    m_pQCurve->setColor(m_pRefFoil->color());
    m_pQCurve->setStipple(m_pRefFoil->lineStipple());
    m_pQCurve->setWidth(m_pRefFoil->lineWidth());
    m_pMCurve->setColor(m_pModFoil->color());
    m_pMCurve->setStipple(m_pModFoil->lineStipple());
    m_pMCurve->setWidth(m_pModFoil->lineWidth());
    m_pQCurve->setName(tr("Q - Reference"));
    m_pMCurve->setName(tr("Q - Specification"));
    m_pQVCurve->setName(tr("Q - Viscous"));
    m_pQVCurve->setColor(QColor(50,170,0));
    m_pQVCurve->setStipple(0);

    m_pReflectedCurve->setLineStyle(m_ReflectedStyle);
    m_pReflectedCurve->setName(tr("Reflected"));

    m_bTrans   = false;
    m_bSpline  = false;
    m_bSplined = true;
    m_bRefFoil = true;
    m_bModFoil = false;
    m_bGetPos  = false;
    m_bMark    = false;
    m_bMarked  = false;
    m_bSmooth  = false;

    m_QGraph.setDrawRect(m_rGraphRect);
    m_QGraph.initializeGraph();
    m_pQCurve->setVisible(true);
    m_prbSpecAlpha->setChecked(true);

    onSpecal();

    if (XDirect::curFoil() && m_pXFoil->lqspec)
    {
        m_pRefFoil->copyFoil(XDirect::curFoil());
        m_pRefFoil->setColor(m_pQCurve->color().red(), m_pQCurve->color().green(), m_pQCurve->color().blue(), m_pQCurve->color().alpha());
        strFoilName = XDirect::curFoil()->name();
    }
    else
    {
        // XFoil is not initialized
        //is there anything in the database ?
        if(Objects2d::foilCount())
        {
            pFoil = Objects2d::foilAt(0);
            strFoilName = pFoil->name();
            m_pRefFoil->copyFoil(pFoil);
            m_pRefFoil->setColor(m_pQCurve->color().red(), m_pQCurve->color().green(), m_pQCurve->color().blue(), m_pQCurve->color().alpha());
            initXFoil(m_pRefFoil);
        }
        else
        {
            //nothing to initialize
            if(m_bFullInverse)
            {
                m_pdeSpec->setValue(0.0);
                m_pdeTAngle->setValue(0.0);
                m_pdeTGapx->setValue(0.0);
                m_pdeTGapy->setValue(0.0);
            }
            else
            {
                m_pieIter->setValue(m_pXFoil->niterq);
            }

            clear();
            checkActions();
            return false;
        }
    }


    //XFOIL has already been initialized so retrieve the foil
    for (int i=1; i<=m_pXFoil->n; i++)
    {
        m_pRefFoil->m_x[i-1]  = m_pXFoil->x[i];
        m_pRefFoil->m_y[i-1]  = m_pXFoil->y[i];
        m_pRefFoil->m_xb[i-1] = m_pXFoil->x[i];
        m_pRefFoil->m_yb[i-1] = m_pXFoil->y[i];
    }

    m_pRefFoil->m_n          = m_pXFoil->n;
    m_pRefFoil->m_nb         = m_pXFoil->n;
    m_pRefFoil->setName(strFoilName);
    m_pRefFoil->initFoil();
    m_pModFoil->setName(strFoilName + tr(" Modified"));

    setFoil();
    checkActions();
    return true;
}


/**
 * Sets the scale for the Foil and QGraph display
 * @param CltRect the client area
 */
void XInverse::setXInverseScale(QRect CltRect)
{
    m_rCltRect = CltRect;

    int h = CltRect.height();
    int h4 = h/3;
    m_rGraphRect = QRect(0, 0, + m_rCltRect.width(), m_rCltRect.height()-h4);
    m_QGraph.setMargin(50);
    m_QGraph.setDrawRect(m_rGraphRect);

    resetScale();
}



/**
 * Sets the angle at the trailing edge
 * @param a the angle in degrees
 */
void XInverse::setTAngle(double a)
{
    m_pXFoil->agte = a/180.0;
}


/**
 * Sets the trailing edge gap.
 */
void XInverse::setTGap(double tr, double ti)
{
    m_pXFoil->dzte = complex<double>(tr,ti);
}


/**
 * Sets up the interface
 */
void XInverse::setupLayout()
{
    QGroupBox *pSpecBox = new QGroupBox(tr("Specification"));
    {
        QGridLayout *pSpecLayout = new QGridLayout;
        {
            m_prbSpecAlpha = new QRadioButton(tr("Alpha"));
            m_prbSpecCl = new QRadioButton(tr("Cl"));
            m_plabSpecif = new QLabel(tr("Alpha = "));
            m_plabSpecif->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
            m_pdeSpec   = new DoubleEdit(1.23);
            QString strong = tr("Enter a value + \"Enter\" to generate the reference QSpec curve");
            m_pdeSpec->setToolTip(strong);
            pSpecLayout->addWidget(m_prbSpecAlpha,1,1);
            pSpecLayout->addWidget(m_prbSpecCl,1,2);
            pSpecLayout->addWidget(m_plabSpecif,2,1);
            pSpecLayout->addWidget(m_pdeSpec,2,2);
            pSpecBox->setLayout(pSpecLayout);
        }
    }

    QGroupBox *pModBox = new QGroupBox(tr("Modification"));
    {
        QGridLayout *pModLayout = new QGridLayout;
        {
            QString strong;
            m_pchShowSpline    = new QCheckBox(tr("ShowSpline"));
            strong = tr("Toggles the visibility of the spline used to modify the QSpec curve.");
            m_pchShowSpline->setToolTip(strong);
            m_pchTangentSpline = new QCheckBox(tr("Tangent Spline"));
            strong = tr("When checked, forces the spline to be tangent to the QSpec curve at the \n"
                        "spline's endpoints. This is done by constraining the position of the spline's\n"
                        "second and penultimate control points.");
            m_pchTangentSpline->setToolTip(strong);
            m_ppbNewSpline     = new QPushButton(tr("New Spline"));
            strong = tr("Click to initiate the definition of a new spline, then select two points\n"
                        "on the QSpec curve.");
            m_ppbNewSpline->setToolTip(strong);
            m_ppbApplySpline   = new QPushButton(tr("Apply Spline"));
            strong = tr("Click to modify the QSpec curve using the spline geometry");
            m_ppbApplySpline->setToolTip(strong);
            m_ppbResetQSpec    = new QPushButton(tr("Reset QSpec"));
            strong = tr("Resets the QSpec curve to match the base foil's geometry.");
            m_ppbResetQSpec->setToolTip(strong);
            m_ppbPert          = new QPushButton(tr("Pert"));
            strong = "XFoil doc: \n"
                     "The PERT command allows manual input of the complex mapping coefficients\n"
                     "Cn which determine the geometry. These coefficients are normally determined\n"
                     "from Qspec(s) (this is the essence of the inverse method). The PERT command is\n"
                     "provided simply as a means of allowing generation of geometric perturbation modes,\n"
                     "possibly for external optimization or whatever.";
            m_ppbPert->setToolTip(strong);
            m_ppbNewSpline->setCheckable(true);
            pModLayout->addWidget(m_pchShowSpline,1,1);
            pModLayout->addWidget(m_pchTangentSpline,1,2);
            pModLayout->addWidget(m_ppbNewSpline,2,1);
            pModLayout->addWidget(m_ppbApplySpline,2,2);
            pModLayout->addWidget(m_ppbResetQSpec,3,1);
            pModLayout->addWidget(m_ppbPert,3,2);
        }
        pModBox->setLayout(pModLayout);
    }

    QGroupBox *pSmoothBox = new QGroupBox(tr("Smoothing"));
    {
        QGridLayout *pSmoothLayout = new QGridLayout;
        {
            m_ppbSmooth = new QPushButton(tr("Smooth QSpec"));
            QString strong = "XFoil doc: \n"
                             "Qspec can be smoothed with the SMOO command, which normally operates on the entire\n"
                             "distribution, but can be confined to a target segment whose endpoints are selected\n"
                             "with the MARK command.  The smoothing acts to alleviate second derivatives in Qspec(s),\n"
                             "so that with many consecutive SMOO commands Qspec(s) will approach a straight line over\n"
                             "the target segment.  If the slope-matching flag is set, the endpoint slopes are preserved.";
            m_ppbSmooth->setToolTip(strong);
            m_ppbSmooth->setCheckable(true);
            m_ppbFilter = new QPushButton(tr("Hanning Filter"));
            strong= "XFoil doc: \n"
                    "The FILT command is an alternative smoothing procedure which acts on the Fourier \n"
                    "coefficients of Qspec directly, and is global in its effect. It is useful for \n"
                    "\"cleaning up\" the entire Qspec(s) distribution if noise is present from some geometric\n"
                    "glitch on the airfoil  surface. Also, unintended noise might be introduced into Qspec \n"
                    "from a poor modification via the cursor.\n";
            strong += "FILT acts by multiplying the Fourier coefficients by a Hanning window filter function \n"
                      "raised to the power of a filter parameter \"F\".  This tapers off the high frequencies \n"
                      "of Qspec to varying degrees.  A value of F = 0.0 gives no filtering, F = 1.0 gives the \n"
                      "standard Hanning filter, F = 2.0 applies the Hanning filter twice, etc. The standard \n"
                      "Hanning filter appears to be a bit too drastic, so a filter parameter of F = 0.2 is \n"
                      "currently used.  Hence, issuing FILT five times corresponds to the standard Hanning filter.";
            m_ppbFilter->setToolTip(strong);
            QLabel *lab0 = new QLabel(tr("Filter parameter"));
            lab0->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
            m_pdeFilterParam = new DoubleEdit(0.1,3);
            m_pdeFilterParam->setToolTip(strong);
            pSmoothLayout->addWidget(m_ppbSmooth,1,1);
            pSmoothLayout->addWidget(m_ppbFilter,1,2);
            pSmoothLayout->addWidget(lab0,2,1);
            pSmoothLayout->addWidget(m_pdeFilterParam,2,2);
        }
        pSmoothBox->setLayout(pSmoothLayout);
    }

    QGroupBox *pConstraintsBox = new QGroupBox(tr("Constraints"));
    {
        QGridLayout *pTELayout = new QGridLayout;
        {
            QLabel *lab1 = new QLabel(tr("T.E. Angle"));
            QLabel *lab2 = new QLabel(tr("T.E. Gap dx/c"));
            QLabel *lab3 = new QLabel(tr("T.E. Gap dy/c"));
            lab1->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
            lab2->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
            lab3->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
            m_pdeTAngle = new DoubleEdit(0.000, 3);
            m_pdeTGapx  = new DoubleEdit(0.000, 3);
            m_pdeTGapy  = new DoubleEdit(0.000, 3);
            QString strong ="XFoil doc: \n"
                            "The trailing edge gap is initialized from the initial airfoil and can \n"
                            "be changed with TGAP.  To reduce the \"corrupting\" effect of the \n"
                            "constraint-driven corrections, a good rule of thumb is that the \n"
                            "Qspec distribution should be modified so as to preserve the total CL.\n"
                            "The CL is simply twice the area under the Qspec(s) curve (=  2 x circulation),\n"
                            "so that this area should be preserved.";
            m_pdeTGapx->setToolTip(strong);
            m_pdeTGapy->setToolTip(strong);


            pTELayout->addWidget(lab1,1,1);
            pTELayout->addWidget(lab2,2,1);
            pTELayout->addWidget(lab3,3,1);
            pTELayout->addWidget(m_pdeTAngle,1, 2);
            pTELayout->addWidget(m_pdeTGapx, 2, 2);
            pTELayout->addWidget(m_pdeTGapy, 3, 2);
            m_pchSymm = new QCheckBox(tr("Symmetric foil"));
            strong ="XFoil doc: \n"
                    "The symmetry-forcing option (SYMM toggle) is useful when a symmetric\n"
                    "airfoil is being designed.  If active, this option zeroes out all\n"
                    "antisymmetric (camber) Qspec changes, and doubles all symmetric \n"
                    "(thickness) changes.  This unfortunately has the annoying side \n"
                    "effect of also doubling the numerical roundoff noise in Qspec \n"
                    "every time a MODI operation is performed.  This noise sooner or later\n"
                    "becomes visible as high-frequency wiggles which double with each \n"
                    "MODI command.  Issuing FILT occasionally keeps this parasitic \n"
                    "noise growth under control.";
            m_pchSymm->setToolTip(strong);
        }
        QVBoxLayout *pConstraintsLayout = new QVBoxLayout;
        {
            pConstraintsLayout->addLayout(pTELayout);
            pConstraintsLayout->addWidget(m_pchSymm);
        }
        pConstraintsBox->setLayout(pConstraintsLayout);
    }

    m_pwtFInv = new QWidget(this);
    {
        QVBoxLayout *pFInvLayout = new QVBoxLayout;
        {
            m_ppbExec = new QPushButton(tr("Execute"));
            QString strong ="XFoil doc: \n"
                            "The EXEC command generates a new buffer airfoil corresponding to the\n"
                            "current Qspec distribution.  If subsequent operations on this airfoil\n"
                            "are to be performed, it is necessary to first generate a current\n"
                            "airfoil from this buffer airfoil using PANE (refine globally) at the \n"
                            "top level menu.  This seemingly complicated sequence is necessary \n"
                            "because the airfoil points generated by EXEC are uniformly spaced\n"
                            "in the circle plane, which gives a rather poor point (panel node)\n"
                            "spacing distribution on the physical airfoil.";
            m_ppbExec->setToolTip(strong);
            pFInvLayout->addWidget(pSpecBox);
            pFInvLayout->addWidget(pModBox);
            pFInvLayout->addWidget(pSmoothBox);
            pFInvLayout->addWidget(pConstraintsBox);
            pFInvLayout->addWidget(m_ppbExec);
            //            pFInvLayout->addStretch(1);
        }
        m_pwtFInv->setLayout(pFInvLayout);
    }

    //specific MInv Controls
    QHBoxLayout *pMSpecLayout = new QHBoxLayout;
    {
        m_pleMAlphaSpec     = new QLineEdit(tr("Alpha = "));
        m_pleMClSpec        = new QLineEdit(tr("Cl ="));
        m_pleMAlphaSpec->setEnabled(false);
        m_pleMClSpec->setEnabled(false);
        pMSpecLayout->addWidget(m_pleMAlphaSpec);
        pMSpecLayout->addWidget(m_pleMClSpec);
    }

    QGroupBox *pMSplinesBox = new QGroupBox(tr("Modification"));
    {
        QGridLayout *pMSplineslayout = new QGridLayout;
        {
            QString strong;

            m_ppbMark           = new QPushButton(tr("Mark for modification"));
            strong ="Click to initiate the selection of the portion of the QSpec curve to be used \n"
                    "for the modification of the geometry, then select two points on the QSpec curve.";
            m_ppbMark->setToolTip(strong);
            m_pchMShowSpline    = new QCheckBox(tr("ShowSpline"));
            strong = tr("Toggles the visibility of the spline used to modify the QSpec curve.");
            m_pchMShowSpline->setToolTip(strong);
            m_pchMTangentSpline = new QCheckBox(tr("Tangent Spline"));
            strong = tr("When checked, forces the spline to be tangent to the QSpec curve at the \n"
                        "spline's endpoints. This is done by constraining the position of the spline's\n"
                        "second and penultimate control points.");
            m_pchMTangentSpline->setToolTip(strong);
            m_ppbMNewSpline     = new QPushButton(tr("New Spline"));
            strong = tr("Click to initiate the definition of a new spline, then select two points\n"
                        "on the QSpec curve.");
            m_ppbMNewSpline->setToolTip(strong);
            m_ppbMApplySpline   = new QPushButton(tr("Apply Spline"));
            strong = tr("Click to modify the QSpec curve using the spline geometry");
            m_ppbMApplySpline->setToolTip(strong);
            m_ppbMSmooth        = new QPushButton(tr("Smooth"));
            strong ="XFoil doc: \n"
                    "Qspec can be smoothed with the SMOO command, which normally operates \n"
                    "on the entire distribution, but can be confined to a target segment whose\n"
                    "endpoints are selectedwith the MARK command.  The smoothing acts to alleviate\n"
                    "second derivatives in Qspec(s), so that with many consecutive SMOO commands\n"
                    "Qspec(s) will approach a straight line over the target segment.  If the\n"
                    "slope-matching flag is set, the endpoint slopes are preserved.";
            m_ppbMSmooth->setToolTip(strong);
            m_ppbMSmooth->setCheckable(true);
            m_ppbMResetQSpec    = new QPushButton(tr("Reset QSpec"));
            strong = tr("Resets the QSpec curve to match the base foil's geometry.");
            m_ppbMResetQSpec->setToolTip(strong);
            m_ppbMNewSpline->setCheckable(true);
            m_ppbMark->setCheckable(true);
            pMSplineslayout->addWidget(m_pchMShowSpline,1,1);
            pMSplineslayout->addWidget(m_pchMTangentSpline,1,2);
            pMSplineslayout->addWidget(m_ppbMNewSpline,2,1);
            pMSplineslayout->addWidget(m_ppbMApplySpline,2,2);
            pMSplineslayout->addWidget(m_ppbMark,3,1,1,2);
            pMSplineslayout->addWidget(m_ppbMSmooth,4,1);
            pMSplineslayout->addWidget(m_ppbMResetQSpec,4,2);
        }
        pMSplinesBox->setLayout(pMSplineslayout);
    }

    QGroupBox *pFoilBox = new QGroupBox(tr("Foil"));
    {
        QVBoxLayout *pFoilLayout = new QVBoxLayout;
        {
            m_pchCpxx           = new QCheckBox(tr("End Point Constraint"));
            QString strong = "XFoil doc: \n"
                             "If extra smoothness in the surface speed is required, the CPXX command\n"
                             "just before EXEC will enable the addition of two additional modes which\n"
                             "allow the second derivative in the pressure at the endpoints to be\n"
                             "unchanged from the starting airfoil.  The disadvantage of this option is\n"
                             "that the resulting surface speed Q will now deviate more from the\n"
                             "specified speed Qspec.  It is allowable to repeatedly modify Qspec,\n"
                             "set or reset the CPXX option, and issue the EXEC command in any order.";
            m_pchCpxx->setToolTip(strong);
            m_ppbMExec          = new QPushButton(tr("Execute"));
            strong ="XFoil doc: \n"
                    "The EXEC command generates a new buffer airfoil corresponding to the\n"
                    "current Qspec distribution.  If subsequent operations on this airfoil\n"
                    "are to be performed, it is necessary to first generate a current\n"
                    "airfoil from this buffer airfoil using PANE (refine globally) at the \n"
                    "top level menu.  This seemingly complicated sequence is necessary \n"
                    "because the airfoil points generated by EXEC are uniformly spaced\n"
                    "in the circle plane, which gives a rather poor point (panel node)\n"
                    "spacing distribution on the physical airfoil.";
            m_ppbMExec->setToolTip(strong);
            pFoilLayout->addWidget(m_pchCpxx);
            pFoilLayout->addWidget(m_ppbMExec);
            QHBoxLayout *pMaxIterLayout = new QHBoxLayout;
            {
                pMaxIterLayout->addStretch();
                QLabel *lab10 = new QLabel(tr("Max Iterations"));
                m_pieIter = new IntEdit(this);
                QString strong = "XFoil doc: \n"
                                 "EXEC requests the number of Newton iterations to be performed in the inverse calculation.\n"
                                 "Although as many as six iterations may be required for convergence to machine zero,\n"
                                 "it is _not_ necessary to fully converge a Mixed-Inverse case.  Two iterations are usually\n"
                                 "sufficient to get very close to the new geometry.";
                m_pieIter->setToolTip(strong);
                pMaxIterLayout->addWidget(lab10);
                pMaxIterLayout->addWidget(m_pieIter);
            }
            pFoilLayout->addLayout(pMaxIterLayout);
            //            pFoilLayout->addStretch();
        }
        pFoilBox->setLayout(pFoilLayout);
    }

    m_pwtMInv = new QWidget(this);
    {
        QVBoxLayout *pMInvLayout = new QVBoxLayout;
        {
            pMInvLayout->addLayout(pMSpecLayout);
            pMInvLayout->addWidget(pMSplinesBox);
            pMInvLayout->addWidget(pFoilBox);
            pMInvLayout->addStretch();
        }
        m_pwtMInv->setLayout(pMInvLayout);
    }

    m_pswInv = new QStackedWidget;
    {
        m_pswInv->addWidget(m_pwtFInv);
        m_pswInv->addWidget(m_pwtMInv);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pswInv);

        m_pmteOutput = new MinTextEdit(this);
        m_pmteOutput->setAcceptRichText(true);
        pMainLayout->addWidget(m_pmteOutput);
    }
    setLayout(pMainLayout);

    connectSignals();
}



/**
 * Performs a smoothing operation of the specification cuve between two end points
 * @param Pos1 the first end point
 * @param Pos2 the seconf end point
 */
void XInverse::smooth(int Pos1, int Pos2)
{
    if(Pos1 ==-1)
    {
        //smooth it all
        m_pmteOutput->append(tr("Smoothing the entire distribution.\n"));
        Pos1 = 1;
        Pos2 = m_pXFoil->nsp;
    }
    else
        m_pmteOutput->append(tr("Smoothing the selected portion.\n"));

    m_bGetPos = false;
    if (abs(Pos2-Pos1)<=2) return;
    if (Pos1>Pos2)
    {
        int tmp = Pos2;
        Pos2  = Pos1;
        Pos1  = tmp;
    }
    if(m_bFullInverse)
    {
        m_pXFoil->smooq(Pos1,Pos2,1);
        m_pXFoil->cncalc(m_pXFoil->qspec[1], false);
        m_pXFoil->qspcir();
        m_pXFoil->lqspec = true;
    }
    else
    {
        // added v1.17, i.e. different sequence as for Full Inverse
        m_pXFoil->smooq(Pos1,Pos2,1);
        m_pXFoil->splqsp(1);
        m_pXFoil->clcalc(m_pXFoil->xcmref,m_pXFoil->ycmref);
        //        pXFoil->lqspec = true; ?? should we
    }
    createMCurve();
    m_bSmooth = false;
    cancelSmooth();
    updateView();
}


/**
 * Refreshes the display
 */
void XInverse::updateView()
{
    if(s_p2dWidget)
    {
        s_p2dWidget->update();
    }
}


/**
 * Overrides the QWidget's wheelEvent method.
 * Dispatches the event
 * @param event the QMouseEvent
 */
void XInverse::zoomEvent(QPointF pos, double zoomFactor)
{
    releaseZoom();

    if(m_QGraph.isInDrawRect(pos))
    {
        if (m_bXPressed)
        {
            //zoom x scale
            m_QGraph.setAutoX(false);
            m_QGraph.scaleXAxis(1.0/zoomFactor);
        }
        else if(m_bYPressed)
        {
            //zoom y scale
            m_QGraph.setAutoY(false);
            m_QGraph.scaleYAxis(1.0/zoomFactor);
        }
        else
        {
            //zoom both
            m_QGraph.setAuto(false);
            m_QGraph.scaleAxes(1.0/zoomFactor);
        }
        m_QGraph.setAutoXUnit();
        m_QGraph.setAutoYUnit();
    }
    else
    {
        double scale = m_fScale;

        if(m_bYPressed)
        {
            m_fYScale *= zoomFactor;
        }
        else
        {
            m_fScale *= zoomFactor;
            int a = int((m_rCltRect.right() + m_rCltRect.left())/2);
            m_ptOffset.rx() = a + int((m_ptOffset.x()-a)*m_fScale/scale);
        }
    }
    updateView();
}


