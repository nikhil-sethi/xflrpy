/****************************************************************************

    xflr5 v6
    Copyright (C) Andre Deperrois 
    GNU General Public License v3

*****************************************************************************/

#include <QGroupBox>
#include <QGridLayout>

#include "gl3dshadow.h"


#include <xfl3d/controls/gllightdlg.h>
#include <xfl3d/controls/w3dprefs.h>
#include <xfl3d/gl_globals.h>
#include <xflcore/trace.h>
#include <xflgeom/geom_globals.h>
#include <xflgeom/geom3d/triangle3d.h>
#include <xflwidgets/customwts/exponentialslider.h>
#include <xflwidgets/wt_globals.h>


gl3dShadow::gl3dShadow(QWidget *pParent) : gl3dTestGLView(pParent)
{
    setWindowTitle("Shadows");
    m_pglLightDlg = new GLLightDlg;
    m_pglLightDlg->setgl3dView(this);

    m_bResetObjects = true;
    m_fboDepthMap=0;
    m_texDepthMap=0;

    m_uDepthLightViewMatrix = -1;

    m_attrDepthPos = -1;

    QPalette palette;
    palette.setColor(QPalette::WindowText, s_TextColor);
    palette.setColor(QPalette::Text,       s_TextColor);

    QColor clr = s_BackgroundColor;
    clr.setAlpha(0);
    palette.setColor(QPalette::Window, clr);
    palette.setColor(QPalette::Base,   clr);

    QFrame *pFrame = new QFrame(this);
    {
        pFrame->setCursor(Qt::ArrowCursor);

        pFrame->setFrameShape(QFrame::NoFrame);
        pFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        pFrame->setMinimumWidth(350);
        QVBoxLayout *pMainLayout = new QVBoxLayout;
        {
            QGroupBox *pObjBox = new QGroupBox("Object position, model space");
            {
                QGridLayout*pObjectLayout = new QGridLayout;
                {
                    QLabel *plabX = new QLabel("X:");
                    QLabel *plabY = new QLabel("Y:");
                    QLabel *plabZ = new QLabel("Z:");

                    m_peslXObj   = new ExponentialSlider(Qt::Horizontal);
                    m_peslXObj->setMinimum(0);
                    m_peslXObj->setMaximum(100);
                    m_peslXObj->setTickInterval(10);
                    m_peslXObj->setTickPosition(QSlider::TicksBelow);
                    connect(m_peslXObj, SIGNAL(sliderMoved(int)), SLOT(onObjectPos()));

                    m_peslYObj = new ExponentialSlider(Qt::Horizontal);
                    m_peslYObj->setMinimum(0);
                    m_peslYObj->setMaximum(100);
                    m_peslYObj->setTickInterval(10);
                    m_peslYObj->setTickPosition(QSlider::TicksBelow);
                    connect(m_peslYObj, SIGNAL(sliderMoved(int)), SLOT(onObjectPos()));

                    m_peslZObj  = new ExponentialSlider(false, 1, Qt::Horizontal);
                    m_peslZObj->setMinimum(0);
                    m_peslZObj->setMaximum(100);
                    m_peslZObj->setTickInterval(10);
                    m_peslZObj->setTickPosition(QSlider::TicksBelow);
                    connect(m_peslZObj, SIGNAL(sliderMoved(int)), SLOT(onObjectPos()));

                    pObjectLayout->addWidget(plabX ,     2, 1);
                    pObjectLayout->addWidget(m_peslXObj, 2, 2);

                    pObjectLayout->addWidget(plabY,      3, 1);
                    pObjectLayout->addWidget(m_peslYObj, 3, 2);

                    pObjectLayout->addWidget(plabZ,      4, 1);
                    pObjectLayout->addWidget(m_peslZObj, 4, 2);

                    pObjectLayout->setColumnStretch(2,1);
                }
                pObjBox->setLayout(pObjectLayout);
            }


            m_pchLightDlg = new QCheckBox("Light settings");
            connect(m_pchLightDlg, SIGNAL(clicked(bool)), this, SLOT(onLightSettings(bool)));

            pMainLayout->addWidget(pObjBox);
            pMainLayout->addWidget(m_pchLightDlg);
        }
        pFrame->setLayout(pMainLayout);
        pFrame->setStyleSheet("QFrame{background-color: transparent;}");
        setWidgetStyle(pFrame, palette);
    }

    m_Object.set(0,0,0.25);

    m_peslXObj->setRange(-100, 100);
    m_peslYObj->setRange(-100, 100);
    m_peslZObj->setRange(   0, 200);

    m_peslXObj->setExpValuef(m_Object.x*100.0f);
    m_peslYObj->setExpValuef(m_Object.y*100.0f);
    m_peslZObj->setExpValuef(m_Object.z*100.0f);

    connect(&m_CtrlTimer, SIGNAL(timeout()), SLOT(onControls()));
    m_CtrlTimer.start(250);
}


