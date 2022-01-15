/****************************************************************************

    xflr5 v6
    Copyright (C) Andre Deperrois 
    GNU General Public License v3

*****************************************************************************/

#include <QOpenGLPaintDevice>
#include <QKeyEvent>
#include <QFrame>
#include <QVBoxLayout>

#include "gl3dsurfaceplot.h"



#include <xfl3d/gl_globals.h>
#include <xfl3d/controls/w3dprefs.h>


gl3dSurfacePlot::gl3dSurfacePlot(QWidget *pParent) : gl3dSurface(pParent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    m_ValMin = 1e10;
    m_ValMax = -1e10;

    m_Size_x = 23;
    m_Size_y = 23;
    m_PointArray.resize(m_Size_x*m_Size_y);

    m_bResetSurface = true;

    m_bDisplaySurface = false;


    QFrame *pFrame = new QFrame(this);
    {
        QPalette palette;
        palette.setColor(QPalette::WindowText, s_TextColor);
        palette.setColor(QPalette::Text, s_TextColor);

        QColor clr = s_BackgroundColor;
        clr.setAlpha(0);
        palette.setColor(QPalette::Window, clr);
        palette.setColor(QPalette::Base, clr);
        pFrame->setCursor(Qt::ArrowCursor);
        //        pFrame->setAutoFillBackground(true);
        pFrame->setPalette(palette);
        pFrame->setFrameShape(QFrame::NoFrame);
        pFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        QVBoxLayout *pMainLayout = new QVBoxLayout;
        {
            m_pchGrid    = new QCheckBox("Grid");
            connect(m_pchGrid, SIGNAL(clicked(bool)), SLOT(update()));
            m_pchGrid->setChecked(true);
            m_pchContour = new QCheckBox("Contour lines");
            connect(m_pchContour, SIGNAL(clicked(bool)), SLOT(update()));


            pMainLayout->addWidget(m_pchGrid);
            pMainLayout->addWidget(m_pchContour);
        }

        pFrame->setLayout(pMainLayout);
    }
}


void gl3dSurfacePlot::setTriangle(Vector3d* vertices)
{
    for(int i=0; i<3; i++) m_TriangleVertex[i] = vertices[i];
}


gl3dSurfacePlot::~gl3dSurfacePlot()
{
}


void gl3dSurfacePlot::glRenderView()
{
    if(m_bDisplaySurface)
    {
        paintColourMap(m_vboSurface);
        if(m_bGrid) paintGrid();
        if(m_pchContour->isChecked())
            paintSegments(m_vboContourLines, W3dPrefs::s_OutlineStyle);
    }
    //    paintPanel2d();
    paintTriangle(m_vboTriangle, false, true, Qt::darkRed);
    paintPolygon();
    for(int i=0; i<m_Vertices.size(); i++)
    {
        paintSphere(m_Vertices.at(i), m_RefLength/100.0, Qt::red);
    }

}


void gl3dSurfacePlot::glMake3dObjects()
{
    if(m_bResetSurface)
    {
        glMakeSurface();
        QVector<double> nodevalues(m_PointArray.size());
        for(int i=0; i<m_PointArray.size(); i++) nodevalues[i] = m_PointArray.at(i).z;
        glMakeQuadContoursOnGrid( m_vboContourLines, m_Size_x, m_Size_y, m_PointArray, nodevalues, true);

        //		m_pglStdBuffers->glMakeTriangle(m_TriangleVertex[0], m_TriangleVertex[1], m_TriangleVertex[2]);
        glMakePolygon();
    }
    m_bResetSurface = false;
}


void gl3dSurfacePlot::glMakePolygon()
{
    if(m_Vertices.size()<=0) return;
    int polyArraySize = (m_Vertices.size()+1) * 3; // three vertex components
    QVector<float> pPolyVertexArray(polyArraySize);

    int iVtx=0;
    for(int i=0; i<m_Vertices.size(); i++)
    {
        pPolyVertexArray[iVtx++] = m_Vertices.at(i).xf();
        pPolyVertexArray[iVtx++] = m_Vertices.at(i).yf();
        pPolyVertexArray[iVtx++] = m_Vertices.at(i).zf();
    }
    // close it
    pPolyVertexArray[iVtx++] = m_Vertices.at(0).xf();
    pPolyVertexArray[iVtx++] = m_Vertices.at(0).yf();
    pPolyVertexArray[iVtx++] = m_Vertices.at(0).zf();
    Q_ASSERT(iVtx==polyArraySize);

    m_vboPolygon.destroy();
    m_vboPolygon.create();
    m_vboPolygon.bind();
    m_vboPolygon.allocate(pPolyVertexArray.data(), pPolyVertexArray.size()* int(sizeof(float)));

    m_vboPolygon.release();
}



void gl3dSurfacePlot::paintPolygon()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    QMatrix4x4 idMatrix;
    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);


        m_vboPolygon.bind();
        m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
        m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));

        m_shadLine.setUniformValue(m_locLine.m_UniColor, QColor(255,0,0));

        if(m_bUse120StyleShaders) glLineWidth(5);
        else m_shadLine.setUniformValue(m_locLine.m_Thickness, 5);
        m_shadLine.setUniformValue(m_locLine.m_Pattern, GLStipple(Line::SOLID));

        if(m_vboPolygon.size()>=0)
        {
            glDrawArrays(GL_LINE_STRIP, 0, m_Vertices.size());
        }

        m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
        m_vboPolygon.release();
    }
    m_shadLine.release();
}
