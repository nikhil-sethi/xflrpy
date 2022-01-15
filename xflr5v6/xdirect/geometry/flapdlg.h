/****************************************************************************

    FlapDlg class
    Copyright (C) 2004-2009 André Deperrois 

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
#include <QDialogButtonBox>

class Foil;
class DoubleEdit;

class FlapDlg : public QDialog
{
    Q_OBJECT
    friend class AFoil;
    friend class XDirect;


    public:
        FlapDlg(QWidget *pParent);


    private:
        void enableTEFlap(bool bEnable);
        void enableLEFlap(bool bEnable);
        void readParams();
        void setupLayout();


    protected:
        void keyPressEvent(QKeyEvent *pEvent) override;
        void initDialog();

    private slots:
        void onApply();
        void onLEFlapCheck(int);
        void onTEFlapCheck(int);
        void onChanged();
        void accept() override;
        void onButton(QAbstractButton *pButton);

    private:
        bool m_bTEFlap;
        bool m_bLEFlap;
        bool m_bApplied;
        bool m_bModified;

        double m_LEXHinge, m_LEYHinge, m_LEFlapAngle;
        double m_TEXHinge, m_TEYHinge, m_TEFlapAngle;
        Foil const *m_pMemFoil;
        Foil *m_pBufferFoil;

        QWidget *m_pParent;

        DoubleEdit *m_pdeLEYHinge, *m_pdeLEXHinge, *m_pdeLEFlapAngle;
        DoubleEdit *m_pdeTEYHinge, *m_pdeTEXHinge, *m_pdeTEFlapAngle;

        QCheckBox *m_pchLEFlapCheck;
        QCheckBox *m_pchTEFlapCheck;

        QDialogButtonBox *m_pButtonBox;
};


