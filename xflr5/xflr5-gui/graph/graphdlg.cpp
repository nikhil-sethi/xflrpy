/****************************************************************************

    GraphDlg  Classes
    Copyright (C) 2009-2019 Andre Deperrois

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


#include <QFontDialog>
#include <QColorDialog>
#include <QPalette>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QDebug>

#include <graph/graph.h>
#include <graph/graphdlg.h>
#include <miarex/miarex.h>
#include <misc/color/colorbutton.h>
#include <misc/line/linebtn.h>
#include <misc/line/linepickerdlg.h>
#include <misc/text/doubleedit.h>
#include <misc/text/intedit.h>
#include <misc/text/textclrbtn.h>
#include <objects/objects2d/polar.h>
#include <objects/objects3d/wpolar.h>


int GraphDlg::s_iActivePage = 0;


GraphDlg::GraphDlg(QWidget *pParent): QDialog(pParent)
{
    setWindowTitle(tr("Graph Settings"));

    m_pParent = pParent;

    m_pGraph    = nullptr;
    m_bApplied = true;
    m_bVariableChanged = false;

    m_XSel = 0;
    m_YSel = 1;

    m_pTitleFont = m_pLabelFont = nullptr;

    setupLayout();
    connectSignals();
}


void GraphDlg::connectSignals()
{
    connect(m_pctrlTitleClr, SIGNAL(clickedTB()),  this, SLOT(onTitleColor()));
    connect(m_pctrlLabelClr, SIGNAL(clickedTB()),  this, SLOT(onLabelColor()));

    connect(m_pctrlTitleButton, SIGNAL(clicked()),  this, SLOT(onTitleFont()));
    connect(m_pctrlLabelButton, SIGNAL(clicked()),  this, SLOT(onLabelFont()));

    connect(m_pctrlXAuto, SIGNAL(clicked()), this, SLOT(onAutoX()));
    connect(m_pctrlYAuto, SIGNAL(clicked()), this, SLOT(onAutoY()));
    connect(m_pctrlYInverted, SIGNAL(clicked()), this, SLOT(onYInverted()));

    connect(m_pctrlXMajGridShow, SIGNAL(stateChanged(int)), this, SLOT(onXMajGridShow(int)));
    connect(m_pctrlYMajGridShow, SIGNAL(stateChanged(int)), this, SLOT(onYMajGridShow(int)));
    connect(m_pctrlXMinGridShow, SIGNAL(stateChanged(int)), this, SLOT(onXMinGridShow(int)));
    connect(m_pctrlYMinGridShow, SIGNAL(stateChanged(int)), this, SLOT(onYMinGridShow(int)));

    connect(m_pctrlAxisStyle, SIGNAL(clickedLB()), this, SLOT(onAxisStyle()));
    connect(m_pctrlXMajGridStyle, SIGNAL(clickedLB()), this, SLOT(onXMajGridStyle()));
    connect(m_pctrlYMajGridStyle, SIGNAL(clickedLB()), this, SLOT(onYMajGridStyle()));
    connect(m_pctrlXMinGridStyle, SIGNAL(clickedLB()), this, SLOT(onXMinGridStyle()));
    connect(m_pctrlYMinGridStyle, SIGNAL(clickedLB()), this, SLOT(onYMinGridStyle()));

    connect(m_pctrlAutoXMinUnit, SIGNAL(clicked()), this, SLOT(onAutoMinGrid()));
    connect(m_pctrlAutoYMinUnit, SIGNAL(clicked()), this, SLOT(onAutoMinGrid()));

    connect(m_pctrlGraphBorder, SIGNAL(stateChanged(int)), this, SLOT(onGraphBorder(int)));
    connect(m_pctrlGraphBack, SIGNAL(clicked()), this, SLOT(onGraphBackColor()));
    connect(m_pctrlBorderStyle, SIGNAL(clicked()), this, SLOT(onBorderStyle()));

    /*    connect(m_pctrlXSel, SIGNAL(itemActivated ( QListWidgetItem*)), SLOT(OnVariableChanged()));
    connect(m_pctrlYSel, SIGNAL(itemActivated ( QListWidgetItem*)), SLOT(OnVariableChanged()));
    connect(m_pctrlXSel, SIGNAL(itemClicked ( QListWidgetItem*)), SLOT(OnVariableChanged()));
    connect(m_pctrlYSel, SIGNAL(itemClicked ( QListWidgetItem*)), SLOT(OnVariableChanged()));*/
    connect(m_pctrlXSel, SIGNAL(itemSelectionChanged()), SLOT(onVariableChanged()));
    connect(m_pctrlYSel, SIGNAL(itemSelectionChanged()), SLOT(onVariableChanged()));

    connect(m_pctrlXSel, SIGNAL(itemDoubleClicked (QListWidgetItem *)), SLOT(onOK()));
    connect(m_pctrlYSel, SIGNAL(itemDoubleClicked (QListWidgetItem *)), SLOT(onOK()));
}




