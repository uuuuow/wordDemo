#pragma once
#include "InputWidget.h"

class CCoordinate4Points : public CInputWidget
{
	Q_OBJECT

public:
	CCoordinate4Points(QWidget *parent);
	~CCoordinate4Points();
	virtual void initActual(QVector<QPointF> &points);
};
