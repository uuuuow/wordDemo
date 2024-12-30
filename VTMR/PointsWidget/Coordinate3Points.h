#pragma once
#include "InputWidget.h"

class CCoordinate3Points : public CInputWidget
{
	Q_OBJECT

public:
	CCoordinate3Points(QWidget *parent);
	~CCoordinate3Points();
	virtual void initActual(QVector<QPointF> &points);
};