void GraphDlg::fillVariableList()
{
    m_pctrlXSel->clear();
    m_pctrlYSel->clear();

    switch(m_pGraph->graphType())
    {
        case  GRAPH::INVERSEGRAPH:
        {
            m_pctrlXSel->addItem("X - Chord");
            m_pctrlYSel->addItem("Q - Velocity");
            break;
        }
        case GRAPH::OPPGRAPH:
        {
            //foil oppoint graph variables
            m_pctrlXSel->addItem("X - chord");
            m_pctrlYSel->addItem("Cp");
            m_pctrlYSel->addItem("Q - Velocity");
            m_pctrlYSel->addItem("sqrt(Max. Shear Coefficient)");
            m_pctrlYSel->addItem("Top Side D* and Theta");
            m_pctrlYSel->addItem("Bottom Side D* and Theta");
            m_pctrlYSel->addItem("Log(Re_Theta)");
            m_pctrlYSel->addItem("Re_Theta");
            m_pctrlYSel->addItem("Amplification Ratio");
            m_pctrlYSel->addItem("Dissipation Coefficient");
            m_pctrlYSel->addItem("Wall shear stress");
            m_pctrlYSel->addItem("Edge Velocity");
            m_pctrlYSel->addItem("Kinematic Shape Parameter");
            break;
        }
        case GRAPH::POLARGRAPH:
        {
            //foil polar graph variables
            for(int iVar=0; iVar<15; iVar++)
            {
                m_pctrlXSel->addItem(Polar::variableName(iVar));
                m_pctrlYSel->addItem(Polar::variableName(iVar));
            }
            break;
        }
        case GRAPH::POPPGRAPH:
        {
            //wing graph variable
            m_pctrlXSel->addItem(tr("Y - span"));

            m_pctrlYSel->addItem(tr("Induced Angle"));                        //0
            m_pctrlYSel->addItem(tr("Total Angle"));                        //1
            m_pctrlYSel->addItem(tr("Local lift coef."));                    //2
            m_pctrlYSel->addItem(tr("Local Lift C.Cl/M.A.C."));                //3
            m_pctrlYSel->addItem(tr("Airfoil viscous drag coef."));            //4
            m_pctrlYSel->addItem(tr("Induced drag coef."));                    //5
            m_pctrlYSel->addItem(tr("Total drag coef."));                    //6
            m_pctrlYSel->addItem(tr("Local Drag C.Cd/M.A.C."));              //7
            m_pctrlYSel->addItem(tr("Airfoil Pitching moment coef."));       //8
            m_pctrlYSel->addItem(tr("Total Pitching moment coef."));         //9
            m_pctrlYSel->addItem(tr("Reynolds"));                            //10
            m_pctrlYSel->addItem(tr("Top Transition x-pos%"));               //11
            m_pctrlYSel->addItem(tr("Bottom Transition x-pos%"));            //12
            m_pctrlYSel->addItem(tr("Centre of Pressure x-pos%"));           //13
            m_pctrlYSel->addItem(tr("Bending moment"));                      //14
            break;
        }
        case GRAPH::WPOLARGRAPH:
        {
            //WingPolar Graph Variables
            for(int iVar=0; iVar<50; iVar++)
            {
                m_pctrlXSel->addItem(Miarex::WPolarVariableName(iVar));
                m_pctrlYSel->addItem(Miarex::WPolarVariableName(iVar));
            }
            break;
        }
        case GRAPH::CPGRAPH:
        {
            m_pctrlXSel->addItem(tr("X - Chord"));
            m_pctrlYSel->addItem(tr("Cp"));
            break;
        }
        case GRAPH::STABTIMEGRAPH:
        {
            m_pctrlXSel->addItem(tr("X - Chord"));
            m_pctrlYSel->addItem(tr("Cp"));
            break;
        }
        case GRAPH::OTHERGRAPH:
            return;
    }
    m_pctrlXSel->adjustSize();
    m_pctrlYSel->adjustSize();
}


void GraphDlg::reject()
{
    m_pGraph->copySettings(&m_SaveGraph);
    done(QDialog::Rejected);
}



void GraphDlg::showEvent(QShowEvent *event)
{
    m_pTabWidget->setCurrentIndex(s_iActivePage);
    event->ignore();
}


void GraphDlg::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
                m_pButtonBox->setFocus();
                return;
            }
            else
            {
                onOK();
                return;
            }
            break;
        }
        case Qt::Key_Escape:
        {
            reject();
            break;
        }
        default:
            event->ignore();
    }
}


void GraphDlg::onActivePage(int index)
{
    s_iActivePage = index;
}




