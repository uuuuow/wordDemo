#pragma once
#include "PointsManage.h"
#include "Utils.h"
#include "Command.h"
/*
*车载空间自标定
*/
//MOVING_CAM_ONESELF
class CMovingCamSpace : public CPointsManage
{
	Q_OBJECT

public:
	CMovingCamSpace(VTCalibration mode, QObject *parent);
	~CMovingCamSpace();

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
