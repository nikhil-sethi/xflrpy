/****************************************************************************

    TextBtn Class
    Copyright (C) 2013-2016 Andre Deperrois 

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



#include <QPen>
#include <QPainter>
#include <QFontMetrics>

#include <misc/options/settings.h>
#include <xflcore/xflcore.h>
#include <xflwidgets/text/textclrbtn.h>




TextClrBtn::TextClrBtn(QWidget *parent)
    : QAbstractButton(parent)
{
    QSizePolicy szPolicyExpanding;
    szPolicyExpanding.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
    szPolicyExpanding.setVerticalPolicy(QSizePolicy::Minimum);
    setSizePolicy(szPolicyExpanding);

    m_TextColor = Qt::red;
    m_BackgroundColor = Qt::green;

    m_TextFont.setStyleHint(QFont::Cursive, QFont::OpenGLCompatible);
    m_TextFont.setFamily(m_TextFont.defaultFamily());
    m_TextFont.setPointSize(9);

}



void TextClrBtn::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
            emit clickedTB();
    }
    else
        QWidget::mouseReleaseEvent(event);
}


QSize TextClrBtn::sizeHint() const
{
    QFontMetrics fm(Settings::s_TextFont);
    int w = 13 * fm.averageCharWidth();
    int h = fm.height();
    return QSize(w, h);
}


void TextClrBtn::setTextColor(QColor const & color)
{
    m_TextColor = color;
    update();
}


void TextClrBtn::setBackgroundColor(QColor const & color)
{
    m_BackgroundColor = color;
    update();
}


void TextClrBtn::setFont(QFont const & font)
{
    m_TextFont = font;
    update();
}


void TextClrBtn::paintEvent(QPaintEvent *event)
{
    QRect r = rect();

    QPainter painter(this);
    painter.save();

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setFont(m_TextFont);

/*    QStyleOption opt;
    opt.initFrom(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);*/

    QFontMetrics fm(m_TextFont);
    int w = fm.horizontalAdvance(text());

    QBrush backBrush(m_BackgroundColor);
    painter.setBrush(backBrush);
    painter.setBackgroundMode(Qt::TransparentMode);

    QColor ContourColor = Qt::lightGray;
    if(isEnabled())
    {
        if(isDown()) ContourColor = Qt::red;
        else         ContourColor = Qt::gray;
    }

    QPen ContourPen(ContourColor);
    painter.setPen(ContourPen);
    r.adjust(0,2,-1,-3);
    painter.drawRoundedRect(r,5,40);

    QPen LinePen(m_TextColor);
    painter.setPen(LinePen);
    painter.drawText(r.center().x()-w/2, r.height() * 7/8, text());

    painter.restore();

    event->accept();
}
