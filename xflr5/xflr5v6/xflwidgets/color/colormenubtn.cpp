/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#include <QScreen>
#include <QMenu>
#include <QPainter>
#include <QStyleOption>
#include <QWidgetAction>

#include <xflwidgets/color/colormenubtn.h>
#include <xflwidgets/color/colorpicker.h>

QColor ColorMenuBtn::s_BackgroundColor = Qt::black;

ColorMenuBtn::ColorMenuBtn(QWidget *pParent, bool bShowLeft) : QPushButton(pParent)
{
    m_bHasBackGround = false;
    m_bIsCurrent     = false;
    m_bMouseHover    = false;
    m_Color = Qt::darkGray;
    m_bShowLeft = bShowLeft;
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}


ColorMenuBtn::~ColorMenuBtn()
{
}


void ColorMenuBtn::mousePressEvent(QMouseEvent *)
{
    ColorPicker *pColorPicker = new ColorPicker;

    connect(pColorPicker, SIGNAL(colorChanged(QColor)), SLOT(onColorChanged(QColor)));
    pColorPicker->initDialog(m_Color);

    QPoint pt = mapToGlobal(rect().bottomLeft());
    if(m_bShowLeft)
    {
        pt = mapToGlobal(rect().bottomRight());
        int w = pColorPicker->width();
        pt.rx() -= w;
    }
    pColorPicker->move(pt);

//    pColorPicker->setWindowModality(Qt::WindowModal);
    pColorPicker->show();
    pColorPicker->setFocus();
}


void ColorMenuBtn::onColorChanged(QColor newclr)
{
    setColor(newclr);
    emit clickedCB(m_Color);
}


void ColorMenuBtn::setColor(QColor const & color)
{
    m_Color = color;
    update();
}


void ColorMenuBtn::paintEvent(QPaintEvent *pEvent)
{
    QColor paintcolor;
    QPalette palette;
    QColor backcolor= palette.window().color();

    if(isEnabled()) paintcolor = m_Color;
    else
    {
        if(isDown()) paintcolor = m_Color.lighter(150);
        else         paintcolor = palette.color(QPalette::Disabled, QPalette::Button);
    }

    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBackgroundMode(Qt::TransparentMode);

    QRect r = rect();


    if(m_bMouseHover && isEnabled())
    {
        backcolor = palette.highlight().color();
    }
    else if(m_bHasBackGround)
    {
        backcolor = s_BackgroundColor;
    }

    QPen blackPen(Qt::black, 1, Qt::SolidLine);
    QBrush colorbrush(paintcolor);
    painter.setBrush(colorbrush);
    painter.setPen(blackPen);
    painter.drawRoundedRect(r, 5, 25, Qt::RelativeSize);

    if(m_bIsCurrent)
    {
        QPalette myPal;
        QPen contourpen(myPal.highlight().color());
        contourpen.setStyle(Qt::DotLine);
        contourpen.setWidth(2);
        painter.setPen(contourpen);
        painter.drawRect(r.marginsRemoved(QMargins(2,2,2,2)));
        //        painter.drawRect(r);
        //        backcolor = myPal.highlight().color();
    }

    pEvent->accept();
}

