/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#include <QPainter>


#include "curveactiondelegate.h"
#include <xflwidgets/wt_globals.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflcore/xflcore.h>
#include <xflcore/linestyle.h>
#include <xflwidgets/line/linemenu.h>

CurveActionDelegate::CurveActionDelegate(QObject *pParent) : QItemDelegate(pParent)
{
    m_Precision.clear();
    m_iActionColumn = -1;
}


QWidget *CurveActionDelegate::createEditor(QWidget *pParent, const QStyleOptionViewItem &, const QModelIndex & index) const
{
    if(index.column()==m_iActionColumn)
    {
        return nullptr;
    }

    xfl::enumItemType type = m_ItemType.at(index.column());
    switch (type)
    {
        case xfl::STRINGITEM:
        {
            QLineEdit *pEditor = new QLineEdit(pParent);
            pEditor->setAlignment(Qt::AlignLeft);
            return pEditor;
        }
        case xfl::INTEGERITEM:
        {
            IntEdit *pEditor = new IntEdit(pParent);
            int value = index.model()->data(index, Qt::EditRole).toInt();
            pEditor->setValue(value);// redundant with setEditorData?
            return pEditor;
        }
        case xfl::DOUBLEITEM:
        {
            DoubleEdit *pEditor = new DoubleEdit(pParent);
            pEditor->setDigits(m_Precision[index.column()]);
            double value = index.model()->data(index, Qt::EditRole).toDouble();
            pEditor->setValue(value);// redundant with setEditorData?
            return pEditor;
        }
        case xfl::CHECKBOXITEM:
        {
            break;
        }
        case xfl::LINEITEM:
        {
/*            LineMenu *pEditor = new LineMenu(pParent);
            LineStyle ls = index.model()->data(index, Qt::EditRole).value<LineStyle>();
            pEditor->setTheStyle(ls);// redundant with setEditorData?
            pEditor->move(option.rect.center());
            return pEditor;*/
            break;
        }
        case xfl::ACTIONITEM:
        {
            break;
        }
    }
    return nullptr;
}


void CurveActionDelegate::setEditorData(QWidget *pEditor, const QModelIndex &index) const
{
    if(index.column()==m_iActionColumn)
    {
        return;
    }
    xfl::enumItemType type = m_ItemType.at(index.column());
    switch (type)
    {
        case xfl::STRINGITEM:
        {
            QLineEdit *pLineEdit = static_cast<QLineEdit*>(pEditor);
            pLineEdit->setText(index.model()->data(index, Qt::EditRole).toString());
            break;
        }
        case xfl::INTEGERITEM:
        {
            int value = index.model()->data(index, Qt::EditRole).toInt();
            IntEdit *pDE = static_cast<IntEdit*>(pEditor);
            pDE->setValueNoFormat(value);
            break;
        }
        case xfl::DOUBLEITEM:
        {
            double value = index.model()->data(index, Qt::EditRole).toDouble();
            DoubleEdit *pDE = static_cast<DoubleEdit*>(pEditor);
            pDE->setValueNoFormat(value);
            break;
        }
        case xfl::CHECKBOXITEM:
        {
            break;
        }
        case xfl::LINEITEM:
        {
/*            LineStyle ls = index.model()->data(index, Qt::EditRole).value<LineStyle>();
            LineMenu *pLM = static_cast<LineMenu*>(pEditor);
            pLM->setTheStyle(ls); */
            break;
        }
        case xfl::ACTIONITEM:
        {
            break;
        }
    }
}


