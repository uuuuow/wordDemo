#include "Magnifier.h"
#include <QPainter>

CMagnifier::CMagnifier(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setMinimumSize(280, 280);
	setMaximumSize(280, 280);
	m_labelShowMultiple = new QLabel("1.0",this);
	m_labelShowMultiple->setMaximumWidth(40);
	m_labelShowMultiple->setMinimumWidth(40);
	m_labelShowMultiple->hide();
	startTimer(500);
}

CMagnifier::~CMagnifier()
{
}

void CMagnifier::timerEvent(QTimerEvent *e)
{
	QDateTime time = QDateTime::currentDateTime();
	if (m_lastTime.secsTo(time) > 2) {
		m_labelShowMultiple->hide();
	}
}

void CMagnifier::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	QRect target(0, 0, width(), height());
	int x = width();
	int y = height();
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
	painter.drawImage(target, m_showImage, m_showRect);

	//画十字线
	painter.setPen(QPen(Qt::white, 1, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
	painter.drawLine(QLine(QPoint(width() / 2, 0), QPoint(width() / 2, height() / 2 - 6)));
	painter.drawLine(QLine(QPoint(width() / 2, height()), QPoint(width() / 2, height() / 2 + 6)));
	painter.setPen(Qt::red);
	painter.drawPoint(QPoint(width() / 2, height() / 2));
	painter.setPen(Qt::white);
	painter.drawLine(QLine(QPoint(0, height() / 2), QPoint(width() / 2 - 6, height() / 2)));
	painter.drawLine(QLine(QPoint(width(), height() / 2), QPoint(width() / 2 + 6, height() / 2)));
}

void CMagnifier::setMultiple(int n)
{
	if (n < 0)
		return;
	if (1 == n) {
		m_multiple += 0.1f;
		if (3.0f < m_multiple) {
			m_multiple = 3.0f;
		}
	}
	else {
		m_multiple -= 0.1f;
		if (1.0f > m_multiple) {
			m_multiple = 1.0f;
		}
	}
	auto temp = QString::number(m_multiple, 'f', 1);
	m_labelShowMultiple->setText(QString("%1").arg(temp));
	auto w = m_labelShowMultiple->width();
	m_labelShowMultiple->move(width() - w, 0);
	m_labelShowMultiple->show();
	m_lastTime = QDateTime::currentDateTime();
}

void CMagnifier::setLabelPixmap(QImage *image, QPoint &pos)
{
	if (0 >= image->width() || 0 >= image->height())
		return;

	m_showImage = image->copy(0, 0, image->width(), image->height());
	if (m_multiple <= 0)
		m_multiple = 1;

	auto x = pos.x();
	auto y = pos.y();

	auto resultImageW = width() / m_multiple;
	auto resultImageH = height() / m_multiple;

	auto resultImageX = x - resultImageW / 2;
	auto resultImageY = y - resultImageH / 2;

	m_showRect = QRect(resultImageX, resultImageY, resultImageW, resultImageH);
	update();
}
