/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/



#include "actiondelegate.h"
#include <xflcore/xflcore.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/wt_globals.h>


ActionDelegate::ActionDelegate(QObject *pParent) : QItemDelegate(pParent)
{
    m_bLabelFirstRow = false;
    m_iCheckColumn  = -1;
    m_iActionColumn = -1;
    m_Digits.clear();
}


QWidget *ActionDelegate::createEditor(QWidget *pParent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
    if(index.column()==m_iCheckColumn)
    {
        return nullptr;
    }
    else if(index.column()==m_iActionColumn)
    {
        return nullptr;
    }
    else
    {
        if(m_bLabelFirstRow && index.row()==0)
        {
            QLineEdit *pEditor = new QLineEdit(pParent);
            pEditor->setAlignment(Qt::AlignLeft);
            return pEditor;
        }
        else
        {
            if(m_Digits[index.column()]>=0)
            {
                //we have a number
                DoubleEdit *pEditor = new DoubleEdit(pParent);
                pEditor->setDigits(m_Digits[index.column()]);
                double value = index.model()->data(index, Qt::EditRole).toDouble();
                pEditor->setValue(value);

                return pEditor;
            }
            else
            {
                //we have a string
                QLineEdit *pEditor = new QLineEdit(pParent);
                pEditor->setAlignment(Qt::AlignLeft);
                return pEditor;
            }
        }
    }
}


void ActionDelegate::setEditorData(QWidget *pEditor, const QModelIndex &index) const
{
    if(index.column()==m_iCheckColumn)
    {
    }
    else if(index.column()==m_iActionColumn)
    {
    }
    else
    {
        if(m_bLabelFirstRow && index.row()==0)
        {
            QLineEdit *pLineEdit = static_cast<QLineEdit*>(pEditor);
            pLineEdit->setText(index.model()->data(index, Qt::EditRole).toString());
        }
        else
        {
            if(m_Digits[index.column()]>=0)
            {
                double value = index.model()->data(index, Qt::EditRole).toDouble();
                DoubleEdit *pDE = static_cast<DoubleEdit*>(pEditor);
                pDE->setValueNoFormat(value);
            }
            else
            {
                QLineEdit *pLineEdit = static_cast<QLineEdit*>(pEditor);
                pLineEdit->setText(index.model()->data(index, Qt::EditRole).toString());
            }
        }
    }
}


void ActionDelegate::setModelData(QWidget *pEditor, QAbstractItemModel *model,
                                           const QModelIndex &index) const
{
    if(index.column()==m_iCheckColumn)
    {
        model->setData(index, QString(), Qt::EditRole); // to force the dataChanged signal and force repaint
    }
    else if(index.column()==m_iActionColumn)
    {
    }
    else
    {
        if(m_bLabelFirstRow && index.row()==0)
        {
            QLineEdit *pLineEdit = static_cast<QLineEdit*>(pEditor);
                model->setData(index, pLineEdit->text(), Qt::EditRole);
        }
        else
        {
            if(m_Digits.at(index.column())>=0)
            {
                DoubleEdit *pDE = static_cast<DoubleEdit*>(pEditor);
                pDE->readValue();
                model->setData(index, pDE->value(), Qt::EditRole);
            }
            else
            {
                QLineEdit *pLineEdit = static_cast<QLineEdit*>(pEditor);
                model->setData(index, pLineEdit->text(), Qt::EditRole);
            }
        }
    }
}


void ActionDelegate::updateEditorGeometry(QWidget *pEditor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    pEditor->setGeometry(option.rect);
}


void ActionDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString strange;
    QStyleOptionViewItem floatoption = option;

    int col = index.column();

    if(col==m_iCheckColumn)
    {
        QFontMetrics fm(option.font);

        QColor backcolor;
        QColor crosscolor;
        if (option.showDecorationSelected && (option.state & QStyle::State_Selected)) {
            QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                      ? QPalette::Normal : QPalette::Disabled;
            if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
                cg = QPalette::Inactive;

            backcolor = option.palette.color(cg, QPalette::Highlight);
            crosscolor = option.palette.color(cg, QPalette::HighlightedText);
        }
        else
        {
            backcolor = option.palette.base().color();
            crosscolor = option.palette.windowText().color();
        }
        bool bChecked = index.model()->data(index, Qt::UserRole).toBool();
        QItemDelegate::drawBackground(painter, option, index); // paint the background, using palette colors, including stylesheet mods
        drawCheckBox(painter, bChecked, option.rect, fm.height(), false, false, crosscolor, backcolor);
    }
    else if(col==m_iActionColumn)
    {
        floatoption.displayAlignment = Qt::AlignCenter;
        QVariant const & var = index.model()->data(index, Qt::DisplayRole);
//        QItemDelegate::paint(painter, floatoption, index);
        drawDisplay(painter, floatoption, floatoption.rect, var.toString());
        drawFocus(painter, floatoption, floatoption.rect);
    }

    else
    {
        bool bEnabled = true;
        if(m_iCheckColumn>=0)
        {
            // get the checked status in the action column
            QModelIndex sibling = index.sibling(index.row(), m_iCheckColumn);
            bEnabled = index.model()->data(sibling, Qt::UserRole).toBool();
        }
        if(!bEnabled)
        {
            floatoption.palette.setColor(QPalette::Normal,   QPalette::Text, Qt::gray);
            floatoption.palette.setColor(QPalette::Active,   QPalette::Text, Qt::gray);
            floatoption.palette.setColor(QPalette::Inactive, QPalette::Text, Qt::gray);
            floatoption.palette.setColor(QPalette::Disabled, QPalette::Text, Qt::gray);
            floatoption.font.setItalic(true);
        }
        QVariant const & var = index.model()->data(index, Qt::DisplayRole);
        if(!var.isNull() && var.isValid())
        {
            if(m_bLabelFirstRow && index.row()==0)
            {
                floatoption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
                floatoption.font.setWeight(QFont::Bold);
                strange = var.toString();
            }
            else
            {
                if(col<m_Digits.count() && m_Digits.at(col)>=0)
                {
                    bool bOK(false);
                    double dble = var.toDouble(&bOK);
                    if(bOK)
                    {
                        floatoption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
/*                        if(xfl::g_bLocalize) strange = QString("%L1").arg(dble, 0, 'f', m_Digits.at(col));
                        else            strange = QString("%1" ).arg(dble, 0, 'f', m_Digits.at(col));*/
                        if(xfl::isLocalized()) strange = QString("%L1").arg(dble, 0, 'g');
                        else                   strange = QString("%1" ).arg(dble, 0, 'g');
                    }
                    else
                        strange = var.toString();
                }
                else
                {
                    floatoption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
                    strange = var.toString();
                }
            }
        }

        drawDisplay(painter, floatoption, floatoption.rect, strange);
        drawFocus(painter, floatoption, floatoption.rect);
    }
}