void CurveActionDelegate::setModelData(QWidget *pEditor, QAbstractItemModel *pModel, const QModelIndex &index) const
{
    if(index.column()==m_iActionColumn)
    {
        return;
    }

    xfl::enumItemType type = m_ItemType.at(index.column());
    switch (type)
    {
        case xfl::STRINGITEM:
        {
            QLineEdit *pLineEdit = static_cast<QLineEdit*>(pEditor);
//            pModel->setData(index, pLineEdit->text(), Qt::DisplayRole); // not sure
            pModel->setData(index, pLineEdit->text(), Qt::EditRole);
            break;
        }
        case xfl::INTEGERITEM:
        {
            IntEdit *pIE = static_cast<IntEdit*>(pEditor);
            pIE->readValue();
            pModel->setData(index, pIE->value(), Qt::EditRole);
            break;
        }
        case xfl::DOUBLEITEM:
        {
            DoubleEdit *pDE = static_cast<DoubleEdit*>(pEditor);
            pDE->readValue();
            pModel->setData(index, pDE->value(), Qt::EditRole);
            break;
        }
        case xfl::CHECKBOXITEM:
        {
            break;
        }
        case xfl::LINEITEM:
        {
/*            LineMenu *pLM = static_cast<LineMenu*>(pEditor);
            LineStyle ls = pLM->theStyle();
            pModel->setData(index, QVariant::fromValue(ls), Qt::EditRole); */
            break;
        }
        case xfl::ACTIONITEM:
        {
            QLineEdit *pLineEdit = static_cast<QLineEdit*>(pEditor);
            pModel->setData(index, pLineEdit->text(), Qt::EditRole);
            break;
        }
    }
}


void CurveActionDelegate::updateEditorGeometry(QWidget *pEditor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    pEditor->setGeometry(option.rect);
}


void CurveActionDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString strong;
    QStyleOptionViewItem myOption = option;
    QColor linecolor;
    int row = index.row();
    int col = index.column();
    (void)row;
    (void)col;

    xfl::enumItemType type = m_ItemType.at(index.column());
    switch (type)
    {
        case xfl::STRINGITEM:
        {
            myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
            QItemDelegate::paint(painter, myOption, index);
            break;
        }
        case xfl::INTEGERITEM:
        {
            myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
            QItemDelegate::paint(painter, myOption, index);
            break;
        }
        case xfl::DOUBLEITEM:
        {
            myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
            QItemDelegate::paint(painter, myOption, index);
            break;
        }
        case xfl::CHECKBOXITEM:
        {
            Qt::CheckState checkstate=Qt::PartiallyChecked;

            if     (index.data(Qt::CheckStateRole)==2) checkstate=Qt::Checked;
            else if(index.data(Qt::CheckStateRole)==1) checkstate=Qt::PartiallyChecked;
            else if(index.data(Qt::CheckStateRole)==0) checkstate=Qt::Unchecked;

            QColor backclr = option.palette.window().color();
            QColor crossclr = option.palette.windowText().color();
            QFontMetrics fm(option.font);

            if(option.state & QStyle::State_Selected)
            {
                backclr = option.palette.highlight().color();
                drawCheckBox(painter, checkstate, option.rect, fm.height(), true, false, crossclr, backclr);
            }
            else
                drawCheckBox(painter, checkstate, option.rect, fm.height(), false, false, crossclr, backclr);
            break;
        }
        case xfl::LINEITEM:
        {
            if (index.data(Qt::DisplayRole).canConvert<LineStyle>())
            {
                LineStyle ls = qvariant_cast<LineStyle>(index.data());
                QRect r = option.rect;

                QItemDelegate::paint(painter, option, index); // paint the background, using palette colors, including stylesheet mods

                if(ls.m_bIsEnabled)
                {
                    painter->save();
                    QPen LinePen;
                    if(ls.m_bIsEnabled)
                    {
                        linecolor = ls.m_Color;
                        LinePen.setStyle(xfl::getStyle(ls.m_Stipple));
                        LinePen.setWidth(ls.m_Width);
                    }
                    else
                    {
                        linecolor = Qt::gray;
                        LinePen.setStyle(xfl::getStyle(Line::SOLID));
                        LinePen.setWidth(1);
                    }
                    LinePen.setColor(linecolor);
                    painter->setPen(LinePen);
                    painter->drawLine(r.left()+5, r.center().y(), r.right()-5, r.center().y());

                    LinePen.setStyle(Qt::SolidLine);
                    painter->setPen(LinePen);

                    QColor backcolor;
                    if (option.state & QStyle::State_Selected) backcolor = option.palette.highlight().color();
                    else                                       backcolor = option.palette.base().color();

                    if(ls.m_Symbol>0 && ls.m_bIsEnabled) xfl::drawSymbol(*painter, ls.m_Symbol, backcolor, linecolor, r.center());
                    painter->restore();
                }
            }
            else
            {
                QItemDelegate::paint(painter, option, index);
            }
            break;
        }
        case::xfl::ACTIONITEM:
        {
            QItemDelegate::paint(painter, option, index);
        }
    }
}



