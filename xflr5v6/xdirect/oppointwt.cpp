/****************************************************************************

    OpPointWidget Class
    Copyright (C) 2016-2016 André Deperrois 

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

#include "oppointwt.h"



#include <QAction>


#include <globals/mainframe.h>

#include <xdirect/xdirect.h>
#include <xdirect/xdirectstyledlg.h>
#include <xflcore/constants.h>
#include <xflcore/displayoptions.h>
#include <xflcore/xflcore.h>
#include <xflgraph/controls/graphdlg.h>
#include <xflgraph/graph.h>
#include <xflobjects/objects_global.h>
#include <xflobjects/objects2d/objects2d.h>

MainFrame *OpPointWt::s_pMainFrame(nullptr);
XDirect *OpPointWt::s_pXDirect(nullptr);
/**
*The public constructor
*/
OpPointWt::OpPointWt(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);

    QSizePolicy sizepol;
    sizepol.setHorizontalPolicy(QSizePolicy::Expanding);
    sizepol.setVerticalPolicy(QSizePolicy::Expanding);
    setSizePolicy(sizepol);

    m_bTransFoil   = false;
    m_bTransGraph  = false;
    m_bAnimate     = false;
    m_bBL          = false;
    m_bPressure    = false;
    m_bNeutralLine = true;
//    m_bShowPanels  = false;
    m_bXPressed = m_bYPressed = false;

    m_NeutralStyle  = {true, Line::DASHDOT, 1, QColor(155, 155,155), Line::NOSYMBOL};
    m_BLStyle       = {true, Line::DASH,    2, QColor(255, 55,  55), Line::NOSYMBOL};
    m_PressureStyle = {true, Line::SOLID,   2, QColor( 95, 155, 95), Line::NOSYMBOL};

    m_fScale = m_fYScale = 1.0;
    m_pCpGraph = nullptr;
}


/**
*Overrides the keyPressEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void OpPointWt::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_V:
        {
            if(m_pCpGraph->isInDrawRect(m_LastPoint))
            {
                GraphDlg::setActivePage(0);
                onGraphSettings();
            }
            pEvent->accept();
            break;
        }
        case Qt::Key_G:
        {
            if(m_pCpGraph->isInDrawRect(m_LastPoint))
            {
                onGraphSettings();
            }
            pEvent->accept();
            break;
        }
        case Qt::Key_R:
            if(m_pCpGraph->isInDrawRect(m_LastPoint))
                m_pCpGraph->setAuto(true);
            else setFoilScale();
            update();
            break;

        case Qt::Key_X:
            m_bXPressed = true;
            break;

        case Qt::Key_Y:
            m_bYPressed = true;
            break;

        default:pEvent->ignore();

    }
}


/**
*Overrides the keyReleaseEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void OpPointWt::keyReleaseEvent(QKeyEvent *pEvent)
{
    Q_UNUSED(pEvent);
    m_bXPressed = m_bYPressed = false;
}



/**
*Overrides the mousePressEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void OpPointWt::mousePressEvent(QMouseEvent *pEvent)
{
    QPoint pt(pEvent->x(), pEvent->y()); //client coordinates

    if(pEvent->buttons() & Qt::LeftButton)
    {
        if (m_pCpGraph->isInDrawRect(pEvent->pos()))
        {
            m_bTransGraph = true;
        }
        else
        {
            m_bTransFoil = true;
        }
        m_LastPoint.setX(pt.x());
        m_LastPoint.setY(pt.y());
        setCursor(Qt::ClosedHandCursor);
//        if(!m_bAnimate) update();
    }

    pEvent->accept();
}


/**
*Overrides the mouseReleaseEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void OpPointWt::mouseReleaseEvent(QMouseEvent *pEvent)
{
    m_bTransGraph = false;
    m_bTransFoil  = false;
    setCursor(Qt::CrossCursor);
    pEvent->accept();
}


/**
*Overrides the mouseMoveEvent method of the base class.
*/
void OpPointWt::mouseMoveEvent(QMouseEvent *pEvent)
{
    QPoint pt;
    double scale;
    double a;

    pt.setX(pEvent->x());
    pt.setY(pEvent->y()); //client coordinates
    setFocus();

    if (pEvent->buttons() & Qt::LeftButton)
    {
        if(m_bTransGraph)
        {
            QPoint point;
            double xu, yu, x1, y1, xmin, xmax, ymin, ymax;

            point = pEvent->pos();

            // we translate the curves inside the graph
            m_pCpGraph->setAuto(false);
            x1 =  m_pCpGraph->clientTox(m_LastPoint.x()) ;
            y1 =  m_pCpGraph->clientToy(m_LastPoint.y()) ;

            xu = m_pCpGraph->clientTox(point.x());
            yu = m_pCpGraph->clientToy(point.y());

            xmin = m_pCpGraph->xMin() - xu+x1;
            xmax = m_pCpGraph->xMax() - xu+x1;
            ymin = m_pCpGraph->yMin() - yu+y1;
            ymax = m_pCpGraph->yMax() - yu+y1;

            m_pCpGraph->setWindow(xmin, xmax, ymin, ymax);
        }
        else if(m_bTransFoil)
        {
            // we translate the airfoil
            m_FoilOffset.rx() += pt.x()-m_LastPoint.x();
            m_FoilOffset.ry() += pt.y()-m_LastPoint.y();
        }
    }
    else if (Objects2d::curFoil() && ((pEvent->buttons() & Qt::MidButton) || pEvent->modifiers().testFlag(Qt::AltModifier)))
    {
        // we zoom the graph or the foil
        if(Objects2d::curFoil())
        {
            //zoom the foil
            scale = m_fScale;

            if(pt.y()-m_LastPoint.y()<0) m_fScale /= 1.02;
            else                         m_fScale *= 1.02;

            a = rect().center().x();

            m_FoilOffset.rx() = a + (m_FoilOffset.x()-a)/scale*m_fScale;
        }
    }

    m_LastPoint = pt;
    if(!m_bAnimate) update();

    pEvent->accept();

}


