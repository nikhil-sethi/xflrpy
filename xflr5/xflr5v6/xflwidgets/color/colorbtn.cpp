/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#include <QMenu>
#include <QPainter>
#include <QStyleOption>
#include <QWidgetAction>

#include <xflwidgets/color/colorbtn.h>
#include <xflwidgets/color/colorpicker.h>

ColorBtn::ColorBtn(QWidget *pParent) : QAbstractButton(pParent)
{
    m_bHasBackGround = false;
    m_bIsCurrent     = false;
    m_bMouseHover    = false;
    m_Color = Qt::darkGray;

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}


QSize ColorBtn::sizeHint() const
{
    QFont font; // the application's default font, avoid custom font to ensure fixed size widget
    QFontMetrics fm(font);
    int w = 3*fm.averageCharWidth();
    int h = fm.height();
    return QSize(w, h);
}


void ColorBtn::setColor(QColor const & color)
{
    m_Color = color;
    update();
}


void ColorBtn::mousePressEvent(QMouseEvent *pEvent)
{
    if (pEvent->button() == Qt::LeftButton)
    {
        emit clicked(false);
        emit clickedCB(m_Color);
        pEvent->accept();
        return;
    }
    else
    {
        QAbstractButton::mousePressEvent(pEvent);
    }
}


void ColorBtn::onSetColor(QColor clr)
{
    m_Color = clr;
    update();
}


bool ColorBtn::event(QEvent* pEvent)
{
    if (pEvent->type() == QEvent::Enter)
    {
        m_bMouseHover = true;
        update();
    }
    if (pEvent->type()==QEvent::Leave)
    {
        m_bMouseHover = false;
        update();
    }
    return QWidget::event(pEvent); // Or whatever parent class you have.
}


void ColorBtn::paintEvent(QPaintEvent *pEvent)
{
    QColor paintcolor;

    if(isEnabled()) paintcolor = m_Color;
    else
    {
        if(isDown()) paintcolor = m_Color.lighter(150);
        else         paintcolor = Qt::lightGray;
    }

    paintcolor.setAlpha(255);
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBackgroundMode(Qt::TransparentMode);
    QRect r = rect();


    QPen blackPen(Qt::black, 1, Qt::SolidLine);
    QBrush colorbrush(paintcolor);
    painter.setBrush(colorbrush);
    painter.setPen(blackPen);
    painter.drawRoundedRect(r, 5, 25, Qt::RelativeSize);

    QPalette myPal;

    if(m_bMouseHover && isEnabled())
    {
        QPen contourpen(myPal.highlight().color());
        contourpen.setStyle(Qt::SolidLine);
        contourpen.setWidth(2);
        painter.setPen(contourpen);
        painter.drawRect(r.marginsRemoved(QMargins(2,2,2,2)));
    }
    else if(m_bIsCurrent)
    {
        QPen contourpen(myPal.highlight().color());
        contourpen.setStyle(Qt::DotLine);
        contourpen.setWidth(3);
        painter.setPen(contourpen);
        painter.drawRect(r.marginsRemoved(QMargins(2,2,2,2)));
    }

    pEvent->accept();
}

