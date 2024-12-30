#include "TakenWidget.h"
#include <QPainter>
#include <QDebug>
#include "Dedistortion.h"
#include <QImage>
#include "vtmr.h"

CTakenWidget::CTakenWidget(CVTMR *vtmr, QWidget *parent):
	QWidget(parent), m_vtmr(vtmr)
{
	setMouseTracking(true);
	setAutoFillBackground(true);
	setCursor(Qt::CrossCursor);

	//stack = std::make_shared<QUndoStack>(this);

	connect(this, SIGNAL(sigPointsChange()), parent, SLOT(onPointsChange()));
	connect(this, SIGNAL(sigAddPoint(QPoint)), parent, SLOT(onAddPoint(QPoint)));
	connect(this, SIGNAL(sigAddLine(QPoint, QPoint, QPoint)), parent, SLOT(onAddLine(QPoint, QPoint, QPoint)));
	connect(this, SIGNAL(sigAddEllipse()), parent, SLOT(onAddEllipse()));
	connect(this, SIGNAL(sigAddRectangle()), parent, SLOT(onAddRectangl()));
	connect(this, SIGNAL(sigDelPoint(QPoint)), parent, SLOT(onDelPoint(QPoint)));
	connect(this, SIGNAL(sigImageScale(int, QImage*, QPoint)), m_vtmr, SLOT(onImageScale(int, QImage*, QPoint)));
	connect(this, SIGNAL(sigMousePos(QPoint)), parent, SLOT(onNotifyMousePos(QPoint)));
	connect(this, SIGNAL(sigAddPartZoom(QImage *, QPoint, QPoint)), parent, SLOT(onAddPartZoom(QImage *, QPoint, QPoint)));
	installEventFilter(this);
}

CTakenWidget::~CTakenWidget()
{
}

bool CTakenWidget::eventFilter(QObject *obj, QEvent *ev)
{
	QKeyEvent *event = nullptr;
	if (ev->type() == QEvent::KeyPress) {
		event = static_cast<QKeyEvent*> (ev);
	}
	else if (ev->type() == QEvent::KeyRelease) {
		event = static_cast<QKeyEvent*> (ev);
		onKeyReleaseEvent(event);
	}

	return QWidget::eventFilter(obj, ev);
}

void CTakenWidget::setShape(VTToolType shape)
{
	m_toolType = shape;
}

void CTakenWidget::paintEvent(QPaintEvent *ev)
{
    QPainter painter(this);
	painter.drawImage(0, 0, m_dstImage);
}

void CTakenWidget::mousePressEvent(QMouseEvent *ev)
{
    if(ev->button() == Qt::LeftButton)  //当鼠标左键按下
    {
		m_mousePress = true;
		m_mousePosCenter = ev->pos();
    }
	ev->ignore();
}

void CTakenWidget::mouseMoveEvent(QMouseEvent *ev)
{
    if(ev->buttons()&Qt::LeftButton)   //如果鼠标左键按着的同时移动鼠标
    {
		m_mousePosEnd = ev->pos();
		if (m_mousePosCenter != m_mousePosEnd && m_mousePress) 
			emit sigPointsChange();
    }

	emit sigMousePos(ev->pos());

	if (0 == m_dstImage.width() ||
		0 == m_dstImage.height() ||
		ev->pos().rx() > m_dstImage.width() ||
		ev->pos().ry() > m_dstImage.height())
		return;
	auto image = new QImage;
	*image = m_dstImage.copy(QRect(0, 0, m_dstImage.width(), m_dstImage.height()));
	emit sigImageScale(-1, image, ev->pos());
	ev->ignore();
}
void CTakenWidget::mouseReleaseEvent(QMouseEvent *ev)
{
    if(ev->button() == Qt::LeftButton)   //如果鼠标左键释放
    {
		m_mousePress = false;
		auto srcImagePos = QPoint(ev->pos().x(), ev->pos().y());

		if (TOOL_POINT == m_toolType) {
			emit sigAddPoint(srcImagePos);
		}
		else if (TOOL_ERASER == m_toolType) {
			emit sigDelPoint(srcImagePos);
		}
		else if (TOOL_LINE == m_toolType) {
			emit sigAddLine(m_mousePosCenter, m_mousePosBegin, m_mousePosEnd);
		}
		else if (TOOL_ELLIPSE == m_toolType) {
			emit sigAddEllipse();
		}
		else if (TOOL_RECTANGLE == m_toolType) {
			emit sigAddRectangle();
		}
		else if (TOOL_PARTZOOM == m_toolType) {
			auto begin = m_mousePosCenter, end = m_mousePosEnd;
			if (end.x() - begin.x() > 0 && end.y() - begin.y() > 0 && !m_dstImage.isNull()) {
				auto image = new QImage;
				*image = m_dstImage.copy(m_dstImage.rect());
				emit sigAddPartZoom(image, begin, end);
			}
		}
    }
	ev->ignore();
}

