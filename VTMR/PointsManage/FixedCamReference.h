#pragma once
#include "PointsManage.h"
#include <QVector>
#include "Utils.h"
//地面参照物
//FIXED_CAM_GROUND_REFERENCE

class CFixedCamReference : public CPointsManage
{
	Q_OBJECT

public:
	CFixedCamReference(VTCalibration mode, QObject *parent);
	~CFixedCamReference();

public:
	virtual bool fixdFirstFrame();
	virtual bool imagePointsReady(int index);
	virtual bool actualPointsReady();
	virtual void addPoints(int index, QPoint point);
	virtual void addLine(int index, QPoint center, QPoint begin, QPoint end);
	virtual void calcPointP0();
	virtual void notifyVelocity();
	virtual void initForecast();
	virtual void doForecast(int index);
private:
	int m_nearLineIndex = -1;
	int m_farLineIndex = -1;
};
