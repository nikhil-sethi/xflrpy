/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QDialog>
#include <QLabel>
#include <QDialogButtonBox>


class IntEdit;

class IntValueDlg : public QDialog
{
    public:
        IntValueDlg(QWidget *pParent);

        void setValue(int intvalue);
        int value() const;

        void setLabel(QString label) {m_pLabel->setText(label);}

    private:
        void setupLayout();

        void keyPressEvent(QKeyEvent *pEvent);
        void showEvent(QShowEvent *pEvent);

    private:
        QLabel *m_pLabel;
        QDialogButtonBox *m_pButtonBox;
        IntEdit *m_pieIntEdit;
};



