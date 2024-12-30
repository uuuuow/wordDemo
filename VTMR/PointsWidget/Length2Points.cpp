#include "Length2Points.h"
#include <QDoubleSpinBox>
#include <QPainter>

#define DOUBLESPINBOX_WIDTH 100
#define POINT_TEXT_W 100
#define POINT_TEXT_H 50
#define LINE_LENTH 440

CLength2Points::CLength2Points(QWidget *parent)
	: CLength2Coordinate(parent)
{
	m_zeroPoint = QPoint(50, 20);
	resize(340, 100);

	m_pointP0 = new QLabel("P0", this);
	m_pointP0->setMaximumSize(POINT_TEXT_W, POINT_TEXT_H);
	m_pointP0->setMinimumSize(POINT_TEXT_W, POINT_TEXT_H);

	m_pointP1 = new QLabel("P1", this);
	m_pointP1->setMaximumSize(POINT_TEXT_W, POINT_TEXT_H);
	m_pointP1->setMinimumSize(POINT_TEXT_W, POINT_TEXT_H);


	m_pointA = QPointF(0, 0);
	m_pointB = QPointF(LINE_LENTH, 0);

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

	m_unit = new QLabel("单位：（米）", this);

	int spinW = m_lAB->width();
	int spinH = m_lAB->height();
	QPointF tmp;
	tmp = m_zeroPoint + m_pointB + QPointF(-1 * LINE_LENTH / 2, 0) + QPointF(-1 * spinW / 2, -1 * spinH / 2);
	m_lAB->move(tmp.x(), tmp.y());
	m_lAB->setToolTip("|P0-P1|");

	tmp = m_zeroPoint + m_pointA + QPointF(-1 * spinW / 2, 10);
	m_pointP0->move(tmp.x(), tmp.y());

	tmp = m_zeroPoint + m_pointB + QPointF(-1 * spinW / 2, 10);
	m_pointP1->move(tmp.x(), tmp.y());
	connect(this, SIGNAL(sigValueChanged()), this, SLOT(onValueChanged()));

	m_LBRefenceDes = new QLabel(QString("参照物描述:"), this);
	m_LBZhi = new QLabel(QString("至"), this);
	m_LBZhi->setFixedWidth(30);
	m_LERefDes1 = new QLineEdit(this);
	m_LERefDes1->setFixedWidth(200);
	m_LERefDes2 = new QLineEdit(this);
	m_LERefDes2->setFixedWidth(200);

	connect(m_LERefDes1, &QLineEdit::textChanged, [=](const QString &text) {m_referenceDesFrom = text; });
	connect(m_LERefDes2, &QLineEdit::textChanged, [=](const QString &text) {m_referenceDesTo = text; });

	tmp = m_pointP0->pos() + QPointF(0, 80);
	m_LBRefenceDes->move(tmp.x(), tmp.y());
	m_LERefDes1->move(m_LBRefenceDes->pos().x()+ m_LBRefenceDes->width() + 10, m_LBRefenceDes->pos().y());
	m_LBZhi->move(m_LERefDes1->pos().x() + m_LERefDes1->width(), m_LERefDes1->pos().y());
	m_LERefDes2->move(m_LBZhi->pos().x()+ m_LBZhi->width(), m_LBZhi->pos().y());

	setFixedSize(QSize(560, 140));
}

CLength2Points::~CLength2Points()
{
}

void CLength2Points::init(QVector<QPointF> &points)
{
	float x0 = points[0].x();
	float y0 = points[0].y();

	float x1 = points[1].x();
	float y1 = points[1].y();

	if (INVALID_VALUE_FLOAT == x0
		|| INVALID_VALUE_FLOAT == x1)
		return;

	m_L1 = sqrtf((x0 - x1)*(x0 - x1) - (y0 - y1)*(y0 - y1)) * 1000;
	m_lAB->setValue(m_L1*1.0 / 1000);

	emit sigValueChanged();

	m_LERefDes1->setText(m_referenceDesFrom);
	m_LERefDes2->setText(m_referenceDesTo);
}

void CLength2Points::onValueChanged()
{
	if (0 == m_L1) {
		QVector<QPointF> points = {
			QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
			QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
			QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
			QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
			QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT)
		};
		setActualPoints(points);

		m_pointP0->setText("P0");
		m_pointP1->setText("P1");
		return;
	}

	QPointF P0 = QPointF(0, 0);
	QPointF P1 = QPointF(m_L1 / 1000.0, 0);

	QString tmp = QString("P0(%1,%2)").arg(P0.x()).arg(P0.y());
	m_pointP0->setText(tmp);

	tmp = QString("P1(%1,%2)").arg(P1.x()).arg(P1.y());
	m_pointP1->setText(tmp);

	QVector<QPointF> points = {
		QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
		QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
		QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
		QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
		QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT)
	};
	points[0] = P0;
	points[1] = P1;

	setActualPoints(points);
}

void CLength2Points::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

	QPainter painter(this);

	// 反走样
	painter.setRenderHint(QPainter::Antialiasing, true);

	// 绘制图标
	painter.setPen(QColor(0, 160, 230));
	QPointF P0 = m_zeroPoint + m_pointA;
	QPointF P1 = m_zeroPoint + m_pointB;
	painter.drawLine(P0, P1);
}