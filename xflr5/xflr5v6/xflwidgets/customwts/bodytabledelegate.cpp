/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#include <xflcore/xflcore.h>
#include <xflwidgets/customwts/doubleedit.h>


#include "bodytabledelegate.h"


BodyTableDelegate::BodyTableDelegate(QObject *parent) : QItemDelegate(parent)
{
}


QWidget *BodyTableDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex & index ) const
{
    DoubleEdit *pEditor = new DoubleEdit(parent);
    pEditor->setAlignment(Qt::AlignRight);

    pEditor->setDigits(m_Precision[index.column()]);
    return pEditor;
}


void BodyTableDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    double value = index.model()->data(index, Qt::EditRole).toDouble();
    DoubleEdit *pDE = static_cast<DoubleEdit*>(editor);
    pDE->setValue(value);
}


void BodyTableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    DoubleEdit *pDE = static_cast<DoubleEdit*>(editor);
    pDE->readValue();
    model->setData(index, pDE->value(), Qt::EditRole);
}


void BodyTableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString strange;
    QStyleOptionViewItem myOption = option;
    myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;

    double dble = index.model()->data(index, Qt::DisplayRole).toDouble();

    if(xfl::isLocalized())
        strange = QString("%L1").arg(dble ,0,'f',  m_Precision.at(index.column()));
    else
        strange = QString("%1").arg(dble ,0,'f',  m_Precision.at(index.column()));

    drawDisplay(painter, myOption, myOption.rect, strange);
    drawFocus(painter, myOption, myOption.rect);
}


void BodyTableDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}










