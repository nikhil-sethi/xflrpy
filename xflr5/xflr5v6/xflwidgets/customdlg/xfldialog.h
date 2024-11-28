/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QDialog>
#include <QLabel>
#include <QDialogButtonBox>


class XflDialog : public QDialog
{
    Q_OBJECT
    public:
        XflDialog(QWidget *pParent);

        bool bChanged() const {return m_bChanged;}
        bool bDescriptionChanged() const {return m_bDescriptionChanged;}

        static void setConfirmDiscard(bool bConfirm) {s_bConfirmDiscard=bConfirm;}

    protected slots:
        void reject() override;
        virtual void keyPressEvent(QKeyEvent *pEvent) override;
        virtual void onButton(QAbstractButton*pButton);
        virtual void onApply() {} // base class method does nothing
        virtual void onRestore() {} // base class method does nothing


    protected:
        void setButtons(QDialogButtonBox::StandardButtons buttons);

    protected:
        QDialogButtonBox *m_pButtonBox;

        bool m_bChanged;
        bool m_bDescriptionChanged;
        static bool s_bConfirmDiscard;
};


