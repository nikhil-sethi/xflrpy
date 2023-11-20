/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/


#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QKeyEvent>

class PlainTextOutput;

class TextDlg : public QDialog
{
    Q_OBJECT

    public:
        TextDlg(QString const &text, QWidget *pParent=nullptr);
        void keyPressEvent(QKeyEvent *pEvent) override;
        QSize sizeHint() const override {return  QSize(1000,500);}
        void setQuestion(QString const &quest) {m_plabQuestion->setText(quest);}
        QString const &newText() const {return m_NewText;}

    private:
        void setupLayout();

    private slots:
        void accept() override;
        void onButton(QAbstractButton*pButton);

    private:
        QDialogButtonBox *m_pButtonBox;
        QLabel *m_plabQuestion;
        PlainTextOutput *m_ppto;
        QString m_NewText;
};

