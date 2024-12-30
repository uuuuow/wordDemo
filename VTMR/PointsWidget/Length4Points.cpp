#include "Length4Points.h"
#include <QPainter>
#include <QLabel>
#include <QDoubleSpinBox>

#define DOUBLESPINBOX_WIDTH 100
#define IMAGE_LENTH 200
#define POINT_TEXT_W 100
#define POINT_TEXT_H 50

CLength4Points::CLength4Points(QWidget *parent)
	: CLength2Coordinate(parent)
{
	setFixedSize(460, 380);
	m_zeroPoint = QPoint(120, 80);

	m_pointP1 = new QLabel(this);
	m_pointP1->setScaledContents(true);
	m_pointP2 = new QLabel(this);
	m_pointP2->setScaledContents(true);
	m_pointP3 = new QLabel(this);
	m_pointP3->setScaledContents(true);
	m_pointP4 = new QLabel(this);
	m_pointP4->setScaledContents(true);
	onValueChanged();

	QPointF pointA = QPointF(0, 0);
	QPointF pointB = QPointF(0, IMAGE_LENTH - 1);
	QPointF pointC = QPointF(IMAGE_LENTH - 1, IMAGE_LENTH - 1);
	QPointF pointD = QPointF(IMAGE_LENTH - 1, 0);

	m_image = new QImage(IMAGE_LENTH, IMAGE_LENTH, QImage::Format_ARGB32);
	m_image->fill(QColor(0, 0, 0, 0));

	QPainter painter(m_image);
	painter.setPen(QColor(0, 160, 230));
	painter.drawLine(pointA, pointB);
	painter.drawLine(pointB, pointC);
	painter.drawLine(pointC, pointD);
	painter.drawLine(pointD, pointA);

	painter.drawLine(QPointF(0, IMAGE_LENTH - 1), QPointF(IMAGE_LENTH - 1, 0));
	painter.end();

	m_l12 = new QDoubleSpinBox(this);
	m_l12->setMaximumWidth(DOUBLESPINBOX_WIDTH);
	m_l12->setMinimumWidth(DOUBLESPINBOX_WIDTH);
	m_l12->setMaximum(99999);
	m_l12->setMinimum(0);
	connect(m_l12, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
		[=](double d) {
			m_L1 = d; 
			emit sigValueChanged(); 
		}
	);

	m_l23 = new QDoubleSpinBox(this);
	m_l23->setMaximumWidth(DOUBLESPINBOX_WIDTH);
	m_l23->setMinimumWidth(DOUBLESPINBOX_WIDTH);
	m_l23->setMaximum(99999);
	m_l23->setMinimum(0);
	connect(m_l23, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
		[=](double d) {
			m_L2 = d; 
			emit sigValueChanged(); 
		}
	);

	m_l34 = new QDoubleSpinBox(this);
	m_l34->setMaximumWidth(DOUBLESPINBOX_WIDTH);
	m_l34->setMinimumWidth(DOUBLESPINBOX_WIDTH);
	m_l34->setMaximum(99999);
	m_l34->setMinimum(0);
	connect(m_l34, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
		[=](double d) {
			m_L3 = d; 
			emit sigValueChanged(); 
		}
	);

	m_l41 = new QDoubleSpinBox(this);
	m_l41->setMaximumWidth(DOUBLESPINBOX_WIDTH);
	m_l41->setMinimumWidth(DOUBLESPINBOX_WIDTH);
	m_l41->setMaximum(99999);
	m_l41->setMinimum(0);
	connect(m_l41, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
		[=](double d) {
			m_L4 = d; 
			emit sigValueChanged(); 
		}
	);

	m_l13 = new QDoubleSpinBox(this);
	m_l13->setMaximumWidth(DOUBLESPINBOX_WIDTH);
	m_l13->setMinimumWidth(DOUBLESPINBOX_WIDTH);
	m_l13->setMaximum(99999);
	m_l13->setMinimum(0);
	connect(m_l13, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
		[=](double d) {
			m_L5 = d; emit sigValueChanged(); 
		}
	);

	connect(this, SIGNAL(sigValueChanged()), this, SLOT(onValueChanged()));

	OnLayout();
}

CLength4Points::~CLength4Points()
{
}