gl3dShadow::~gl3dShadow()
{
    delete m_pglLightDlg;
    m_pglLightDlg = nullptr;
}


void gl3dShadow::onLightSettings(bool bShow)
{
    m_bLightVisible = bShow;
    if(m_pglLightDlg) m_pglLightDlg->setVisible(bShow);
    update();
}


void gl3dShadow::onControls()
{
    if(m_pchLightDlg->isChecked() && m_pglLightDlg && !m_pglLightDlg->isVisible())
    {
        m_pchLightDlg->setChecked(false);
        m_bLightVisible = false;
        update();
    }
}


void gl3dShadow::onObjectPos()
{
    m_Object.x = m_peslXObj->expValuef()/100.0f;
    m_Object.y = m_peslYObj->expValuef()/100.0f;
    m_Object.z = m_peslZObj->expValuef()/100.0f;

//    glSetupLight();

    m_bResetObjects = true;
    update();
}


void gl3dShadow::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dShadow");
    {
    }
    settings.endGroup();
}


void gl3dShadow::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dShadow");
    {
    }
    settings.endGroup();
}


void gl3dShadow::initializeGL()
{
    gl3dView::initializeGL();
    //setup the depth shader
    QString vsrc(":/resources/shaders/shadow/depth_VS.glsl");
    QString fsrc(":/resources/shaders/shadow/depth_FS.glsl");
    m_shadDepth.addShaderFromSourceFile(QOpenGLShader::Vertex, vsrc);
    if(m_shadDepth.log().length())
    {
        QString strange = QString::asprintf("%s", QString("Depth vertex shader log:"+m_shadDepth.log()).toStdString().c_str());
        Trace(strange);
    }

    m_shadDepth.addShaderFromSourceFile(QOpenGLShader::Fragment, fsrc);
    if(m_shadDepth.log().length())
    {
        QString strange = QString::asprintf("%s", QString("Depth fragment shader log:"+m_shadDepth.log()).toStdString().c_str());
        Trace(strange);
    }

    m_shadDepth.link();
    m_shadDepth.bind();
    {
        m_attrDepthPos = m_shadDepth.attributeLocation("vertexPosition_modelSpace");
        m_uDepthLightViewMatrix = m_shadDepth.uniformLocation("LightViewMatrix");
        m_shadDepth.setUniformValue(m_uDepthLightViewMatrix, m_LightViewMatrix);
    }
    m_shadDepth.release();
}


void gl3dShadow::glMake3dObjects()
{
    if(m_bResetObjects)
    {
        double side = 1;

        QVector<Triangle3d> triangles;
        makeSphere(side/8.0, 2, triangles);
        for(int i=0; i<triangles.size(); i++)
            triangles[i].translate(m_Object);

        glMakeTriangles3Vtx(triangles, true, m_vboSphere);
        glMakeTrianglesOutline(triangles, Vector3d(), m_vboSphereEdges);

        glMakeQuadTex(side, m_vboQuad);

//        setReferenceLength(2*side);
        setReferenceLength(1.0);
        m_bResetObjects = false;
    }
}


