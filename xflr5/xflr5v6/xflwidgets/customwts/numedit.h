/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once


#include <QLineEdit>
#include <QKeyEvent>



class NumEdit : public QLineEdit
{
    Q_OBJECT
    public:
        NumEdit(QWidget *pWidget);

    protected:
        void showEvent(QShowEvent*pEvent) override;
        void focusInEvent (QFocusEvent * pEvent) override;
        void focusOutEvent (QFocusEvent * pEvent) override;
        QSize sizeHint() const override;

        virtual void readValue() = 0;
        virtual void formatValue() = 0;

    signals:
        void valueChanged();

    public slots:
        void paste();

};

