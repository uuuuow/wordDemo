#pragma once

#include <QWidget>
#include "ui_Zoom.h"
#include <QImage>
#include "Utils.h"

struct ZoomPair
{
	QWidget *srcWnd;
	QWidget *scaleWnd;
};

class CZoom : public QWidget
{
	Q_OBJECT

public:
	CZoom(int type, float scale, QImage *image, QPoint begin, QPoint end, QWidget *parent = Q_NULLPTR);
	CZoom(int type, float scale, QImage *image, QPoint begin, QPoint end, QPoint pos, QWidget *parent = Q_NULLPTR);
	~CZoom();
	float getScale() {return m_scale;}
	QPoint getBeginPos() { return m_beginPos; }
	QPoint getEndPos() { return m_endPos; }
	static void getLines(QRect r1, QRect r2, QVector<VTLine> &lines);
	static void rect2Wnd(QImage *image, QVector<VTZoomPairRect> &zoomRects, QVector<ZoomPair> &vecZoom, QWidget *parent);
protected:
	void paintEvent(QPaintEvent *ev);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *ev);
signals:
	//局部缩放窗口移动或者缩放
	void sigPartZoomChange(QWidget *widget);
	//局部缩放窗口移动
	void sigPartZoomMove(QWidget *widget);
	//关闭局部缩放窗口
	void sigClosePartZoom(QWidget *widget);
private:
	Ui::Zoom ui;
	QImage m_imageBG;
	QImage m_image;
	float m_scale = 2.0;
	bool m_drag = false;
	QPoint m_mouseStartPoint;
	QPoint m_windowTopLeftPoint;
	QPoint m_beginPos;
	QPoint m_endPos;
	int m_type = 0;//0-原图，1-缩放图
};
