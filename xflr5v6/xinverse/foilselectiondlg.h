/****************************************************************************

    FoilSelectionDlg Classe
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
#include <QListWidget>
#include <QVector>
#include <QStringList>
#include <QAbstractButton>
#include <QDialogButtonBox>

class Foil;

class FoilSelectionDlg : public QDialog
{
    Q_OBJECT

    public:
        FoilSelectionDlg(QWidget *pParent);

        void setFoilName(QString name) {m_FoilName=name;}
        QString const &selectedFoilName() const {return m_FoilName;}
        void initDialog(const QVector<Foil *> *FoilList, const QStringList &FoilSelList);

        void setFoilList(QVector<Foil*> const &foilList) {m_FoilList = foilList;}
        QVector<Foil*> const &foilList() const {return m_FoilList;}
        QStringList const &foilSelectionList() const {return m_FoilSelectionList;}

    private slots:
        void onOK();
        void onSelChangeList(QListWidgetItem *);
        void onDoubleClickList(QListWidgetItem *);
        void onSelectAll();
        void onButton(QAbstractButton *pButton);

    private:
        void setupLayout();

        QDialogButtonBox *m_pButtonBox;
        QPushButton *m_ppbSelectAll;
        QListWidget *m_plwNameList;
        QString m_FoilName;

        QVector<Foil*> m_FoilList; /** in input, the array of all foil pointers, in output, pointers to the selected foils @todo make two lists*/
        QStringList m_FoilSelectionList;
};