void OpPointWt::mouseDoubleClickEvent (QMouseEvent *pEvent)
{
    Q_UNUSED(pEvent);
    setCursor(Qt::CrossCursor);
    if (m_pCpGraph->isInDrawRect(pEvent->pos()))
    {
        onGraphSettings();
        update();
    }
}


/**
 * The user has requested an edition of the settings of the active graph
 */
void OpPointWt::onGraphSettings()
{
    GraphDlg grDlg(this);
    grDlg.setGraph(m_pCpGraph);

//    QAction *action = qobject_cast<QAction *>(sender());
//    grDlg.setActivePage(0);

    if(grDlg.exec() == QDialog::Accepted)
    {
        emit graphChanged(m_pCpGraph);
    }
    update();
}


/**
*Overrides the resizeEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void OpPointWt::resizeEvent(QResizeEvent *pEvent)
{
    Q_UNUSED(pEvent);
    if(m_pCpGraph)
    {
        int h = rect().height();
        int h4 = h/3;
        QRect rGraphRect = QRect(0, 0, + rect().width(), rect().height()-h4);
        m_pCpGraph->setMargin(50);
        m_pCpGraph->setDrawRect(rGraphRect);
        m_pCpGraph->initializeGraph();
    }
    setFoilScale();
}

/**
 * Sets the Foil scale in the OpPoint view.
 */
void OpPointWt::resetGraphScale()
{
    if(m_pCpGraph)
    {
        m_pCpGraph->setAuto(true);
        update();
    }
}

/**
 * Sets the Foil scale in the OpPoint view.
 */
void OpPointWt::setFoilScale()
{
    int iMargin = 53;
    if(m_pCpGraph)
    {
        iMargin = m_pCpGraph->margin();
        int h =  m_pCpGraph->clientRect()->height();
        m_FoilOffset.rx() = rect().left() + iMargin;
        m_FoilOffset.ry() = (rect().height()+h)/2;
//        m_fScale = rect().width()-2.0*iMargin;
//        if(m_pCpGraph && m_pCpGraph->yVariable()<2)
        {
            double p0  = m_pCpGraph->xToClient(0.0);
            double p1  = m_pCpGraph->xToClient(1.0);
            m_fScale =  (p1-p0);
        }
    }
    else
    {
        m_FoilOffset.rx() = rect().left() + iMargin;
        m_FoilOffset.ry() = rect().center().y();

        m_fScale = rect().width()-2.0*iMargin;
    }
    m_fYScale = 1.0;
}


