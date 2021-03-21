/****************************************************************************

    View3dTestDlg Class
    Copyright (C) 2019 Andre Deperrois

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*****************************************************************************/

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDebug>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QPushButton>
#include <QRadioButton>
#include <QSplitter>
#include <QSurfaceFormat>
#include <QTextEdit>
#include <QVBoxLayout>

#include "view3dtestdlg.h"

#include <viewwidgets/glWidgets/gl3dtestview.h>


struct Version {
    const char *str;
    int major;
    int minor;
};


static struct Version versions[] = {
    { "1.0", 1, 0 },
    { "1.1", 1, 1 },
    { "1.2", 1, 2 },
    { "1.3", 1, 3 },
    { "1.4", 1, 4 },
    { "1.5", 1, 5 },
    { "2.0", 2, 0 },
    { "2.1", 2, 1 },
    { "3.0", 3, 0 },
    { "3.1", 3, 1 },
    { "3.2", 3, 2 },
    { "3.3", 3, 3 },
    { "4.0", 4, 0 },
    { "4.1", 4, 1 },
    { "4.2", 4, 2 },
    { "4.3", 4, 3 },
    { "4.4", 4, 4 },
    { "4.5", 4, 5 }
};


struct Profile {
    const char *str;
    QSurfaceFormat::OpenGLContextProfile profile;
};


static struct Profile profiles[] = {
    { "none", QSurfaceFormat::NoProfile },
    { "core", QSurfaceFormat::CoreProfile },
    { "compatibility", QSurfaceFormat::CompatibilityProfile }
};


struct Option {
    const char *str;
    QSurfaceFormat::FormatOption option;
};

static struct Option options[] = {
    { "deprecated functions (not forward compatible)", QSurfaceFormat::DeprecatedFunctions },
    { "debug context", QSurfaceFormat::DebugContext },
    { "stereo buffers", QSurfaceFormat::StereoBuffers }
    // This is not a QSurfaceFormat option but is helpful to determine if the driver
    // allows compiling old-style shaders with core profile.

};


View3dTestDlg::View3dTestDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle("OpenGL context info");
    setupLayout();
}


void View3dTestDlg::addVersions(QLayout *layout)
{
    QHBoxLayout *pHBoxLayout = new QHBoxLayout;
    {
        pHBoxLayout->setSpacing(20);
        QLabel *label = new QLabel(tr("Context &version: "));
        pHBoxLayout->addWidget(label);
        m_pVersionCbBox = new QComboBox;
        m_pVersionCbBox->setMinimumWidth(60);
        label->setBuddy(m_pVersionCbBox);
        pHBoxLayout->addWidget(m_pVersionCbBox);
        for (size_t i = 0; i < sizeof(versions) / sizeof(Version); ++i) {
            m_pVersionCbBox->addItem(QString::fromLatin1(versions[i].str));
            if (versions[i].major == 3 && versions[i].minor == 3)
                m_pVersionCbBox->setCurrentIndex(m_pVersionCbBox->count() - 1);
        }

        QPushButton *pBtn = new QPushButton(tr("Create context"));
        connect(pBtn, SIGNAL(clicked()), this, SLOT(onCreateContext()));
        pBtn->setMinimumSize(120, 40);
        pHBoxLayout->addWidget(pBtn);
    }

    layout->addItem(pHBoxLayout);
}


void View3dTestDlg::addProfiles(QLayout *layout)
{
    QGroupBox *pGroupBox = new QGroupBox(tr("Profile"));
    {
        QVBoxLayout *pVBoxLayout = new QVBoxLayout;
        {
            for (size_t i = 0; i < sizeof(profiles) / sizeof(Profile); ++i)
                pVBoxLayout->addWidget(new QRadioButton(QString::fromLatin1(profiles[i].str)));
            static_cast<QRadioButton *>(pVBoxLayout->itemAt(0)->widget())->setChecked(true);
        }
        pGroupBox->setLayout(pVBoxLayout);
        m_profiles = pVBoxLayout;
    }
    layout->addWidget(pGroupBox);
}


void View3dTestDlg::addOptions(QLayout *pLayout)
{
    QGroupBox *pGroupBox = new QGroupBox(tr("Options"));
    {
        QVBoxLayout *pVBoxLayout = new QVBoxLayout;
        {
            for (size_t i = 0; i < sizeof(options) / sizeof(Option); ++i)
                pVBoxLayout->addWidget(new QCheckBox(QString::fromLatin1(options[i].str)));
            pGroupBox->setLayout(pVBoxLayout);
        }
        m_pOptionsLayout = pVBoxLayout;
    }
    pLayout->addWidget(pGroupBox);
}


