#pragma once

#include "PointsManage.h"
#include "Command.h"
//二维目标车
//MOVING_CAM_2D_TARGET_VEHICLE

class CMovingCam2DTargetVehicle : public CPointsManage
{
	Q_OBJECT

public:
	CMovingCam2DTargetVehicle(VTCalibration mode, QObject *parent);
	~CMovingCam2DTargetVehicle();
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
