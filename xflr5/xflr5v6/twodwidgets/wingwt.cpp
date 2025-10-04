/****************************************************************************

    WingWidget Class
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

#include <QPainter>
#include <QDebug>
#include <QContextMenuEvent>

#include "wingwt.h"
#include <miarex/miarex.h>

#include <xfl3d/controls/w3dprefs.h>
#include <xflcore/displayoptions.h>
#include <xflcore/xflcore.h>
#include <xflgraph/graph.h>
#include <xflobjects/objects3d/objects3d.h>
#include <xflobjects/objects3d/plane.h>
#include <xflobjects/objects3d/planeopp.h>
#include <xflobjects/objects3d/wpolar.h>


Miarex *WingWidget::s_pMiarex = nullptr;

WingWidget::WingWidget(QWidget *pParent) : QWidget(pParent)
{
    setMouseTracking(true);

    m_bTrans = false;

    m_pGraph = nullptr;

    m_WingScale = 1.0;
    m_ptOffset.rx() = 0;
    m_ptOffset.ry() = 0;
}


void WingWidget::setWingGraph(Graph *pGraph)
{
    m_pGraph = pGraph;
}


void WingWidget::keyPressEvent(QKeyEvent *pEvent)
{
    //    bool bShift = false;
    //    if(event->modifiers() & Qt::ShiftModifier)   bShift =true;

    switch (pEvent->key())
    {
        case Qt::Key_R:
            onResetWingScale();
            pEvent->accept();
            return;

        default:
            QWidget::keyPressEvent(pEvent);
    }

    pEvent->ignore();
}


void WingWidget::mouseMoveEvent(QMouseEvent *pEvent)
{
    setFocus();

    // we translate the Plane
    if (pEvent->buttons() & Qt::LeftButton)
    {
        QPointF Delta;
        Delta.setX(pEvent->pos().x() - m_LastPoint.x());
        Delta.setY(pEvent->pos().y() - m_LastPoint.y());
        m_ptOffset.rx() += Delta.x();
        m_ptOffset.ry() += Delta.y();
        update();
    }
    else if ((pEvent->buttons() & Qt::MidButton) || pEvent->modifiers().testFlag(Qt::AltModifier))
    {
        //zoom the wing

        if(pEvent->pos().y()-m_LastPoint.y()<0) m_WingScale /= 1.02;
        else                                   m_WingScale *= 1.02;

        double a = rect().center().x();

        m_ptOffset.rx() = a + (m_ptOffset.x()-a)/m_WingScale;

        update();
    }
    m_LastPoint = pEvent->pos();
}


void WingWidget::mousePressEvent(QMouseEvent *pEvent)
{
    if (pEvent->buttons() & Qt::LeftButton)
    {
        m_LastPoint = pEvent->pos();
    }
}


void WingWidget::mouseReleaseEvent(QMouseEvent *)
{
    m_bTrans = false;
}


void WingWidget::paintEvent(QPaintEvent *)
{
    if(s_pMiarex->m_bResetTextLegend) s_pMiarex->drawTextLegend();

    QPainter painter(this);
    painter.save();
    painter.fillRect(rect(), DisplayOptions::backgroundColor());
    painter.setFont(DisplayOptions::textFont());

    QPen TextPen(DisplayOptions::textColor());
    TextPen.setWidth(1);
    painter.setPen(TextPen);


    if(s_pMiarex->m_pCurPlane)
    {
        paintWing(painter, m_ptOffset, m_WingScale);
        if(s_pMiarex->m_pCurPOpp && s_pMiarex->m_pCurPOpp->isVisible())
        {
            if (s_pMiarex->m_bXTop || s_pMiarex->m_bXBot) paintXTr(painter, m_ptOffset, m_WingScale);
            if (s_pMiarex->m_bXCP) paintXCP(painter, m_ptOffset, m_WingScale);
            if (s_pMiarex->m_bXCmRef) paintXCmRef(painter, m_ptOffset, m_WingScale);
        }

        painter.setBackgroundMode(Qt::TransparentMode);
        painter.setOpacity(1);

        if(s_pMiarex->m_bResetTextLegend) s_pMiarex->drawTextLegend();
        if(!s_pMiarex->m_PixText.isNull()) painter.drawPixmap(QPoint(0,0), s_pMiarex->m_PixText);
    }
    painter.restore();
}


/**
 * Draws the wing in the 2D operating point view
 * @param painter a reference to the QPainter object on which the view shall be drawn
 * @param ORef the origin of the tip of the root chord, in client coordinates
 * @param scale the scaling factor with which to draw the wing
 */
