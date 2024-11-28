/****************************************************************************

    SaveOptions Class
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
#include <QCheckBox>
#include <QSettings>

class IntEdit;

class SaveOptions : public QWidget
{
    Q_OBJECT
    public:
        SaveOptions(QWidget *parent = nullptr);

        void initWidget();

        static void saveOpps(bool b)  {s_bOpps=b;}
        static void savePOpps(bool b)  {s_bPOpps=b;}
        static bool bOpps() {return s_bOpps;}
        static bool bPOpps() {return s_bPOpps;}

        static void setAutoLoadLast(bool b) {s_bAutoLoadLast=b;}
        static void setAutoSave(bool b) {s_bAutoSave=b;}
        static bool bAutoLoadLast() {return s_bAutoLoadLast;}
        static bool bAutoSave() {return s_bAutoSave;}

        static int saveInterval() {return s_SaveInterval;}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    public slots:
        void onOK();

    private:
        void setupLayout();
        void readParams();

        static bool s_bOpps, s_bPOpps, s_bAutoSave, s_bAutoLoadLast;
        static int s_SaveInterval;

        IntEdit *m_pieInterval;
        QCheckBox *m_pchOpps, *m_pchWOpps;
        QCheckBox *m_pchAutoSave, *m_pchAutoLoadLast;
};

