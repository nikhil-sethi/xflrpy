/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef GLRENDERWINDOW_H
#define GLRENDERWINDOW_H

#include <QWindow>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QTextStream>
#include <QOpenGLTexture>

QT_FORWARD_DECLARE_CLASS(QOpenGLContext)
QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

class GLRenderWindow : public QWindow
{
    Q_OBJECT

public:
	GLRenderWindow(const QSurfaceFormat &format);
	~GLRenderWindow();
	QOpenGLContext *context() { return m_pContext; }
	void exposeEvent(QExposeEvent *) Q_DECL_OVERRIDE;
	void setForceGLSL110(bool enable) { m_forceGLSL110 = enable; }

signals:
    void ready();
    void error(const QString &msg);

private slots:
    void render();

private:
	void init();
	void setupVertexAttribs();
	void makeObject();
	void rotateBy(int xAngle, int yAngle, int zAngle);

	QOpenGLContext *m_pContext;
	QOpenGLTexture *m_pOpenGLTexture;
	QOpenGLShaderProgram *m_pShaderProgram;

	bool m_initialized;
	bool m_forceGLSL110;
	int m_posAttr, m_colAttr, m_matrixUniform;
	QOpenGLVertexArrayObject m_vao;
	QOpenGLBuffer m_vbo;
	float m_angle;
	int xRot, yRot, zRot;
};

#endif // RENDERWINDOW_H
