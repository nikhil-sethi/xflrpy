/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#include <QPushButton>
#include <QPainter>
#include <QPaintEvent>
#include <QVBoxLayout>
#include <QColorDialog>

#include "colorgraddlg.h"
#include <xflcore/xflcore.h>
#include <xflwidgets/color/colorbtn.h>

ColorGradDlg::ColorGradDlg(QVector<QColor>const &clrs, QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Colour gradients");

    setAutoFillBackground(true);
    m_Clr=clrs;

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Discard, this);
    {
        m_prb2Colors = new QRadioButton("2 colors");
        m_prb3Colors = new QRadioButton("3 colors");
        m_prb2Colors->setAutoFillBackground(true);
        m_prb3Colors->setAutoFillBackground(true);
        connect(m_prb2Colors, SIGNAL(clicked(bool)), SLOT(onNColors()));
        connect(m_prb3Colors, SIGNAL(clicked(bool)), SLOT(onNColors()));

        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
        m_pButtonBox->addButton(m_prb2Colors, QDialogButtonBox::ActionRole);
        m_pButtonBox->addButton(m_prb3Colors, QDialogButtonBox::ActionRole);
    }

    m_pDiscreteClrFrame = new QFrame(this);
    makeCtrlFrameLayout();
    if(m_Clr.size()==2)
    {
        m_prb2Colors->setChecked(true);
    }
    else
    {
         m_prb3Colors->setChecked(true);
    }
}


void ColorGradDlg::makeCtrlFrameLayout()
{
    qDeleteAll(m_pDiscreteClrFrame->children()); // removes the layout  and destroys the color btns

/*    for(int i=0; i<m_pColorBtn.size(); i++)
    {
        delete m_pColorBtn[i];
        m_pColorBtn[i] = nullptr;
    } */

    m_pColorBtn.resize(m_Clr.size());
    QVBoxLayout*pClrBtnLayout = new QVBoxLayout;
    {
        for(int i=m_pColorBtn.size()-1; i>=0; i--)
        {
            m_pColorBtn[i] = new ColorBtn;
            m_pColorBtn[i]->setColor(m_Clr.at(i));
            connect(m_pColorBtn[i], SIGNAL(clickedCB(QColor)), SLOT(onColorBtn()));
            pClrBtnLayout->addWidget(m_pColorBtn[i]);
            if(i>0) pClrBtnLayout->addStretch();
        }
    }
    m_pDiscreteClrFrame->setLayout(pClrBtnLayout);
}


void ColorGradDlg::on2ColorsDefault()
{
    if(m_Clr.size()!=2)
    {
        m_Clr.resize(2);
/*        for(int i=0; i<m_pColorBtn.size(); i++)   delete m_pColorBtn[i];
        m_pColorBtn.resize(2);*/
    }

    m_Clr[0].setRgb(255,255,255);
    m_Clr[1].setRgb( 56, 44, 35);

    makeCtrlFrameLayout();
    update();

}


void ColorGradDlg::on3ColorsDefault()
{
    if(m_Clr.size()!=3)
    {
        m_Clr.resize(3);

        for(int i=0; i<m_Clr.size(); i++)
        {
//           m_ClrPos << double(i)/double(m_Clr.size()-1);
        }

/*        for(int i=0; i<m_pColorBtn.size(); i++)   delete m_pColorBtn[i];
        m_pColorBtn.resize(m_Clr.size());*/
    }
    m_Clr[0] = Qt::red;
    m_Clr[1] = Qt::green;
    m_Clr[2] = Qt::blue;

    makeCtrlFrameLayout();
    update();
}


void ColorGradDlg::onNColors()
{
     QRadioButton *pSenderBtn = dynamic_cast<QRadioButton*>(sender());
     if(pSenderBtn==m_prb2Colors)
     {
         on2ColorsDefault();
     }
     else if(pSenderBtn==m_prb3Colors)
     {
         on3ColorsDefault();
     }
}


void ColorGradDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok) == pButton)       accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)  reject();
}



void ColorGradDlg::resizeEvent(QResizeEvent *pEvent)
{
    QDialog::resizeEvent(pEvent);

    showEvent(nullptr);
}