void GraphDlg::onAutoMinGrid()
{
    bool bAuto;
    bAuto = m_pctrlAutoXMinUnit->isChecked();
    m_pGraph->setAutoXMinUnit(bAuto);
    m_pctrlXMinorUnit->setEnabled(!bAuto);

    bAuto = m_pctrlAutoYMinUnit->isChecked();
    m_pGraph->setAutoYMinUnit(bAuto);
    m_pctrlYMinorUnit->setEnabled(!bAuto);
}


void GraphDlg::onAutoX()
{
    bool bAuto = m_pctrlXAuto->checkState() == Qt::Checked;
    m_pctrlXMin->setEnabled(!bAuto);
    m_pctrlXMax->setEnabled(!bAuto);
    m_pctrlXUnit->setEnabled(!bAuto);
    m_pctrlXOrigin->setEnabled(!bAuto);
    setApplied(false);
}


void GraphDlg::onAutoY()
{
    bool bAuto = m_pctrlYAuto->checkState() == Qt::Checked;
    m_pctrlYMin->setEnabled(!bAuto);
    m_pctrlYMax->setEnabled(!bAuto);
    m_pctrlYUnit->setEnabled(!bAuto);
    m_pctrlYOrigin->setEnabled(!bAuto);
    setApplied(false);
}

void GraphDlg::onAxisStyle()
{
    LinePickerDlg dlg(this);
    dlg.initDialog(0, m_pGraph->axisStyle(), m_pGraph->axisWidth(), m_pGraph->axisColor());

    if(QDialog::Accepted==dlg.exec())
    {
        m_pGraph->setAxisData(dlg.lineStyle(), dlg.lineWidth(), dlg.lineColor());
        m_pctrlAxisStyle->setStyle(dlg.lineStyle());
        m_pctrlAxisStyle->setWidth(dlg.lineWidth());
        m_pctrlAxisStyle->setColor(dlg.lineColor());
        setApplied(false);
    }
}

void GraphDlg::onBorderStyle()
{
    LinePickerDlg dlg(this);
    int s,w;
    QColor color;
    s = m_pGraph->borderStyle();
    w = m_pGraph->borderWidth();
    color = m_pGraph->borderColor();
    dlg.initDialog(0,s,w,color);

    if(QDialog::Accepted==dlg.exec())
    {
        m_pGraph->setBorderColor(dlg.lineColor());
        m_pGraph->setBorderStyle(dlg.lineStyle());
        m_pGraph->setBorderWidth(dlg.lineWidth());
        m_pctrlBorderStyle->setStyle(dlg.lineStyle());
        m_pctrlBorderStyle->setWidth(dlg.lineWidth());
        m_pctrlBorderStyle->setColor(dlg.lineColor());
        setApplied(false);
    }
}

void GraphDlg::onGraphBorder(int state)
{
    bool bShow = (state==Qt::Checked);
    m_pGraph->setBorder(bShow);
    setApplied(false);
}


void GraphDlg::onGraphBackColor()
{
    QColor BkColor = m_pGraph->backgroundColor();
    BkColor = QColorDialog::getColor(BkColor);
    if(BkColor.isValid()) m_pGraph->setBkColor(BkColor);

    m_pctrlGraphBack->setColor(m_pGraph->backgroundColor());
    setButtonColors();
    setApplied(false);
}


void GraphDlg::onLabelColor()
{
    QColor color = m_pGraph->labelColor();
    color = QColorDialog::getRgba(color.rgba());

    m_pGraph->setLabelColor(color);
    m_pctrlLabelClr->setTextColor(color);

    setApplied(false);
    update();

}



void GraphDlg::onLabelFont()
{
    bool ok;
    QFont LabelFont("Courier");
    m_pGraph->getLabelFont(LabelFont);
    QFont font = QFontDialog::getFont(&ok, LabelFont, this);

    if (ok)
    {
        m_pctrlLabelButton->setFont(font);
        m_pctrlLabelButton->setText(font.family()+QString(" %1").arg(font.pointSize()));
        m_pctrlLabelClr->setFont(font);
        m_pGraph->setLabelFont(font);
        setApplied(false);
    }
}


void GraphDlg::onButton(QAbstractButton *pButton)
{
    if (     m_pButtonBox->button(QDialogButtonBox::Ok)      == pButton)  onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)  reject();
    else if (m_pButtonBox->button(QDialogButtonBox::Reset)   == pButton)  onRestoreParams();
}


void GraphDlg::onOK()
{
    applyChanges();

    m_XSel = m_pctrlXSel->currentRow();
    m_YSel = m_pctrlYSel->currentRow();
    m_pGraph->setVariables(m_pctrlXSel->currentRow(), m_pctrlYSel->currentRow());

    accept();
}


