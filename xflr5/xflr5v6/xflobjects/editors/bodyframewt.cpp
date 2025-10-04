/****************************************************************************

    BodyFrameWt Class
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
#include <QtDebug>

#include "bodyframewt.h"
#include <xflcore/displayoptions.h>
#include <xflobjects/editors/bodyscaledlg.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflobjects/objects3d/body.h>


bool BodyFrameWt::s_bCurFrameOnly = false;

BodyFrameWt::BodyFrameWt(QWidget *pParent, Body *pBody) : Section2dWt(pParent)
{
    m_pBody = pBody;

    m_pShowCurFrameOnly = nullptr;
    createActions();
    createContextMenu();
    setCursor(Qt::CrossCursor);
}


void BodyFrameWt::setScale()
{
    if(!m_pBody)
    {
        //scale is set by user zooming
        m_fRefScale = rect().width();
        m_fScale = m_fRefScale;
    }
    else
    {
        m_fRefScale = (double(rect().width()))/(m_pBody->length()/15.0);
        m_fScale = m_fRefScale;

    }

    m_ptOffset.rx() = rect().width()/2;
    m_ptOffset.ry() = rect().height()/2;

    m_ViewportTrans = QPoint(0,0);
}


/**
*Overrides the resizeEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void BodyFrameWt::resizeEvent (QResizeEvent *event)
{
    Q_UNUSED(event);
    setScale();
}



void BodyFrameWt::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.save();
    painter.fillRect(rect(), DisplayOptions::backgroundColor());

    drawScaleLegend(painter);
    drawBackImage(painter);

    paintGrids(painter);
    drawFrameLines();
    drawFramePoints();

    painter.restore();
}


void BodyFrameWt::drawFrameLines()
{
    if(!m_pBody) return;

    int k;
    Vector3d Point;
    double hinc, u, v;
    int nh;

    QPainter painter(this);
    painter.save();
    nh = 23;
    //    xinc = 0.1;
    hinc = 1.0/double(nh-1);

    QPen framePen(m_pBody->color());
    framePen.setWidth(2);
    painter.setPen(framePen);

    QPolygonF rightPolyline, leftPolyline;

    if(m_pBody->m_LineType ==xfl::BODYSPLINETYPE)
    {
        if(m_pBody->activeFrame())
        {
            u = m_pBody->getu(m_pBody->activeFrame()->m_Position.x);

            v = 0.0;
            for (k=0; k<nh; k++)
            {
                m_pBody->getPoint(u,v,true, Point);
                rightPolyline.append(QPointF(Point.y*m_fScale+m_ptOffset.x(), Point.z* -m_fScale + m_ptOffset.y()));
                leftPolyline.append(QPointF(-Point.y*m_fScale+m_ptOffset.x(), Point.z* -m_fScale + m_ptOffset.y()));
                v += hinc;
            }
        }
    }
    else
    {
        Frame *pFrame = m_pBody->activeFrame();
        if(pFrame)
        {
            for (k=0; k<m_pBody->sideLineCount();k++)
            {
                rightPolyline.append(QPointF( pFrame->m_CtrlPoint[k].y*m_fScale+m_ptOffset.x(), pFrame->m_CtrlPoint[k].z* -m_fScale + m_ptOffset.y()));
                leftPolyline.append( QPointF(-pFrame->m_CtrlPoint[k].y*m_fScale+m_ptOffset.x(), pFrame->m_CtrlPoint[k].z* -m_fScale + m_ptOffset.y()));
            }
        }
    }

    painter.drawPolyline(rightPolyline);
    painter.drawPolyline(leftPolyline);

    framePen.setStyle(Qt::DashLine);
    framePen.setWidth(1);
    painter.setPen(framePen);

    if(!s_bCurFrameOnly)
    {
        for(int j=0; j<m_pBody->frameCount(); j++)
        {
            if(m_pBody->frame(j)!=m_pBody->activeFrame())
            {
                rightPolyline.clear();
                leftPolyline.clear();

                if(m_pBody->m_LineType ==xfl::BODYSPLINETYPE)
                {
                    u = m_pBody->getu(m_pBody->frame(j)->m_Position.x);

                    v = 0.0;
                    for (k=0; k<nh; k++)
                    {
                        m_pBody->getPoint(u,v,true, Point);
                        rightPolyline.append(QPointF(Point.y*m_fScale+m_ptOffset.x(), Point.z* -m_fScale + m_ptOffset.y()));
                        leftPolyline.append(QPointF(-Point.y*m_fScale+m_ptOffset.x(), Point.z* -m_fScale + m_ptOffset.y()));
                        v += hinc;
                    }

                }
                else
                {
                    for (k=0; k<m_pBody->sideLineCount();k++)
                    {
                        rightPolyline.append(QPointF( m_pBody->frame(j)->m_CtrlPoint[k].y*m_fScale+m_ptOffset.x(), m_pBody->frame(j)->m_CtrlPoint[k].z* -m_fScale + m_ptOffset.y()));
                        leftPolyline.append( QPointF(-m_pBody->frame(j)->m_CtrlPoint[k].y*m_fScale+m_ptOffset.x(), m_pBody->frame(j)->m_CtrlPoint[k].z* -m_fScale + m_ptOffset.y()));
                    }
                }

                painter.drawPolyline(rightPolyline);
                painter.drawPolyline(leftPolyline);
            }
        }
    }
    painter.restore();
}




void BodyFrameWt::drawFramePoints()
{
    if(!m_pBody->activeFrame()) return;

    Frame *m_pFrame = m_pBody->activeFrame();
    QPainter painter(this);
    painter.save();

    QPen pointPen(m_pBody->color());


    for (int k=0; k<m_pFrame->pointCount();k++)
    {
        if(Frame::selectedIndex()==k)
        {
            pointPen.setWidth(4);
            pointPen.setColor(Qt::red);
        }
        else if(Frame::highlightedIndex()==k)
        {
            pointPen.setWidth(4);
            pointPen.setColor(m_pBody->color().lighter());
        }
        else
        {
            pointPen.setWidth(2);
            pointPen.setColor(m_pBody->color());
        }

        painter.setPen(pointPen);
        QRectF rectF( m_pFrame->m_CtrlPoint[k].y *  m_fScale -3 +m_ptOffset.x(),
                      m_pFrame->m_CtrlPoint[k].z * -m_fScale -3 +m_ptOffset.y(),
                      7,7);
        painter.drawEllipse(rectF);
    }
    painter.restore();
}



void BodyFrameWt::setBody(Body *pBody)
{
    m_pBody = pBody;
    setScale();
}



void BodyFrameWt::onInsertPt()
{
    Vector3d real = mousetoReal(m_PointDown);
    real.z = real.y;
    real.y = real.x;
    real.x = m_pBody->activeFrame()->position().x;
    if(m_pBody->activeFrame())
    {
        m_pBody->insertPoint(real);
        emit objectModified();
    }
}


void BodyFrameWt::onRemovePt()
{
    if(m_pBody->activeFrame())
    {
        Vector3d real = mousetoReal(m_PointDown);
        real.z = real.y;
        real.y = real.x;
        real.x = m_pBody->activeFrame()->position().x;

        int n =   m_pBody->activeFrame()->isPoint(real, m_fScale/m_fRefScale);
        if (n>=0)
        {
            for (int i=0; i<m_pBody->frameCount();i++)
            {
                m_pBody->frame(i)->removePoint(n);
            }
            m_pBody->setNURBSKnots();
            emit objectModified();
        }
    }
}


void BodyFrameWt::onScaleFrame()
{
    if(!m_pBody) return;

    BodyScaleDlg dlg(this);

    dlg.m_FrameID = m_pBody->m_iActiveFrame;
    dlg.initDialog(true);

    if(dlg.exec()==QDialog::Accepted)
    {
        m_pBody->scale(dlg.m_XFactor, dlg.m_YFactor, dlg.m_ZFactor, dlg.m_bFrameOnly, dlg.m_FrameID);
        emit objectModified();

    }
}


int BodyFrameWt::highlightPoint(Vector3d real)
{
    if(!m_pBody->activeFrame()) Frame::setHighlighted(-1);
    else
    {
        real.z = real.y;
        real.y = real.x;
        real.x = m_pBody->activeFrame()->position().x;
        Frame::setHighlighted(m_pBody->activeFrame()->isPoint(real, m_fScale/m_fRefScale));
    }
    return Frame::highlightedIndex();
}



int BodyFrameWt::selectPoint(Vector3d real)
{
    if(!m_pBody->activeFrame()) Frame::setSelected(-1);
    else
    {
        real.z = real.y;
        real.y = real.x;
        real.x = m_pBody->activeFrame()->position().x;
        Frame::setSelected(m_pBody->activeFrame()->isPoint(real, m_fScale/m_fRefScale));
    }
    emit pointSelChanged();
    return Frame::selectedIndex();
}


void BodyFrameWt::dragSelectedPoint(double x, double y)
{
    if (!m_pBody->activeFrame() || (Frame::selectedIndex()<0) || (Frame::selectedIndex() > m_pBody->activeFrame()->pointCount()))
        return;

    if(Frame::selectedIndex()==0 || Frame::selectedIndex()==m_pBody->activeFrame()->pointCount()-1) x=0.0;
    x = std::max(x,0.0);
    m_pBody->activeFrame()->setSelectedPoint({m_pBody->activeFrame()->position().x, x, y});
}


void BodyFrameWt::drawScaleLegend(QPainter &painter)
{
    painter.save();
    QPen TextPen(DisplayOptions::textColor());
    painter.setPen(TextPen);
    painter.drawText(5,10, QString(tr("X-Scale = %1")).arg(m_fScale/m_fRefScale,4,'f',1));
    painter.drawText(5,22, QString(tr("Y-Scale = %1")).arg(m_fScaleY*m_fScale/m_fRefScale,4,'f',1));
    painter.drawText(5,34, QString(tr("x  = %1")).arg(m_MousePos.x * Units::mtoUnit(),7,'f',2) + Units::lengthUnitLabel());
    painter.drawText(5,46, QString(tr("y  = %1")).arg(m_MousePos.y * Units::mtoUnit(),7,'f',2) + Units::lengthUnitLabel());
    painter.restore();
}



void BodyFrameWt::createActions()
{
    m_ActionList.clear();

    QAction *pScaleBody = new QAction(tr("Scale Frame"), this);
    connect(pScaleBody,  SIGNAL(triggered()), this, SLOT(onScaleFrame()));
    m_ActionList.append(pScaleBody);

    QAction *pSeparator0 = new QAction(this);
    pSeparator0->setSeparator(true);
    m_ActionList.append(pSeparator0);

    QAction *pInsertPt = new QAction(tr("Insert Control Point")+"\tShift+Click", this);
    connect(pInsertPt, SIGNAL(triggered()), this, SLOT(onInsertPt()));
    m_ActionList.append(pInsertPt);

    QAction *pRemovePt = new QAction(tr("Remove Control Point")+"\tCtrl+Click", this);
    connect(pRemovePt, SIGNAL(triggered()), this, SLOT(onRemovePt()));
    m_ActionList.append(pRemovePt);

    QAction *pSeparator1 = new QAction(this);
    pSeparator1->setSeparator(true);
    m_ActionList.append(pSeparator1);

    m_pShowCurFrameOnly = new QAction(tr("Show Current Frame Only"), this);
    m_pShowCurFrameOnly->setCheckable(true);
    connect(m_pShowCurFrameOnly, SIGNAL(triggered()), this, SLOT(onShowCurFrameOnly()));
    m_ActionList.append(m_pShowCurFrameOnly);

    QAction *pResetScaleAction = new QAction(tr("Reset Scales"), this);
    connect(pResetScaleAction, SIGNAL(triggered()), this, SLOT(onResetScales()));
    m_ActionList.append(pResetScaleAction);

    QAction *pGridSettingsAction = new QAction(tr("Grid Settings"), this);
    connect(pGridSettingsAction, SIGNAL(triggered()), this, SLOT(onGridSettings()));
    m_ActionList.append(pGridSettingsAction);

    QAction *pSeparator2 = new QAction(this);
    pSeparator2->setSeparator(true);
    m_ActionList.append(pSeparator2);

    QAction *pLoadBackImage = new QAction(tr("Load background image")   +"\tCtrl+Shift+I", this);
    connect(pLoadBackImage, SIGNAL(triggered()), this, SLOT(onLoadBackImage()));
    m_ActionList.append(pLoadBackImage);

    QAction *pClearBackImage = new QAction(tr("Clear background image") +"\tCtrl+Shift+I", this);
    connect(pClearBackImage, SIGNAL(triggered()), this, SLOT(onClearBackImage()));
    m_ActionList.append(pClearBackImage);

}



void BodyFrameWt::onShowCurFrameOnly()
{
    s_bCurFrameOnly = !s_bCurFrameOnly;
    m_pShowCurFrameOnly->setChecked(s_bCurFrameOnly);
    update();
}








