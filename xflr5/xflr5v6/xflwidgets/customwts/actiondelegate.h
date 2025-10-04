/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QItemDelegate>

class ActionDelegate : public QItemDelegate
{
    Q_OBJECT

    public:
        ActionDelegate(QObject *pParent = nullptr);
        QWidget *createEditor(QWidget *pParent, const QStyleOptionViewItem &, const QModelIndex &index) const;

        void setEditorData(QWidget *pEditor, const QModelIndex &index) const;
        void setModelData(QWidget *pEditor, QAbstractItemModel *model, const QModelIndex &index) const;
        void updateEditorGeometry(QWidget *pEditor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

        void setDigits(QVector<int> const &PrecisionList) {m_Digits = PrecisionList;}
        void setActionColumn(int iActionCol) {m_iActionColumn=iActionCol;}
        void setCheckColumn(int iCheckCol) {m_iCheckColumn=iCheckCol;}

        void setName(QString const &name) {m_Name=name;}

        void setLabelFirstRow(bool bEnable) {m_bLabelFirstRow=bEnable;}



    private:
        QVector<int> m_Digits; ///table of float precisions for each column
        int m_iActionColumn; /// usually the last column; will display sort of an action button
        int m_iCheckColumn; /// usually the first column; will display a checkbox

        bool m_bLabelFirstRow; /// true if the first row contains text and not numbers

        QString m_Name; /// debug info

};