void GraphDlg::applyChanges()
{
    m_pGraph->setAutoX(m_pctrlXAuto->isChecked());
    m_pGraph->setXMin(m_pctrlXMin->value());
    m_pGraph->setXMax(m_pctrlXMax->value());
    m_pGraph->setX0(m_pctrlXOrigin->value());
    m_pGraph->setXUnit(m_pctrlXUnit->value());

    m_pGraph->setAutoY(m_pctrlYAuto->isChecked());
    m_pGraph->setYMin(m_pctrlYMin->value());
    m_pGraph->setYMax(m_pctrlYMax->value());
    m_pGraph->setY0(m_pctrlYOrigin->value());
    m_pGraph->setYUnit(m_pctrlYUnit->value());

    double MinUnit;
    if(!m_pctrlAutoXMinUnit->isChecked())
    {
        MinUnit = m_pctrlXMinorUnit->value();
        m_pGraph->setXMinorUnit(MinUnit);
        m_pGraph->setAutoXMinUnit(false);
    }
    else
        m_pGraph->setAutoXMinUnit(true);

    if(!m_pctrlAutoYMinUnit->isChecked())
    {
        MinUnit = m_pctrlYMinorUnit->value();
        m_pGraph->setYMinorUnit(MinUnit);
        m_pGraph->setAutoYMinUnit(false);
    }
    else
        m_pGraph->setAutoYMinUnit(true);

    m_pGraph->setMargin(m_pctrlMargin->value());

}


void GraphDlg::onRestoreParams()
{
    m_pGraph->copySettings(&m_SaveGraph);

    setControls();
    setApplied(true);

    if(m_pParent) m_pParent->update();
}


void GraphDlg::onTitleColor()
{
    QColor color = m_pGraph->titleColor();
    color = QColorDialog::getRgba(color.rgba());

    m_pGraph->setTitleColor(color);
    m_pctrlTitleClr->setTextColor(color);

    setApplied(false);
    update();
}


void GraphDlg::onTitleFont()
{
    bool bOk;
    QFont TitleFont("Arial");
    m_pGraph->getTitleFont(TitleFont);

    QFont font = QFontDialog::getFont(&bOk, TitleFont, this);

    if (bOk)
    {
        m_pctrlTitleButton->setFont(font);
        m_pctrlTitleButton->setText(font.family()+QString(" %1").arg(font.pointSize()));
        m_pctrlTitleClr->setFont(font);
        m_pGraph->setTitleFont(font);
        setApplied(false);
    }
}


void GraphDlg::onVariableChanged()
{
    m_bVariableChanged = true;
}


void GraphDlg::onMargin()
{
    m_pGraph->setMargin(m_pctrlMargin->value());
}


void GraphDlg::onXMajGridStyle()
{
    LinePickerDlg dlg(this);
    int s,w;
    QColor color;
    bool bShow;
    m_pGraph->bXMajGrid(bShow,color,s,w);
    dlg.initDialog(0,s,w,color);

    if(QDialog::Accepted==dlg.exec())
    {
        m_pGraph->setXMajGrid(bShow, dlg.lineColor(), dlg.lineStyle(), dlg.lineWidth());
        m_pctrlXMajGridStyle->setStyle(dlg.lineStyle());
        m_pctrlXMajGridStyle->setWidth(dlg.lineWidth());
        m_pctrlXMajGridStyle->setColor(dlg.lineColor());
        setApplied(false);
    }
}

void GraphDlg::onXMinGridStyle()
{
    LinePickerDlg dlg(this);
    int s,w;
    QColor color;
    bool bShow, bAuto;
    double unit;
    m_pGraph->bXMinGrid(bShow, bAuto,color,s,w,unit);
    dlg.initDialog(0,s,w,color);

    if(QDialog::Accepted==dlg.exec())
    {
        m_pGraph->setXMinGrid(bShow, bAuto, dlg.lineColor(), dlg.lineStyle(), dlg.lineWidth(),unit);
        m_pctrlXMinGridStyle->setStyle(dlg.lineStyle());
        m_pctrlXMinGridStyle->setWidth(dlg.lineWidth());
        m_pctrlXMinGridStyle->setColor(dlg.lineColor());
        setApplied(false);
    }

}


void GraphDlg::onXMajGridShow(int state)
{
    bool bShow = (state==Qt::Checked);
    m_pGraph->setXMajGrid(bShow);
    m_pctrlXMajGridStyle->setEnabled(bShow);
    setApplied(false);
}


void GraphDlg::onXMinGridShow(int state)
{
    bool bShow = (state==Qt::Checked);
    m_pGraph->setXMinGrid(bShow);
    m_pctrlXMinGridStyle->setEnabled(bShow);
    m_pctrlAutoXMinUnit->setEnabled(bShow);
    m_pctrlXMinorUnit->setEnabled(bShow && !m_pGraph->bAutoXMin());

    setApplied(false);
}


