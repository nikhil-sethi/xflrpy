/****************************************************************************

    GridSettingsDlg Class
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

#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>

#include "afoil.h"
#include "gridsettingsdlg.h"
#include <misc/line/linepickerdlg.h>
#include <misc/line/linebtn.h>
#include <misc/text/doubleedit.h>


GridSettingsDlg::GridSettingsDlg(QWidget *pParent): QDialog(pParent)
{
    setWindowTitle(tr("Grid Options"));
    m_pParent = pParent;

    m_bScale = false;

    m_bXGrid     = false;
    m_XUnit  = 0.05;
    m_XStyle = 1;
    m_XWidth = 1;
    m_XColor = QColor(150,150,150);

    m_bYGrid     = false;
    m_YUnit  = 0.05;
    m_YStyle = 1;
    m_YWidth = 1;
    m_YColor = QColor(150,150,150);

    m_bXMinGrid  = false;
    m_XMinUnit = 0.01;
    m_XMinStyle  = 2;
    m_XMinWidth  = 1;
    m_XMinColor  = QColor(70,70,70);

    m_bYMinGrid  = false;
    m_YMinUnit = 0.01;
    m_YMinStyle  = 2;
    m_YMinWidth  = 1;
    m_YMinColor  = QColor(70,70,70);

    m_NeutralStyle = 3;
    m_NeutralWidth = 1;
    m_NeutralColor = QColor(70,70,70);

    setupLayout();

    connect(OKButton, SIGNAL(clicked()),this, SLOT(onOK()));
    connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    connect(m_pctrlScale, SIGNAL(clicked()), this, SLOT(onScale()));
    connect(m_pctrlNeutralShow, SIGNAL(clicked(bool)), this, SLOT(onNeutralShow(bool)));
    connect(m_pctrlXMajShow, SIGNAL(clicked(bool)), this, SLOT(onXMajShow(bool)));
    connect(m_pctrlYMajShow, SIGNAL(clicked(bool)), this, SLOT(onYMajShow(bool)));
    connect(m_pctrlXMinShow, SIGNAL(clicked(bool)), this, SLOT(onXMinShow(bool)));
    connect(m_pctrlYMinShow, SIGNAL(clicked(bool)), this, SLOT(onYMinShow(bool)));
    connect(m_pctrlNeutralStyle, SIGNAL(clickedLB()), this, SLOT(onNeutralStyle()));
    connect(m_pctrlXMajStyle, SIGNAL(clickedLB()), this, SLOT(onXMajStyle()));
    connect(m_pctrlYMajStyle, SIGNAL(clickedLB()), this, SLOT(onYMajStyle()));
    connect(m_pctrlXMinStyle, SIGNAL(clickedLB()), this, SLOT(onXMinStyle()));
    connect(m_pctrlYMinStyle, SIGNAL(clickedLB()), this, SLOT(onYMinStyle()));

}

void GridSettingsDlg::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Escape:
        {
            done(0);
            break;
        }
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!OKButton->hasFocus() && !CancelButton->hasFocus())
            {
                OKButton->setFocus();
//                m_bApplied  = true;
            }
            else
            {
                QDialog::accept();
            }
            break;
        }
        default:
            event->ignore();
    }
}


void GridSettingsDlg::initDialog()
{
    m_pctrlNeutralStyle->setStyle(m_NeutralStyle, m_NeutralWidth, m_NeutralColor,0);

    m_pctrlXMajStyle->setStyle(m_XStyle, m_XWidth, m_XColor,0);
    m_pctrlXMinStyle->setStyle(m_XMinStyle, m_XMinWidth, m_XMinColor,0);
    m_pctrlYMajStyle->setStyle(m_YStyle, m_YWidth, m_YColor,0);
    m_pctrlYMinStyle->setStyle(m_YMinStyle, m_YMinWidth, m_YMinColor,0);

    m_pctrlNeutralStyle->setEnabled(m_bNeutralLine);
    m_pctrlXMajStyle->setEnabled(m_bXGrid);
    m_pctrlYMajStyle->setEnabled(m_bYGrid);
    m_pctrlXMinStyle->setEnabled(m_bXMinGrid);
    m_pctrlYMinStyle->setEnabled(m_bYMinGrid);
    m_pctrlXUnit->setEnabled(m_bXGrid);
    m_pctrlYUnit->setEnabled(m_bYGrid);
    m_pctrlXMinUnit->setEnabled(m_bXMinGrid);
    m_pctrlYMinUnit->setEnabled(m_bYMinGrid);

    m_pctrlScale->setChecked(m_bScale);
    m_pctrlNeutralShow->setChecked(m_bNeutralLine);
    m_pctrlXMajShow->setChecked(m_bXGrid);
    m_pctrlYMajShow->setChecked(m_bYGrid);
    m_pctrlXMinShow->setChecked(m_bXMinGrid);
    m_pctrlYMinShow->setChecked(m_bYMinGrid);

    m_pctrlXUnit->setValue(m_XUnit);
    m_pctrlYUnit->setValue(m_YUnit);
    m_pctrlXMinUnit->setValue(m_XMinUnit);
    m_pctrlYMinUnit->setValue(m_YMinUnit);
}


void GridSettingsDlg::setupLayout()
{
    QGridLayout *pGridData = new QGridLayout;
    {
        m_pctrlNeutralShow = new QCheckBox(tr("Neutral Line"));
        m_pctrlScale       = new QCheckBox(tr("X-Scale"));
        m_pctrlXMajShow = new QCheckBox(tr("X Major Grid"));
        m_pctrlYMajShow = new QCheckBox(tr("Y Major Grid"));
        m_pctrlXMinShow = new QCheckBox(tr("X Minor Grid"));
        m_pctrlYMinShow = new QCheckBox(tr("Y Minor Grid"));

        m_pctrlNeutralStyle = new LineBtn(this);
        m_pctrlXMajStyle = new LineBtn(this);
        m_pctrlYMajStyle = new LineBtn(this);
        m_pctrlXMinStyle = new LineBtn(this);
        m_pctrlYMinStyle = new LineBtn(this);

        m_pctrlXUnit = new DoubleEdit;
        m_pctrlYUnit = new DoubleEdit;
        m_pctrlXMinUnit = new DoubleEdit;
        m_pctrlYMinUnit = new DoubleEdit;
        m_pctrlXUnit->setPrecision(3);
        m_pctrlYUnit->setPrecision(3);
        m_pctrlXMinUnit->setPrecision(3);
        m_pctrlYMinUnit->setPrecision(3);

        pGridData->addWidget(m_pctrlNeutralShow,1,1);
        pGridData->addWidget(m_pctrlXMajShow,2,1);
        pGridData->addWidget(m_pctrlYMajShow,3,1);
        pGridData->addWidget(m_pctrlXMinShow,4,1);
        pGridData->addWidget(m_pctrlYMinShow,5,1);

        pGridData->addWidget(m_pctrlNeutralStyle,1,2);
        pGridData->addWidget(m_pctrlXMajStyle,2,2);
        pGridData->addWidget(m_pctrlYMajStyle,3,2);
        pGridData->addWidget(m_pctrlXMinStyle,4,2);
        pGridData->addWidget(m_pctrlYMinStyle,5,2);

        pGridData->addWidget(m_pctrlScale,1,3);
        pGridData->addWidget(m_pctrlXUnit,2,3);
        pGridData->addWidget(m_pctrlYUnit,3,3);
        pGridData->addWidget(m_pctrlXMinUnit,4,3);
        pGridData->addWidget(m_pctrlYMinUnit,5,3);
    }

    QHBoxLayout *pCommandButtons = new QHBoxLayout;
    {
        OKButton      = new QPushButton(tr("Accept"));
        CancelButton  = new QPushButton(tr("Cancel"));

        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(OKButton);
        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(CancelButton);
        pCommandButtons->addStretch(1);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addLayout(pGridData);
        pMainLayout->addLayout(pCommandButtons);
    }
    setLayout(pMainLayout);
}


void GridSettingsDlg::onNeutralStyle()
{
    LinePickerDlg dlg(this);
    dlg.initDialog(0,m_NeutralStyle,m_NeutralWidth,m_NeutralColor);

    if(QDialog::Accepted==dlg.exec())
    {
        m_NeutralStyle = dlg.lineStyle();
        m_NeutralWidth = dlg.lineWidth();
        m_NeutralColor = dlg.lineColor();
        m_pctrlNeutralStyle->setStyle(dlg.lineStyle());
        m_pctrlNeutralStyle->setWidth(dlg.lineWidth());
        m_pctrlNeutralStyle->setColor(dlg.lineColor());
    }
}


void GridSettingsDlg::onXMajStyle()
{
    LinePickerDlg dlg(this);
    dlg.initDialog(0,m_XStyle,m_XWidth,m_XColor);

    if(QDialog::Accepted==dlg.exec())
    {
        m_XStyle = dlg.lineStyle();
        m_XWidth = dlg.lineWidth();
        m_XColor = dlg.lineColor();
        m_pctrlXMajStyle->setStyle(dlg.lineStyle());
        m_pctrlXMajStyle->setWidth(dlg.lineWidth());
        m_pctrlXMajStyle->setColor(dlg.lineColor());
    }
}

void GridSettingsDlg::onXMinStyle()
{
    LinePickerDlg dlg(this);
    dlg.initDialog(0,m_XMinStyle,m_XMinWidth,m_XMinColor);

    if(QDialog::Accepted==dlg.exec())
    {
        m_XMinStyle = dlg.lineStyle();
        m_XMinWidth = dlg.lineWidth();
        m_XMinColor = dlg.lineColor();
        m_pctrlXMinStyle->setStyle(dlg.lineStyle());
        m_pctrlXMinStyle->setWidth(dlg.lineWidth());
        m_pctrlXMinStyle->setColor(dlg.lineColor());
    }

}

void GridSettingsDlg::onYMajStyle()
{
    LinePickerDlg dlg(this);

    dlg.initDialog(0,m_YStyle,m_YWidth,m_YColor);

    if(QDialog::Accepted==dlg.exec())
    {
        m_YStyle = dlg.lineStyle();
        m_YWidth = dlg.lineWidth();
        m_YColor = dlg.lineColor();
        m_pctrlYMajStyle->setStyle(dlg.lineStyle());
        m_pctrlYMajStyle->setWidth(dlg.lineWidth());
        m_pctrlYMajStyle->setColor(dlg.lineColor());
    }
}

void GridSettingsDlg::onYMinStyle()
{
    LinePickerDlg dlg(this);

    dlg.initDialog(0,m_YMinStyle,m_YMinWidth,m_YMinColor);

    if(QDialog::Accepted==dlg.exec())
    {
        m_YMinStyle = dlg.lineStyle();
        m_YMinWidth = dlg.lineWidth();
        m_YMinColor = dlg.lineColor();
        m_pctrlYMinStyle->setStyle(dlg.lineStyle());
        m_pctrlYMinStyle->setWidth(dlg.lineWidth());
        m_pctrlYMinStyle->setColor(dlg.lineColor());
    }
}

void GridSettingsDlg::onNeutralShow(bool bShow)
{
    m_bNeutralLine = bShow;
    m_pctrlNeutralStyle->setEnabled(m_bNeutralLine);
}

void GridSettingsDlg::onScale()
{
    m_bScale = m_pctrlScale->isChecked();
}


void GridSettingsDlg::onXMajShow(bool bShow)
{
    m_bXGrid = bShow;
    m_pctrlXMajStyle->setEnabled(m_bXGrid);
    m_pctrlXUnit->setEnabled(m_bXGrid);
}





void GridSettingsDlg::onYMajShow(bool bShow)
{
    m_bYGrid = bShow;
    m_pctrlYMajStyle->setEnabled(m_bYGrid);
    m_pctrlYUnit->setEnabled(m_bYGrid);
}


void GridSettingsDlg::onXMinShow(bool bShow)
{
    m_bXMinGrid = bShow;
    m_pctrlXMinStyle->setEnabled(m_bXMinGrid);
    m_pctrlXMinUnit->setEnabled(m_bXMinGrid);
}


void GridSettingsDlg::onYMinShow(bool bShow)
{
    m_bYMinGrid = bShow;
    m_pctrlYMinStyle->setEnabled(m_bYMinGrid);
    m_pctrlYMinUnit->setEnabled(m_bYMinGrid);
}

void GridSettingsDlg::onOK()
{
    m_XUnit = m_pctrlXUnit->value();
    m_YUnit = m_pctrlYUnit->value();
    m_XMinUnit = m_pctrlXMinUnit->value();
    m_YMinUnit = m_pctrlYMinUnit->value();
    accept();
}