/**
*Overrides the wheelEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void OpPointWt::wheelEvent(QWheelEvent *pEvent)
{
    QPoint pt;
#if QT_VERSION >= 0x050F00
    pt = pEvent->position().toPoint();
#else
    pt = pEvent->pos();
#endif

    if(m_pCpGraph && m_pCpGraph->isInDrawRect(pt))
    {
        double zoomFactor=1.0;

        if(pEvent->angleDelta().y()>0)
        {
            if(!DisplayOptions::bReverseZoom()) zoomFactor = 1./1.06;
            else                                zoomFactor = 1.06;
        }
        else
        {
            if(!DisplayOptions::bReverseZoom()) zoomFactor = 1.06;
            else                                zoomFactor = 1./1.06;
        }

        if (m_bXPressed)
        {
            //zoom x scale
            m_pCpGraph->setAutoX(false);
            m_pCpGraph->scaleXAxis(1./zoomFactor);
        }
        else if(m_bYPressed)
        {
            //zoom y scale
            m_pCpGraph->setAutoY(false);
            m_pCpGraph->scaleYAxis(1./zoomFactor);
        }
        else
        {
            //zoom both
            m_pCpGraph->setAuto(false);
            m_pCpGraph->scaleAxes(1./zoomFactor);
        }

        m_pCpGraph->setAutoXUnit();
        m_pCpGraph->setAutoYUnit();
        update();
    }
    else if(Objects2d::curFoil())
    {
        double zoomFactor=1.0;

        if(pEvent->angleDelta().y()>0)
        {
            if(!DisplayOptions::bReverseZoom()) zoomFactor = 1./1.06;
            else                                zoomFactor = 1.06;
        }
        else
        {
            if(!DisplayOptions::bReverseZoom()) zoomFactor = 1.06;
            else                                zoomFactor = 1./1.06;
        }

        double scale = m_fScale;


        if(m_bYPressed)
        {
            m_fYScale *= zoomFactor;
        }
        else
        {
            m_fScale *= zoomFactor;
        }

        int a = int((rect().right()+rect().left())/2);

        m_FoilOffset.rx() = a + int((m_FoilOffset.x()-a)/scale*m_fScale);

//        if(!m_bAnimate)
            update();
    }
}


/**
*Overrides the paintEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void OpPointWt::paintEvent(QPaintEvent *pEvent)
{
    QPainter painter(this);
    painter.save();

    painter.fillRect(rect(), DisplayOptions::backgroundColor());
    paintGraph(painter);
    paintOpPoint(painter);

    painter.restore();

    pEvent->accept();
}


/**
 * Draws the graph
 * @param painter a reference to the QPainter object with which to draw
 */
void OpPointWt::paintGraph(QPainter &painter)
{
    if(!m_pCpGraph) return;

    painter.save();

    QFontMetrics fm(DisplayOptions::textFont());
    int fmheight = fm.height();
    int fmWidth  = fm.averageCharWidth();
//  draw  the graph
    if(m_pCpGraph->clientRect()->width()>200 && m_pCpGraph->clientRect()->height()>150)
    {
        m_pCpGraph->drawGraph(painter);
        QPoint Place(m_pCpGraph->clientRect()->right()-73*fmWidth, m_pCpGraph->clientRect()->top()+fmheight);
        m_pCpGraph->drawLegend(painter, Place, DisplayOptions::textFont(), DisplayOptions::textColor(), DisplayOptions::backgroundColor());
    }


    if(m_pCpGraph->isInDrawRect(m_LastPoint) && DisplayOptions::bMousePos())
    {
        QPen textPen(DisplayOptions::textColor());

        painter.setPen(textPen);

        painter.drawText(m_pCpGraph->clientRect()->width()-14*fm.averageCharWidth(), m_pCpGraph->clientRect()->top()+  fmheight, QString("x = %1").arg(m_pCpGraph->clientTox(m_LastPoint.x()),9,'f',5));
        painter.drawText(m_pCpGraph->clientRect()->width()-14*fm.averageCharWidth(), m_pCpGraph->clientRect()->top()+2*fmheight, QString("y = %1").arg(m_pCpGraph->clientToy(m_LastPoint.y()),9,'f',5));
    }
    painter.restore();
}


/**
 * Draws the Cp Graph and the foil
 * @param painter a reference to the QPainter object with which to draw
 */
