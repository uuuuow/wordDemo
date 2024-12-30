#include "Coordinate2Points.h"
#include "PointItem.h"

//实际坐标输入，两点
CCoordinate2Points::CCoordinate2Points(QWidget *parent)
	: CInputWidget(parent)
{
}

CCoordinate2Points::~CCoordinate2Points()
{
}

void CCoordinate2Points::initActual(QVector<QPointF> &points)
{
	m_actualPoints = points;
	m_actualPoints[0] = QPointF(0, 0);

	addItem(0);
	addItem(1);
	m_itemList[0]->setEnableXY(false, false);
	m_itemList[1]->setEnableXY(true, false);
}
