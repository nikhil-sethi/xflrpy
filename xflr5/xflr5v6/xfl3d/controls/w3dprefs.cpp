/****************************************************************************

    W3dPrefsDlg Class
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

#include <QGroupBox>
#include <QLabel>
#include <QCheckBox>
#include <QGridLayout>
#include <QColorDialog>
#include <QPushButton>

#include "w3dprefs.h"
#include <xflwidgets/line/linemenu.h>
#include <xflwidgets/line/linebtn.h>
#include <xflwidgets/color/colorbtn.h>
#include <xflwidgets/customwts/intedit.h>


bool W3dPrefs::s_bAutoAdjustScale = true;
bool W3dPrefs::s_bWakePanels = false;

double W3dPrefs::s_MassRadius = .017;
QColor W3dPrefs::s_MassColor = QColor(67, 151, 169);


LineStyle W3dPrefs::s_AxisStyle      = {true, Line::DASHDOT, 1, QColor(150,150,150),     Line::NOSYMBOL};
LineStyle W3dPrefs::s_VLMStyle       = {true, Line::SOLID,   1, QColor(180,180,180),     Line::NOSYMBOL};

LineStyle W3dPrefs::s_OutlineStyle   = {true, Line::SOLID,   1, QColor(73,73,73),        Line::NOSYMBOL};
LineStyle W3dPrefs::s_XCPStyle       = {true, Line::SOLID,   2, QColor(50, 150, 50),     Line::NOSYMBOL};
LineStyle W3dPrefs::s_MomentStyle    = {true, Line::SOLID,   2, QColor(200, 100, 100),   Line::NOSYMBOL};
LineStyle W3dPrefs::s_IDragStyle     = {true, Line::SOLID,   2, QColor(255,200,0),       Line::NOSYMBOL};
LineStyle W3dPrefs::s_DownwashStyle  = {true, Line::SOLID,   2, QColor(255, 100, 100),   Line::NOSYMBOL};
LineStyle W3dPrefs::s_WakeStyle      = {true, Line::SOLID,   1, QColor(0, 150, 200),     Line::NOSYMBOL};
LineStyle W3dPrefs::s_CpStyle        = {true, Line::SOLID,   2, QColor(255,0,0),         Line::NOSYMBOL};
LineStyle W3dPrefs::s_StreamStyle    = {true, Line::SOLID,   2, QColor(200, 150, 255),   Line::NOSYMBOL};

LineStyle W3dPrefs::s_VDragStyle     = {true, Line::SOLID,   1, QColor(200,100,220),     Line::NOSYMBOL};
LineStyle W3dPrefs::s_TopStyle       = {true, Line::SOLID,   1, QColor(171, 103, 220),   Line::NOSYMBOL};
LineStyle W3dPrefs::s_BotStyle       = {true, Line::DASH,    1, QColor(171, 103, 220),   Line::NOSYMBOL};

int W3dPrefs::s_iChordwiseRes = 31;
int W3dPrefs::s_iBodyAxialRes = 23;
int W3dPrefs::s_iBodyHoopRes  = 17;
bool W3dPrefs::s_bAnimateTransitions = true;
bool W3dPrefs::s_bEnableClipPlane = false;


W3dPrefs::W3dPrefs(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("3D Styles"));
    setupLayout();

    connect(m_plbAxis,               SIGNAL(clickedLB(LineStyle)),   SLOT(on3DAxis()));
    connect(m_plbOutline,            SIGNAL(clickedLB(LineStyle)),   SLOT(onOutline()));
    connect(m_plbVLMMesh,            SIGNAL(clickedLB(LineStyle)),   SLOT(onVLMMesh()));
    connect(m_plbTopTrans,           SIGNAL(clickedLB(LineStyle)),   SLOT(onTopTrans()));
    connect(m_plbBotTrans,           SIGNAL(clickedLB(LineStyle)),   SLOT(onBotTrans()));
    connect(m_plbLift,               SIGNAL(clickedLB(LineStyle)),   SLOT(onXCP()));
    connect(m_plbMoments,            SIGNAL(clickedLB(LineStyle)),   SLOT(onMoments()));
    connect(m_plbInducedDrag,        SIGNAL(clickedLB(LineStyle)),   SLOT(onIDrag()));
    connect(m_plbViscousDrag,        SIGNAL(clickedLB(LineStyle)),   SLOT(onVDrag()));
    connect(m_plbDownwash,           SIGNAL(clickedLB(LineStyle)),   SLOT(onDownwash()));
    connect(m_plbStreamLines,        SIGNAL(clickedLB(LineStyle)),   SLOT(onStreamLines()));
    connect(m_plbWakePanels,         SIGNAL(clickedLB(LineStyle)),   SLOT(onWakePanels()));
    connect(m_pcbMassColor,          SIGNAL(clicked()),     SLOT(onMasses()));
}


void W3dPrefs::initDialog()
{
    m_plbAxis->setTheStyle(s_AxisStyle);
    m_plbOutline->setTheStyle(s_OutlineStyle);
    m_plbVLMMesh->setTheStyle(s_VLMStyle);
    m_plbLift->setTheStyle(s_XCPStyle);
    m_plbMoments->setTheStyle(s_MomentStyle);
    m_plbInducedDrag->setTheStyle(s_IDragStyle);
    m_plbViscousDrag->setTheStyle(s_VDragStyle);
    m_plbDownwash->setTheStyle(s_DownwashStyle);
    m_plbWakePanels->setTheStyle(s_WakeStyle);
    m_plbStreamLines->setTheStyle(s_StreamStyle);
    m_plbTopTrans->setTheStyle(s_TopStyle);
    m_plbBotTrans->setTheStyle(s_BotStyle);

    m_pcbMassColor->setColor(s_MassColor);
    m_pieChordwiseRes->setValue(s_iChordwiseRes);
    m_pieBodyAxialRes->setValue(s_iBodyAxialRes);
    m_pcieBodyHoopRes->setValue(s_iBodyHoopRes);

    m_pchAnimateTransitions->setChecked(s_bAnimateTransitions);
    m_pchAutoAdjustScale->setChecked(s_bAutoAdjustScale);
    m_pchEnableClipPlane->setChecked(s_bEnableClipPlane);
}



void W3dPrefs::setupLayout()
{
    QLabel *lab1 = new QLabel(tr("Axis"));
    QLabel *lab2 = new QLabel(tr("Outline"));
    QLabel *lab3 = new QLabel(tr("VLM Mesh"));
    QLabel *lab4 = new QLabel(tr("Top transition"));
    QLabel *lab5 = new QLabel(tr("Bottom transition"));
    QLabel *lab6 = new QLabel(tr("Lift"));
    QLabel *lab7 = new QLabel(tr("Moments"));
    QLabel *lab8 = new QLabel(tr("Induced Drag"));
    QLabel *lab9 = new QLabel(tr("Viscous Drag"));
    QLabel *lab10 = new QLabel(tr("Downwash"));
    QLabel *lab11 = new QLabel(tr("WakePanels"));
    QLabel *lab12 = new QLabel(tr("Streamlines"));
    QLabel *lab13 = new QLabel(tr("Masses"));

    lab1->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    lab2->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    lab3->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    lab4->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    lab5->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    lab6->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    lab7->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    lab8->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    lab9->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    lab10->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    lab11->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    lab12->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    lab13->setAlignment(Qt::AlignVCenter|Qt::AlignRight);

    m_plbAxis     = new LineBtn(this);
    m_plbOutline  = new LineBtn(this);
    m_plbVLMMesh  = new LineBtn(this);
    m_plbTopTrans = new LineBtn(this);
    m_plbBotTrans = new LineBtn(this);
    m_plbLift     = new LineBtn(this);
    m_plbMoments  = new LineBtn(this);
    m_plbInducedDrag = new LineBtn(this);
    m_plbViscousDrag = new LineBtn(this);
    m_plbDownwash    = new LineBtn(this);
    m_plbWakePanels  = new LineBtn(this);
    m_plbStreamLines  = new LineBtn(this);
    m_pcbMassColor = new ColorBtn;

    QGroupBox *pColorPrefsBox = new QGroupBox(tr("Color settings"));
    {
        QGridLayout *pColorPrefsLayout = new QGridLayout;
        {
            pColorPrefsLayout->setColumnStretch(1,1);
            pColorPrefsLayout->setColumnStretch(2,2);
            pColorPrefsLayout->setColumnStretch(3,1);
            pColorPrefsLayout->setColumnStretch(4,2);
            pColorPrefsLayout->addWidget(lab1,1,1);
            pColorPrefsLayout->addWidget(lab2,2,1);
            pColorPrefsLayout->addWidget(lab3,3,1);
            pColorPrefsLayout->addWidget(lab4,4,1);
            pColorPrefsLayout->addWidget(lab5,5,1);
            pColorPrefsLayout->addWidget(lab6,6,1);
            pColorPrefsLayout->addWidget(lab7,1,3);
            pColorPrefsLayout->addWidget(lab8,2,3);
            pColorPrefsLayout->addWidget(lab9,3,3);
            pColorPrefsLayout->addWidget(lab10,4,3);
            pColorPrefsLayout->addWidget(lab11,5,3);
            pColorPrefsLayout->addWidget(lab12,6,3);
            pColorPrefsLayout->addWidget(lab13,7,3);

            pColorPrefsLayout->addWidget(m_plbAxis,1,2);
            pColorPrefsLayout->addWidget(m_plbOutline,2,2);
            pColorPrefsLayout->addWidget(m_plbVLMMesh,3,2);
            pColorPrefsLayout->addWidget(m_plbTopTrans,4,2);
            pColorPrefsLayout->addWidget(m_plbBotTrans,5,2);
            pColorPrefsLayout->addWidget(m_plbLift,6,2);
            pColorPrefsLayout->addWidget(m_plbMoments,1,4);
            pColorPrefsLayout->addWidget(m_plbInducedDrag,2,4);
            pColorPrefsLayout->addWidget(m_plbViscousDrag,3,4);
            pColorPrefsLayout->addWidget(m_plbDownwash,4,4);
            pColorPrefsLayout->addWidget(m_plbWakePanels,5,4);
            pColorPrefsLayout->addWidget(m_plbStreamLines,6,4);
            pColorPrefsLayout->addWidget(m_pcbMassColor,7,4);
        }
        pColorPrefsBox->setLayout(pColorPrefsLayout);
    }

    QGroupBox *pTessBox = new QGroupBox(tr("Tessellation"));
    {
        QVBoxLayout *pTessLayout = new QVBoxLayout;
        {
            QLabel *pTessLabel = new QLabel(tr("Increase the number of points to improve the resolution\n"
                                               "of the surfaces.This may reduce the display speed.\n"));
            QLabel *pTessChords = new QLabel(tr("Wing chordwise direction"));
            m_pieChordwiseRes = new IntEdit(37,this);
            QLabel *pTessBodyAxial = new QLabel(tr("Body axial direction"));
            m_pieBodyAxialRes = new IntEdit(29, this);
            QLabel *pTessBodyHoop = new QLabel(tr("Body hoop direction"));
            m_pcieBodyHoopRes = new IntEdit(17,this);

            QHBoxLayout *pChordResLayout = new QHBoxLayout;
            {
                pChordResLayout->addStretch();
                pChordResLayout->addWidget(pTessChords);
                pChordResLayout->addWidget(m_pieChordwiseRes);
            }
            QHBoxLayout *pAxialResLayout = new QHBoxLayout;
            {
                pAxialResLayout->addStretch();
                pAxialResLayout->addWidget(pTessBodyAxial);
                pAxialResLayout->addWidget(m_pieBodyAxialRes);
            }
            QHBoxLayout *pHoopResLayout = new QHBoxLayout;
            {
                pHoopResLayout->addStretch();
                pHoopResLayout->addWidget(pTessBodyHoop);
                pHoopResLayout->addWidget(m_pcieBodyHoopRes);
            }
            pTessLayout->addWidget(pTessLabel);
            pTessLayout->addLayout(pChordResLayout);
            pTessLayout->addLayout(pAxialResLayout);
            pTessLayout->addLayout(pHoopResLayout);
        }
        pTessBox->setLayout(pTessLayout);
    }

    m_pchAnimateTransitions = new QCheckBox(tr("Animate view transitions"));
    m_pchAutoAdjustScale = new QCheckBox(tr("Auto Adjust 3D scale"));
    m_pchAutoAdjustScale->setToolTip(tr("Automatically adjust the 3D scale to fit the plane in the display when switching between planes"));
    m_pchEnableClipPlane = new QCheckBox(tr("Enable clip plane"));

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close | QDialogButtonBox::RestoreDefaults);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }


    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(pColorPrefsBox);
        pMainLayout->addWidget(pTessBox);
        pMainLayout->addWidget(m_pchAnimateTransitions);
        pMainLayout->addWidget(m_pchAutoAdjustScale);
        pMainLayout->addWidget(m_pchEnableClipPlane);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void W3dPrefs::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Close)           == pButton)  onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::RestoreDefaults) == pButton)  onRestoreDefaults();
}


void W3dPrefs::onOutline()
{
    LineMenu lm(nullptr, false);
    //	lm.enableSubMenus(true, true, true, false);
    lm.initMenu(s_OutlineStyle);
    lm.exec(QCursor::pos());

    s_OutlineStyle = lm.theStyle();
    m_plbOutline->setTheStyle(s_OutlineStyle);
}


void W3dPrefs::on3DAxis()
{
    LineMenu lm(nullptr, false);
    //	lm.enableSubMenus(true, true, true, false);
    lm.initMenu(s_AxisStyle);
    lm.exec(QCursor::pos());

    s_AxisStyle = lm.theStyle();
    m_plbAxis->setTheStyle(s_AxisStyle);
}


void W3dPrefs::onTopTrans()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_TopStyle);
    lm.exec(QCursor::pos());

    s_TopStyle = lm.theStyle();
    m_plbTopTrans->setTheStyle(s_TopStyle);
}


void W3dPrefs::onBotTrans()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_BotStyle);
    lm.exec(QCursor::pos());

    s_BotStyle = lm.theStyle();
    m_plbBotTrans->setTheStyle(s_BotStyle);
}


void W3dPrefs::onIDrag()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_IDragStyle);
    lm.exec(QCursor::pos());

    s_IDragStyle = lm.theStyle();
    m_plbInducedDrag->setTheStyle(s_IDragStyle);
}


void W3dPrefs::onVDrag()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_VDragStyle);
    lm.exec(QCursor::pos());

    s_VDragStyle = lm.theStyle();
    m_plbViscousDrag->setTheStyle(s_VDragStyle);
}


void W3dPrefs::onXCP()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_XCPStyle);
    lm.exec(QCursor::pos());

    s_XCPStyle = lm.theStyle();
    m_plbLift->setTheStyle(s_XCPStyle);
}


void W3dPrefs::onMoments()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_MomentStyle);
    lm.exec(QCursor::pos());

    s_MomentStyle = lm.theStyle();
    m_plbMoments->setTheStyle(s_MomentStyle);
}


void W3dPrefs::onDownwash()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_DownwashStyle);
    lm.exec(QCursor::pos());

    s_DownwashStyle = lm.theStyle();
    m_plbDownwash->setTheStyle(s_DownwashStyle);
}


void W3dPrefs::onStreamLines()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_StreamStyle);
    lm.exec(QCursor::pos());

    s_StreamStyle = lm.theStyle();
    m_plbStreamLines->setTheStyle(s_StreamStyle);
}


void W3dPrefs::onWakePanels()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_WakeStyle);
    lm.exec(QCursor::pos());

    s_WakeStyle = lm.theStyle();
    m_plbWakePanels->setTheStyle(s_WakeStyle);
}


void W3dPrefs::onVLMMesh()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(s_VLMStyle);
    lm.exec(QCursor::pos());

    s_VLMStyle = lm.theStyle();
    m_plbVLMMesh->setTheStyle(s_VLMStyle);
}


void W3dPrefs::onMasses()
{
    QColorDialog::ColorDialogOptions dialogOptions = QColorDialog::ShowAlphaChannel;
#ifdef Q_OS_MAC
#if QT_VERSION >= 0x040700
    dialogOptions |= QColorDialog::DontUseNativeDialog;
#endif
#endif
    QColor Color = QColorDialog::getColor(s_MassColor,
                                          this, "Select the color", dialogOptions);
    if(Color.isValid()) s_MassColor = Color;
    m_pcbMassColor->setColor(s_MassColor);

    update();
}


void W3dPrefs::saveSettings(QSettings &settings)
{
    settings.beginGroup("W3dPrefs");
    {
        s_AxisStyle.loadSettings(  settings, "3DAxisStyle");
        s_VLMStyle.saveSettings(     settings, "VLMStyle");
        s_OutlineStyle .saveSettings(settings, "OutlineStyle");
        s_XCPStyle.saveSettings(     settings, "XCPStyle");
        s_MomentStyle.saveSettings(  settings, "MomentStyle");
        s_IDragStyle.saveSettings(   settings, "IDragStyle");
        s_VDragStyle.saveSettings(   settings, "VDragStyle");
        s_DownwashStyle.saveSettings(settings, "DownwashStyle");
        s_WakeStyle.saveSettings(    settings, "WakeStyle");
        s_CpStyle.saveSettings(      settings, "CpStyle");
        s_TopStyle.saveSettings(     settings, "TopStyle");
        s_BotStyle.saveSettings(     settings, "BotStyle");
        s_StreamStyle.saveSettings(  settings, "StreamLinesStyle");


        settings.setValue("showWakePanels", s_bWakePanels);

        settings.setValue("MassColor", s_MassColor);

        settings.setValue("AutoAdjustScale", s_bAutoAdjustScale);
        settings.setValue("AnimateTransitions", s_bAnimateTransitions);
        settings.setValue("EnableClipPlane", s_bEnableClipPlane);

        settings.setValue("ChordwiseResolution", s_iChordwiseRes);
        settings.setValue("BodyAxialRes", s_iBodyAxialRes);
        settings.setValue("BodyHoopRes", s_iBodyHoopRes);

    }
    settings.endGroup();
}


void W3dPrefs::loadSettings(QSettings &settings)
{
    resetDefaults();
    settings.beginGroup("W3dPrefs");
    {
        s_AxisStyle.loadSettings(    settings, "3DAxisStyle");
        s_VLMStyle.loadSettings(     settings, "VLMStyle");
        s_OutlineStyle .loadSettings(settings, "OutlineStyle");
        s_XCPStyle.loadSettings(     settings, "XCPStyle");
        s_MomentStyle.loadSettings(  settings, "MomentStyle");
        s_IDragStyle.loadSettings(   settings, "IDragStyle");
        s_VDragStyle.loadSettings(   settings, "VDragStyle");
        s_DownwashStyle.loadSettings(settings, "DownwashStyle");
        s_WakeStyle.loadSettings(    settings, "WakeStyle");
        s_CpStyle.loadSettings(      settings, "CpStyle");
        s_TopStyle.loadSettings(     settings, "TopStyle");
        s_BotStyle.loadSettings(     settings, "BotStyle");
        s_StreamStyle.loadSettings(  settings, "StreamLinesStyle");

        s_MassColor           = settings.value("MassColor",           s_MassColor).value<QColor>();
        s_bWakePanels         = settings.value("showWakePanels",      s_bWakePanels).toBool();
        s_bEnableClipPlane    = settings.value("EnableClipPlane",     s_bEnableClipPlane).toBool();

        s_bAutoAdjustScale    = settings.value("AutoAdjustScale",     s_bAutoAdjustScale).toBool();
        s_bAnimateTransitions = settings.value("AnimateTransitions",  s_bAnimateTransitions).toBool();
        s_iChordwiseRes       = settings.value("ChordwiseResolution", s_iChordwiseRes).toInt();
        s_iBodyAxialRes       = settings.value("BodyAxialRes",        s_iBodyAxialRes).toInt();
        s_iBodyHoopRes        = settings.value("BodyHoopRes",         s_iBodyHoopRes).toInt();
    }
    settings.endGroup();
}


void W3dPrefs::onRestoreDefaults()
{
    resetDefaults();
    initDialog();
}


void W3dPrefs::resetDefaults()
{
    s_bWakePanels = false;

    s_MassColor = QColor(67, 151, 169);

    s_bAnimateTransitions = false;
    s_iChordwiseRes = 31;
    s_iBodyAxialRes = 23;
    s_iBodyHoopRes  = 17;
}


void W3dPrefs::onOK()
{
    readSettings();
    accept();
}

void W3dPrefs::readSettings()
{
    s_bAnimateTransitions = m_pchAnimateTransitions->isChecked();
    s_bAutoAdjustScale = m_pchAutoAdjustScale->isChecked();
    s_bEnableClipPlane = m_pchEnableClipPlane->isChecked();

    s_iChordwiseRes = m_pieChordwiseRes->value();
    s_iBodyAxialRes = m_pieBodyAxialRes->value();
    s_iBodyHoopRes  = m_pcieBodyHoopRes->value();

}





