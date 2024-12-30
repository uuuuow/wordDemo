#include "PointsManage.h"
#include "Utils.h"
#include "Global.h"
#include "InputWidget.h"
#include "KeyTrack.h"
#include <qDebug>
#include "FixedCam1D.h"
#include "FixedCam2D.h"
#include "MovingCamSpace.h"
#include "MovingCam2DOneself.h"
#include "MovingCam2DTargetVehicle.h"
#include "FixedCamReference.h"
#include "TwoPoints.h"

CPointsManage::CPointsManage(VTCalibration mode, QObject *parent)
	: QObject(parent)
{
	connect(this, SIGNAL(sigPointsChange()), parent, SLOT(onPointsChange()));
	connect(this, SIGNAL(sigNotifyStatus(int)), parent, SLOT(onNotifyStatus(int)));
	connect(this, SIGNAL(sigNotifyVelocity(QString)), parent, SLOT(onNotifyVelocity(QString)));
	connect(this, SIGNAL(sigNotifyVelocity()), parent, SLOT(onNotifyVelocity()));
}

CPointsManage::~CPointsManage()
{
}

void CPointsManage::createPointsManage(QObject *obj, VTCalibration mode, CPointsManage **manage)
{
	switch (mode)
	{
		//车身自标定
	case FIXED_CAM_ONESELF:
	{
		*manage = new CTwoPoints(mode, obj);
	}
	break;
	case FIXED_CAM_GROUND_REFERENCE:
	{
		*manage = new CFixedCamReference(mode, obj);
	}
	break;
	//一维线性
	case FIXED_CAM_1D:
	{
		*manage = new CFixedCam1D(mode, obj);
	}
	break;
	//二维线性
	case FIXED_CAM_2D:
	{
		*manage = new CFixedCam2D(mode, obj);
	}
	break;
	//车载摄像头
	//空间标定
	case MOVING_CAM_SPACE:
	{
		*manage = new CTwoPoints(mode, obj);
	}
	break;
	//二维本车
	case MOVING_CAM_2D_ONESELF:
	{
		*manage = new CMovingCam2DOneself(mode, obj);
	}
	break;
	//二维目标车
	case MOVING_CAM_2D_TARGET_VEHICLE:
	{
		*manage = new CMovingCam2DTargetVehicle(mode, obj);
	}
	break;

	default:
		*manage = nullptr;
		break;
	}
}

int  CPointsManage::getFrameCount(VTProjectInfo &info)
{
	return info.frameInfos.size();
}

qint64 CPointsManage::getPTS(VTProjectInfo &info, int index)
{
	return info.frameInfos[index].pts;
}

void CPointsManage::getImagePoints(VTProjectInfo &info, int index, QVector<QPoint>& points)
{
	points = info.frameInfos[index].imagePoints;
	if (--index < 0)
		return;
	if (FIXED_CAM_2D == info.mode) {
		points[6] = info.frameInfos[index].imagePoints[1];
		points[7] = info.frameInfos[index].imagePoints[2];
		points[8] = info.frameInfos[index].imagePoints[3];
		points[9] = info.frameInfos[index].imagePoints[4];
	}
	
}

QString CPointsManage::getImagePath(VTProjectInfo &info, int index)
{
	return info.frameInfos[index].imagePath;
}

int CPointsManage::getFrameEditStutas(VTProjectInfo &info, int index)
{
	return info.frameInfos[index].frameEditStutas;
}

bool CPointsManage::theSamePoint(QPoint pos, QPoint imagePoint)
{
	if (imagePoint.x() - 5 <= pos.x() && pos.x() <= imagePoint.x() + 5) {
		if (imagePoint.y() - 5 <= pos.y() && pos.y() <= imagePoint.y() + 5) {
			return true;
		}
	}
	return false;
}

void CPointsManage::clearImagePoints(VTProjectInfo &info, int index)
{
	if (!fixdFirstFrame()) {
		for (int n = 0; n < IMAGE_POINTS_COUNT; ++n) {
			info.frameInfos[index].imagePoints[n] = QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT);
		}
	}
}

void CPointsManage::delPoints(VTProjectInfo &info, int index, QPoint point)
{
	if (info.frameInfos.size() == 0)
		return;
	if (index >= info.frameInfos.size())
		return;
	for (int n = 0; n < IMAGE_POINTS_COUNT; ++n) {
		if (theSamePoint(point, info.frameInfos[index].imagePoints[n])) {
			info.frameInfos[index].imagePoints[n] = QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT);
			info.frameInfos[index].frameEditStutas = 0;
			auto log = QString("帧[%1] 删除 P%2(%3,%4)").arg(index).arg(n).arg(point.x()).arg(point.y());
			g_logBrowser->append(log);
			emit sigNotifyStatus(0);
			emit sigPointsChange();
			break;
		}
	}

	for (int n = 0; n < info.imageLines.size(); ++n) {
		if (info.imageLines[n].size() != 3)
			continue;
		QPoint begin = info.imageLines[n][1];
		QPoint end = info.imageLines[n][2];

		if (!availableImagePoint(begin) || !availableImagePoint(end))
			continue;
		QLine line;
		line.setP1(begin);
		line.setP2(end);
		if (0 == pointOnLine(point, line)) {
			info.imageLines[n][0] = QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT);
			info.imageLines[n][1] = QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT);
			info.imageLines[n][2] = QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT);

			info.frameInfos[index].frameEditStutas = 0;
			auto log = QString("帧[%1] 删除 line%2[(%3,%4) (%5,%6)]").arg(index).arg(n+1).arg(begin.x()).arg(begin.y()).arg(end.x()).arg(end.y());
			g_logBrowser->append(log);
			emit sigNotifyStatus(0);
			emit sigPointsChange();
		}
	}
}