void OpPointWt::paintOpPoint(QPainter &painter)
{
    Foil *pCurFoil = Objects2d::curFoil();

    if (!pCurFoil || !pCurFoil->name().length())
        return;

    QString Result, str, str1;

    painter.save();

    if(m_bNeutralLine)
    {
        QPen NeutralPen(m_NeutralStyle.m_Color);
        NeutralPen.setStyle(xfl::getStyle(m_NeutralStyle.m_Stipple));
        NeutralPen.setWidth(m_NeutralStyle.m_Width);
        painter.setPen(NeutralPen);
        painter.drawLine(rect().left(),  int(m_FoilOffset.y()),
                         rect().right(), int(m_FoilOffset.y()));
    }
    if(!m_pCpGraph->isInDrawRect(m_LastPoint) && DisplayOptions::bMousePos())
    {
        QPen textPen(DisplayOptions::textColor());
        QFontMetrics fm(DisplayOptions::textFont());
        int fmheight  = fm.height();
        painter.setPen(textPen);

        Vector3d real = mousetoReal(m_LastPoint);
        painter.drawText(m_pCpGraph->clientRect()->width()-14*fm.averageCharWidth(),
                         m_pCpGraph->clientRect()->height() + fmheight, QString("x = %1")
                         .arg(real.x,9,'f',5));
        painter.drawText(m_pCpGraph->clientRect()->width()-14*fm.averageCharWidth(),
                         m_pCpGraph->clientRect()->height() + 2*fmheight, QString("y = %1")
                         .arg(real.y,9,'f',5));
    }

    double Alpha = 0.0;
    if(Objects2d::curOpp()) Alpha = Objects2d::curOpp()->aoa();

    xfl::drawFoil(painter, pCurFoil, -Alpha, m_fScale, m_fScale*m_fYScale, m_FoilOffset);
    if(pCurFoil->pointStyle()>0)
        xfl::drawFoilPoints(painter, pCurFoil, -Alpha, m_fScale,m_fScale*m_fYScale, m_FoilOffset, DisplayOptions::backgroundColor());

    if(m_bPressure && Objects2d::curOpp()) paintPressure(painter, m_fScale, m_fScale*m_fYScale);
    if(m_bBL && Objects2d::curOpp())       paintBL(painter, Objects2d::curOpp(), m_fScale, m_fScale*m_fYScale);


    // Write Titles and results
    QString strong;

    painter.setFont(DisplayOptions::textFont());
    int D = 0;
    int ZPos(0), XPos(0);
    QPen WritePen(DisplayOptions::textColor());
    painter.setPen(WritePen);

    QFontMetrics fm(DisplayOptions::textFont());
    int dD = fm.height();

    //write the foil's properties

    int Back = 5;

    if(pCurFoil->m_bTEFlap) Back +=3;

    int LeftPos = rect().left()+10;
    ZPos = rect().bottom() - 10 - Back*dD;

    D = 0;
    str1 = pCurFoil->name();
    painter.drawText(LeftPos,ZPos+D, str1+str);
    D += dD;

    str = "%";
    str1 = QString(tr("Thickness         = %1")).arg(pCurFoil->thickness()*100.0, 6, 'f', 2);
    painter.drawText(LeftPos,ZPos+D, str1+str);
    D += dD;

    str1 = QString(tr("Max. Thick.pos.   = %1")).arg(pCurFoil->xThickness()*100.0, 6, 'f', 2);
    painter.drawText(LeftPos,ZPos+D, str1+str);
    D += dD;

    str1 = QString(tr("Max. Camber       = %1")).arg( pCurFoil->camber()*100.0, 6, 'f', 2);
    painter.drawText(LeftPos,ZPos+D, str1+str);
    D += dD;

    str1 = QString(tr("Max. Camber pos.  = %1")).arg(pCurFoil->xCamber()*100.0, 6, 'f', 2);
    painter.drawText(LeftPos,ZPos+D, str1+str);
    D += dD;

    str1 = QString(tr("Number of Panels  =  %1")).arg( pCurFoil->m_n);
    painter.drawText(LeftPos,ZPos+D, str1);
    D += dD;

    if(pCurFoil->m_bTEFlap)
    {
        str1 = QString(tr("Flap Angle = %1")+QChar(0260)).arg( pCurFoil->m_TEFlapAngle, 7, 'f', 2);
        painter.drawText(LeftPos,ZPos+D, str1);
        D += dD;

        str1 = QString(tr("XHinge     = %1")).arg( pCurFoil->m_TEXHinge, 6, 'f', 1);
        strong="%";
        painter.drawText(LeftPos,ZPos+D, str1+strong);
        D += dD;

        str1 = QString(tr("YHinge     = %1")).arg( pCurFoil->m_TEYHinge, 6, 'f', 1);
        strong="%";
        painter.drawText(LeftPos,ZPos+D, str1+strong);
    }

    Back = 6;
    Polar   *pPolar   = Objects2d::curPolar();
    OpPoint *pOpPoint = Objects2d::curOpp();
    if(pOpPoint)
    {
        Back = 12;
        if(pOpPoint->m_bTEFlap) Back++;
        if(pOpPoint->m_bLEFlap) Back++;
        if(pOpPoint->m_bViscResults && qAbs(pOpPoint->Cd)>0.0) Back++;
        if(pPolar->isFixedLiftPolar()) Back++;
        if(!pPolar->isFixedSpeedPolar() && !pPolar->isFixedaoaPolar()) Back++;
    }

    int dwidth = fm.horizontalAdvance(tr("TE Hinge Moment/span = 0123456789"));

    ZPos = rect().bottom()-Back*dD - 10;
    XPos = rect().right()-dwidth-20;
    D=0;


    if(pPolar)
    {
        if     (pPolar->isFixedSpeedPolar())  str1 = tr("Fixed speed polar");
        else if(pPolar->isFixedLiftPolar())   str1 = tr("Fixed lift polar");
        else if(pPolar->isRubberChordPolar()) str1 = tr("Rubber chord polar");
        else if(pPolar->isFixedaoaPolar())    str1 = tr("Fixed a.o.a. polar");

        painter.drawText(XPos,ZPos, dwidth, dD, Qt::AlignRight | Qt::AlignTop, str1);
        D += dD;
        if(pPolar->isFixedSpeedPolar())
        {
            xfl::ReynoldsFormat(strong, pPolar->Reynolds());
            strong ="Reynolds = " + strong;
            painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
            D += dD;
            strong = QString("Mach = %1").arg( pPolar->Mach(),9,'f',3);
            painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
            D += dD;
        }
        else if(pPolar->isFixedLiftPolar())
        {
            xfl::ReynoldsFormat(strong, pPolar->Reynolds());
            strong = tr("Re.sqrt(Cl) = ") + strong;
            painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
            D += dD;

            strong = QString(tr("M.sqrt(Cl) = %1")).arg(pPolar->Mach(),9,'f',3);
            painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
            D += dD;
        }
        else if(pPolar->isRubberChordPolar())
        {
            xfl::ReynoldsFormat(strong, pPolar->Reynolds());
            strong = tr("Re.sqrt(Cl) = ") + strong;
            painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
            D += dD;

            strong = QString("Mach = %1").arg(pPolar->Mach(),9,'f',3);
            painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
            D += dD;
        }
        else if(pPolar->isFixedaoaPolar())
        {
            strong = QString("Alpha = %1 ").arg(pPolar->aoa(),10,'f',2)+QChar(0260);
            painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
            D += dD;
            strong = QString("Mach = %1").arg(pPolar->Mach(),9,'f',3);
            painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
            D += dD;
        }

        strong = QString("NCrit = %1").arg(pPolar->NCrit(),9,'f',3);
        painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
        D += dD;

        strong = QString(tr("Forced Upper Trans. = %1")).arg(pPolar->XtrTop(),9,'f',3);
        painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
        D += dD;
        strong = QString(tr("Forced Lower Trans. = %1")).arg(pPolar->XtrBot(), 9, 'f', 3);
        painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
        D += dD;

        if(pOpPoint)
        {
            if(!pPolar->isFixedSpeedPolar())
            {
                xfl::ReynoldsFormat(Result, pOpPoint->Reynolds());
                Result = "Re = "+ Result;
                painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
                D += dD;
            }
            if(pPolar->isFixedLiftPolar())
            {
                Result = QString("Ma = %1").arg(pOpPoint->m_Mach, 9, 'f', 4);
                painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
                D += dD;
            }
            if(!pPolar->isFixedaoaPolar())
            {
                Result = QString(tr("Alpha = %1")+QChar(0260)).arg(pOpPoint->m_Alpha, 8, 'f', 2);
                painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
                D += dD;
            }
            Result = QString(tr("Cl = %1")).arg(pOpPoint->Cl, 9, 'f', 3);
            painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
            D += dD;

            Result = QString(tr("Cm = %1")).arg(pOpPoint->Cm, 9, 'f', 3);
            painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
            D += dD;

            Result = QString(tr("Cd = %1")).arg(pOpPoint->Cd, 9, 'f', 3);
            painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
            D += dD;

            if(pOpPoint->m_bViscResults && qAbs(pOpPoint->Cd)>0.0)
            {
                Result = QString(tr("L/D = %1")).arg(pOpPoint->Cl/pOpPoint->Cd, 9, 'f', 3);
                painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
                D += dD;
            }

            Result = QString(tr("Upper Trans. = %1")).arg(pOpPoint->Xtr1, 9, 'f', 3);
            painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
            D += dD;

            Result = QString(tr("Lower Trans. = %1")).arg(pOpPoint->Xtr2, 9, 'f', 3);
            painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
            D += dD;

            if(pOpPoint->m_bTEFlap)
            {
                Result = QString(tr("TE Hinge Moment/span = %1")).arg(pOpPoint->m_TEHMom, 9, 'e', 2);
                painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
                D += dD;
            }

            if(pOpPoint->m_bLEFlap)
            {
                Result = QString(tr("LE Hinge Moment/span = %1")).arg(pOpPoint->m_LEHMom, 9, 'e', 2);
                painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
            }
        }
    }

    painter.restore();
}


