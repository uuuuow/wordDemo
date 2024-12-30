#include "Length3Points.h"
#include <QDoubleSpinBox>
#include <QPainter>

#define DOUBLESPINBOX_WIDTH 80
#define POINT_TEXT_W 100
#define POINT_TEXT_H 50
#define IMAGE_LENTH 200

CLength3Points::CLength3Points(QWidget *parent)
	: CLength2Coordinate(parent)
{
	m_zeroPoint = QPoint(50, 20);
	resize(340, 200);

	m_pointP1 = new QLabel("P1", this);
	m_pointP1->setMaximumSize(POINT_TEXT_W, POINT_TEXT_H);
	m_pointP1->setMinimumSize(POINT_TEXT_W, POINT_TEXT_H);

	m_pointP21 = new QLabel("P2", this);
	m_pointP21->setMaximumSize(POINT_TEXT_W, POINT_TEXT_H);
	m_pointP21->setMinimumSize(POINT_TEXT_W, POINT_TEXT_H);

	m_pointP22 = new QLabel("P2", this);
	m_pointP22->setMaximumSize(POINT_TEXT_W, POINT_TEXT_H);
	m_pointP22->setMinimumSize(POINT_TEXT_W, POINT_TEXT_H);

	m_pointP3 = new QLabel("P3", this);
	m_pointP3->setMaximumSize(POINT_TEXT_W, POINT_TEXT_H);
	m_pointP3->setMinimumSize(POINT_TEXT_W, POINT_TEXT_H);


	m_pointA = QPointF(0, 0);
	m_pointB1 = QPointF(IMAGE_LENTH, 0);
	m_pointB2 = QPointF(0, 100);
	m_pointC = QPointF(IMAGE_LENTH, 100);

	m_lAB = new QDoubleSpinBox(this);
	m_lAB->setMaximumWidth(DOUBLESPINBOX_WIDTH);
	m_lAB->setMinimumWidth(DOUBLESPINBOX_WIDTH);
	m_lAB->setMaximum(99999);
	m_lAB->setMinimum(0);
	connect(m_lAB, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
		[=](double d) {
			m_valueAB = d; 
			m_L1 = d * 1000; 
			emit sigValueChanged(); 
		}
	);

	m_lBC = new QDoubleSpinBox(this);
	m_lBC->setMaximumWidth(DOUBLESPINBOX_WIDTH);
	m_lBC->setMinimumWidth(DOUBLESPINBOX_WIDTH);
	m_lBC->setMaximum(99999);
	m_lBC->setMinimum(0);
	connect(m_lBC, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
		[=](double d) {
			m_valueBC = d; 
			m_L2 = d * 1000; 
			emit sigValueChanged(); 
		}
	);

	connect(this, SIGNAL(sigValueChanged()), this, SLOT(onValueChanged()));

	int spinW = m_lBC->width();
	int spinH = m_lBC->height();
	QPointF tmp;
	tmp = m_zeroPoint + m_pointB1 + QPointF(-1 * IMAGE_LENTH / 2, 0) + QPointF(-1 * spinW/2, -1 * spinH/ 2);
	m_lAB->move(tmp.x(), tmp.y());
	m_lAB->setToolTip("|P1-P2|");

	tmp = m_zeroPoint + m_pointB2 + QPointF(IMAGE_LENTH / 2, 0) + QPointF(-1 * spinW / 2, -1 * spinH / 2);
	m_lBC->move(tmp.x(), tmp.y());
	m_lBC->setToolTip("|P2-P3|");

	tmp = m_zeroPoint + m_pointA + QPointF(-1 * spinW / 2, 0);
	m_pointP1->move(tmp.x(), tmp.y());

	tmp = m_zeroPoint + m_pointB1 + QPointF(-1 * spinW / 2, 0);
	m_pointP21->move(tmp.x(), tmp.y());

	tmp = m_zeroPoint + m_pointB2 + QPointF(-1 * spinW / 2, 0);
	m_pointP22->move(tmp.x(), tmp.y());

	tmp = m_zeroPoint + m_pointC + QPointF(-1 * spinW / 2, 0);
	m_pointP3->move(tmp.x(), tmp.y());
}

CLength3Points::~CLength3Points()
{
}

void CLength3Points::init(QVector<QPointF> &points)
{
	float x1 = points[1].x();
	float y1 = points[1].y();

	float x2 = points[2].x();
	float y2 = points[2].y();

	float x3 = points[3].x();
	float y3 = points[3].y();

	if ( INVALID_VALUE_FLOAT == x1
		|| INVALID_VALUE_FLOAT == x2
		|| INVALID_VALUE_FLOAT == x3 )
		return;

	m_L1 = sqrtf((x2 - x1)*(x2 - x1) - (y2 - y1)*(y2 - y1)) * 1000;
	m_L2 = sqrtf((x2 - x3)*(x2 - x3) - (y2 - y3)*(y2 - y3)) * 1000;

	m_lAB->setValue(m_L1*1.0 / 1000);
	m_lBC->setValue(m_L2*1.0 / 1000);

	emit sigValueChanged();
}

void CLength3Points::onValueChanged()
{
	if (0 == m_L1 || 0 == m_L2) {
		QVector<QPointF> points = {
			QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
			QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
			QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
			QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
			QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT)
		};
		setActualPoints(points);

		m_pointP1->setText("P1");
		m_pointP21->setText("P2");
		m_pointP22->setText("P2");
		m_pointP3->setText("P3");
		return;
	}
	
	QPointF P1 = QPointF(0, 0);
	QPointF P2 = QPointF(m_L1 / 1000.0, 0);
	QPointF P3 = QPointF(m_L2 / 1000.0, 0);

	QString tmp = QString("P1(%1,%2)").arg(P1.x()).arg(P1.y());
	m_pointP1->setText(tmp);

	tmp = QString("P2(%1,%2)").arg(P2.x()).arg(P2.y());
	m_pointP21->setText(tmp);

	tmp = QString("P2(%1,%2)").arg(P2.x()).arg(P2.y());
	m_pointP22->setText(tmp);

	tmp = QString("P3(%1,%2)").arg(P3.x()).arg(P3.y());
	m_pointP3->setText(tmp);

	QVector<QPointF> points = {
		QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
		QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
		QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
		QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
		QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT)
	};
	points[1] = P1;
	points[2] = P2;
	points[3] = P3;

	setActualPoints(points);
}

void CLength3Points::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

	QPainter painter(this);

	// 反走样
	painter.setRenderHint(QPainter::Antialiasing, true);

	// 绘制图标
	painter.setPen(QColor(0, 160, 230));
	painter.drawLine(m_zeroPoint + m_pointA, m_zeroPoint + m_pointB1);
	painter.drawLine(m_zeroPoint + m_pointB2, m_zeroPoint + m_pointC);
}