void GraphDlg::onYInverted()
{
    m_pGraph->setInverted(m_pctrlYInverted->checkState() == Qt::Checked);
    setApplied(false);
}


void GraphDlg::onYMajGridShow(int state)
{
    bool bShow = (state==Qt::Checked);
    m_pGraph->setYMajGrid(bShow);
    m_pctrlYMajGridStyle->setEnabled(bShow);
    setApplied(false);
}

void GraphDlg::onYMajGridStyle()
{
    LinePickerDlg dlg(this);
    int s,w;
    QColor color;
    bool bShow;
    m_pGraph->yMajGrid(bShow,color,s,w);
    dlg.initDialog(0,s,w,color);

    if(QDialog::Accepted==dlg.exec())
    {
        m_pGraph->setYMajGrid(bShow, dlg.lineColor(), dlg.lineStyle(), dlg.lineWidth());
        m_pctrlYMajGridStyle->setStyle(dlg.lineStyle());
        m_pctrlYMajGridStyle->setWidth(dlg.lineWidth());
        m_pctrlYMajGridStyle->setColor(dlg.lineColor());
    }
}


void GraphDlg::onYMinGridShow(int state)
{
    bool bShow = (state==Qt::Checked);
    m_pGraph->setYMinGrid(bShow);
    m_pctrlYMinGridStyle->setEnabled(bShow);
    m_pctrlAutoYMinUnit->setEnabled(bShow);
    m_pctrlYMinorUnit->setEnabled(bShow && !m_pGraph->bAutoYMin());

    setApplied(false);
}


void GraphDlg::onYMinGridStyle()
{
    LinePickerDlg dlg(this);
    int s,w;
    QColor color;
    bool bShow, bAuto;
    double unit;
    m_pGraph->bYMinGrid(bShow, bAuto,color,s,w,unit);
    dlg.initDialog(0,s,w,color);

    if(QDialog::Accepted==dlg.exec())
    {
        m_pGraph->setYMinGrid(bShow, bAuto, dlg.lineColor(), dlg.lineStyle(), dlg.lineWidth(),unit);
        m_pctrlYMinGridStyle->setStyle(dlg.lineStyle());
        m_pctrlYMinGridStyle->setWidth(dlg.lineWidth());
        m_pctrlYMinGridStyle->setColor(dlg.lineColor());
        setApplied(false);
    }
}


void GraphDlg::setApplied(bool bApplied)
{
    m_bApplied = bApplied;
    //    ApplyButton->setEnabled(!bApplied);
}


void GraphDlg::setButtonColors()
{
    m_pctrlTitleClr->setTextColor(m_pGraph->titleColor());
    m_pctrlTitleClr->setBackgroundColor(m_pGraph->backgroundColor());

    m_pctrlLabelClr->setTextColor(m_pGraph->labelColor());
    m_pctrlLabelClr->setBackgroundColor(m_pGraph->backgroundColor());
}


