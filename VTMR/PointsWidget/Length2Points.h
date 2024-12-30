#pragma once

#include "Length2Coordinate.h"
#include <QLabel>
#include <QDoubleSpinBox>
#include <QLineEdit>

class CLength2Points : public CLength2Coordinate
{
	Q_OBJECT

public:
	CLength2Points(QWidget *parent);
	~CLength2Points();

	virtual void init(QVector<QPointF> &points);
	void paintEvent(QPaintEvent *event);

private:
	int m_L1 = 0;

	QPoint m_zeroPoint;

	double m_valueAB;
	double m_valueBC;

	QLabel *m_pointP0 = nullptr;
	QLabel *m_pointP1 = nullptr;

	QLabel* m_unit = nullptr;

	QPointF m_pointA;
	QPointF m_pointB;

	QDoubleSpinBox *m_lAB = nullptr;
	QLabel *m_LBRefenceDes = nullptr;
	QLabel *m_LBZhi = nullptr;
	QLineEdit *m_LERefDes1 = nullptr;
	QLineEdit *m_LERefDes2 = nullptr;
public slots:
	void onValueChanged();

signals:
	void sigValueChanged();
};
