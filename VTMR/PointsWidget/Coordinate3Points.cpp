#include "Coordinate3Points.h"
#include "PointItem.h"

CCoordinate3Points::CCoordinate3Points(QWidget *parent)
	: CInputWidget(parent)
{
}

CCoordinate3Points::~CCoordinate3Points()
{
}

void CCoordinate3Points::initActual(QVector<QPointF> &points)
{
	m_actualPoints = points;
	addItem(1);
	addItem(2);
	addItem(3);
	m_itemList[1]->setEnableXY(true, true);
	m_itemList[2]->setEnableXY(true, true);
	m_itemList[3]->setEnableXY(true, true);
}