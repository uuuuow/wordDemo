#pragma once
#include "InputWidget.h"

class CCoordinate2Points : public CInputWidget
{
	Q_OBJECT

public:
	CCoordinate2Points(QWidget *parent);
	~CCoordinate2Points();
	virtual void initActual(QVector<QPointF> &points);
};
