/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QList>
#include <QItemDelegate>
#include <QStandardItemModel>
#include <xflwidgets/customwts/doubleedit.h>

class CtrlTableDelegate : public QItemDelegate
{
    Q_OBJECT

    public:
        CtrlTableDelegate(QObject *parent = nullptr);

        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        void setEditorData(QWidget *pEditor, const QModelIndex &index) const;
        void setModelData(QWidget *pEditor, QAbstractItemModel *pAbstractItemModel, const QModelIndex &index) const;
        void updateEditorGeometry(QWidget *pEditor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        void drawCheck(QPainter *painter, const QStyleOptionViewItem &option, const QRect &, Qt::CheckState state) const;
        bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

        void setPrecision(QVector<int> const &precision) {m_Precision=precision;}
        void setEditable(QVector<bool> const &editable) {m_bEditable=editable;}

    private:
        QVector<int> m_Precision; ///table of float precisions for each column
        QVector<bool> m_bEditable;
};