void View3dTestDlg::keyPressEvent(QKeyEvent *pEvent)
{
    //    bool bShift = false;
    //    if(event->modifiers() & Qt::ShiftModifier)   bShift =true;
    bool bCtrl = (pEvent->modifiers() & Qt::ControlModifier);
    switch (pEvent->key())
    {
        case Qt::Key_W:
            if(bCtrl) reject();
            break;
        case Qt::Key_Escape:
            reject();
            break;
        default:
            QWidget::keyPressEvent(pEvent);
    }
    pEvent->ignore();
}



void View3dTestDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QSplitter *pHSplit = new QSplitter;
        {
            QWidget *pSettings = new QWidget();
            {
                QVBoxLayout *pSettingsLayout = new QVBoxLayout;
                {
                    addVersions(pSettingsLayout);
                    addProfiles(pSettingsLayout);
                    addOptions(pSettingsLayout);
                }
                pSettings->setLayout(pSettingsLayout);
            }
            pHSplit->addWidget(pSettings);

            m_pctrlglOutput = new QPlainTextEdit;
            m_pctrlglOutput->setReadOnly(true);
            m_pctrlglOutput->setFont(QFont("Courier"));
            pHSplit->addWidget(m_pctrlglOutput);

            pHSplit->setStretchFactor(1, 3);
        }
        m_pStackWt = new QStackedWidget;
        {
            m_pgl3dTestView = new gl3dTestView;
            m_pStackWt->addWidget(m_pgl3dTestView);
            connect(m_pgl3dTestView, SIGNAL(ready()), this, SLOT(onRenderWindowReady()));
        }

        pMainLayout->addWidget(pHSplit);
        pMainLayout->addWidget(m_pStackWt);
    }
    setLayout(pMainLayout);
}


void View3dTestDlg::onCreateContext()
{
    QSurfaceFormat fmt;

    int idx = m_pVersionCbBox->currentIndex();
    if (idx < 0)  return;
    fmt.setVersion(versions[idx].major, versions[idx].minor);

    for (size_t i=0; i<sizeof(profiles)/sizeof(Profile); ++i)
    {
        if (static_cast<QRadioButton *>(m_profiles->itemAt(int(i))->widget())->isChecked()) {
            fmt.setProfile(profiles[i].profile);
            break;
        }
    }

    for (size_t i=0; i<sizeof(options)/sizeof(Option); ++i)
    {
        if (static_cast<QCheckBox *>(m_pOptionsLayout->itemAt(int(i))->widget())->isChecked()) {
            if (options[i].option)
                fmt.setOption(options[i].option);
        }
    }


    fmt.setRenderableType(QSurfaceFormat::OpenGL);

    m_pctrlglOutput->clear();

    m_pStackWt->removeWidget(m_pgl3dTestView);
    delete m_pgl3dTestView;

    m_pgl3dTestView = new gl3dTestView;
    connect(m_pgl3dTestView, SIGNAL(ready()), this, SLOT(onRenderWindowReady()));
//    connect(m_pgl3dView, &GLRenderWindow::error, this, &View3dTestDlg::renderWindowError);

    m_pgl3dTestView->setFormat(fmt);

    m_pStackWt->addWidget(m_pgl3dTestView);
    m_pStackWt->setCurrentWidget(m_pgl3dTestView);
    m_pgl3dTestView->repaint(); // force context initialization

    if (!m_pgl3dTestView->context())
    {
        m_pctrlglOutput->appendPlainText("Failed to create context");
//        delete m_pgl3dTestView;
        return;
    }
}