void WingWidget::paintWing(QPainter &painter, QPointF ORef, double scale)
{
    if(!s_pMiarex->m_pCurPlane)    return;

    double scalex  = scale;
    double scaley  = scale;

    Wing *pWing = s_pMiarex->m_pCurPlane->m_Wing;

    painter.save();
    QPen WingPen(W3dPrefs::s_OutlineStyle.m_Color);
    WingPen.setStyle(xfl::getStyle(W3dPrefs::s_OutlineStyle.m_Stipple));
    WingPen.setWidth(W3dPrefs::s_OutlineStyle.m_Width);

    painter.setPen(WingPen);

    //Right Wing
    int x = int(ORef.x());
    int y = int(ORef.y());
    for (int i=0; i<pWing->NWingSection()-1;i++)
    {
        x +=int(pWing->Length(i)*scalex);
        painter.drawLine(x,                                 y+int(pWing->Offset(i)*scaley),
                         x+int(pWing->Length(i+1)*scalex),  y+int(pWing->Offset(i+1)*scaley));

        painter.drawLine(x+int(pWing->Length(i+1)*scalex),  y+int(pWing->Offset(i+1)*scaley),
                         x+int(pWing->Length(i+1)*scalex),  y+int((pWing->Offset(i+1)+pWing->Chord(i+1))*scaley));

        painter.drawLine(x+int(pWing->Length(i+1)*scalex),  y+int((pWing->Offset(i+1)+pWing->Chord(i+1))*scaley),
                         x,                                 y+int((pWing->Offset(i)+pWing->Chord(i))*scaley));

        painter.drawLine(x,                                 y+int((pWing->Offset(i)+pWing->Chord(i))*scaley),
                         x,                                 y+int(pWing->Offset(i)*scaley));
    }


    //LeftWing
    x = int(ORef.x());
    y = int(ORef.y());

    for (int i=0; i<pWing->NWingSection()-1;i++)
    {
        x -= int(pWing->Length(i)*scalex);
        painter.drawLine(x,                                   y+int(pWing->Offset(i)*scaley),
                         x-int(pWing->Length(i+1)*scalex), y+int(pWing->Offset(i+1)*scaley));

        painter.drawLine(x-int(pWing->Length(i+1)*scalex), y+int(pWing->Offset(i+1)*scaley),
                         x-int(pWing->Length(i+1)*scalex), y+int((pWing->Offset(i+1)+pWing->Chord(i+1))*scaley));

        painter.drawLine(x-int(pWing->Length(i+1)*scalex), y+int((pWing->Offset(i+1)+pWing->Chord(i+1))*scaley),
                         x,                                y +int((pWing->Offset(i)+pWing->Chord(i))*scaley));

        painter.drawLine(x,                                y +int((pWing->Offset(i)+pWing->Chord(i))*scaley),
                         x,                                y+int(pWing->Offset(i)*scaley));
    }


    /*    QPen SymPen(QColor(155,128,190));
    painter.setPen(SymPen);
    painter.setBackgroundMode(Qt::TransparentMode);

    painter.drawLine(ORef.x(), ORef.y()-20, ORef.x(), ORef.y()+75);*/
    painter.restore();
}



/**
 * Draws the position of the reference point for the moments in the operating view
 * @param painter a reference to the QPainter object on which the view shall be drawn
 * @param ORef the origin of the tip of the root chord, in client coordinates
 * @param scale the scaling factor with which to draw the wing
 */
void WingWidget::paintXCmRef(QPainter & painter, QPointF ORef, double scale)
{
    //Draws the moment reference point on the 2D view
    if(!s_pMiarex->m_pCurPlane || !s_pMiarex->m_pCurWPolar)    return;

    painter.save();
    QPointF O(ORef);
    QPointF offset;

    double scaley;

    offset.rx() = ORef.x();
    offset.ry() = ORef.y();
    //    scalex  = scale;
    scaley  = scale;
    O.rx() = offset.x();
    O.ry() = offset.y();

    QPen XCmRefPen(DisplayOptions::textColor());
    painter.setPen(XCmRefPen);

    double XCm = O.x() ;
    double YCm = O.y() + s_pMiarex->m_pCurWPolar->CoG().x*scaley;
    int size = 3;
    QRectF CM(XCm-size, YCm-size, 2*size, 2*size);
    painter.drawEllipse(CM);

    painter.drawText(int(XCm+10), int(YCm-5), tr("Moment ref. location"));

    painter.restore();
}


/**
 * Draws the lift line and the position of the center of pressure in the operating view
 * @param painter a reference to the QPainter object on which the view shall be drawn
 * @param ORef the origin of the tip of the root chord, in client coordinates
 * @param scale the scaling factor with which to draw the wing
 */
