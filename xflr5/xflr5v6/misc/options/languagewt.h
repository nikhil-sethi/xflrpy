/****************************************************************************

    LanguageWt Class
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

#include <QListWidget>
#include <QStringList>


class LanguageWt : public QWidget
{
    Q_OBJECT

    friend class MainFrame;
    public:
        LanguageWt(QWidget *pParent);

        void hideEvent(QHideEvent *pEvent) override;

    public slots:
        void readLanguage();

    private:
        void setupLayout();
        void initWidget();
        QStringList findQmFiles();
        QString languageName(const QString &qmFile);

        QListWidget *m_plwLanguageList;
        QMap<QString, QString> qmFileForLanguage;

        bool m_bChanged;

};