void CTakenWidget::wheelEvent(QWheelEvent *ev)
{
	int scale = 0;
	/*if (0 == m_dstImage.width() ||
		0 == m_dstImage.height() ||
		ev->pos().rx() > m_dstImage.width() ||
		ev->pos().ry() > m_dstImage.height())
		return;*/

	auto image = new QImage;
	*image = m_dstImage.copy(QRect(0, 0, m_dstImage.width(), m_dstImage.height()));
	if (ev->angleDelta().y() > 0) {
		scale = 1;
		emit sigImageScale(scale, image, ev->position().toPoint());
	}
	else {
		scale = 0;
		emit sigImageScale(scale, image, ev->position().toPoint());
	}
	ev->ignore();
}

void CTakenWidget::onKeyReleaseEvent(QKeyEvent *ev)
{
	if (ev->key() == Qt::Key_Left) {
		emit sigPreviousFrame();
		//qDebug() << "asd";
	} else if (ev->key() == Qt::Key_Right)
	{
		emit sigNextFrame();
	}
	if (TOOL_POINT == m_toolType || TOOL_LINE == m_toolType) {
		int x = QCursor::pos().x();
		int y = QCursor::pos().y();

		if (ev->key() == Qt::Key_Left) {
			--x;
			QCursor::setPos(QPoint(x, y));
		}
		else if (ev->key() == Qt::Key_Right) {
			++x;
			QCursor::setPos(QPoint(x, y));
		}
		else if (ev->key() == Qt::Key_Up) {
			--y;
			QCursor::setPos(QPoint(x, y));
		}
		else if (ev->key() == Qt::Key_Down) {
			++y;
			QCursor::setPos(QPoint(x, y));
		}
		else if (ev->key() == Qt::Key_Space) {
			auto point = mapFromGlobal(QCursor::pos());
			emit sigAddPoint(point);
		}
		else if (ev->key() == Qt::Key_P) {

		}
	}
	else if (TOOL_ERASER == m_toolType) {
		if (ev->key() == Qt::Key_Backspace || ev->key() == Qt::Key_Delete) {
			auto point = mapFromGlobal(QCursor::pos());
			emit sigDelPoint(point);
		}
	}
	
	ev->accept();
}


void CTakenWidget::drawFrame(QString strFramePath)
{
	if (m_srcImage.load(strFramePath)) {
		emit sigPointsChange();
	}
}