void WingWidget::paintXCP(QPainter & painter, QPointF ORef, double scale)
{
    //Draws the lift line and center of pressure position on the the 2D view
    if(!s_pMiarex->m_pCurPlane)    return;

    Wing *pWing = s_pMiarex->m_pCurPlane->m_Wing;


    QPointF From, To;

    int nStart=0;
    if(s_pMiarex->m_pCurPOpp->analysisMethod()==xfl::LLTMETHOD) nStart = 1; else nStart = 0;

    QPointF O(ORef);
    QPointF offset;
    offset.rx() = ORef.x();
    offset.ry() = ORef.y();
    O.rx() = offset.x();
    O.ry() = offset.y();

    double scalex  = scale;
    double scaley  = scale;

    QPen XCPPen(W3dPrefs::s_XCPStyle.m_Color);
    XCPPen.setWidth(W3dPrefs::s_XCPStyle.m_Width);
    XCPPen.setStyle(xfl::getStyle(W3dPrefs::s_XCPStyle.m_Stipple));
    painter.setPen(XCPPen);

    PlaneOpp const *pPOpp = s_pMiarex->curPOpp();
    if(!pPOpp)  return;  // something went wrong upstream
    WingOpp const *pWOpp = pPOpp->wingOpp(0);
    if(!pWOpp)  return;  // something went wrong upstream

    painter.save();

    double XCp = O.x() + pWOpp->m_CP.y*scalex;
    double YCp = O.y() + pWOpp->m_CP.x*scaley;

    int size = 3;
    QRectF CP(XCp-size, YCp-size, 2*size, 2*size);
    painter.drawEllipse(CP);

    double offLE = pWing->getOffset(pWOpp->m_SpanPos[nStart]*2.0/pWOpp->m_Span);
    double y = (offLE+pWOpp->m_Chord[nStart]*pWOpp->m_XCPSpanRel[nStart])*scaley;
    From = QPointF(O.x()+pWOpp->m_SpanPos[nStart]*scalex,    O.y()+y );

    for (int m=nStart; m<pWOpp->m_NStation; m++)
    {
        offLE = pWing->getOffset(pWOpp->m_SpanPos[m]*2.0/pWOpp->m_Span);
        y = (offLE+pWOpp->m_Chord[m]*pWOpp->m_XCPSpanRel[m])*scaley;
        To = QPointF(O.x()+pWOpp->m_SpanPos[m]*scalex,    O.y()+y );
        painter.drawLine(From, To);
        From = To;
    }

    int x = int(rect().width()/2);
    int y1 = rect().bottom();
    painter.drawLine(x-60,  y1- 20, x-40,  y1 - 20);
    painter.drawText(x-35, y1 - 18, tr("Centre of Pressure"));
    painter.restore();
}



/**
 * Draws the laminar to turbulent translition lines in the operating view
 * @param painter a reference to the QPainter object on which the view shall be drawn
 * @param ORef the origin of the tip of the root chord, in client coordinates
 * @param scale the scaling factor with which to draw the wing
 */
