#pragma once
#include "Length2Coordinate.h"
#include "Utils.h"
#include <QLabel>
#include <QDoubleSpinBox>

class CLength3Points : public CLength2Coordinate
{
	Q_OBJECT

public:
	CLength3Points(QWidget *parent);
	~CLength3Points();

	virtual void init(QVector<QPointF> &points);
	void paintEvent(QPaintEvent *event);

private:
	int m_L1 = 0;
	int m_L2 = 0;

	QPoint m_zeroPoint;

	double m_valueAB;
	double m_valueBC;

	QLabel *m_pointP1 = nullptr;
	QLabel *m_pointP21 = nullptr;
	QLabel *m_pointP22 = nullptr;
	QLabel *m_pointP3 = nullptr;

	QPointF m_pointA;
	QPointF m_pointB1;
	QPointF m_pointB2;
	QPointF m_pointC;

	QDoubleSpinBox *m_lAB = nullptr;
	QDoubleSpinBox *m_lBC = nullptr;

public slots:
	void onValueChanged();

signals:
	void sigValueChanged();
};
