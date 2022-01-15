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


#pragma once

#include <QDialog>
#include <QCheckBox>
#include <QSettings>
#include <QDialogButtonBox>

#include <xflcore/linestyle.h>

class LineBtn;
class ColorBtn;
class IntEdit;

class W3dPrefs : public QDialog
{
    Q_OBJECT

    friend class Miarex;
    friend class MainFrame;
    friend class GL3dBodyDlg;

    public:
        W3dPrefs(QWidget *pParent);
        void initDialog();

        static int chordwiseRes() {return s_iChordwiseRes;}
        static int bodyAxialRes() {return s_iBodyAxialRes;}
        static int bodyHoopRes() {return s_iBodyHoopRes;}

    private slots:
        void on3DAxis();
        void onOutline();
        void onTopTrans();
        void onBotTrans();
        void onXCP();
        void onMoments();
        void onIDrag();
        void onVDrag();
        void onDownwash();
        void onWakePanels();
        void onStreamLines();
        void onVLMMesh();
        void onMasses();
        void onRestoreDefaults();
        void onOK();
        void onButton(QAbstractButton *pButton);

    private:
        void setupLayout();
        void readSettings();
        static void saveSettings(QSettings &settings);
        static void loadSettings(QSettings &settings);
        static void resetDefaults();

        LineBtn *m_plbAxis, *m_plbOutline, *m_plbVLMMesh, *m_plbTopTrans, *m_plbBotTrans;
        LineBtn *m_plbLift, *m_plbMoments, *m_plbInducedDrag, *m_plbViscousDrag, *m_plbDownwash;
        LineBtn *m_plbStreamLines, *m_plbWakePanels;

        ColorBtn *m_pcbMassColor;

        QCheckBox *m_pchAnimateTransitions, *m_pchAutoAdjustScale;
        QCheckBox *m_pchEnableClipPlane;
        IntEdit *m_pieChordwiseRes, *m_pieBodyAxialRes, *m_pcieBodyHoopRes;

        QDialogButtonBox *m_pButtonBox;

    public:
        static double s_MassRadius;
        static QColor s_MassColor;

        static LineStyle s_AxisStyle;
        static LineStyle s_VLMStyle;
        static LineStyle s_OutlineStyle;
        static LineStyle s_XCPStyle;
        static LineStyle s_MomentStyle;
        static LineStyle s_IDragStyle;
        static LineStyle s_VDragStyle;
        static LineStyle s_TopStyle;
        static LineStyle s_BotStyle;
        static LineStyle s_DownwashStyle;
        static LineStyle s_StreamStyle;
        static LineStyle s_WakeStyle;
        static LineStyle s_CpStyle;


        static bool s_bWakePanels;

        static bool s_bAutoAdjustScale;

        static int s_iChordwiseRes,s_iBodyAxialRes, s_iBodyHoopRes;
        static bool s_bAnimateTransitions;
        static bool s_bEnableClipPlane;

};


