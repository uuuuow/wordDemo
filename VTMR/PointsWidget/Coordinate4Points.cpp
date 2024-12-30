#include "Coordinate4Points.h"
#include "PointItem.h"

CCoordinate4Points::CCoordinate4Points(QWidget *parent)
	: CInputWidget(parent)
{
}

CCoordinate4Points::~CCoordinate4Points()
{
}

void CCoordinate4Points::initActual(QVector<QPointF> &points)
{
	m_actualPoints = points;
	addItem(1);
	addItem(2);
	addItem(3);
	addItem(4);
	m_itemList[1]->setEnableXY(true, true);
	m_itemList[2]->setEnableXY(true, true);
	m_itemList[3]->setEnableXY(true, true);
	m_itemList[4]->setEnableXY(true, true);
}