/**
 * The method which draws the pressure arrows in the OpPoint view.
 * @param painter a reference to the QPainter object with which to draw
 * @param pOpPoint the OpPoint object to draw
 * @param scale the scale of the view
 */
void OpPointWt::paintPressure(QPainter &painter, double scalex, double scaley)
{
    if(!Objects2d::curFoil()) return;
    if(!Objects2d::curOpp()) return;
    if(!Objects2d::curOpp()->bViscResults()) return;

    double alpha = -Objects2d::curOpp()->m_Alpha*PI/180.0;
    double cosa = cos(alpha);
    double sina = sin(alpha);
    double x, y ,xs, ys, xe, ye, dx, dy, x1, x2, y1, y2, r2;
    double cp;
    QPointF offset = m_FoilOffset;

    painter.save();

    QPen CpvPen(m_PressureStyle.m_Color);
    CpvPen.setStyle(xfl::getStyle(m_PressureStyle.m_Stipple));
    CpvPen.setWidth(m_PressureStyle.m_Width);
    painter.setPen(CpvPen);


    for(int i=0; i<Objects2d::curFoil()->m_n; i++)
    {
        if(Objects2d::curOpp()->m_bViscResults) cp = Objects2d::curOpp()->Cpv[i];
        else                                  cp = Objects2d::curOpp()->Cpi[i];
        x = Objects2d::curFoil()->m_x[i];
        y = Objects2d::curFoil()->m_y[i];

        xs = (x-0.5)*cosa - y*sina + 0.5;
        ys = (x-0.5)*sina + y*cosa;

        if(cp>0)
        {
            x += Objects2d::curFoil()->m_nx[i] * cp * 0.05;
            y += Objects2d::curFoil()->m_ny[i] * cp * 0.05;

            xe = (x-0.5)*cosa - y*sina + 0.5;
            ye = (x-0.5)*sina + y*cosa;
            painter.drawLine(int(xs*scalex + offset.x()), int(-ys*scaley + offset.y()),
                             int(xe*scalex + offset.x()), int(-ye*scaley + offset.y()));

            dx = xe - xs;
            dy = ye - ys;
            r2 = sqrt(dx*dx + dy*dy);
            if(r2!=0.0) //you can never be sure...
            {
                dx = dx/r2;
                dy = dy/r2;
            }

            x1 = xs + 0.0085*dx + 0.005*dy;
            y1 = ys + 0.0085*dy - 0.005*dx;
            x2 = xs + 0.0085*dx - 0.005*dy;
            y2 = ys + 0.0085*dy + 0.005*dx;

            painter.drawLine( int(xs*scalex + offset.x()), int(-ys*scaley + offset.y()),
                              int(x1*scalex + offset.x()), int(-y1*scaley + offset.y()));

            painter.drawLine( int(xs*scalex + offset.x()), int(-ys*scaley + offset.y()),
                              int(x2*scalex + offset.x()), int(-y2*scaley + offset.y()));
        }
        else
        {
            x += -Objects2d::curFoil()->m_nx[i] * cp *0.05;
            y += -Objects2d::curFoil()->m_ny[i] * cp *0.05;

            xe = (x-0.5)*cosa - y*sina+ 0.5;
            ye = (x-0.5)*sina + y*cosa;
            painter.drawLine( int(xs*scalex + offset.x()), int(-ys*scaley + offset.y()),
                              int(xe*scalex + offset.x()), int(-ye*scaley + offset.y()));

            dx = xe - xs;
            dy = ye - ys;
            r2 = sqrt(dx*dx + dy*dy);
            if(r2!=0.0) //you can never be sure...
            {
                dx = -dx/r2;
                dy = -dy/r2;
            }

            x1 = xe + 0.0085*dx + 0.005*dy;
            y1 = ye + 0.0085*dy - 0.005*dx;
            x2 = xe + 0.0085*dx - 0.005*dy;
            y2 = ye + 0.0085*dy + 0.005*dx;

            painter.drawLine( int(xe*scalex + offset.x()), int(-ye*scaley + offset.y()),
                              int(x1*scalex + offset.x()), int(-y1*scaley + offset.y()));

            painter.drawLine( int(xe*scalex + offset.x()), int(-ye*scaley + offset.y()),
                              int(x2*scalex + offset.x()), int(-y2*scaley + offset.y()));
        }
    }
    //last draw lift at XCP position
    QPen LiftPen(m_PressureStyle.m_Color);
    LiftPen.setStyle(xfl::getStyle(m_PressureStyle.m_Stipple));
    LiftPen.setWidth(m_PressureStyle.m_Width+1);
    painter.setPen(LiftPen);

    xs =  (Objects2d::curOpp()->m_XCP-0.5)*cosa  + 0.5;
    ys = -(Objects2d::curOpp()->m_XCP-0.5)*sina ;

    xe = xs;
    ye = ys - Objects2d::curOpp()->Cl/10.0;

    painter.drawLine( int(xs*scalex + offset.x()), int(ys*scaley + offset.y()),
                      int(xs*scalex + offset.x()), int(ye*scaley + offset.y()));

    dx = xe - xs;
    dy = ye - ys;
    r2 = sqrt(dx*dx + dy*dy);
    dx = -dx/r2;
    dy = -dy/r2;

    x1 = xe + 0.0085*dx + 0.005*dy;
    y1 = ye + 0.0085*dy - 0.005*dx;
    x2 = xe + 0.0085*dx - 0.005*dy;
    y2 = ye + 0.0085*dy + 0.005*dx;

    painter.drawLine( int(xe*scalex + offset.x()), int(ye*scaley + offset.y()),
                      int(x1*scalex + offset.x()), int(y1*scaley + offset.y()));

    painter.drawLine( int(xe*scalex + offset.x()), int(ye*scaley + offset.y()),
                      int(x2*scalex + offset.x()), int(y2*scaley + offset.y()));

    painter.restore();
}


