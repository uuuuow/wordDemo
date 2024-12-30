#pragma once

#include <QWidget>
#include <QLabel>
#include <QPointF>
#include "Length2Coordinate.h"
#include <QDoubleSpinBox>

/*
*长度转坐标
*/
class CLength4Points : public CLength2Coordinate
{
	Q_OBJECT

public:
	CLength4Points(QWidget *parent);
	~CLength4Points();

	virtual void init(QVector<QPointF> &points);

protected:
	void paintEvent(QPaintEvent *event);
	void CalcPoint2();
	void CalcPoint4();
	void OnLayout();

private:
	float m_L1 = 0.0f;
	float m_L2 = 0.0f;
	float m_L3 = 0.0f;
	float m_L4 = 0.0f;
	float m_L5 = 0.0f;

	QPoint m_zeroPoint;

	QImage *m_image;

	QLabel *m_pointP1 = nullptr;
	QLabel *m_pointP2 = nullptr;
	QLabel *m_pointP3 = nullptr;
	QLabel *m_pointP4 = nullptr;

	QDoubleSpinBox *m_l12 = nullptr;
	QDoubleSpinBox *m_l23 = nullptr;
	QDoubleSpinBox *m_l34 = nullptr;
	QDoubleSpinBox *m_l41 = nullptr;
	QDoubleSpinBox *m_l13 = nullptr;

	QVector<QPointF> m_points = {
		QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
		QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
		QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
		QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
		QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT)
	};

public slots:
	void onValueChanged();

signals:
	void sigValueChanged();
};
