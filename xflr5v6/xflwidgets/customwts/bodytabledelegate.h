/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once


#include <QItemDelegate>



class BodyTableDelegate : public QItemDelegate
{
    Q_OBJECT

    public:
        BodyTableDelegate(QObject *parent = nullptr);

        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        void setEditorData(QWidget *editor, const QModelIndex &index) const override;
        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
        void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        void setPrecision(QVector<int> PrecisionList){m_Precision = PrecisionList;}


    private:
        QVector<int> m_Precision; ///table of float precisions for each column

};






