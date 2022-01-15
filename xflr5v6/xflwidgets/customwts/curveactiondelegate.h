/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QItemDelegate>



namespace xfl
{
    typedef enum {STRINGITEM, INTEGERITEM, DOUBLEITEM, LINEITEM, CHECKBOXITEM, ACTIONITEM} enumItemType;
}


class CurveActionDelegate : public QItemDelegate
{
    Q_OBJECT

    public:
        CurveActionDelegate(QObject *pParent = nullptr);
        QWidget *createEditor(QWidget *pParent, const QStyleOptionViewItem &, const QModelIndex &index) const override;

        void setEditorData(QWidget *pEditor, const QModelIndex &index) const override;
        void setModelData(QWidget *pEditor, QAbstractItemModel *pModel,
                          const QModelIndex &index) const override;
        void updateEditorGeometry(QWidget *pEditor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;


        void setActionColumn(int iActionCol) {m_iActionColumn=iActionCol;}

        void setPrecision(QVector<int> const &PrecisionList) {m_Precision = PrecisionList;}

        void setItemType(int idx, xfl::enumItemType itemtype) {if(idx>=0 && idx<m_ItemType.size()) m_ItemType[idx]=itemtype;}
        void setItemTypes(QVector<xfl::enumItemType> itemtypes) {m_ItemType=itemtypes;}

    private:
        QVector<int> m_Precision; ///table of float precisions for each column
        int m_iActionColumn;

        QVector<xfl::enumItemType> m_ItemType;
};

