#pragma once

#include "PointsManage.h"
//二维线性
//FIXED_CAM_2D

class CFixedCam2D : public CPointsManage
{
	Q_OBJECT

public:
	CFixedCam2D(VTCalibration mode, QObject *parent);
	~CFixedCam2D();

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