void WingWidget::paintXTr(QPainter & painter, QPointF ORef, double scale)
{
    //Draws the transition lines on the 2D view
    if(!s_pMiarex->m_pCurPlane)    return;

    Wing *pWing = s_pMiarex->m_pCurPlane->wing();
    painter.save();

    double y;
    int m,nStart;
    if(s_pMiarex->m_pCurPOpp->analysisMethod()==xfl::LLTMETHOD) nStart = 1; else nStart = 0;

    QPointF O(ORef);
    QPointF offset, From, To;

    double offLE;
    double scalex, scaley;
    offset.rx() = ORef.x();
    offset.ry() = ORef.y();
    scalex  = scale;
    scaley  = scale;

    O.rx() = offset.x();
    O.ry() = offset.y();

    QPen TopPen(W3dPrefs::s_TopStyle.m_Color);
    TopPen.setStyle(xfl::getStyle(W3dPrefs::s_TopStyle.m_Stipple));
    TopPen.setWidth(W3dPrefs::s_TopStyle.m_Width);
    painter.setPen(TopPen);

    if (s_pMiarex->m_bXTop)
    {
        offLE = pWing->getOffset(s_pMiarex->m_pCurPOpp->m_pWOpp[0]->m_SpanPos[nStart]*2.0/s_pMiarex->m_pCurPOpp->m_pWOpp[0]->m_Span);
        y = (offLE+s_pMiarex->m_pCurPOpp->m_pWOpp[0]->m_Chord[nStart]*s_pMiarex->m_pCurPOpp->m_pWOpp[0]->m_XTrTop[nStart])*scaley;
        From = QPointF(O.x()+s_pMiarex->m_pCurPOpp->m_pWOpp[0]->m_SpanPos[nStart]*scalex,    O.y()+y);

        for (m=nStart; m<s_pMiarex->m_pCurPOpp->m_pWOpp[0]->m_NStation; m++)
        {
            offLE = pWing->getOffset(s_pMiarex->m_pCurPOpp->m_pWOpp[0]->m_SpanPos[m]*2.0/s_pMiarex->m_pCurPOpp->m_pWOpp[0]->m_Span);
            y = (offLE+s_pMiarex->m_pCurPOpp->m_pWOpp[0]->m_Chord[m]*s_pMiarex->m_pCurPOpp->m_pWOpp[0]->m_XTrTop[m])*scaley;

            To = QPointF(O.x()+s_pMiarex->m_pCurPOpp->m_pWOpp[0]->m_SpanPos[m]*scalex, O.y()+y );
            painter.drawLine(From, To);
            From  = To;
        }


        int x = int(double(rect().width())/2);
        int y = rect().bottom();
        painter.drawLine(x-60,  y - 50, x-40,  y - 50);
        painter.drawText(x-35, y - 48, tr("Top transition"));

    }


    QPen BotPen(W3dPrefs::s_BotStyle.m_Color);
    BotPen.setStyle(xfl::getStyle(W3dPrefs::s_BotStyle.m_Stipple));
    BotPen.setWidth(W3dPrefs::s_BotStyle.m_Width);

    painter.setPen(BotPen);
    if (s_pMiarex->m_bXBot)
    {
        offLE = pWing->getOffset(s_pMiarex->m_pCurPOpp->m_pWOpp[0]->m_SpanPos[nStart]*2.0/s_pMiarex->m_pCurPOpp->m_pWOpp[0]->m_Span);
        y = (offLE+s_pMiarex->m_pCurPOpp->m_pWOpp[0]->m_Chord[nStart]*s_pMiarex->m_pCurPOpp->m_pWOpp[0]->m_XTrBot[nStart])*scaley;
        From = QPointF(O.x() +s_pMiarex->m_pCurPOpp->m_pWOpp[0]->m_SpanPos[nStart]*scalex, O.y()+y );
        for (m=nStart; m<s_pMiarex->m_pCurPOpp->m_pWOpp[0]->m_NStation; m++)
        {
            offLE = pWing->getOffset(s_pMiarex->m_pCurPOpp->m_pWOpp[0]->m_SpanPos[m]*2.0/s_pMiarex->m_pCurPOpp->m_pWOpp[0]->m_Span);
            y = (offLE+s_pMiarex->m_pCurPOpp->m_pWOpp[0]->m_Chord[m]*s_pMiarex->m_pCurPOpp->m_pWOpp[0]->m_XTrBot[m])*scaley;
            To = QPointF(O.x()+s_pMiarex->m_pCurPOpp->m_pWOpp[0]->m_SpanPos[m]*scalex, O.y()+y );
            painter.drawLine(From, To);
            From  = To;
        }

        int x = int(rect().width()/2);
        int y = rect().bottom();
        painter.drawLine(x-60,  y - 35, x-40,  y - 35);
        painter.drawText(x-35, y - 33, tr("Bottom transition"));

    }
    painter.restore();
}


void WingWidget::resizeEvent (QResizeEvent *)
{
    s_pMiarex->m_bResetTextLegend = true;
    setWingScale();
    update();
}


void WingWidget::wheelEvent (QWheelEvent *pEvent)
{
    double zoomFactor=1.0;

    if(pEvent->angleDelta().y()>0)
    {
        if(!DisplayOptions::bReverseZoom()) zoomFactor = 1./1.06;
        else                          zoomFactor = 1.06;
    }
    else
    {
        if(!DisplayOptions::bReverseZoom()) zoomFactor = 1.06;
        else                          zoomFactor = 1./1.06;
    }

    m_WingScale *= zoomFactor;


    update();
}



void WingWidget::setWingScale()
{
    m_ptOffset.rx() = rect().width()/2.0;
    m_ptOffset.ry() = rect().height()/4.0;

    if(s_pMiarex->m_pCurPlane && m_pGraph)
    {
        m_WingScale = (rect().width()-2*m_pGraph->margin())/s_pMiarex->m_pCurPlane->planformSpan();
    }
}



void WingWidget::onResetWingScale()
{
    setWingScale();
    update();
}