void GraphDlg::setControls()
{
    m_pctrlXAuto->setChecked(m_pGraph->bAutoX());
    m_pctrlYAuto->setChecked(m_pGraph->bAutoY());

    m_pctrlXMin->setValue(m_pGraph->xMin());
    m_pctrlXMax->setValue(m_pGraph->xMax());
    m_pctrlXOrigin->setValue(m_pGraph->xOrigin());
    m_pctrlXUnit->setValue(m_pGraph->xUnit());
    m_pctrlYMin->setValue(m_pGraph->yMin());
    m_pctrlYMax->setValue(m_pGraph->yMax());
    m_pctrlYOrigin->setValue(m_pGraph->yOrigin());
    m_pctrlYUnit->setValue(m_pGraph->yUnit());

    onAutoX();
    onAutoY();

    setButtonColors();

    QFont font;
    m_pGraph->getLabelFont(font);
    m_pctrlLabelButton->setText(font.family()+QString(" %1").arg(font.pointSize()));
    m_pctrlLabelButton->setFont(font);

    m_pGraph->getTitleFont(font);
    m_pctrlTitleButton->setText(font.family()+QString(" %1").arg(font.pointSize()));
    m_pctrlTitleButton->setFont(font);

    bool bState, bAuto;
    QColor color;
    int style, width;
    double unit;

    m_pGraph->bXMajGrid(bState, color, style, width);
    m_pctrlXMajGridShow->setChecked(bState);
    m_pctrlXMajGridStyle->setColor(color);
    m_pctrlXMajGridStyle->setStyle(style);
    m_pctrlXMajGridStyle->setWidth(width);
    m_pctrlXMajGridStyle->setEnabled(bState);

    m_pGraph->bXMinGrid(bState, bAuto,color, style, width, unit);
    m_pctrlXMinGridShow->setChecked(bState);
    m_pctrlXMinGridStyle->setColor(color);
    m_pctrlXMinGridStyle->setStyle(style);
    m_pctrlXMinGridStyle->setWidth(width);
    m_pctrlXMinGridStyle->setEnabled(bState);
    m_pctrlXMinorUnit->setValue(unit);
    m_pctrlAutoXMinUnit->setChecked(bAuto);
    m_pctrlAutoXMinUnit->setEnabled(bState);
    m_pctrlXMinorUnit->setEnabled(!bAuto && bState);

    m_pGraph->yMajGrid(bState, color, style, width);
    m_pctrlYMajGridShow->setChecked(bState);
    m_pctrlYMajGridStyle->setColor(color);
    m_pctrlYMajGridStyle->setStyle(style);
    m_pctrlYMajGridStyle->setWidth(width);
    m_pctrlYMajGridStyle->setEnabled(bState);

    m_pGraph->bYMinGrid(bState, bAuto,color, style, width, unit);
    m_pctrlYMinGridShow->setChecked(bState);
    m_pctrlYMinGridStyle->setColor(color);
    m_pctrlYMinGridStyle->setStyle(style);
    m_pctrlYMinGridStyle->setWidth(width);
    m_pctrlYMinGridStyle->setEnabled(bState);
    m_pctrlYMinorUnit->setValue(unit);
    m_pctrlAutoYMinUnit->setChecked(bAuto);
    m_pctrlAutoYMinUnit->setEnabled(bState);
    m_pctrlYMinorUnit->setEnabled(!bAuto && bState);

    m_pctrlAxisStyle->setColor(m_pGraph->axisColor());
    m_pctrlAxisStyle->setStyle(m_pGraph->axisStyle());
    m_pctrlAxisStyle->setWidth(m_pGraph->axisWidth());

    m_pctrlGraphBorder->setChecked(m_pGraph->hasBorder());
    m_pctrlBorderStyle->setColor(m_pGraph->borderColor());
    m_pctrlBorderStyle->setStyle(m_pGraph->borderStyle());
    m_pctrlBorderStyle->setWidth(m_pGraph->borderWidth());

    m_pctrlGraphBack->setColor(m_pGraph->backgroundColor());

    m_pctrlMargin->setValue(m_pGraph->margin());

    m_pctrlYInverted->setChecked(m_pGraph->bInverted());

    fillVariableList();

    m_pctrlXSel->setCurrentRow(m_pGraph->xVariable());
    m_pctrlYSel->setCurrentRow(m_pGraph->yVariable());
    m_bVariableChanged = false;

    setApplied(true);
}



