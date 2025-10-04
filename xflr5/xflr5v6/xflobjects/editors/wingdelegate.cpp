/****************************************************************************

    WingDelegate Class
    Copyright (C) 2009 André Deperrois 

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
 
#include <QComboBox>

#include <xflobjects/editors/wingdelegate.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflobjects/objects2d/objects2d.h>
#include <xflobjects/objects2d/foil.h>


WingDelegate::WingDelegate(QObject *parent)
 : QStyledItemDelegate(parent)
{
    m_pWingDlg = parent;

}


QWidget *WingDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex & index ) const
{
    if(index.column()!=5 && index.column()!=7 && index.column()!=9)
    {
        DoubleEdit *pEditor = new DoubleEdit(parent);
        pEditor->setAlignment(Qt::AlignRight);

        pEditor->setDigits(m_Precision[index.column()]);
        if(index.column()==6)
        {
            pEditor->setMin(1);
        }
        if(index.column()==8)
        {
            pEditor->setMin(1);
        }
        return pEditor;
    }
    else
    {
        QString strong;
        QComboBox *pEditor = new QComboBox(parent);
        pEditor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        //fill comboboxes here
        if(index.column()==5)
        {
            for(int i=0; i<Objects2d::foilCount(); i++)
            {
                Foil *pFoil = Objects2d::foilAt(i);
                strong = pFoil->name();
                pEditor->addItem(strong);
            }
        }
        else if(index.column()==7)
        {
            pEditor->addItem(tr("Uniform"));
            pEditor->addItem(tr("Cosine"));
        }
        else if(index.column()==9)
        {
            pEditor->addItem(tr("Uniform"));
            pEditor->addItem(tr("Cosine"));
            pEditor->addItem(tr("Sine"));
            pEditor->addItem(tr("-Sine"));
        }
        return pEditor;
    }
}


void WingDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if(index.column()!=5 && index.column()!=7 && index.column()!=9)
    {
        double value = index.model()->data(index, Qt::EditRole).toDouble();
        DoubleEdit *pDE = static_cast<DoubleEdit*>(editor);
        pDE->setValue(value);
    }
    else
    {
        QString strong = index.model()->data(index, Qt::EditRole).toString();
        QComboBox *pCbBox = static_cast<QComboBox*>(editor);
        int pos = pCbBox->findText(strong);
        if (pos>=0) pCbBox->setCurrentIndex(pos);
        else        pCbBox->setCurrentIndex(0);
    }
}


void WingDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if(index.column()!=5 && index.column()!=7 && index.column()!=9)
    {
        DoubleEdit *pDE = static_cast<DoubleEdit*>(editor);
        pDE->readValue();
        double value = pDE->value();
        model->setData(index, value, Qt::EditRole);
    }
    else
    {
        QString strong;
        QComboBox *pCbBox = static_cast<QComboBox*>(editor);
        int sel = pCbBox->currentIndex();
        if (sel >=0) strong = pCbBox->itemText(sel);
        model->setData(index, strong, Qt::EditRole);
    }
}


void WingDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString strong;
    QStyleOptionViewItem myOption = option;
    QAbstractItemModel const *pModel = index.model();

    if(index.column()!=5 && index.column()!=7 && index.column()!=9)
    {
        myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
        strong = QString("%L1").arg(pModel->data(index, Qt::DisplayRole).toDouble(),0,'f', m_Precision[index.column()]);
    }
    else
    {
        myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
        strong = pModel->data(index, Qt::DisplayRole).toString();
    }

    if(index.row()> pModel->rowCount()) strong=" ";
    if(index.row()== pModel->rowCount()-1)
    {
        if(index.column()==3 ||index.column()>=6) strong = " ";
    }

    painter->drawText(myOption.rect, myOption.displayAlignment , strong);
}


void WingDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}