void CLength4Points::init(QVector<QPointF> &points)
{
	float x1 = points[1].x();
	float y1 = points[1].y();

	float x2 = points[2].x();
	float y2 = points[2].y();

	float x3 = points[3].x();
	float y3 = points[3].y();

	float x4 = points[4].x();
	float y4 = points[4].y();

	m_points = points;

	if (INVALID_VALUE_FLOAT == x1
		|| INVALID_VALUE_FLOAT == y1
		|| INVALID_VALUE_FLOAT == x2
		|| INVALID_VALUE_FLOAT == y2
		|| INVALID_VALUE_FLOAT == x3
		|| INVALID_VALUE_FLOAT == y3
		|| INVALID_VALUE_FLOAT == x4
		|| INVALID_VALUE_FLOAT == y4 )
		return;

	m_L1 = sqrtf((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
	m_L2 = sqrtf((x2 - x3)*(x2 - x3) + (y2 - y3)*(y2 - y3));
	m_L3 = sqrtf((x3 - x4)*(x3 - x4) + (y3 - y4)*(y3 - y4));
	m_L4 = sqrtf((x4 - x1)*(x4 - x1) + (y4 - y1)*(y4 - y1));
	m_L5 = sqrtf((x1 - x3)*(x1 - x3) + (y1 - y3)*(y1 - y3));

	m_l12->setValue(m_L1);
	m_l23->setValue(m_L2);
	m_l34->setValue(m_L3);
	m_l41->setValue(m_L4);
	m_l13->setValue(m_L5);
}

void CLength4Points::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	//OnLayout();
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.drawImage(m_zeroPoint, *m_image);
	QWidget::paintEvent(event);
}

void CLength4Points::OnLayout()
{
	QPointF pointA = QPointF(0, 0);
	QPointF pointB = QPointF(0, IMAGE_LENTH - 1);
	QPointF pointC = QPointF(IMAGE_LENTH - 1, IMAGE_LENTH - 1);
	QPointF pointD = QPointF(IMAGE_LENTH - 1, 0);

	//布局
	int spinW = m_l13->width();
	int spinH = m_l13->height();
	QPointF tmp;
	tmp = m_zeroPoint + QPointF(-20, IMAGE_LENTH / 2) + QPointF(-1 * spinW, -1 * spinH / 2);
	m_l41->setDecimals(4);
	m_l41->move(tmp.x(), tmp.y());
	m_l41->setToolTip("|P4-P1|");

	tmp = m_zeroPoint + pointC + QPointF(-1 * IMAGE_LENTH / 2, 20) + QPointF(-1 * spinW / 2, 0);
	m_l12->setDecimals(4);
	m_l12->move(tmp.x(), tmp.y());
	m_l12->setToolTip("|P1-P2|");

	tmp = m_zeroPoint + pointD + QPointF(20, IMAGE_LENTH / 2) + QPointF(0, -1 * spinH / 2);
	m_l23->setDecimals(4);
	m_l23->move(tmp.x(), tmp.y());
	m_l23->setToolTip("|P2-P3|");

	tmp = m_zeroPoint + QPointF(IMAGE_LENTH / 2, -20) + QPointF(-1 * spinW / 2, -1 * spinH);
	m_l34->setDecimals(4);
	m_l34->move(tmp.x(), tmp.y());
	m_l34->setToolTip("|P3-P4|");

	tmp = m_zeroPoint + QPointF(IMAGE_LENTH / 2, IMAGE_LENTH / 2) + QPointF(-1 * spinW / 2, -1 * spinH / 2);
	m_l13->setDecimals(4);
	m_l13->move(tmp.x(), tmp.y());
	m_l13->setToolTip("|P1-P3|");

	tmp = m_zeroPoint + pointA + QPointF(-1 * POINT_TEXT_W, -1 * (POINT_TEXT_H));
	m_pointP4->move(tmp.x(), tmp.y());

	tmp = m_zeroPoint + pointB + QPointF(-1 * POINT_TEXT_W, 20);
	m_pointP1->move(tmp.x(), tmp.y());

	tmp = m_zeroPoint + pointC + QPointF(0, 20);
	m_pointP2->move(tmp.x(), tmp.y());

	tmp = m_zeroPoint + pointD + QPointF(0, -1 * (POINT_TEXT_H));
	m_pointP3->move(tmp.x(), tmp.y());
}

void CLength4Points::onValueChanged()
{
	if (0 >= m_L1 || 0 >= m_L2 || 0 >= m_L3 || 0 >= m_L4 || 0 >= m_L5) {
		for (int n = 0; n < m_points.size(); n++){
			m_points[n] = QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT);
		}
		setActualPoints(m_points);

		QString tmp = QString("P1 x:          \n   y:          ");
		m_pointP1->setText(tmp);

		tmp = QString("P2 x:          \n   y:          ");
		m_pointP2->setText(tmp);

		tmp = QString("P3 x:          \n   y:          ");
		m_pointP3->setText(tmp);

		tmp = QString("P4 x:          \n   y:          ");
		m_pointP4->setText(tmp);
		return;
	}

	m_points[1] = QPointF(0, 0);
	m_points[3] = QPointF(m_L5, 0);

	CalcPoint2();
	CalcPoint4();

	QString tmp = QString("P1 x: %1\n   y: %2").arg(QString::number(m_points[1].x(), 'f', 4)).arg(QString::number(m_points[1].y(), 'f', 4));
	m_pointP1->setText(tmp);

	tmp = QString("P2 x: %1\n   y: %2").arg(QString::number(m_points[2].x(), 'f', 4)).arg(QString::number(m_points[2].y(), 'f', 4));
	m_pointP2->setText(tmp);

	tmp = QString("P3 x: %1\n   y: %2").arg(QString::number(m_points[3].x(), 'f', 4)).arg(QString::number(m_points[3].y(), 'f', 4));
	m_pointP3->setText(tmp);

	tmp = QString("P4 x: %1\n   y: %2").arg(QString::number(m_points[4].x(), 'f', 4)).arg(QString::number(m_points[4].y(), 'f', 4));
	m_pointP4->setText(tmp);

	setActualPoints(m_points);
}

void CLength4Points::CalcPoint2()
{
	//x² + y² = L1²
	//y² + (L3 - x)² = L2²
	float x = abs((m_L1 * m_L1 - m_L2 * m_L2 + m_L5 * m_L5) / (2 * m_L5));
	float y = -1 * abs(sqrtf(m_L1 * m_L1 - x * x));

	m_points[2] = QPointF(x, y);
}

void CLength4Points::CalcPoint4()
{
	//x² + y² = L4²
	//y² + (L5 - x)² = L3²
	float x = abs((m_L4 * m_L4 - m_L3 * m_L3 + m_L5 * m_L5) / (2 * m_L5));
	float y = abs(sqrtf(m_L4 * m_L4 - x * x));
	m_points[4] = QPointF(x, y);
}