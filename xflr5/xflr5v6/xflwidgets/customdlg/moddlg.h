/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/


#pragma once

#include <QDialog>
#include <QTextEdit>
#include <QLabel>
#include <QDialogButtonBox>



class ModDlg : public QDialog
{
    Q_OBJECT

    public:
        ModDlg(QWidget *pParent);

    public:
        void initDialog();
        void setQuestion(QString quest) {m_Question=quest;}

    private slots:
        void onSaveAsNew();
        void onButton(QAbstractButton*pButton);

    private:
        void setupLayout();
        void keyPressEvent(QKeyEvent *pEvent) override;

    private:
        QLabel * m_plabQuestion;
        QDialogButtonBox *m_pButtonBox;
        QPushButton *m_pSaveNewButton;

        QString m_Question;
};




