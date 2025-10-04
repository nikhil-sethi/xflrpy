/****************************************************************************

    FoilPolarDlg Class
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
#include <QRadioButton>
#include <QLabel>
#include <QString>
#include <QLineEdit>
#include <QSettings>
#include <QDialogButtonBox>

#include <xflcore/core_enums.h>

class DoubleEdit;
class Polar;


class FoilPolarDlg : public QDialog
{
    Q_OBJECT

    public:
        FoilPolarDlg(QWidget *pParent=nullptr);

        void readParams();
        void initDialog();
        void setPlrName();
        void setupLayout();
        void connectSignals();
        void setDensity();

        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    public slots:
        void onAutoName();
        void onOK();
        void onPolarType();
        void onNameChanged();
        void editingFinished();
        void onFluiUnit();
        void onCalcReynolds();
        void onButton(QAbstractButton *pButton);

    public:

        QRadioButton *m_prbAuto1, *m_prbAuto2;

        QLabel *m_plabRe, *m_plabReUnit;
        QLabel *m_plabMach;

        QLineEdit *m_pleAnalysisName;
        QRadioButton *m_rbtype1;
        QRadioButton *m_rbtype2;
        QRadioButton *m_rbtype3;
        QRadioButton *m_rbtype4;

        DoubleEdit *m_pdeReynolds;
        DoubleEdit *m_pdeMach;

        DoubleEdit *m_pdeChord, *m_pdeMass, *m_pdeSpan;
        QLabel *m_plabLengthUnit1, *m_plabLengthUnit2, *m_plabMassUnit;

        QRadioButton *m_prbFluidUnit1, *m_prbFluidUnit2;
        QLabel *m_plabRho, *m_plabNu, *m_plabViscosityUnit, *m_plabDensityUnit;
        DoubleEdit *m_pdeDensity, *m_pdeViscosity;

        QDialogButtonBox *m_pButtonBox;


        DoubleEdit *m_pdeNCrit;
        DoubleEdit *m_pdeTopTrans;
        DoubleEdit *m_pdeBotTrans;

        bool  m_bAutoName;

        QString m_FoilName;
        QString m_PlrName;

        static int s_UnitType;
        static double s_Viscosity, s_Density;
        static double s_Chord, s_Span, s_Mass;

        static Polar s_RefPolar;    /**< Using a static reference to store default data */

        static QByteArray s_WindowGeometry;

};