void GraphDlg::setupLayout()
{
    QFontMetrics fm(font());

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Discard | QDialogButtonBox::Reset);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
    }


    m_pTabWidget = new QTabWidget(this);
    m_pScalePage    = new QWidget(this);
    m_pGridPage     = new QWidget(this);
    m_pFontPage     = new QWidget(this);
    m_pVariablePage = new QWidget(this);

    //________Variable Page______________________
    QVBoxLayout *pVariablePageLayout = new QVBoxLayout(this);
    {
        QHBoxLayout *pAxisNamesLayout = new QHBoxLayout;
        {
            QLabel *YAxis = new QLabel(tr("YAxis"));
            QLabel *vs = new QLabel(tr("vs."));
            QLabel *XAxis = new QLabel(tr("XAxis"));
            pAxisNamesLayout->addStretch(1);
            pAxisNamesLayout->addWidget(YAxis);
            pAxisNamesLayout->addStretch(1);
            pAxisNamesLayout->addWidget(vs);
            pAxisNamesLayout->addStretch(1);
            pAxisNamesLayout->addWidget(XAxis);
            pAxisNamesLayout->addStretch(1);
        }

        QHBoxLayout *pVariableBoxLayout = new QHBoxLayout;
        {
            m_pctrlXSel = new QListWidget;
            m_pctrlYSel = new QListWidget;
            pVariableBoxLayout->addWidget(m_pctrlYSel);
            pVariableBoxLayout->addWidget(m_pctrlXSel);
        }

        pVariablePageLayout->addLayout(pAxisNamesLayout);
        pVariablePageLayout->addLayout(pVariableBoxLayout);
    }
    m_pVariablePage->setLayout(pVariablePageLayout);

    //________Font Page______________________

    QVBoxLayout *pFontPageLayout = new QVBoxLayout;
    {
        QGroupBox *pFontBox = new QGroupBox(tr("Fonts"));
        {
            QGridLayout *pFontButtonsLayout = new QGridLayout;
            {
                QLabel *lab1  = new QLabel(tr("Title"));
                QLabel *lab2  = new QLabel(tr("Label"));
                QLabel *lab402  = new QLabel(tr("Font"));
                QLabel *lab403  = new QLabel(tr("Color"));
                lab1->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                lab2->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                lab402->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
                lab403->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
                pFontButtonsLayout->addWidget(lab402,1,2);
                pFontButtonsLayout->addWidget(lab403,1,3);
                pFontButtonsLayout->addWidget(lab1,2,1);
                pFontButtonsLayout->addWidget(lab2,3,1);

                m_pctrlTitleButton  = new QPushButton();
                m_pctrlLabelButton  = new QPushButton();

                pFontButtonsLayout->addWidget(m_pctrlTitleButton,2,2);
                pFontButtonsLayout->addWidget(m_pctrlLabelButton,3,2);

                m_pctrlTitleClr  = new TextClrBtn(this);
                m_pctrlTitleClr->setText(tr("Title Color"));
                m_pctrlLabelClr  = new TextClrBtn(this);
                m_pctrlLabelClr->setText(tr("Label Color"));

                pFontButtonsLayout->addWidget(m_pctrlTitleClr,2,3);
                pFontButtonsLayout->addWidget(m_pctrlLabelClr,3,3);
            }
            pFontBox->setLayout(pFontButtonsLayout);
        }

        QGroupBox *pBackBox = new QGroupBox(tr("BackGround"));
        {
            QGridLayout *pBackDataLayout = new QGridLayout;
            {
                QLabel *GraphBackLabel = new QLabel(tr("Graph Background"));
                GraphBackLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                m_pctrlGraphBorder = new QCheckBox(tr("Graph Border"));

                m_pctrlGraphBack = new ColorButton;
                m_pctrlGraphBack->setMinimumWidth(100);
                m_pctrlBorderStyle = new LineBtn(this);
                m_pctrlBorderStyle->setMinimumWidth(100);

                pBackDataLayout->addWidget(GraphBackLabel,1,1);
                pBackDataLayout->addWidget(m_pctrlGraphBorder,2,1,1,1,Qt::AlignRight | Qt::AlignVCenter);

                pBackDataLayout->addWidget(m_pctrlGraphBack,1,2);
                pBackDataLayout->addWidget(m_pctrlBorderStyle,2,2);

                pBackDataLayout->setColumnStretch(0,1);
            }
            pBackBox->setLayout(pBackDataLayout);
        }
        QGroupBox *pPaddingBox = new QGroupBox(tr("Padding"));
        {
            QHBoxLayout *pPaddingLayout = new QHBoxLayout;
            {
                QLabel *pMarginLabel = new QLabel(tr("Margin"));
                QLabel *pMarginUnit = new QLabel(tr("pixels"));
                m_pctrlMargin = new IntEdit(31, this);
                pPaddingLayout->addStretch(1);
                pPaddingLayout->addWidget(pMarginLabel);
                pPaddingLayout->addWidget(m_pctrlMargin);
                pPaddingLayout->addWidget(pMarginUnit);
            }
            pPaddingBox->setLayout(pPaddingLayout);
        }

        pFontPageLayout->addWidget(pFontBox);
        pFontPageLayout->addStretch(1);
        pFontPageLayout->addWidget(pBackBox);
        pFontPageLayout->addStretch(1);
        pFontPageLayout->addWidget(pPaddingBox);
        pFontPageLayout->addStretch(1);
    }
    m_pFontPage->setLayout(pFontPageLayout);
    //________End Font Page______________________

    //________Scale Page______________________

    QGridLayout *pScalePageLayout = new QGridLayout;
    {
        QLabel *XAxis2 = new QLabel(tr("X Axis"));
        QLabel *YAxis2 = new QLabel(tr("Y Axis"));
        XAxis2->setAlignment(Qt::AlignCenter);
        YAxis2->setAlignment(Qt::AlignCenter);

        QLabel *MinLabel = new QLabel(tr("Min"));
        QLabel *MaxLabel = new QLabel(tr("Max"));
        QLabel *OriginLabel = new QLabel(tr("Origin"));
        QLabel *UnitLabel = new QLabel(tr("Unit"));
        MinLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        MaxLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        OriginLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        UnitLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        //    ScaleData->addStretch(1);
        pScalePageLayout->addWidget(MinLabel,5,1);
        pScalePageLayout->addWidget(MaxLabel,6,1);
        pScalePageLayout->addWidget(OriginLabel,7,1);
        pScalePageLayout->addWidget(UnitLabel,8,1);

        m_pctrlXAuto    = new QCheckBox(tr("Auto Scale"));
        m_pctrlXMin     = new DoubleEdit;
        m_pctrlXMax     = new DoubleEdit;
        m_pctrlXOrigin  = new DoubleEdit;
        m_pctrlXUnit    = new DoubleEdit;

        pScalePageLayout->addWidget(XAxis2,2,2);
        pScalePageLayout->addWidget(m_pctrlXAuto,4,2);
        pScalePageLayout->addWidget(m_pctrlXMin,5,2);
        pScalePageLayout->addWidget(m_pctrlXMax,6,2);
        pScalePageLayout->addWidget(m_pctrlXOrigin,7,2);
        pScalePageLayout->addWidget(m_pctrlXUnit,8,2);

        m_pctrlYInverted = new QCheckBox(tr("Inverted Axis"));
        m_pctrlYAuto     = new QCheckBox(tr("Auto Scale"));
        m_pctrlYMin      = new DoubleEdit;
        m_pctrlYMax      = new DoubleEdit;
        m_pctrlYOrigin   = new DoubleEdit;
        m_pctrlYUnit     = new DoubleEdit;

        pScalePageLayout->addWidget(YAxis2,2,3);
        pScalePageLayout->addWidget(m_pctrlYInverted,3,3);
        pScalePageLayout->addWidget(m_pctrlYAuto,4,3);
        pScalePageLayout->addWidget(m_pctrlYMin,5,3);
        pScalePageLayout->addWidget(m_pctrlYMax,6,3);
        pScalePageLayout->addWidget(m_pctrlYOrigin,7,3);
        pScalePageLayout->addWidget(m_pctrlYUnit,8,3);
        pScalePageLayout->setRowStretch(9,1);
    }
    m_pScalePage->setLayout(pScalePageLayout);
    //________End Scale Page______________________

    //________Axis Page______________________
    QGridLayout *pAxisDataLayout = new QGridLayout;
    {
        pAxisDataLayout->setRowStretch(0,1);
        QLabel *AxisStyleLabel = new QLabel(tr("Axis Style"));

        m_pctrlXMajGridShow = new QCheckBox(tr("X Major Grid"));
        m_pctrlYMajGridShow = new QCheckBox(tr("Y Major Grid"));
        m_pctrlXMinGridShow = new QCheckBox(tr("X Minor Grid"));
        m_pctrlYMinGridShow = new QCheckBox(tr("Y Minor Grid"));
        m_pctrlAutoXMinUnit = new QCheckBox(tr("Auto Unit"));
        m_pctrlAutoYMinUnit = new QCheckBox(tr("Auto Unit"));

        m_pctrlAxisStyle = new LineBtn(this);

        m_pctrlXMajGridStyle = new LineBtn(this);
        m_pctrlYMajGridStyle = new LineBtn(this);
        m_pctrlXMinGridStyle = new LineBtn(this);
        m_pctrlYMinGridStyle = new LineBtn(this);

        m_pctrlXMinorUnit = new DoubleEdit;
        m_pctrlYMinorUnit = new DoubleEdit;

        pAxisDataLayout->addWidget(AxisStyleLabel,1,1);
        pAxisDataLayout->addWidget(m_pctrlXMajGridShow,2,1);
        pAxisDataLayout->addWidget(m_pctrlYMajGridShow,3,1);
        pAxisDataLayout->addWidget(m_pctrlXMinGridShow,4,1);
        pAxisDataLayout->addWidget(m_pctrlYMinGridShow,5,1);

        pAxisDataLayout->addWidget(m_pctrlAxisStyle,1,2);
        pAxisDataLayout->addWidget(m_pctrlXMajGridStyle,2,2);
        pAxisDataLayout->addWidget(m_pctrlYMajGridStyle,3,2);
        pAxisDataLayout->addWidget(m_pctrlXMinGridStyle,4,2);
        pAxisDataLayout->addWidget(m_pctrlYMinGridStyle,5,2);

        pAxisDataLayout->addWidget(m_pctrlAutoXMinUnit,4,3);
        pAxisDataLayout->addWidget(m_pctrlAutoYMinUnit,5,3);
        pAxisDataLayout->addWidget(m_pctrlXMinorUnit,4,4);
        pAxisDataLayout->addWidget(m_pctrlYMinorUnit,5,4);
    }
    m_pGridPage->setLayout(pAxisDataLayout);
    //________End Axis Page______________________


    m_pTabWidget->addTab(m_pVariablePage, tr("Variables"));
    m_pTabWidget->addTab(m_pScalePage, tr("Scales"));
    m_pTabWidget->addTab(m_pGridPage, tr("Axis and Grids"));
    m_pTabWidget->addTab(m_pFontPage, tr("Fonts and BackGround"));

    m_pTabWidget->setCurrentIndex(s_iActivePage);
    connect(m_pTabWidget, SIGNAL(currentChanged (int)), this, SLOT(onActivePage(int)));

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pTabWidget);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void GraphDlg::setGraph(Graph *pGraph)
{
    m_pGraph = pGraph;
    m_SaveGraph.copySettings(m_pGraph);
    setControls();
}



void GraphDlg::setActivePage(int iPage)
{
    s_iActivePage =iPage;
}
