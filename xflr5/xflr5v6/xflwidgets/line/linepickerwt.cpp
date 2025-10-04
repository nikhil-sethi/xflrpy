/****************************************************************************

    LinePicker Class
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

#include <QDebug>
#include <QLabel>
#include <QGridLayout>
#include <QColorDialog>

#include "linepickerwt.h"

#include <xflwidgets/line/linebtn.h>
#include <xflwidgets/line/linecbbox.h>
#include <xflwidgets/line/linedelegate.h>


LinePickerWt::LinePickerWt(QWidget *pParent) : QWidget(pParent)
{
    setupLayout();
    connectSignals();
}


void LinePickerWt::connectSignals()
{
    connect(m_plcbPointStyle,  SIGNAL(activated(int)), SLOT(onPointStyle(int)));
    connect(m_plcbLineStyle,   SIGNAL(activated(int)), SLOT(onLineStyle(int)));
    connect(m_plcbLineWidth,   SIGNAL(activated(int)), SLOT(onLineWidth(int)));
    connect(m_plbLineColor,    SIGNAL(clickedLB(LineStyle)), SLOT(onLineColor()));
}


void LinePickerWt::setupLayout()
{
    QGridLayout *pStyleLayout = new QGridLayout;
    {
        QLabel *pLab0 = new QLabel(tr("Points"));
        QLabel *pLab1 = new QLabel(tr("Style"));
        QLabel *pLab2 = new QLabel(tr("Width"));
        QLabel *pLab3 = new QLabel(tr("Color"));
        pLab0->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        pLab1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        pLab2->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        pLab3->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        pLab0->setMinimumWidth(60);
        pLab1->setMinimumWidth(60);
        pLab2->setMinimumWidth(60);
        pLab3->setMinimumWidth(60);
//        m_plcbPointStyle->showPoints(true);
        m_plcbLineStyle  = new LineCbBox(NLINESTYLES, this);
        m_plcbLineWidth  = new LineCbBox(NLINEWIDTHS, this);
        m_plcbPointStyle = new LineCbBox(NPOINTSTYLES, this);

        m_pLineStyleDelegate  = new LineDelegate(m_plcbLineStyle);
        m_pLineWidthDelegate  = new LineDelegate(m_plcbLineWidth);
        m_pPointStyleDelegate = new LineDelegate(m_plcbPointStyle);
        m_plcbLineStyle->setItemDelegate(m_pLineStyleDelegate);
        m_plcbLineWidth->setItemDelegate(m_pLineWidthDelegate);
        m_plcbPointStyle->setItemDelegate(m_pPointStyleDelegate);

        m_plbLineColor  = new LineBtn;

        QFont fnt;
        QFontMetrics fm(fnt);
        m_plcbLineStyle->setMinimumWidth( 17*fm.averageCharWidth());
        m_plcbLineWidth->setMinimumWidth( 17*fm.averageCharWidth());
        m_plcbPointStyle->setMinimumWidth(17*fm.averageCharWidth());
        m_plbLineColor->setMinimumWidth(  17*fm.averageCharWidth());
        m_plbLineColor->setMinimumHeight(m_plcbLineStyle->minimumSizeHint().height());

        pStyleLayout->addWidget(pLab0,           1,1);
        pStyleLayout->addWidget(m_plcbPointStyle,1,2);
        pStyleLayout->addWidget(pLab1,           2,1);
        pStyleLayout->addWidget(m_plcbLineStyle, 2,2);
        pStyleLayout->addWidget(pLab2,           3,1);
        pStyleLayout->addWidget(m_plcbLineWidth, 3,2);
        pStyleLayout->addWidget(pLab3,           4,1);
        pStyleLayout->addWidget(m_plbLineColor,  4,2);
    }
    setLayout(pStyleLayout);
}


void LinePickerWt::fillBoxes()
{
    m_plbLineColor->setTheStyle(m_LineStyle);

    QVector<LineStyle> LineStipple(NLINESTYLES), LineWidth(NLINEWIDTHS), LinePoints(NPOINTSTYLES);
    LineStipple.fill(m_LineStyle);
    LineWidth.fill(m_LineStyle);
    LinePoints.fill(m_LineStyle);
    for(int i=0; i<NLINESTYLES; i++)  LineStipple[i].m_Stipple = LineStyle::convertLineStyle(i);
    for(int i=0; i<NLINEWIDTHS; i++)  LineWidth[i].m_Width     = i;
    for(int i=0; i<NPOINTSTYLES; i++) LinePoints[i].m_Symbol   = LineStyle::convertSymbol(i);

    m_pLineStyleDelegate->setLineStyle(LineStipple);
    m_pLineWidthDelegate->setLineStyle(LineWidth);
    m_pPointStyleDelegate->setLineStyle(LinePoints);

    m_plcbLineStyle->setLine(m_LineStyle);
    m_plcbLineWidth->setLine(m_LineStyle);
    m_plcbPointStyle->setLine(m_LineStyle);

    m_plcbLineStyle->setCurrentIndex(m_LineStyle.m_Stipple);
    m_plcbLineWidth->setCurrentIndex(m_LineStyle.m_Width-1);
//    m_plcbPointStyle->setCurrentIndex(m_LineStyle.m_Symbol);
}


void LinePickerWt::onPointStyle(int val)
{
    m_LineStyle.m_Symbol = LineStyle::convertSymbol(val);
    fillBoxes();
    update();
    emit styleChanged(m_LineStyle);
}


void LinePickerWt::onLineStyle(int val)
{
    m_LineStyle.m_Stipple = LineStyle::convertLineStyle(val);
    fillBoxes();
    update();
    emit styleChanged(m_LineStyle);
}


void LinePickerWt::onLineWidth(int val)
{
    m_LineStyle.m_Width = val+1;
    fillBoxes();
    update();
    emit styleChanged(m_LineStyle);
}


void LinePickerWt::onLineColor()
{
    QColorDialog::ColorDialogOptions dialogOptions = QColorDialog::ShowAlphaChannel;
#ifdef Q_OS_MAC
#if QT_VERSION >= 0x040700
    dialogOptions |= QColorDialog::DontUseNativeDialog;
#endif
#endif
    QColor Color = QColorDialog::getColor(m_LineStyle.m_Color, this, "Color Selection", dialogOptions);
    if(Color.isValid()) m_LineStyle.m_Color = Color;

    fillBoxes();
    update();
    emit styleChanged(m_LineStyle);
}


void LinePickerWt::setLineColor(QColor color)
{
    m_LineStyle.m_Color = color;
    fillBoxes();
    repaint();
}


void LinePickerWt::setPointStyle(int pointStyle)
{
    m_LineStyle.m_Symbol = LineStyle::convertSymbol(pointStyle);
    fillBoxes();
    repaint();
}


void LinePickerWt::setLineStipple(int lineStyle)
{
    m_LineStyle.m_Stipple = LineStyle::convertLineStyle(lineStyle);
    fillBoxes();
    repaint();
}


void LinePickerWt::setLineWidth(int width)
{
    m_LineStyle.m_Width = width;
    fillBoxes();
    repaint();
}


void LinePickerWt::enableBoxes(bool bEnable)
{
    if(bEnable)
    {
        m_plcbLineStyle->setLine( m_LineStyle);
        m_plcbLineWidth->setLine( m_LineStyle);
        m_plcbPointStyle->setLine(m_LineStyle);
        m_plbLineColor->setColor(m_LineStyle.m_Color);
        m_plbLineColor->setStipple(m_LineStyle.m_Stipple);
        m_plbLineColor->setWidth(m_LineStyle.m_Width);
        m_plbLineColor->setPointStyle(m_LineStyle.m_Symbol);
    }
    else
    {
        m_plcbLineStyle->setLine(Line::SOLID, 1, QColor(100,100,100), Line::NOSYMBOL);
        m_plcbLineWidth->setLine(Line::SOLID, 1, QColor(100,100,100), Line::NOSYMBOL);
        m_plcbPointStyle->setLine(Line::SOLID, 1, QColor(100,100,100), Line::NOSYMBOL);
        m_plbLineColor->setTheStyle(Line::SOLID, 1, QColor(100,100,100), Line::NOSYMBOL);
    }

}