/**
 * The method which draws the boundary layer in the OpPoint view.
 * @param painter a reference to the QPainter object with which to draw
 * @param pOpPoint the OpPoint object to draw
 * @param scale the scale of the view
 */
void OpPointWt::paintBL(QPainter &painter, OpPoint* pOpPoint, double scalex, double scaley)
{
    if(!Objects2d::curFoil() || !pOpPoint) return;

    QPointF offset, From, To;
    double x=0,y=0;
    double alpha = -pOpPoint->aoa()*PI/180.0;
    double cosa = cos(alpha);
    double sina = sin(alpha);

    if(!pOpPoint->m_bViscResults || !pOpPoint->m_bBL) return;

    painter.save();

    offset = m_FoilOffset;

    QPen WakePen(m_BLStyle.m_Color);
    WakePen.setStyle(xfl::getStyle(m_BLStyle.m_Stipple));
    WakePen.setWidth(m_BLStyle.m_Width);

    painter.setPen(WakePen);

    x = (pOpPoint->blx.xd1[1]-0.5)*cosa - pOpPoint->blx.yd1[1]*sina + 0.5;
    y = (pOpPoint->blx.xd1[1]-0.5)*sina + pOpPoint->blx.yd1[1]*cosa;
    From.rx() =  x*scalex + offset.x();
    From.ry() = -y*scaley + offset.y();
    for (int i=2; i<=pOpPoint->blx.nd1; i++)
    {
        x = (pOpPoint->blx.xd1[i]-0.5)*cosa - pOpPoint->blx.yd1[i]*sina + 0.5;
        y = (pOpPoint->blx.xd1[i]-0.5)*sina + pOpPoint->blx.yd1[i]*cosa;
        To.rx() =  x*scalex + offset.x();
        To.ry() = -y*scaley + offset.y();
        painter.drawLine(From, To);
        From = To;
    }

    x = (pOpPoint->blx.xd2[0]-0.5)*cosa - pOpPoint->blx.yd2[0]*sina + 0.5;
    y = (pOpPoint->blx.xd2[0]-0.5)*sina + pOpPoint->blx.yd2[0]*cosa;
    From.rx() =  x*scalex + offset.x();
    From.ry() = -y*scaley + offset.y();
    for (int i=1; i<pOpPoint->blx.nd2; i++)
    {
        x = (pOpPoint->blx.xd2[i]-0.5)*cosa - pOpPoint->blx.yd2[i]*sina + 0.5;
        y = (pOpPoint->blx.xd2[i]-0.5)*sina + pOpPoint->blx.yd2[i]*cosa;
        To.rx() =  x*scalex + offset.x();
        To.ry() = -y*scaley + offset.y();
        painter.drawLine(From, To);
        From = To;
    }

    x = (pOpPoint->blx.xd3[0]-0.5)*cosa - pOpPoint->blx.yd3[0]*sina + 0.5;
    y = (pOpPoint->blx.xd3[0]-0.5)*sina + pOpPoint->blx.yd3[0]*cosa;
    From.rx() =  x*scalex + offset.x();
    From.ry() = -y*scaley + offset.y();
    for (int i=1; i<pOpPoint->blx.nd3; i++)
    {
        x = (pOpPoint->blx.xd3[i]-0.5)*cosa - pOpPoint->blx.yd3[i]*sina + 0.5;
        y = (pOpPoint->blx.xd3[i]-0.5)*sina + pOpPoint->blx.yd3[i]*cosa;
        To.rx() =  x*scalex + offset.x();
        To.ry() = -y*scaley + offset.y();
        painter.drawLine(From, To);
        From = To;
    }
    painter.restore();
}


