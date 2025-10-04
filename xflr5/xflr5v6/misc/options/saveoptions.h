/****************************************************************************

    SaveOptions Class
    Copyright (C) 2018 André Deperrois

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

class IntEdit;

class SaveOptions : public QWidget
{
    friend class MainFrame;
    Q_OBJECT
    public:
        SaveOptions(QWidget *parent = nullptr);

        void initWidget(bool bAutoLoadLast=false, bool bOpps=false, bool bWOpps = true, bool bAutoSave=true, int saveInterval=10);

    public slots:
        void onOK();

    private:
        void setupLayout();
        void readParams();

        bool m_bOpps, m_bWOpps, m_bAutoSave, m_bAutoLoadLast;
        int m_SaveInterval;

        IntEdit *m_pieInterval;
        QCheckBox *m_pchOpps, *m_pchWOpps;
        QCheckBox *m_pchAutoSave, *m_pchAutoLoadLast;
};

