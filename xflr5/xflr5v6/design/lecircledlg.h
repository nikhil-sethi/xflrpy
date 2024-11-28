/****************************************************************************

    LECircleDlg Class
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
#include <QPushButton>

class DoubleEdit;
class QDialogButtonBox;

class LECircleDlg : public QDialog
{
    Q_OBJECT

    public:
        LECircleDlg(QWidget *pParent);

        void initDialog(double radius, bool bShowCircle);
        double radius() const {return m_Radius;}
        bool bShowCircle() const {return  m_bShowCircle;}


    private slots:
        void onOK();


    private:
        void keyPressEvent(QKeyEvent *event) override;
        void setupLayout();

    private:
        QCheckBox *m_pchShow;
        DoubleEdit *m_pdeRadius;

        QDialogButtonBox *m_pButtonBox;

        double m_Radius;
        bool m_bShowCircle;
};

