#include "Zoom.h"
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>
#include "Global.h"
#include <QDoubleValidator>
#include <QPalette>

#define BOUNDARY_WIDTH 8
CZoom::CZoom(int type, float scale, QImage *image, QPoint begin, QPoint end, QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_StyledBackground);
	QString style = QString("QWidget {border-style: solid;border-width: 2px;border-color: %1;background-color: %2;}").arg(ZOOM_LINE_COLOR).arg(ZOOM_LINE_COLOR);
	setStyleSheet(style);

	ui.label->setStyleSheet("border-width:0;background-color:transparent");
	ui.lineEdit->setStyleSheet("border-width:0;background-color:transparent");

	auto doubleValidator = new QDoubleValidator(this);
	doubleValidator->setRange(1, 8);
	doubleValidator->setNotation(QDoubleValidator::StandardNotation);
	doubleValidator->setDecimals(2);
	ui.lineEdit->setValidator(doubleValidator);
	ui.lineEdit->setAlignment(Qt::AlignRight);
	ui.lineEdit->setText(QString("%1").arg(scale));

	connect(ui.lineEdit, &QLineEdit::textChanged,
		[=](const QString &text) {
		auto f = text.toFloat();
		if (f < 1)
			return;
		m_scale = f;
	});

	connect(this, SIGNAL(sigPartZoomChange(QWidget *)), (QObject*)g_takenBG, SLOT(onPartZoomChange(QWidget *)));
	connect(this, SIGNAL(sigPartZoomMove(QWidget *)), (QObject*)g_takenBG, SLOT(onPartZoomMove(QWidget *)));
	connect(this, SIGNAL(sigClosePartZoom(QWidget *)), (QObject*)g_takenBG, SLOT(onClosePartZoom(QWidget *)));

	m_type = type;
	m_scale = scale;
	auto w = end.x() - begin.x();
	auto h = end.y() - begin.y();
	auto sw = w * scale;
	auto sh = h * scale;
	QPoint pos;
	auto pw = parent->width();
	auto ph = parent->height();
	if (begin.x() - sw - 10 > 0 && begin.y() - sh - 10 > 0) {
		pos.setX(begin.x() - sw - 10);
		pos.setY(begin.y() - sh - 10);
	}
	else if (begin.x() - sw - 10 > 0 && begin.y() - sh - 10 < 0) {
		pos.setX(begin.x() - sw - 10);
		pos.setY(begin.y());
	}
	else if (begin.x() - sw - 10 < 0 && begin.y() - sh - 10 > 0) {
		pos.setX(begin.x());
		pos.setY(begin.y() - sh - 10);
	}
	else if (end.x() + sw + 10 <= pw && end.y() + sh + 10 <= ph) {
		pos.setX(end.x() + 10);
		pos.setY(end.y() + 10);
	}
	else if(end.x() + sw + 10 <= pw && end.y() + sh + 10 > ph) {
		pos.setX(end.x() + 10);
		pos.setY(begin.y());
	}
	else if (end.x() + sw + 10 > pw && end.y() + sh + 10 <= ph) {
		pos.setX(begin.x());
		pos.setY(end.y() + 10);
	}
	else {
		pos.setX(begin.x() + 10);
		pos.setY(begin.y() + 10);
	}

	m_beginPos = begin;
	m_endPos = end;

	m_imageBG = image->copy(0, 0, image->width(), image->height());
	m_image = m_imageBG.copy(begin.x(), begin.y(), w, h);
	
	setMinimumWidth(sw);
	setMinimumHeight(sh);
	setMaximumWidth(sw);
	setMaximumHeight(sh);

	if (m_type == 0) {
		ui.label->hide();
		ui.lineEdit->hide();
		move(begin);
	}
	else {
		move(pos);
	}
}

CZoom::CZoom(int type, float scale, QImage *image, QPoint begin, QPoint end, QPoint pos, QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_StyledBackground);
	
	QString style = QString("QWidget {border-style: solid;border-width: 2px;border-color: %1;background-color: %2;}").arg(ZOOM_LINE_COLOR).arg(ZOOM_LINE_COLOR);
	setStyleSheet(style);

	ui.label->setStyleSheet("border-width:0;background-color:transparent");
	ui.lineEdit->setStyleSheet("border-width:0;background-color:transparent");

	auto doubleValidator = new QDoubleValidator(this);
	doubleValidator->setRange(1, 8);
	doubleValidator->setNotation(QDoubleValidator::StandardNotation);
	doubleValidator->setDecimals(2);
	ui.lineEdit->setValidator(doubleValidator);
	ui.lineEdit->setAlignment(Qt::AlignRight);
	ui.lineEdit->setText(QString("%1").arg(scale));

	connect(ui.lineEdit, &QLineEdit::textChanged,
		[=](const QString &text) {
		auto f = text.toFloat();
		if (f < 1)
			return;
		m_scale = f;
	});

	connect(this, SIGNAL(sigPartZoomChange(QWidget *)), (QObject*)g_takenBG, SLOT(onPartZoomChange(QWidget *)));
	connect(this, SIGNAL(sigPartZoomMove(QWidget *)), (QObject*)g_takenBG, SLOT(onPartZoomMove(QWidget *)));
	connect(this, SIGNAL(sigClosePartZoom(QWidget *)), (QObject*)g_takenBG, SLOT(onClosePartZoom(QWidget *)));

	m_type = type;
	m_scale = scale;
	auto w = end.x() - begin.x();
	auto h = end.y() - begin.y();
	auto sw = w * scale;
	auto sh = h * scale;
	
	auto pw = parent->width();
	auto ph = parent->height();

	m_beginPos = begin;
	m_endPos = end;

	m_imageBG = image->copy(0, 0, image->width(), image->height());
	m_image = m_imageBG.copy(begin.x(), begin.y(), w, h);

	setMinimumWidth(sw);
	setMinimumHeight(sh);
	setMaximumWidth(sw);
	setMaximumHeight(sh);

	if (m_type == 0) {
		ui.label->hide();
		ui.lineEdit->hide();
		move(begin);
	}
	else {
		move(pos);
	}
}