void gl3dShadow::glRenderView()
{
    updateLightMatrix();

    // 1. first render to depth map
    renderToShadowMap();

    // 2. then render scene as normal with shadow mapping (using depth map)
    renderToScreen();
}


void gl3dShadow::renderToShadowMap()
{
    initDepthMap();

    // Render into the depth framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboDepthMap);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glClear(GL_DEPTH_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    paintTrianglesToDepthMap(m_vboSphere, QMatrix4x4(), 6);
    paintTrianglesToDepthMap(m_vboQuad,   QMatrix4x4(), 8); //vbo has texture

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void gl3dShadow::renderToScreen()
{
    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,   vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix,  pvmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_TexSampler, 0); //TEXTURE0  is the default anyway
        m_shadSurf.setUniformValue(m_locSurf.m_HasUniColor, 1);
//        m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 1);
        m_shadSurf.setUniformValue(m_locSurf.m_ClipPlane, m_ClipPlanePos);
        m_shadSurf.setUniformValue(m_locSurf.m_Viewport, QVector2D(float(m_GLViewRect.width()), float(m_GLViewRect.height())));
    }
    m_shadSurf.release();

    glViewport(0,0,width()*devicePixelRatio(), height()*devicePixelRatio());
    glCullFace(GL_BACK);

    glActiveTexture(GL_TEXTURE0); // to be consistent with the default sampler2d
    glBindTexture(GL_TEXTURE_2D, m_texDepthMap);

    paintTriangles3VtxShadow(m_vboSphere, Qt::yellow,  false, s_Light.m_bIsLightOn, QMatrix4x4(), 6);
    paintTriangles3VtxShadow(m_vboQuad,   Qt::darkRed, true,  s_Light.m_bIsLightOn, QMatrix4x4(), 8);

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);
    }
    m_shadLine.release();
    paintSegments(m_vboSphereEdges, W3dPrefs::s_OutlineStyle);
}


void gl3dShadow::initDepthMap()
{
    if(m_fboDepthMap != 0)
        return;

    // Create a texture to store the depth map
    glGenTextures(1, &m_texDepthMap);
    glBindTexture(GL_TEXTURE_2D, m_texDepthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // Create a frame-buffer and associate the texture with it.
    glGenFramebuffers(1, &m_fboDepthMap);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboDepthMap);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_texDepthMap, 0);

    // Let OpenGL know that we are not interested in colors for this buffer
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // Cleanup for now.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}


void gl3dShadow::paintTrianglesToDepthMap(QOpenGLBuffer &vbo, QMatrix4x4 const &ModelMat, int stride)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadDepth.bind();
    {
//        m_Shader_Depth.setUniformValue(m_uDepthLightViewMatrix, ModelMat*m_LightViewMatrix);
        m_shadDepth.setUniformValue(m_uDepthLightViewMatrix, m_LightViewMatrix*ModelMat);
        m_shadDepth.enableAttributeArray(m_attrDepthPos);

        vbo.bind();
        {
            int nTriangles = vbo.size()/3/stride/int(sizeof(float)); // three vertices and (3 position components+3 normal components)
            m_shadDepth.setAttributeBuffer(m_attrDepthPos, GL_FLOAT, 0, 3, stride*sizeof(GLfloat));
            glDrawArrays(GL_TRIANGLES, 0, nTriangles*3); // 4 vertices defined but only 3 are used
            m_shadDepth.disableAttributeArray(m_attrDepthPos);
        }
        vbo.release();
    }
    m_shadDepth.release();
}


void gl3dShadow::paintTriangles3VtxShadow(QOpenGLBuffer &vbo, const QColor &backclr, bool bTwoSided, bool bLight,
                                        QMatrix4x4 const &modelmat, int stride)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_UniColor, backclr);

