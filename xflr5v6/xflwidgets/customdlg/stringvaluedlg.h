/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include <QDialogButtonBox>

class StringValueDlg : public QDialog
{
    Q_OBJECT

    public:
        StringValueDlg(QWidget *pParent, QStringList const &list);

        void setStringList(QStringList const &list);
        QString selectedString() const;

    private:
        void setupLayout();

    private slots:
        void onItemDoubleClicked(QListWidgetItem *pItem);

    private:
        QStringList m_StringList;
        QListWidget *m_plwStrings;
        QDialogButtonBox *m_pButtonBox;
};