CZoom::~CZoom()
{
}

void CZoom::getLines(QRect r1, QRect r2, QVector<VTLine> &lines)
{
	auto x1 = r1.x();
	auto y1 = r1.y();
	auto w1 = r1.width();
	auto h1 = r1.height();

	auto x2 = r2.x();
	auto y2 = r2.y();
	auto w2 = r2.width();
	auto h2 = r2.height();

	auto a1 = QPoint(r1.x(), r1.y());
	auto b1 = QPoint(r1.x() + r1.width(), r1.y());
	auto c1 = QPoint(r1.x() + r1.width(), r1.y() + r1.height());
	auto d1 = QPoint(r1.x(), r1.y() + r1.height());

	auto a2 = QPoint(r2.x(), r2.y());
	auto b2 = QPoint(r2.x() + r2.width(), r2.y());
	auto c2 = QPoint(r2.x() + r2.width(), r2.y() + r2.height());
	auto d2 = QPoint(r2.x(), r2.y() + r2.height());

	VTLine line1;
	VTLine line2;
	
	if (b1.x() < a2.x()) { //左边 
		line1.ponit1 = b1;
		line1.ponit2 = a2;

		line2.ponit1 = c1;
		line2.ponit2 = d2;
	}
	else if (c1.y() < a2.y()) {//上边
		line1.ponit1 = d1;
		line1.ponit2 = a2;

		line2.ponit1 = c1;
		line2.ponit2 = b2;
	}
	else if (a1.x() > b2.x()) {//右边
		line1.ponit1 = a1;
		line1.ponit2 = b2;

		line2.ponit1 = d1;
		line2.ponit2 = c2;
	}
	else if (a1.y() > c2.y()) {//下边
		line1.ponit1 = a1;
		line1.ponit2 = d2;

		line2.ponit1 = b1;
		line2.ponit2 = c2;
	}
	else
		return;

	lines.push_back(line1);
	lines.push_back(line2);
}

void CZoom::rect2Wnd(QImage *image, QVector<VTZoomPairRect> &zoomRects, QVector<ZoomPair> &vecZoom, QWidget *parent)
{
	for (auto &iter : zoomRects) {
		ZoomPair zoomPair;
		QPoint begin = iter.srcTopLeft;
		QPoint end = iter.srcBottomRight;
		QPoint pos = iter.scaleTopLeft;
		zoomPair.srcWnd = new CZoom(0, 1.0, image, begin, end, parent);
		zoomPair.srcWnd->show();

		zoomPair.scaleWnd = new CZoom(1, iter.scale, image, begin, end, pos, parent);
		zoomPair.scaleWnd->show();
		vecZoom.push_back(zoomPair);
	}
}

void CZoom::paintEvent(QPaintEvent *ev)
{
	QPainter painter(this);
	QImage image;
	QSize zoomSize = size();
	zoomSize.setWidth(zoomSize.width() - 4);
	zoomSize.setHeight(zoomSize.height() - 4);
	image = m_image.scaled(zoomSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	painter.drawImage(2, 2, image);
}

//拖拽操作
void CZoom::mousePressEvent(QMouseEvent *event)
{
	if (m_type == 0) {
		event->accept();
		return;
	}
	if (event->button() == Qt::LeftButton) {
		m_drag = true;
		//获得鼠标的初始位置
		m_mouseStartPoint = event->globalPos();
		//获得窗口的初始位置
		m_windowTopLeftPoint = this->frameGeometry().topLeft();
	}
	event->accept();
}

void CZoom::mouseMoveEvent(QMouseEvent *event)
{
	if (m_type == 0) {
		event->accept();
		return;
	}
	if (m_drag) {
		//获得鼠标移动的距离
		QPoint distance = event->globalPos() - m_mouseStartPoint;
		//改变窗口的位置
		this->move(m_windowTopLeftPoint + distance);
	}
	event->accept();
}

void CZoom::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		m_drag = false;
		if (m_type != 0) {
			emit sigPartZoomMove(this);
		}
	}
	else if (event->button() == Qt::RightButton) {
		emit sigClosePartZoom(this);
	}
	event->accept();
}

void CZoom::wheelEvent(QWheelEvent *ev)
{
	if (m_type == 0) {
		ev->accept();
		return;
	}
	auto scale = m_scale;

	if (ev->angleDelta().y() > 0) {
		// 进行放大
		scale += 0.1;
		if (scale > 8.0)
			scale = 8.0;
	}
	else {
		scale -= 0.1;
		if (scale < 1.0)
			scale = 1.0;
	}
	if (scale != m_scale) {
		m_scale = scale;
		ui.lineEdit->setText(QString("%1").arg(m_scale));
		
		auto sw = m_image.width() * m_scale;
		auto sh = m_image.height() * m_scale;
		setMinimumWidth(sw);
		setMinimumHeight(sh);
		setMaximumWidth(sw);
		setMaximumHeight(sh);

		emit sigPartZoomChange(this);
	}
	ev->accept();
}
