#ifndef PAINTAREA_H
#define PAINTAREA_H

#include <QWidget>
#include <QMouseEvent>
#include <QPoint>
#include "Utils.h"
#include <QLabel>
class CMarkerPoint;
class CVTMR;
/*
*测速点位提取窗口
*/
class CTakenWidget : public QWidget
{
	Q_OBJECT
public:
	explicit CTakenWidget(CVTMR *vtmr, QWidget *parent = 0);
	~CTakenWidget();
	//显示图像
	void drawFrame(QString strFramePath);
	//返回图像
	QImage *getFrame() {return &m_dstImage;}
	//调整图像
	void adjustDrawImage(int nContrast, int nSaturation, int nBrightness, 
		QVector<QPoint> points, QVector<VTLine> tLines, QVector<VTPoint> tPoints,
		QVector<VTLine> zoomLines);
	static void adjustDrawImage(QImage &image, QVector<QPoint> points, QVector<VTLine> tLines, QVector<VTPoint> tPoints, QVector<VTLine> zoomRect, QVector<VTZoomPairRect> zoomPairRect);
	//设置图形
	void setShape(VTToolType shape);
	//画准星
	static void drawSight(QPainter &painter, QImage &image, QColor color, QPoint pos, QString strP, int width, int height);
	//画局部缩放辅助线
	static void drawPartZoom(QPainter &painter, QVector<VTLine> lines);
private:
	static void drawPoint(QPainter &painter, VTPoint tPoint);
	static void drawDashLine(QPainter &painter, QPoint from, QPoint to);
	static void drawDotEllipse(QPainter &painter, QPoint from, int dx, int dy);
	static void drawDotRect(QPainter &painter, QPoint from, int dx, int dy);
	static void drawRect(QPainter &painter, QPoint from, int dx, int dy, QColor color);
	static void drawLine(QPainter &painter, QPoint from, QPoint to, QColor color);
	void onKeyReleaseEvent(QKeyEvent *ev);
protected:
    virtual void paintEvent(QPaintEvent *ev);
	virtual void mousePressEvent(QMouseEvent *ev);
	virtual void mouseMoveEvent(QMouseEvent *ev);
	virtual void mouseReleaseEvent(QMouseEvent *ev);
	virtual void wheelEvent(QWheelEvent *ev);
	virtual bool eventFilter(QObject *obj, QEvent *ev);

signals:
	//点有变化
	void sigPointsChange();
	//添加标记点
	void sigAddPoint(QPoint point);
	//添加线段
	void sigAddLine(QPoint center, QPoint begin, QPoint end);
	void sigAddEllipse();
	void sigAddRectangle();
	//删除标记点
	void sigDelPoint(QPoint point);
	//放大镜显示图像
	void sigImageScale(int scale, QImage *image, QPoint pos);
	//鼠标坐标
	void sigMousePos(QPoint point);
	//局部放大
	void sigAddPartZoom(QImage *image, QPoint begin, QPoint end);

	void sigNextFrame();
	void sigPreviousFrame();
private:
	VTToolType m_toolType = TOOL_NONE;    //当前图形
	QImage m_srcImage;
	QImage m_dstImage;
	CVTMR *m_vtmr;

	bool m_mousePress = false;
	QPoint m_mousePosBegin;
	QPoint m_mousePosCenter;
	QPoint m_mousePosEnd;

	//撤销 重做 栈
	//std::shared_ptr<QUndoStack> stack;
};
#endif // PAINTAREA_H