int CPointsManage::getPreFrameInfo(VTProjectInfo &info, int index, VTFrameInfo &preFrameInfo)
{
	int ret = -1;
	for (int n = index - 1; n >= 0; --n) {
		if (info.frameInfos[n].actualP0 == QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT))
			continue;
		preFrameInfo = info.frameInfos[n];
		ret = 0;
		break;
	}
	return ret;
}

int CPointsManage::getFrameV(VTProjectInfo &info, int index, float &v)
{
	if (index >= info.frameInfos.size() || index < 0)
		return -1;
	if (info.timeMode == TIME_MODE_PTS)
		v = info.frameInfos[index].v1;
	else if (info.timeMode == TIME_MODE_FPS)
		v = info.frameInfos[index].v2;
	else if (info.timeMode == TIME_MODE_WATER_MARK)
		v = info.frameInfos[index].v3;

	if (v <= 0)
		return -1;
	return 0;
}

void CPointsManage::getVPoints(VTProjectInfo &info, QPolygonF &points, QStringList &strHMSList)
{
	float v = INVALID_VALUE_FLOAT;
	int64_t pts = 0;
	if (info.mode == FIXED_CAM_ONESELF || info.mode == FIXED_CAM_GROUND_REFERENCE || info.mode == MOVING_CAM_SPACE)
		return;

	for (auto &frameInfo : info.frameInfos) {
		pts = frameInfo.pts;
		if (info.timeMode == TIME_MODE_PTS)
			v = frameInfo.v1;
		else if (info.timeMode == TIME_MODE_FPS)
			v = frameInfo.v2;
		else if (info.timeMode == TIME_MODE_WATER_MARK)
			v = frameInfo.v3;

		if (v != INVALID_VALUE_FLOAT && v != 0) {
			points << QPointF(pts / 1000, v);
			strHMSList.push_back(frameInfo.timeHMS);
		}
	}
	return;
}

int CPointsManage::getMPoints(VTProjectInfo &info, QVector<QPointF> &points, QVector<int64_t> &ptss)
{
	for (auto& frameInfo : info.frameInfos) {
		if (g_proInfo.mode == FIXED_CAM_ONESELF || g_proInfo.mode == FIXED_CAM_GROUND_REFERENCE || g_proInfo.mode == MOVING_CAM_SPACE) {
			if (frameInfo.imagePoints[0] == QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT))
				continue;
			points.push_back(frameInfo.imagePoints[0]);
		}
		else {
			if (frameInfo.actualP0 == QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT))
				continue;
			points.push_back(frameInfo.actualP0);
		}
		
		ptss.push_back(frameInfo.pts / 1000);
	}
	return 0;
}

void CPointsManage::clearCalc(VTProjectInfo &info)
{
	for (int n = 0; n < info.frameInfos.size(); ++n) {
		info.frameInfos[n].actualP0 = QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT);
		/*info.frameInfos[n].v1 = 0;
		info.frameInfos[n].v2 = 0;
		info.frameInfos[n].v3 = 0;*/
	}

	/*info.v01 = 0;
	info.v02 = 0;
	info.v03 = 0;
	info.v11 = 0;
	info.v12 = 0;
	info.v13 = 0;*/
}

bool CPointsManage::availableImagePoint(QPoint &point)
{
	if (INVALID_VALUE_INT == point.x())
		return false;
	if (INVALID_VALUE_INT == point.y())
		return false;
	return true;
}

double CPointsManage::pointOnLine(QPoint point, QLine line, bool about/* = true*/)
{
	double ret = 0;
	int x0 = point.x();
	int y0 = point.y();

	int x1 = line.p1().x();
	int y1 = line.p1().y();

	int x2 = line.p2().x();
	int y2 = line.p2().y();
	//y - kx + b = 0
	if (x1 == x2) {
		if (about) {
			if (abs(x0 - x1) <= 2)
				ret = 0;
			else
				ret = x0 - x1;
		}
		else
			ret = x0 - x1;
	}
	else {
		double k = (y1 - y2) * 1.0 / (x1 - x2);
		double b = x1 * k - y1;
		
		if (about) {
			if (abs(y0 - k * x0 + b) <= 2)
				ret = 0;
			else
				ret = y0 - k * x0 + b;
		}
		else
			ret = y0 - k * x0 + b;
	}
	return ret;
}