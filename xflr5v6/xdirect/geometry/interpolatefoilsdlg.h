/****************************************************************************

    InterpolateFoilsDlg Class
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
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QSlider>
#include <QDialogButtonBox>

class XFoil;
class Foil;
class DoubleEdit;

class InterpolateFoilsDlg : public QDialog
{
    Q_OBJECT

    public:
        InterpolateFoilsDlg(QWidget *pParent);
        void initDialog();
        void setupLayout();
        void update();

    private:
        QSize sizeHint() const override {return QSize(500,500);}
        void keyPressEvent(QKeyEvent *event) override;

    private slots:
        void onSelChangeFoil1(int);
        void onSelChangeFoil2(int);
        void onFrac();
        void onOK();
        void onVScroll(int val);
        void onButton(QAbstractButton *pButton);


    private:
        QComboBox *m_pcbFoil1, *m_pcbFoil2;
        QLabel *m_plabCamb1, *m_plabCamb2, *m_plabThick1, *m_plabThick2;
        QLabel *m_plabCamb3, *m_plabThick3;
        QSlider *m_pslMix;
        DoubleEdit *m_pdeFrac;

        QDialogButtonBox *m_pButtonBox;

    public:

        Foil* m_pBufferFoil;

        QWidget *m_pParent;
        double m_Frac;
        QString m_NewFoilName;


        static XFoil *s_pXFoil;
};


