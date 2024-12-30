#pragma once
#include "PointsManage.h"
//一维线性
//FIXED_CAM_1D

class CFixedCam1D : public CPointsManage
{
	Q_OBJECT

public:
	CFixedCam1D(VTCalibration mode, QObject *parent);
	~CFixedCam1D();
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