/**
 * The user has requested the launch of the interface used to define the display style of the Foil
 */
void OpPointWt::onXDirectStyle()
{
    XDirectStyleDlg xdsDlg(this);
    xdsDlg.m_BLStyle       = m_BLStyle;
    xdsDlg.m_PressureStyle = m_PressureStyle;
    xdsDlg.m_NeutralStyle   = m_NeutralStyle;

    if(xdsDlg.exec() == QDialog::Accepted)
    {
        m_BLStyle       = xdsDlg.m_BLStyle;
        m_PressureStyle = xdsDlg.m_PressureStyle;
        m_NeutralStyle  = xdsDlg.m_NeutralStyle;
    }
    update();
}


/**
 * Converts screen coordinates to viewport coordinates
 * @param point the screen coordinates
 * @return the viewport coordinates
 */
Vector3d OpPointWt::mousetoReal(const QPoint &point) const
{
    Vector3d Real;

    Real.x =  (point.x() - m_FoilOffset.x())/m_fScale;
    Real.y = -(point.y() - m_FoilOffset.y())/m_fScale;
    Real.z = 0.0;

    return Real;
}


/**
 * The user has requested to reset the scale of the foil to its automatic default value
 */
void OpPointWt::onResetFoilScale()
{
    setFoilScale();
//    if(!m_bAnimate)
    update();
}


