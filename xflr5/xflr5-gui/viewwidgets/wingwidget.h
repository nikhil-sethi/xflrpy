#ifndef WINGWIDGET_H
#define WINGWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QPixmap>
#include <objects3d/Plane.h>
#include <objects3d/PlaneOpp.h>
#include <QGraph.h>


class WingWidget : public QWidget
{
	Q_OBJECT
public:
	WingWidget(QWidget *pParent = 0);
	~WingWidget();


	void contextMenuEvent (QContextMenuEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void mouseDoubleClickEvent (QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void paintEvent(QPaintEvent *event);
	void resizeEvent (QResizeEvent *event);
	void wheelEvent (QWheelEvent *event);

	void setWingScale();
	void setWingGraph(QGraph *pGraph);

private:
	void paintXCmRef(QPainter &painter, QPointF ORef, double scale);
	void paintXCP(QPainter &painter, QPointF ORef, double scale);
	void paintXTr(QPainter &painter, QPointF ORef, double scale);
	void paintWing(QPainter &painter, QPointF ORef, double scale);


signals:

public slots:
	void onResetWingScale();

public:
	static void *s_pMainFrame;
	static void *s_pMiarex;

private:

	bool m_bTrans;

	double m_WingScale;
	QPointF m_ptOffset;              /**< client offset position for wing display */
	QPoint m_LastPoint;           /**< The client position of the previous mousepress event */
	QGraph * m_pGraph;
	QPixmap m_PixText;
};

#endif // WINGWIDGET_H