//        m_Shader_Surf.setUniformValue(m_uShadowLightViewMatrix, modelmat*m_LightViewMatrix);
        m_shadSurf.setUniformValue(m_uShadowLightViewMatrix, m_LightViewMatrix*modelmat);

        if(bLight)
        {
            m_shadSurf.setUniformValue(m_locSurf.m_Light, 1);
            m_shadSurf.setUniformValue(m_uHasShadow, 1);
        }
        else
        {
            m_shadSurf.setUniformValue(m_locSurf.m_Light, 0);
            m_shadSurf.setUniformValue(m_uHasShadow, 0);
        }

        if(bTwoSided)
        {
            m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 1);
            glDisable(GL_CULL_FACE);
        }
        else
        {
            m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 0);
            glEnable(GL_CULL_FACE);
        }

        m_shadSurf.enableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.enableAttributeArray(m_locSurf.m_attrNormal);

        vbo.bind();
        int nTriangles = vbo.size()/3/stride/int(sizeof(float)); // three vertices and (3 position components+3 normal components)

        m_shadSurf.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0,                 3, stride*sizeof(GLfloat));
        m_shadSurf.setAttributeBuffer(m_locSurf.m_attrNormal, GL_FLOAT, 3*sizeof(GLfloat), 3, stride*sizeof(GLfloat));
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);

        glDrawArrays(GL_TRIANGLES, 0, nTriangles*3);

        glDisable(GL_POLYGON_OFFSET_FILL);

        m_shadSurf.disableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.disableAttributeArray(m_locSurf.m_attrNormal);
        m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 0); // leave things as they were
        glEnable(GL_CULL_FACE);
    }
    m_shadSurf.release();

    vbo.release();
}


void gl3dShadow::paintTriangles3VtxTexture(QOpenGLBuffer &vbo, QOpenGLTexture *pTexture, bool bTwoSided, bool bLight)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    int stride  = 8; // three vertices and (3 position components+3 normal components+2UV components)
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_HasTexture, 1);

        m_shadSurf.setUniformValue(m_uShadowLightViewMatrix, m_LightViewMatrix);
        m_shadSurf.setUniformValue(m_uHasShadow, 0);

        if(bLight) m_shadSurf.setUniformValue(m_locSurf.m_Light, 1);
        else       m_shadSurf.setUniformValue(m_locSurf.m_Light, 0);


        if(bTwoSided)
        {
            m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 1);
            glDisable(GL_CULL_FACE);
        }
        else
        {
            m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 0);
            glEnable(GL_CULL_FACE);
        }

        m_shadSurf.enableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.enableAttributeArray(m_locSurf.m_attrNormal);
        m_shadSurf.enableAttributeArray(m_locSurf.m_attrUV);

        pTexture->bind();
        {
            vbo.bind();
            {
                int nTriangles = vbo.size()/3/stride/int(sizeof(float));

                m_shadSurf.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0,                 3, stride*sizeof(GLfloat));
                m_shadSurf.setAttributeBuffer(m_locSurf.m_attrNormal, GL_FLOAT, 3*sizeof(GLfloat), 3, stride*sizeof(GLfloat));
                m_shadSurf.setAttributeBuffer(m_locSurf.m_attrUV,     GL_FLOAT, 6*sizeof(GLfloat), 2, stride*sizeof(GLfloat));
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);

                glDrawArrays(GL_TRIANGLES, 0, nTriangles*3); // 4 vertices defined but only 3 are used

                glDisable(GL_POLYGON_OFFSET_FILL);
            }
            vbo.release();
        }
        pTexture->release();

        m_shadSurf.disableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.disableAttributeArray(m_locSurf.m_attrNormal);
        m_shadSurf.disableAttributeArray(m_locSurf.m_attrUV);
        // leave things as they were
        m_shadSurf.setUniformValue(m_locSurf.m_TwoSided,   0);
        m_shadSurf.setUniformValue(m_locSurf.m_HasTexture, 0);


        glEnable(GL_CULL_FACE);
    }
    m_shadSurf.release();
}

