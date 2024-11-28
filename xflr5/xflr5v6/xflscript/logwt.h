/****************************************************************************

    LogWt Class

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

#include <QApplication>
#include <QTimer>
#include <QPushButton>
#include <QSettings>
#include <QPlainTextEdit>


class LogWt : public QWidget
{
    Q_OBJECT

    public:
        LogWt(QWidget *pParent=nullptr);

        QSize sizeHint() const override {return QSize(900,550);}
        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        QPushButton *ctrlButton() {return m_ppbButton;}


        void setCancelButton(bool bCancel);
        void setFinished(bool bFinished);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    public slots:
        void onUpdate(QString msg);

    private slots:
        void onCancelClose();

    private:
        void setupLayout();

        bool m_bFinished;

        QPlainTextEdit *m_ppteLogView;
        QPushButton *m_ppbButton;

        QTimer m_Timer;

        static QByteArray s_Geometry;
};


