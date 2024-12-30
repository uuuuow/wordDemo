#pragma once

#include <QWidget>
#include "Utils.h"

class CLength2Coordinate : public QWidget
{
	Q_OBJECT

public:
	CLength2Coordinate(QWidget *parent);
	~CLength2Coordinate();

	QVector<QPointF> &getActualPoints() { return m_actualPoints; };
	void setActualPoints(QVector<QPointF> &points) {m_actualPoints = points;};
	void getReferenceDes(QString &from, QString &to) {
		from = m_referenceDesFrom; to = m_referenceDesTo;
	}
	void setReferenceDes(QString & from, QString & to) {
		m_referenceDesFrom = from; m_referenceDesTo = to;
	}

	virtual void init(QVector<QPointF> &points) = 0;
private:
	QVector<QPointF> m_actualPoints = {
		QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
		QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
		QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
		QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
		QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT)
	};
protected:
	QString m_referenceDesFrom;
	QString m_referenceDesTo;
};
