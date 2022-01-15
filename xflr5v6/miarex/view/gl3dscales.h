/****************************************************************************

    GL3DScales Class
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

#include <QWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QSettings>

class IntEdit;
class DoubleEdit;
class ExponentialSlider;

class Miarex;
class GL3DScales : public QWidget
{
    Q_OBJECT
    friend class MainFrame;
    friend class Miarex;

    public:
        GL3DScales(QWidget *);
        void initDialog();

    private slots:
        void onCpScale();
        void onApply();
        void onLiftEdit();
        void onDragEdit();
        void onVelocityEdit();
        void onLiftScale();
        void onDragScale();
        void onVelocityScale();


    private:
        void showEvent(QShowEvent *event) override;
        void hideEvent(QHideEvent *event) override;

        void setupLayout();
        void readStreamParams();

        static bool loadSettings(QSettings &settings);
        static bool saveSettings(QSettings &settings);

        ExponentialSlider *m_peslLiftScaleSlider, *m_peslDragScaleSlider, *m_peslVelocityScaleSlider;
        DoubleEdit *m_pdeLiftScale, *m_pdeDragScale, *m_pdeVelocityScale;
        QPushButton *m_ppbApply;
        QCheckBox *m_pchAutoCpScale;
        DoubleEdit *m_pdeLegendMin, *m_pdeLegendMax;

        IntEdit *m_pieNXPoint;
        DoubleEdit *m_pdeDeltaL, *m_pdeXFactor, *m_pdeXOffset, *m_pdeZOffset;
        QRadioButton *m_prbLE, *m_pebTE, *m_prbLine;

        QLabel *m_plabLengthUnit1, *m_plabLengthUnit2, *m_plabLengthUnit3;

        static Miarex *s_pMiarex;


    public:
        static int s_pos;
        static int s_NX;
        static double s_DeltaL;
        static double s_XFactor;
        static double s_XOffset, s_ZOffset;
};

