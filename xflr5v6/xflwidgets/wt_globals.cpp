/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/

#include <cmath>

#include <QLayout>
#include <QLayoutItem>
#include <QApplication>
#include <QPalette>
#include <QFontMetrics>
#include <QTabWidget>
#include <QStackedWidget>

#include <xflwidgets/wt_globals.h>


void drawCheckBox(QPainter *painter, bool bChecked, QRect const & r, int side, bool bBackground, bool bContour,
                  const QColor &crossclr, const QColor &backclr)
{
    if(bChecked) drawCheckBox(painter, Qt::Checked,   r, side, bBackground, bContour, crossclr, backclr, Qt::black);
    else         drawCheckBox(painter, Qt::Unchecked, r, side, bBackground, bContour, crossclr, backclr, Qt::black);
}


void drawCheckBox(QPainter *painter, Qt::CheckState state, QRect const & r, int side, bool bBackground, bool bContour,
                  QColor const &crossclr, QColor const &backclr, QColor const &ContourClr)
{
    int h3 = int(double(side/5));

    QPoint center = r.center();
    QRect sR2 = QRect(center.x()-h3-3, center.y()-h3-3, 2*h3+4, 2*h3+4);
    QRect sR3 = QRect(center.x()-h3,   center.y()-h3,   2*h3,   2*h3);

    painter->save();

    if(bBackground)
    {
        painter->fillRect(r, QBrush(backclr));
    }
    painter->setBackgroundMode(Qt::TransparentMode);

    if(state==Qt::Checked)
    {
        QPen checkPen;

        checkPen.setColor(crossclr);
        checkPen.setWidth(2);
        painter->setPen(checkPen);
        painter->drawLine(sR3.left(), sR3.bottom(), sR3.right(), sR3.top());
        painter->drawLine(sR3.left(), sR3.top(),    sR3.right(), sR3.bottom());
    }
    else if(state==Qt::PartiallyChecked)
    {
        painter->setPen(QPen(Qt::gray));
        painter->setBrush(QBrush(Qt::gray));
        painter->drawRoundedRect(sR2, 1, 1, Qt::AbsoluteSize);
    }
    else if(state==Qt::Unchecked)
    {
        // don't draw anything
    }

    if(bContour)
    {
        QPen contourpen(ContourClr);
        contourpen.setWidth(1);
        painter->setPen(contourpen);
        painter->drawRect(sR2);
    }

    painter->restore();

}


QString formatDouble(double d, int decimaldigits, bool bLocalize)
{
    QString str;
    QString format = bLocalize ? "%L1" : "%1";
    if(decimaldigits<0)
    {
        str=QString(format).arg(d,0,'g');
    }
    else
    {
        if ((fabs(d)<=1.e-30 || fabs(d)>=pow(10.0, -decimaldigits)) && d <10000000.0)
        {
            str=QString(format).arg(d,0,'f', decimaldigits);
        }
        else
        {
            str=QString(format).arg(d,0,'g', decimaldigits+1);
        }
    }

    return str;
}


void removeLayout(QWidget* pWidget)
{
    qDeleteAll(pWidget->children());
}


void clearLayout(QLayout *pLayout)
{
    QLayoutItem *pChild = nullptr;
    while ((pChild=pLayout->takeAt(0)) != nullptr)
    {
        if(pChild->layout() != nullptr)
            clearLayout( pChild->layout() );
        else if(pChild->widget() != nullptr)
            delete pChild->widget();
        delete pChild;
    }
}


void setLayoutStyle(QLayout *pLayout, QPalette const &palette)
{
    for (int i=0; i<pLayout->count(); i++)
    {
        QWidget *pWidget = pLayout->itemAt(i)->widget();
        if (pWidget)
            setWidgetStyle(pWidget, palette);
        else
        {
            QLayout *pSubLayout =  pLayout->itemAt(i)->layout();
            if(pSubLayout)
                setLayoutStyle(pSubLayout, palette);
        }
    }
}


void setWidgetStyle(QWidget *pWidget, QPalette const &palette)
{
    pWidget->setPalette(palette);
//    pWidget->setAttribute(Qt::WA_PaintUnclipped);
    pWidget->setAutoFillBackground(false);
    if(pWidget->layout())
        setLayoutStyle(pWidget->layout(), palette);

    QStackedWidget *pStackWt = dynamic_cast<QStackedWidget*>(pWidget);
    if(pStackWt)
    {
        for (int j=0; j<pStackWt->count(); j++)
        {
            setWidgetStyle(pStackWt->widget(j), palette);
        }
    }

    QTabWidget *pTabWt = dynamic_cast<QTabWidget*>(pWidget);
    if(pTabWt)
    {
        for (int j=0; j<pTabWt->count(); j++)
        {
            setWidgetStyle(pTabWt->widget(j), palette);
        }
    }
}