/**
 * The user has toggled the display of the neutral line y=0.
 */
void OpPointWt::onShowNeutralLine()
{
    m_bNeutralLine = !m_bNeutralLine;
    s_pMainFrame->m_pShowNeutralLine->setChecked(m_bNeutralLine);
    update();
}


/**
 * The user has toggled the display of the pressure arrows
 */
void OpPointWt::onShowPressure(bool bPressure)
{
    showPressure(bPressure);
    update();
}


/**
 * The user has toggled the display of the boundary layer
 */
void OpPointWt::onShowBL(bool bBL)
{
    showBL(bBL);
    update();
}


void OpPointWt::saveSettings(QSettings &settings)
{
    settings.beginGroup("OpPointSettings");
    {
        m_BLStyle.saveSettings(      settings, "BLStyle");
        m_PressureStyle.saveSettings(settings, "PressureStyle");
        m_NeutralStyle.saveSettings( settings, "NeutralStyle");
    }
    settings.endGroup();
}


void OpPointWt::loadSettings(QSettings &settings)
{
    settings.beginGroup("OpPointSettings");
    {
        m_BLStyle.loadSettings(      settings, "BLStyle");
        m_PressureStyle.loadSettings(settings, "PressureStyle");
        m_NeutralStyle.loadSettings( settings, "NeutralStyle");
    }
    settings.endGroup();
}

