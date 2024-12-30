#pragma once
#include "PointsManage.h"
#include <QVector>
#include "Utils.h"
//车身自标定
//FIXED_CAM_ONESELF

class CTwoPoints : public CPointsManage
{
	Q_OBJECT

public:
	CTwoPoints(VTCalibration mode, QObject *parent);
	~CTwoPoints();

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
	virtual bool needNotify();

private:
	int m_farLineIndex = -1;
};
