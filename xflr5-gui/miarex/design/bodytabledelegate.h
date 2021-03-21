/****************************************************************************

    BodyTableDelegate Class
    Copyright (C) 2009 Andre Deperrois 

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

#ifndef BODYTABLEDELEGATE_H
#define BODYTABLEDELEGATE_H

#include <QVector>
#include <QItemDelegate>


class BodyTableDelegate : public QItemDelegate
{
    Q_OBJECT
    friend class GL3dBodyDlg;

public:
    BodyTableDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setPointers(int *PrecisionTable, int *pNRows);
    void setPointer(int *PrecisionTable);

private:
    int *m_pNRows;
    int *m_Precision; ///table of float precisions for each column
};

#endif // BODYTABLEDELEGATE_H








