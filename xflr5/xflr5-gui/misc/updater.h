/****************************************************************************

    Updater Class
    Copyright (C) 2018-2019 Andre Deperrois

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

#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QSettings>

class MainFrame;

class Updater : public QObject
{
    Q_OBJECT

public:
    Updater(MainFrame *pMainFrame);
    ~Updater();

    bool hasUpdate();
    QString releaseDate()        const {return m_Date;}
    QString releaseDescription() const {return m_Description;}



    static void setAutoCheck(bool bAuto) {s_bAutoCheck=bAuto;}
    static bool bAutoCheck() {return s_bAutoCheck;}

    static QDate lastCheckDate() {return s_LastCheckDate;}
    static void setLastCheckDate(QDate &date) {s_LastCheckDate=date;}

    static int majorVersion() {return s_AvailableMajorVersion;}
    static int minorVersion() {return s_AvailableMinorVersion;}

    static void loadSettings(QSettings &settings);
    static void saveSettings(QSettings &settings);

signals:
    void finishedUpdate();

public slots:
    void checkForUpdates();

private slots:
    void onDownloadFinished(QNetworkReply *pNetworkReply);


    void onReplyFinished(QNetworkReply*pNetReply);
    void onReadyRead();
    void slotError(QNetworkReply::NetworkError neterror);
    void slotSslErrors(QVector<QSslError> sslerrors);


private:
    QNetworkAccessManager *m_pNetworkAcessManager;
//    QNetworkReply * m_pNetworkReply;
    QString m_Date, m_Description;

    static MainFrame *s_pMainFrame;

    static bool s_bAutoCheck;
    static int s_AvailableMajorVersion;
    static int s_AvailableMinorVersion;
    static QDate s_LastCheckDate;
};