void View3dTestDlg::printFormat(const QSurfaceFormat &format)
{
    m_pctrlglOutput->appendPlainText(tr("   OpenGL version: %1.%2").arg(format.majorVersion()).arg(format.minorVersion()));

    for (size_t i=0; i<sizeof(profiles) / sizeof(Profile); ++i)
        if (profiles[i].profile == format.profile()) {
            m_pctrlglOutput->appendPlainText(tr("   Profile: %1").arg(QString::fromLatin1(profiles[i].str)));
            break;
        }

    QString opts;
    for (size_t i=0; i<sizeof(options) / sizeof(Option); ++i)
        if (format.testOption(options[i].option))
            opts += QString::fromLatin1(options[i].str) + QStringLiteral(" ");
    m_pctrlglOutput->appendPlainText(QString("Options: %1").arg(opts));
    m_pctrlglOutput->appendPlainText(QString("Depth buffer size: %1").arg(QString::number(format.depthBufferSize())));
    m_pctrlglOutput->appendPlainText(QString("Stencil buffer size: %1").arg(QString::number(format.stencilBufferSize())));
    m_pctrlglOutput->appendPlainText(QString("Samples: %1").arg(QString::number(format.samples())));
    m_pctrlglOutput->appendPlainText(QString("Red buffer size: %1").arg(QString::number(format.redBufferSize())));
    m_pctrlglOutput->appendPlainText(QString("Green buffer size: %1").arg(QString::number(format.greenBufferSize())));
    m_pctrlglOutput->appendPlainText(QString("Blue buffer size: %1").arg(QString::number(format.blueBufferSize())));
    m_pctrlglOutput->appendPlainText(QString("Alpha buffer size: %1").arg(QString::number(format.alphaBufferSize())));
    m_pctrlglOutput->appendPlainText(QString("Swap interval: %1").arg(QString::number(format.swapInterval())));

}



void View3dTestDlg::onRenderWindowReady()
{
    QOpenGLContext *pContext = QOpenGLContext::currentContext();
    pContext = m_pgl3dTestView->context();

    QString vendor, renderer, version, glslVersion;
    const GLubyte *p;
    QOpenGLFunctions *f = pContext->functions();
    if ((p = f->glGetString(GL_VENDOR)))
        vendor = QString::fromLatin1(reinterpret_cast<const char *>(p));
    if ((p = f->glGetString(GL_RENDERER)))
        renderer = QString::fromLatin1(reinterpret_cast<const char *>(p));
    if ((p = f->glGetString(GL_VERSION)))
        version = QString::fromLatin1(reinterpret_cast<const char *>(p));
    if ((p = f->glGetString(GL_SHADING_LANGUAGE_VERSION)))
        glslVersion = QString::fromLatin1(reinterpret_cast<const char *>(p));

    m_pctrlglOutput->appendPlainText(QString("*** Context information ***"));
    m_pctrlglOutput->appendPlainText(QString("   Vendor: %1").arg(vendor));
    m_pctrlglOutput->appendPlainText(QString("   Renderer: %1").arg(renderer));
    m_pctrlglOutput->appendPlainText(QString("   OpenGL version: %1").arg(version));
    m_pctrlglOutput->appendPlainText(QString("   GLSL version: %1").arg(glslVersion));

    m_pctrlglOutput->appendPlainText("\n*** QSurfaceFormat from context ***");
    printFormat(pContext->format());

    m_pctrlglOutput->appendPlainText("\n*** QSurfaceFormat from QOpenGLWidget ***");
    printFormat(m_pgl3dTestView->format());

    m_pctrlglOutput->appendPlainText("\n*** Qt build information ***");
    const char *gltype[] = { "Desktop", "GLES 2", "GLES 1" };
    m_pctrlglOutput->appendPlainText(QString("   Qt OpenGL configuration: %1")
                     .arg(QString::fromLatin1(gltype[QOpenGLContext::openGLModuleType()])));
    m_pctrlglOutput->appendPlainText(QString("   Qt OpenGL library handle: %1")
                     .arg(QString::number(qintptr(QOpenGLContext::openGLModuleHandle()), 16)));

    m_pctrlglOutput->moveCursor(QTextCursor::Start);

    m_pctrlglOutput->appendPlainText("\n*** OpenGL support: ***");
    QString strange;
    strange = "   Desktop OpenGL";
    qApp->testAttribute(Qt::AA_UseDesktopOpenGL)? strange += ": true" : strange+=": false";
    m_pctrlglOutput->appendPlainText(strange);

    strange = "   OpenGL ES";
    qApp->testAttribute(Qt::AA_UseOpenGLES)? strange += ": true" : strange+=": false";
    m_pctrlglOutput->appendPlainText(strange);

    strange = "   Software OpenGL";
    qApp->testAttribute(Qt::AA_UseSoftwareOpenGL)? strange += ": true" : strange+=": false";
    m_pctrlglOutput->appendPlainText(strange+"\n");

    m_pctrlglOutput->appendPlainText("*** Shaders ***");
    if(m_pgl3dTestView->bUsing120StyleShaders())
        m_pctrlglOutput->appendPlainText("   Using glsl 120 style shaders\n");
    else
        m_pctrlglOutput->appendPlainText("   Using glsl 330 style shaders\n");
}


void View3dTestDlg::onRenderWindowError(const QString &msg)
{
    m_pctrlglOutput->appendPlainText(QString("An error has occurred:\n%1").arg(msg));
}


