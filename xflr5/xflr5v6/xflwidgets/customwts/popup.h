/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QWidget>
#include <QLabel>



class PopUp : public QWidget
{
    Q_OBJECT

    public:
        PopUp(QWidget *pParent=nullptr);
        PopUp(QString const &message, QWidget *pParent);
        void appendTextMessage(QString const &text);
        void setTextMessage(QString const &text);
        void setDuration(int time_s) {m_Duration=time_s*1000;}
        void setRed();
        void setGreen();
        void setFont(QFont const &fnt);

    protected:
        void showEvent(QShowEvent *) override;
        void mousePressEvent(QMouseEvent *event) override;

    private:
        void setupLayout();


    private:
        QLabel *m_plabMessage;
        int m_Duration; //ms
};

