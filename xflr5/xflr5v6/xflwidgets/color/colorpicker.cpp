/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#include <QApplication>

#include <QColor>
#include <QColorDialog>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

#include <xflwidgets/color/colorpicker.h>
#include <xflwidgets/color/textclrbtn.h>


QColor ColorPicker::s_BackgroundColor = Qt::black;
QColor ColorPicker::s_TextColor = Qt::white;

int ColorPicker::s_nRows = 10;
int ColorPicker::s_nCols = 19;

bool ColorPicker::s_bDontUseNativeDlg = true;

QStringList ColorPicker::s_ColourList;
QStringList ColorPicker::s_ColourNames;

double ColorPicker::s_SatFactor = 1.0;

ColorPicker::ColorPicker(QWidget *pParent) : QWidget(pParent)
{
    setWindowFlag(Qt::Popup);
    setWindowFlag(Qt::FramelessWindowHint);
    setWindowFlag(Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_DeleteOnClose);
    setFocusPolicy(Qt::ClickFocus);


    m_pCB.resize(s_nRows*s_nCols);

    for(int ic=0; ic<s_ColourList.size(); ic++)
    {
        m_pCB[ic] = new ColorBtn;
        m_pCB[ic]->setColor(s_ColourList.at(ic));
        m_pCB[ic]->setToolTip(s_ColourNames.at(ic));
        connect(m_pCB[ic], SIGNAL(clickedCB(QColor)), SLOT(onClickedCB(QColor)));
    }
    setupLayout();

    setSaturation();

    m_Color = Qt::red;
}


void ColorPicker::setupLayout()
{
    QPalette palette;
    palette.setColor(QPalette::WindowText, s_TextColor);
    palette.setColor(QPalette::Window, s_BackgroundColor);

    setAutoFillBackground(true);
    setPalette(palette);

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
//        pMainLayout->setContentsMargins(QMargins(0,0,0,0));
        QGridLayout *pGridLayout = new QGridLayout;
        {
            pGridLayout->setContentsMargins(QMargins(0,0,0,0));
            pGridLayout->setHorizontalSpacing(0);
            pGridLayout->setVerticalSpacing(0);
            pGridLayout->setRowMinimumHeight(2,10);

            int p=0;
            for(int i=0; i<s_nRows; i++)
            {
                for(int j=0; j<s_nCols; j++)
                {
                    pGridLayout->addWidget(m_pCB[p++], i, j);
                }
            }
        }
        QGridLayout *pSliderLayout = new QGridLayout;
        {
            QLabel *pLabSat = new QLabel(tr("Saturation"));
            QLabel *pLabSatMin = new QLabel("0");
            QLabel *pLabSatMax = new QLabel("255");
            m_pslSaturation = new QSlider(Qt::Horizontal);
            m_pslSaturation->setRange(0,255);
            connect(m_pslSaturation, SIGNAL(sliderMoved(int)), SLOT(onSaturationMoved(int)));

            QLabel *pLabAlpha = new QLabel(tr("Alpha"));
            QLabel *pLabAlphaMin = new QLabel("0");
            QLabel *pLabAlphaMax = new QLabel("255");
            m_pslAlpha = new QSlider(Qt::Horizontal);
            m_pslAlpha->setRange(0,255);
            connect(m_pslAlpha, SIGNAL(sliderReleased()), SLOT(onAlphaReleased()));

            pSliderLayout->addWidget(pLabSat,          1,1, Qt::AlignRight | Qt::AlignVCenter);
            pSliderLayout->addWidget(pLabSatMin,       1,2);
            pSliderLayout->addWidget(m_pslSaturation,  1,3);
            pSliderLayout->addWidget(pLabSatMax,       1,4);

            pSliderLayout->addWidget(pLabAlpha,        2,1, Qt::AlignRight | Qt::AlignVCenter);
            pSliderLayout->addWidget(pLabAlphaMin,     2,2);
            pSliderLayout->addWidget(m_pslAlpha,       2,3);
            pSliderLayout->addWidget(pLabAlphaMax,     2,4);
        }

        m_pColorButton = new TextClrBtn();
        m_pColorButton->setText(QObject::tr("Other..."));
        m_pColorButton->setBackgroundColor(s_BackgroundColor);
        m_pColorButton->setTextColor(s_TextColor);
        m_pColorButton->setRoundedRect(false);
        m_pColorButton->setContour(false);

        connect(m_pColorButton, SIGNAL(clickedTB()), SLOT(onOtherColor()));

        pMainLayout->addLayout(pGridLayout);
        pMainLayout->addLayout(pSliderLayout);
        pMainLayout->addWidget(m_pColorButton);

    }
    setLayout(pMainLayout);
}


void ColorPicker::initDialog(QColor color)
{
    m_Color = color;
    m_pslSaturation->setValue(int(s_SatFactor*255.0));
    m_pslAlpha->setValue(color.alpha());
    setActiveColor(color);
    m_bChanged = false;
}


void ColorPicker::keyPressEvent(QKeyEvent *pEvent)
{
    switch(pEvent->key())
    {
        case Qt::Key_Escape:
        {
            hide();
            break;
        }
        default:
            return QWidget::keyPressEvent(pEvent);
    }
}


void ColorPicker::setActiveColor(const QColor &color)
{
    for(int p=0; p<s_nRows*s_nCols; p++)
    {
        if(m_pCB[p])
        {
            if(color.rgb()==m_pCB[p]->color().rgb())
            {
                m_pCB[p]->setCurrent(true);
                m_pCB[p]->setFocus();
                return;
            }
        }
    }
}


void ColorPicker::acceptColor()
{
    emit colorChanged(m_Color);
    close();
}


void ColorPicker::onOtherColor()
{
	QColorDialog::ColorDialogOptions dialogOptions = QColorDialog::ShowAlphaChannel;
#ifdef Q_OS_MAC
    if(s_bDontUseNativeDlg) dialogOptions |= QColorDialog::DontUseNativeDialog;
#endif
    QColor Color = QColorDialog::getColor(m_Color,
                                          this, "Select the color", dialogOptions);
    if(Color.isValid()) m_Color = Color;

    m_bChanged = true;
    acceptColor();
}


void ColorPicker::onClickedCB(QColor clr)
{
    m_Color = clr;
    m_Color.setAlpha(m_pslAlpha->value());

    m_bChanged = true;
    acceptColor();
}


void ColorPicker::onSaturationMoved(int val)
{
    s_SatFactor = double(val)/255.0;
    setSaturation();
    m_bChanged = true;
}


void ColorPicker::onAlphaReleased()
{
    int alpha = m_pslAlpha->value();
    m_Color.setAlpha(alpha);

    m_bChanged = true;
}


void ColorPicker::setSaturation()
{
    QStringList colours;
    for(int ic=0; ic<s_ColourList.size(); ic++)
    {
        QColor clr = s_ColourList.at(ic);
        clr.setHsvF(clr.hueF(), clr.saturationF()*s_SatFactor, clr.valueF());
        colours << clr.name();
        m_pCB[ic]->setColor(clr);
        m_pCB[ic]->setToolTip(clr.name());
    }
}


void ColorPicker::setColorList(QStringList const&colorlist)
{
    s_ColourList = colorlist;
    s_ColourNames = colorlist;
}


QColor ColorPicker::randomColor(bool bLightColor)
{
    int col = rand()%s_ColourList.size();
    int sat = rand()%155+100;
    QColor clr = s_ColourList.at(col);
    clr.setHsv(clr.hue(), sat, clr.value(), 255);

    if(bLightColor) return clr.lighter();
    else            return clr.darker();
}









