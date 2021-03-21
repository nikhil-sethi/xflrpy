/****************************************************************************

    LinePicker Class
    Copyright (C) 2009 Andre Deperrois

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

#include <QPushButton>
#include <QKeyEvent>
#include <QColorDialog>
#include <QLabel>
#include <QGridLayout>

#include "linepickerdlg.h"
#include <globals/globals.h>
#include <misc/options/settings.h>

#include "linebtn.h"
#include "linecbbox.h"
#include "linedelegate.h"


LinePickerDlg::LinePickerDlg(QWidget *pParent): QDialog(pParent)
{
    setWindowTitle(tr("Line Picker"));

    m_bAcceptPointStyle = false;
    m_PointStyle = 0;
    m_LineStyle  = 0;
    m_LineWidth  = 1;
    m_LineColor  = QColor(0,255,0);
    setupLayout();

    m_pPointStyleDelegate = new LineDelegate(m_pctrlPointStyle);
    m_pLineStyleDelegate  = new LineDelegate(m_pctrlLineStyle);
    m_pWidthDelegate      = new LineDelegate(m_pctrlLineWidth);

    m_pctrlPointStyle->setItemDelegate(m_pPointStyleDelegate);
    m_pctrlLineStyle->setItemDelegate(m_pLineStyleDelegate);
    m_pctrlLineWidth->setItemDelegate(m_pWidthDelegate);

    connect(m_pctrlPointStyle, SIGNAL(activated(int)), this, SLOT(onPointStyle(int)));
    connect(m_pctrlLineStyle, SIGNAL(activated(int)), this, SLOT(onLineStyle(int)));
    connect(m_pctrlLineWidth, SIGNAL(activated(int)), this, SLOT(onLineWidth(int)));
    connect(m_pctrlLineColor, SIGNAL(clickedLB()), this, SLOT(onLineColor()));

    connect(OKButton, SIGNAL(clicked()),this, SLOT(accept()));
    connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}


void LinePickerDlg::fillBoxes()
{
    m_pctrlLineColor->setStyle(m_LineStyle);
    m_pctrlLineColor->setWidth(m_LineWidth);
    m_pctrlLineColor->setColor(m_LineColor);


    int LineStyle[5];
    int LineWidth[5];
    int LinePoints[5];
    for (int i=0; i<5;i++)
    {
        LineWidth[i] = m_LineWidth;
        LineStyle[i] = m_LineStyle;
        LinePoints[i] = 0;
    }
    m_pLineStyleDelegate->setPointStyle(LinePoints);
    m_pLineStyleDelegate->setLineWidth(LineWidth); // the same selected width for all styles
    m_pLineStyleDelegate->setLineColor(m_LineColor);

    m_pWidthDelegate->setPointStyle(LinePoints);
    m_pWidthDelegate->setLineStyle(LineStyle); //the same selected style for all widths
    m_pWidthDelegate->setLineColor(m_LineColor);

    for (int i=0; i<5;i++) LinePoints[i]=i;
    m_pPointStyleDelegate->setPointStyle(LinePoints);
    m_pPointStyleDelegate->setLineStyle(LineStyle);
    m_pPointStyleDelegate->setLineWidth(LineWidth);
    m_pPointStyleDelegate->setLineColor(m_LineColor);

    m_pctrlPointStyle->setLine(m_LineStyle, m_LineWidth, m_LineColor, m_PointStyle);
    m_pctrlLineStyle->setLine(m_LineStyle, m_LineWidth, m_LineColor, 0);
    m_pctrlLineWidth->setLine(m_LineStyle, m_LineWidth, m_LineColor, 0);

    m_pctrlPointStyle->setCurrentIndex(m_PointStyle);
    m_pctrlLineStyle->setCurrentIndex(m_LineStyle);
    m_pctrlLineWidth->setCurrentIndex(m_LineWidth-1);
}


void LinePickerDlg::initDialog(int pointStyle, int lineStyle, int lineWidth, QColor lineColor, bool bAcceptPointStyle)
{
    m_bAcceptPointStyle = bAcceptPointStyle;
    m_PointStyle = pointStyle;
    m_LineStyle = lineStyle;
    m_LineWidth = lineWidth;
    m_LineColor = lineColor;
    initDialog();
}



void LinePickerDlg::initDialog()
{
    QString str;
    for (int i=0; i<5; i++)
    {
        str = QString("%1").arg(i);
        m_pctrlLineWidth->addItem(str);
        m_pctrlPointStyle->addItem(str);//string doesn't matter, will be replaced when the boxes are filled
        m_pctrlLineStyle->addItem(str);//string doesn't matter, will be replaced when the boxes are filled
    }

    fillBoxes();
}



void LinePickerDlg::keyPressEvent(QKeyEvent *event)
{
    // Prevent Return Key from closing App
    switch (event->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!OKButton->hasFocus())
            {
                OKButton->setFocus();
                return;
            }
            else
            {

                accept();
                return;
            }
        }
        case Qt::Key_Escape:
        {
            reject();
            return;
        }
    }
}


void LinePickerDlg::onPointStyle(int val)
{
    m_PointStyle = val;
    fillBoxes();
    repaint();
    OKButton->setFocus();
}


void LinePickerDlg::onLineStyle(int val)
{
    m_LineStyle = val;
    fillBoxes();
    repaint();
    OKButton->setFocus();
}


void LinePickerDlg::onLineWidth(int val)
{
    m_LineWidth = val+1;
    fillBoxes();
    repaint();
    OKButton->setFocus();
}


void LinePickerDlg::onLineColor()
{
    QColorDialog::ColorDialogOptions dialogOptions = QColorDialog::ShowAlphaChannel;
#ifdef Q_OS_MAC
#if QT_VERSION >= 0x040700
    dialogOptions |= QColorDialog::DontUseNativeDialog;
#endif
#endif
    QColor Color = QColorDialog::getColor(m_LineColor, this, "Color Selection", dialogOptions);
    if(Color.isValid()) m_LineColor = Color;

    fillBoxes();
    repaint();
    OKButton->setFocus();
}


void LinePickerDlg::setLineColor(QColor color)
{
    m_LineColor = color;
    fillBoxes();
    repaint();
}


void LinePickerDlg::setPointStyle(int pointStyle)
{
    m_PointStyle = pointStyle;
    fillBoxes();
    repaint();
}


void LinePickerDlg::setLineStyle(int lineStyle)
{
    m_LineStyle = lineStyle;
    fillBoxes();
    repaint();
}


void LinePickerDlg::setLineWidth(int width)
{

    m_LineWidth = width;
    fillBoxes();
    repaint();
}


void LinePickerDlg::setupLayout()
{
    QGridLayout *pStyleLayout = new QGridLayout;
    {
        QLabel *lab0 = new QLabel(tr("Points"));
        QLabel *lab1 = new QLabel(tr("Style"));
        QLabel *lab2 = new QLabel(tr("Width"));
        QLabel *lab3 = new QLabel(tr("Color"));
        lab0->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        lab1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        lab2->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        lab3->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        lab0->setMinimumWidth(60);
        lab1->setMinimumWidth(60);
        lab2->setMinimumWidth(60);
        lab3->setMinimumWidth(60);
        m_pctrlPointStyle = new LineCbBox(this);
        m_pctrlPointStyle->showPoints(true);
        m_pctrlLineStyle = new LineCbBox;
        m_pctrlLineWidth = new LineCbBox;
        m_pctrlLineColor = new LineBtn;


        QFontMetrics fm(Settings::s_TextFont);

        m_pctrlPointStyle->setMinimumWidth(17*fm.averageCharWidth());
        m_pctrlLineStyle->setMinimumWidth(17*fm.averageCharWidth());
        m_pctrlLineWidth->setMinimumWidth(17*fm.averageCharWidth());
        m_pctrlLineColor->setMinimumWidth(17*fm.averageCharWidth());
        m_pctrlLineColor->setMinimumHeight(m_pctrlLineStyle->minimumSizeHint().height());

        pStyleLayout->addWidget(lab0,1,1);
        pStyleLayout->addWidget(lab1,2,1);
        pStyleLayout->addWidget(lab2,3,1);
        pStyleLayout->addWidget(lab3,4,1);
        pStyleLayout->addWidget(m_pctrlPointStyle,1,2);
        pStyleLayout->addWidget(m_pctrlLineStyle, 2,2);
        pStyleLayout->addWidget(m_pctrlLineWidth, 3,2);
        pStyleLayout->addWidget(m_pctrlLineColor, 4,2);
    }

    QHBoxLayout *pCommandButtons = new QHBoxLayout;
    {
        OKButton = new QPushButton(tr("OK"));
        CancelButton = new QPushButton(tr("Cancel"));
        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(OKButton);
        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(CancelButton);
        pCommandButtons->addStretch(1);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addStretch(1);
        pMainLayout->addLayout(pStyleLayout);
        pMainLayout->addStretch(1);
        pMainLayout->addLayout(pCommandButtons);
        pMainLayout->addStretch(1);
    }

    setMinimumHeight(170);
    setLayout(pMainLayout);
}
















