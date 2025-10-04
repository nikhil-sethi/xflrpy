/****************************************************************************

    BatchCtrlDlg Class
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

#include <QModelIndex>
#include <QSettings>
#include "batchabstractdlg.h"

class DoubleEdit;


/**
 * @brief Extension of the BacthThreadDlg class to include flaps and other control parameters
 */
class BatchCtrlDlg : public BatchAbstractDlg
{
    Q_OBJECT
    friend class XDirect;
    friend class MainFrame;
    friend class XFoilTask;

    public:
        BatchCtrlDlg(QWidget *pParent=nullptr);
        ~BatchCtrlDlg();

        void initDialog() override;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void setupLayout();
        void readParams() override;
        void startAnalyses();
        void customEvent(QEvent * pEvent) override;

    private slots:
        void onAnalyze() override;


    private:
        int m_nTasks, m_TaskCounter;

        DoubleEdit *m_pdeXHinge, *m_pdeYHinge;
        DoubleEdit *m_pdeAngleMin, *m_pdeAngleMax, *m_pdeAngleDelta;


        static double s_XHinge, s_YHinge;
        static double s_AngleMin, s_AngleMax, s_AngleDelta;
};



