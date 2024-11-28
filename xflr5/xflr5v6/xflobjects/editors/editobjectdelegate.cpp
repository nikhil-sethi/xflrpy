/****************************************************************************

    ViewObjectDelegate Class
    Copyright (C) 2015 André Deperrois

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
#include <QPainter>
#include <QComboBox>


#include "editobjectdelegate.h"

#include <xflcore/core_enums.h>
#include <xflobjects/objects2d/foil.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/customwts/intedit.h>


QVector<Foil*> *EditObjectDelegate::s_poaFoil;


EditObjectDelegate::EditObjectDelegate(QWidget *pParent) : QStyledItemDelegate(pParent)
{
}


QWidget *EditObjectDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex & index ) const
{
    int dataType = index.model()->data(index, Qt::UserRole).toInt();

    switch (dataType)
    {
        case xfl::BOOLVALUE:
        {
            QComboBox *editor = new QComboBox(parent);
            editor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            //fill comboboxes here
            editor->addItem("true");
            editor->addItem("false");
            return editor;
        }
        case xfl::POLARTYPE:
        {
            QComboBox *editor = new QComboBox(parent);
            editor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            //fill comboboxes here
            editor->addItem("FIXEDSPEEDPOLAR");
            editor->addItem("FIXEDLIFTPOLAR");
            editor->addItem("FIXEDAOAPOLAR");
            editor->addItem("STABILITYPOLAR");
            editor->addItem("BETAPOLAR");
            return editor;
        }
        case xfl::ANALYSISMETHOD:
        {
            QComboBox *editor = new QComboBox(parent);
            editor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            //fill comboboxes here
            editor->addItem("LLTMETHOD");
            editor->addItem("VLMMETHOD");
            editor->addItem("PANELMETHOD");
            return editor;
        }
        case xfl::PANELDISTRIBUTION:
        {
            QComboBox *editor = new QComboBox(parent);
            editor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            //fill comboboxes here
            editor->addItem("INVERSESINE");
            editor->addItem("COSINE");
            editor->addItem("SINE");
            editor->addItem("UNIFORM");
            return editor;
        }
        case xfl::REFDIMENSIONS:
        {
            QComboBox *editor = new QComboBox(parent);
            editor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            //fill comboboxes here
            editor->addItem("PLANFORMREFDIM");
            editor->addItem("PROJECTEDREFDIM");
            editor->addItem("MANUALREFDIM");

            return editor;
        }
        case xfl::BODYTYPE:
        {
            QComboBox *editor = new QComboBox(parent);
            editor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            //fill comboboxes here
            editor->addItem("FLATPANELS");
            editor->addItem("NURBS");
            return editor;
        }
        case xfl::FOILNAME:
        {
            QComboBox *editor = new QComboBox(parent);
            editor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            //fill comboboxes here
            for(int i=0; i<s_poaFoil->size(); i++)
            {
                Foil *pFoil = s_poaFoil->at(i);
                editor->addItem(pFoil->name());
            }
            return editor;
        }
        case xfl::BOUNDARYCONDITION:
        {
            QComboBox *editor = new QComboBox(parent);
            editor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            //fill comboboxes here
            editor->addItem("DIRICHLET");
            editor->addItem("NEUMANN");
            return editor;
        }
        case xfl::WINGTYPE:
        {
            /*        QComboBox *editor = new QComboBox(parent);
            editor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            //fill comboboxes here
            editor->addItem("MAINWING");
            editor->addItem("SECONDWING");
            editor->addItem("ELEVATOR");
            editor->addItem("FIN");

            return editor;*/
            return nullptr;
        }
        case xfl::DOUBLEVALUE:
        {
            DoubleEdit *editor = new DoubleEdit(parent);
            editor->setDigits(3);
            return editor;
        }
        case xfl::INTEGER:
        {
            IntEdit *editor = new IntEdit(parent);
            return editor;
        }
        default:
        {
            //String case, only edit the first line with the polar name
            if(index.row()==0 && index.column()==2)  return new QLineEdit(parent);
            else                                     return nullptr;
        }
    }
}


void EditObjectDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    int dataType = index.model()->data(index, Qt::UserRole).toInt();
    if(dataType==xfl::INTEGER)
    {
        int value = index.model()->data(index, Qt::EditRole).toInt();
        IntEdit *pIE = static_cast<IntEdit*>(editor);
        pIE->setValue(value);
    }
    else if(dataType==xfl::DOUBLEVALUE)
    {
        double value = index.model()->data(index, Qt::EditRole).toDouble();
        DoubleEdit *pDE = static_cast<DoubleEdit*>(editor);
        pDE->setValue(value);
    }
    else if(dataType==xfl::STRING)
    {
        QString strong = index.model()->data(index, Qt::EditRole).toString();
        QLineEdit *pLineEdit = dynamic_cast<QLineEdit*>(editor);
        pLineEdit->setText(strong);
    }
    //    else if(dataType==BOOL || dataType==PANELDISTRIBUTION || dataType==FOILNAME || dataType==BODYTYPE)
    else
    {
        QString strong = index.model()->data(index, Qt::EditRole).toString();
        QComboBox *pCbBox = static_cast<QComboBox*>(editor);
        int pos = pCbBox->findText(strong);
        if (pos>=0) pCbBox->setCurrentIndex(pos);
        else        pCbBox->setCurrentIndex(0);
    }
}


void EditObjectDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    int dataType = index.model()->data(index, Qt::UserRole).toInt();
    if(dataType==xfl::INTEGER)
    {
        IntEdit *pIE = static_cast<IntEdit*>(editor);
        int value = pIE->value();
        model->setData(index, value, Qt::EditRole);
    }
    else if(dataType==xfl::DOUBLEVALUE)
    {
        DoubleEdit *pDE = static_cast<DoubleEdit*>(editor);
        double value = pDE->value();
        model->setData(index, value, Qt::EditRole);
    }
    else if(dataType==xfl::STRING)
    {
        QLineEdit *pLineEdit = dynamic_cast<QLineEdit*>(editor);
        model->setData(index, pLineEdit->text(), Qt::EditRole);
    }
    //    else if(dataType==BOOL || dataType==PANELDISTRIBUTION || dataType==FOILNAME ||
    //            dataType==BODYTYPE || dataType==POLARTYPE || dataType==ANALYSISMETHOD || dataType==REFDIMENSIONS)
    else
    {
        QString strong;
        QComboBox *pCbBox = static_cast<QComboBox*>(editor);
        int sel = pCbBox->currentIndex();
        if (sel >=0) strong = pCbBox->itemText(sel);
        model->setData(index, strong, Qt::EditRole);
    }
}


void EditObjectDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int dataType = index.model()->data(index, Qt::UserRole).toInt();
    QString strong;
    QStyleOptionViewItem myOption = option;
    if(dataType==xfl::INTEGER)
    {
        myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
        strong = QString("%L1").arg(index.model()->data(index, Qt::DisplayRole).toInt());
    }
    else if(dataType==xfl::DOUBLEVALUE)
    {
        myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
        double val = index.model()->data(index, Qt::DisplayRole).toDouble();
        if(fabs(val)<0.0001)      strong = QString("%L1").arg(val, 0,'g', 3);
        else if(fabs(val)>1000.0) strong = QString("%L1").arg(val, 0,'f', 1);
        else if(fabs(val)>100.0)  strong = QString("%L1").arg(val, 0,'f', 2);
        else                      strong = QString("%L1").arg(val, 0,'f', 3);
    }
    else if(dataType==xfl::STRING)
    {
        myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
        strong = index.model()->data(index, Qt::DisplayRole).toString();
    }
    else if( dataType==xfl::REFDIMENSIONS)
    {
        myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
        strong = index.model()->data(index, Qt::DisplayRole).toString();
    }
    //    else if(dataType==BOOL || dataType==PANELDISTRIBUTION || dataType==FOILNAME ||
    //            dataType==BODYTYPE || dataType==POLARTYPE || dataType==ANALYSISMETHOD || dataType==REFDIMENSIONS)
    else
    {
        myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
        strong = index.model()->data(index, Qt::DisplayRole).toString();
    }

    QFont myFont(myOption.font);

    /*    if(index.column()==0)
    {
        myFont.setWeight(QFont::Bold);
        painter->setFont(myFont);
        QPen textPen(Qt::red);
        painter->setPen(textPen);
    }
    else
    {
        myFont.setWeight(QFont::Normal);
        painter->setFont(myFont);
        QPen textPen(Qt::white);
        painter->setPen(textPen);
    }*/

    QFontMetrics fm(myFont);
    int w = int(double(fm.height())/2);//pixels

    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());

    QRect sR3 = myOption.rect;
    sR3.setLeft(sR3.left()+w);
    sR3.setRight(sR3.right()-w);
    painter->drawText(sR3, int(myOption.displayAlignment), strong);
}


void EditObjectDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}












