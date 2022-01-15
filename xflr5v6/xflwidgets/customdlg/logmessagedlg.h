/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/


#pragma once


#include <QDialog>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QSettings>



class PlainTextOutput;


class LogMessageDlg : public QWidget
{
    Q_OBJECT

    public:
        LogMessageDlg(QWidget *pParent=nullptr);

        void initDialog(QString title, QString props);

        void setOutputFont(QFont const &fnt);

        const QString outputText() const;
        void setOnTop();

        QSize sizeHint() const override {return QSize(750,550);}

    signals:
        void closeLog();

    protected:
        void keyPressEvent(QKeyEvent *pEvent) override;

    public slots:
        void onAppendMessage(QString const &msg);
        void onClose();
        void clearText();
        void onButton(QAbstractButton *pButton);

    private:
        void setupLayout();

    private:
        PlainTextOutput *m_pptoOutputLog;
        QPushButton *m_ppbClearButton;
        QDialogButtonBox *m_pButtonBox;
};
