/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QFrame>
#include <QRadioButton>
#include <QDialog>
#include <QDialogButtonBox>

class ColorBtn;

class ColorGradDlg : public QDialog
{
    Q_OBJECT
    public:
        ColorGradDlg(QVector<QColor>const &clrs, QWidget *parent = nullptr);

    public slots:
        void onColorBtn();
        void onNColors();
        void on2ColorsDefault();
        void on3ColorsDefault();
        void onButton(QAbstractButton *pButton);
        QVector<QColor> &colours() {return  m_Clr;}

    protected:

        void resizeEvent(QResizeEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        QSize sizeHint() const override {return QSize(550,900);};
        void paintEvent(QPaintEvent *pEvent) override;

    private:
        QColor colour(float tau) const;
        void makeCtrlFrameLayout();

    private:

        QFrame *m_pDiscreteClrFrame;

        QVector<ColorBtn*> m_pColorBtn;
        QVector<QColor> m_Clr;

        QRadioButton *m_prb2Colors;
        QRadioButton *m_prb3Colors;
        QDialogButtonBox *m_pButtonBox;
};

