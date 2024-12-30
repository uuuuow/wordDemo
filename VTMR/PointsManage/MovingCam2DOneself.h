#pragma once

#include "PointsManage.h"
//二维本车
//MOVING_CAM_2D_ONESELF

class CMovingCam2DOneself : public CPointsManage
{
	Q_OBJECT

public:
	CMovingCam2DOneself(VTCalibration mode, QObject *parent);
	~CMovingCam2DOneself();
	void clearImagePoints(int index);

public:
	virtual bool fixdFirstFrame();
	virtual bool imagePointsReady(int index);
	virtual bool actualPointsReady();
	virtual void addPoints(int index, QPoint point);
	virtual void calcPointP0();
	virtual void calcVelocity();
	virtual void notifyVelocity();
	virtual void initForecast();
	virtual void doForecast(int index);
};