void CTakenWidget::drawSight(QPainter &painter, QImage &image, QColor color, QPoint pos, QString strP, int width, int height)
{
	int posX = pos.x();
	int posY = pos.y();
	QPen oldPen = painter.pen();
	painter.fillRect(image.rect(), Qt::transparent);
	painter.setPen(QPen(color, 1, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
	//painter.drawEllipse(pos, 5, 5);
	painter.drawLine(pos + QPoint(-17, 0), pos + QPoint(0, 0));
	painter.drawLine(pos + QPoint(0, 0), pos + QPoint(17, 0));
	painter.drawLine(pos + QPoint(0, 0), pos + QPoint(0, 17));
	painter.drawLine(pos + QPoint(0, -17), pos + QPoint(0, 0));
	//判断边界
	posX -= 20;
	posY -= 20;
	if (posX - 20 < 0)
		posX += 40;
	if (posY - 20 < 0)
		posY += 40;
	if (posX - 20 > width)
		posX -= 40;
	if (posY - 20 > height)
		posY -= 40;
	painter.drawText(posX, posY, strP);
	painter.setPen(oldPen);
}

void CTakenWidget::drawPoint(QPainter &painter, VTPoint tPoint)
{
	QPen oldPen = painter.pen();
	QPen pen(tPoint.color, 4, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
	painter.setPen(pen);
	painter.drawPoint(tPoint.point);
	painter.setPen(oldPen);
}

void CTakenWidget::drawDashLine(QPainter &painter, QPoint from, QPoint to)
{
	QPen oldPen = painter.pen();
	QPen pen(QColor(255, 255, 0), 2, Qt::DashLine, Qt::FlatCap, Qt::MiterJoin);
	painter.setPen(pen);
	painter.drawLine(from, to);
	painter.setPen(oldPen);
}

void CTakenWidget::drawDotEllipse(QPainter &painter, QPoint from, int dx, int dy)
{
	QPen oldPen = painter.pen();
	QPen pen(QColor(0, 0, 0, 100), 2, Qt::DotLine, Qt::FlatCap, Qt::MiterJoin);
	painter.setPen(pen);
	painter.drawEllipse(from, dx, dy);
	painter.setPen(oldPen);
}

void CTakenWidget::drawDotRect(QPainter &painter, QPoint from, int dx, int dy)
{
	QPen oldPen = painter.pen();
	QPen pen(QColor(0, 0, 0, 100), 2, Qt::DotLine, Qt::FlatCap, Qt::MiterJoin);
	painter.setPen(pen);
	painter.drawRect(from.x(), from.y(), dx, dy);
	painter.setPen(oldPen);
}

void CTakenWidget::drawRect(QPainter &painter, QPoint from, int dx, int dy, QColor color)
{
	QPen oldPen = painter.pen();
	QPen pen(color, 2, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
	painter.setPen(pen);
	painter.drawRect(from.x(), from.y(), dx, dy);
	painter.setPen(oldPen);
}

void CTakenWidget::drawLine(QPainter &painter, QPoint from, QPoint to, QColor color)
{
	QPen oldPen = painter.pen();
	QPen pen(color, 2, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
	painter.setRenderHints(QPainter::Antialiasing, true);
	painter.setPen(pen);
	painter.drawLine(from, to);
	painter.setPen(oldPen);
}

void CTakenWidget::drawPartZoom(QPainter &painter, QVector<VTLine> lines)
{
	QPoint pos1, pos2;
	if (lines.size() == 0)
		return;
	QPen oldPen = painter.pen();
	QPen pen(QColor(ZOOM_LINE_COLOR), 2, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
	//去除锯齿
	painter.setRenderHints(QPainter::Antialiasing, true);
	painter.setPen(pen);

	for (auto &line : lines) {
		pos1 = line.ponit1;
		pos2 = line.ponit2;
		painter.drawLine(pos1, pos2);
	}
	painter.setPen(oldPen);
}

void CTakenWidget::adjustDrawImage(int nContrast, int nSaturation, int nBrightness, QVector<QPoint> points, QVector<VTLine> tLines, QVector<VTPoint> tPoints,
	QVector<VTLine> zoomLines){
	m_dstImage = m_srcImage;
	QPainter painter(&m_dstImage);
	QPen pen(QColor(255, 0, 0, 255), 2, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);

	if(width() != m_dstImage.width() || height() != m_dstImage.height())
		resize(m_dstImage.width(), m_dstImage.height());

	//去畸变
	CDedistortion::doDedistortion(m_dstImage);

	//图像增强
	if(0 != nContrast)
		VTAdjustContrast(m_dstImage, nContrast);
	if(0 != nSaturation)
		VTAdjustSaturation(m_dstImage, nSaturation);
	if(0 != nBrightness)
		VTAdjustBrightness(m_dstImage, nBrightness);

	painter.setPen(pen);

	for (auto & line : tLines) {
		if (line.ponit1 == QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT) ||
			line.ponit2 == QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT))
			continue;
		drawLine(painter, line.ponit1, line.ponit2, line.color);
	}

	QColor color;
	QString strP;
	for (int n = 0; n < points.size(); n++) {
		if (QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT) == points[n])
			continue;
		if (0 == n) {
			strP = QString("P0");
			color = QColor(255, 0, 0, 255);
		}
		else if (n < 5) {
			strP = QString("P%1").arg(n);
			color = QColor(0, 255, 0, 255);
		}
		else if (n > 5) {
			strP = QString("P%1").arg(n - 5);
			color = QColor(0, 255, 0, 100);
		}
		drawSight(painter, m_dstImage, color, points[n], strP, width(), height());
	}	

	for (auto &point : tPoints) {
		if (QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT) == point.point)
			continue;
		drawPoint(painter, point);
	}

	if (m_toolType == TOOL_LINE && m_mousePress) {
		m_mousePosBegin.rx() = m_mousePosCenter.x() * 2 - m_mousePosEnd.x();
		m_mousePosBegin.ry() = m_mousePosCenter.y() * 2 - m_mousePosEnd.y();
		drawDashLine(painter, m_mousePosBegin, m_mousePosEnd);
	}
	else if (TOOL_ELLIPSE == m_toolType && m_mousePress) {
		int dx = abs(m_mousePosEnd.x() - m_mousePosCenter.x());
		int dy = abs(m_mousePosEnd.y() - m_mousePosCenter.y());
		drawDotEllipse(painter, m_mousePosCenter, dx, dy);
	}
	else if (TOOL_RECTANGLE == m_toolType && m_mousePress) {
		int dx = abs(m_mousePosEnd.x() - m_mousePosCenter.x());
		int dy = abs(m_mousePosEnd.y() - m_mousePosCenter.y());
		drawDotRect(painter, m_mousePosCenter, dx, dy);
	}

	//局部放大框与原图框之间的连线
	drawPartZoom(painter, zoomLines);

	painter.end();
	//标定点
	update();
}

void CTakenWidget::adjustDrawImage(QImage &image, QVector<QPoint> points, QVector<VTLine> tLines, QVector<VTPoint> tPoints, QVector<VTLine> zoomRect, QVector<VTZoomPairRect> zoomPairRect)
{
	QPainter painter(&image);
	QPen pen(QColor(255, 0, 0, 255), 2, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);

	painter.setPen(pen);

	for (auto & line : tLines) {
		if (line.ponit1 == QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT) ||
			line.ponit2 == QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT))
			continue;
		drawLine(painter, line.ponit1, line.ponit2, line.color);
	}

	QColor color;
	QString strP;
	for (int n = 0; n < points.size(); n++) {
		if (QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT) == points[n])
			continue;
		if (0 == n) {
			strP = QString("P0");
			color = QColor(255, 0, 0, 255);
		}
		else if (n < 5) {
			strP = QString("P%1").arg(n);
			color = QColor(0, 255, 0, 255);
		}
		else if (n > 5) {
			strP = QString("P%1").arg(n - 5);
			color = QColor(0, 255, 0, 100);
		}
		drawSight(painter, image, color, points[n], strP, image.width(), image.height());
	}

	for (auto &point : tPoints) {
		if (QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT) == point.point)
			continue;
		drawPoint(painter, point);
	}

	for (auto & zoomPair : zoomPairRect) {
		int w = abs(zoomPair.srcTopLeft.x() - zoomPair.srcBottomRight.x());
		int h = abs(zoomPair.srcTopLeft.y() - zoomPair.srcBottomRight.y());
		int sw = w * zoomPair.scale;
		int sh = h * zoomPair.scale;
		QImage tmp = image.copy(zoomPair.srcTopLeft.x(), zoomPair.srcTopLeft.y(), w, h);
		QImage scale = tmp.scaled(sw, sh, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		painter.drawImage(zoomPair.scaleTopLeft, scale);
	}

	for (auto & zoom : zoomRect) {
		int w = abs(zoom.ponit2.x() - zoom.ponit1.x());
		int h = abs(zoom.ponit2.y() - zoom.ponit1.y());
		drawRect(painter, zoom.ponit1, w, h, zoom.color);
	}
	painter.end();
}
