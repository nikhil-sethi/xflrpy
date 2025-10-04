/****************************************************************************

    LengthUnitDlgDlg Class
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

#include <QComboBox>
#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>

class LengthUnitDlg : public QDialog
{
    Q_OBJECT
    friend class MainFrame;
    friend class Miarex;

    public:
        LengthUnitDlg(QWidget *parent);

        void getLengthUnitLabel(QString &str);
        QString lengthUnitLabel();
        double mtoUnit()  {return s_mtoUnit;}
        int lengthUnitIndex() {return m_LengthUnitIndex;}

        void initDialog(int lengthUnitInd);
        void setUnits();

    private slots:
        void onSelChanged(const QString &);
        void onButton(QAbstractButton *pButton);

    private:

        QComboBox *m_pcbLength;

        QLabel *m_plabLengthFactor, *m_plabLengthInvFactor;
        QLabel *m_plabQuestion;

    private:
        void SetupLayout();

        int m_LengthUnitIndex;    /**< The index of the user selected unit in the array of length units. @todo use an enumeration instead. */

        double s_mtoUnit;    /**< Conversion factor from meters to the user selected length unit. */


        QDialogButtonBox *m_pButtonBox;

    public:

        QString m_Question;
};