void ColorGradDlg::showEvent(QShowEvent *)
{
    if(!isVisible()) return;

    m_pDiscreteClrFrame->setMinimumWidth(100);
    m_pDiscreteClrFrame->setMinimumHeight(height()-10);
    m_pDiscreteClrFrame->adjustSize();

    QPoint pos2(5,5);
    m_pDiscreteClrFrame->move(pos2);

    int h = height();
    int w = width();

    QPoint posbox(w-m_pButtonBox->width()-5, h-m_pButtonBox->height()-5);
    m_pButtonBox->move(posbox);
}


void ColorGradDlg::onColorBtn()
{
    ColorBtn *pClrbtn = qobject_cast<ColorBtn *>(sender());

    QColor BtnColor = pClrbtn->color();
    BtnColor = QColorDialog::getColor(BtnColor);
    if(BtnColor.isValid())
    {
        pClrbtn->setColor(BtnColor);
        for(int i=0; i<m_pColorBtn.size(); i++)
        {
            if(pClrbtn==m_pColorBtn[i])
            {
                m_Clr[i] = BtnColor;
            }
        }
    }

    update();
}

#define NDISCRETE 5



void ColorGradDlg::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.save();

/*    QLinearGradient grad;
    grad.setStart    (rect().center().x(), rect().top());
    grad.setFinalStop(rect().center().x(), rect().bottom());
    for(int i=0; i<m_Clr.size(); i++)
    {
        double frac = double(i)/double(m_Clr.size()-1);
        grad.setColorAt(frac, m_Clr.at(i));
    }
    QBrush brush(grad);
    painter.fillRect(rect(), brush); */


    QLinearGradient   DiscreteGradient;
    DiscreteGradient.setStart    (rect().center().x(), rect().top());
    DiscreteGradient.setFinalStop(rect().center().x(), rect().bottom());

    for (int i=0; i<NDISCRETE; i++)
    {
        float fi = float(i)/float(NDISCRETE-1);
//        QColor clr = QColor(int(LegendColor::GLGetRed(fi)*255), int(LegendColor::GLGetGreen(fi)*255), int(LegendColor::GLGetBlue(fi)*255));
        QColor clr = colour(1-fi);

        DiscreteGradient.setColorAt(double(fi), clr);
    }
    QBrush discretebrush(DiscreteGradient);
    painter.fillRect(rect(), discretebrush);

    painter.restore();
}


QColor ColorGradDlg::colour(float tau) const
{
    if(tau<=0.0f) return m_Clr.first();

    double df = double(m_Clr.size()-1);
    for(int i=1; i<m_Clr.size(); i++)
    {
        double fi  = double(i)/df;
        if(tau<fi)
        {
            double hue0 = std::max(0.0, m_Clr.at(i-1).hueF()); // hue returns -1 if grey
            double sat0 = m_Clr.at(i-1).saturationF();
            double val0 = m_Clr.at(i-1).valueF();
            double hue1 = std::max(0.0, m_Clr.at(i).hueF());
            double sat1 = m_Clr.at(i).saturationF();
            double val1 = m_Clr.at(i).valueF();

            double t = (fi-tau)/(1/df);

            // hue is undefined for pure grey colors, so use the other color's hue
            if(sat0<0.005 && sat1>0.005) hue0 = hue1;
            if(sat0>0.005 && sat1<0.005) hue1 = hue0;

            double hue = t*hue0+(1-t)*hue1;
            double sat = t*sat0+(1-t)*sat1;
            double val = t*val0+(1-t)*val1;

            return QColor::fromHsvF(hue, sat, val); // does not accept negative hue


/*            double r0 = m_Clr.at(i-1).redF();
            double g0 = m_Clr.at(i-1).greenF();
            double b0 = m_Clr.at(i-1).blueF();
            double r1 = m_Clr.at(i).redF();
            double g1 = m_Clr.at(i).greenF();
            double b1 = m_Clr.at(i).blueF();

            double t = (fi-tau)/(1/df);

            double red   = t*r0+(1-t)*r1;
            double green = t*g0+(1-t)*g1;
            double blue  = t*b0+(1-t)*b1;
            return QColor::fromRgbF(red, green, blue);*/
        }
    }
    return m_Clr.last();
}




