/****************************************************************************

    RenameDlg Class
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
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QListWidget>


class RenameDlg : public QDialog
{
    Q_OBJECT

    public:
        RenameDlg(QWidget *pParent=nullptr);
        void initDialog(QStringList *pStrList, QString const &startName, QString const &question);
        void setOverwriteEnable(bool bEnable){m_bEnableOverwrite = bEnable;}

        QString newName() const {return m_pleName->text();}


    private slots:
        void onOverwrite();
        void onOK();
        void onSelChangeList(int);
        void onDoubleClickList(QListWidgetItem * pItem);
        void onButton(QAbstractButton *pButton);


    private:
        QSize sizeHint() const override {return QSize(500,800);}
        void showEvent(QShowEvent *) override;
        void hideEvent(QHideEvent *) override;
        void keyPressEvent(QKeyEvent *pEvent) override;
        void setupLayout();

        QStringList m_strArray;
        QString m_startName;
        QString m_strQuestion;
        bool m_bEnableOverwrite;
        bool m_bExists;

        QLabel      *m_plabMessage;
        QLineEdit   *m_pleName;
        QListWidget *m_plwNameList;
        QPushButton *m_ppbOverwrite;
        QDialogButtonBox *m_pButtonBox;


        static QByteArray s_Geometry;
};




