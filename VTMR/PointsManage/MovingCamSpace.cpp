#include "MovingCamSpace.h"
#include "Global.h"
#include <QDebug>

CMovingCamSpace::CMovingCamSpace(VTCalibration mode, QObject *parent)
	: CPointsManage(mode, parent)
{
	
}

CMovingCamSpace::~CMovingCamSpace()
{
}

bool CMovingCamSpace::fixdFirstFrame()
{
	int headIndex = g_proInfo.headIndex;
	if (headIndex < 0)
		return false;
	if (availableImagePoint(g_proInfo.frameInfos[headIndex].imagePoints[0]) &&
		availableImagePoint(g_proInfo.frameInfos[headIndex].imagePoints[1]))
		return true;
	return false;
}

bool CMovingCamSpace::imagePointsReady(int index)
{
	if (INVALID_VALUE_INT != g_proInfo.frameInfos[index].imagePoints[0].x())
		return true;
	return false;
}

bool CMovingCamSpace::actualPointsReady()
{
	if (INVALID_VALUE_DOUBLE != g_proInfo.actualPoints[1].x())
		return true;
	return false;
}

void CMovingCamSpace::addPoints(int index, QPoint point)
{
	bool changeFlag = false;
	if (g_proInfo.frameInfos.size() == 0)
		return;
	if (index >= g_proInfo.frameInfos.size())
		return;
	if (point == QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT)) {
		changeFlag = true;
		goto EndHandle;
	}

	if (!fixdFirstFrame()) {
		//配置首帧
		g_proInfo.headIndex = index;
		if (INVALID_VALUE_INT == g_proInfo.frameInfos[index].imagePoints[1].x()) {
			g_proInfo.frameInfos[index].imagePoints[1].setX(point.x());
			g_proInfo.frameInfos[index].imagePoints[1].setY(point.y());
			changeFlag = true;
			auto log = QString("帧[%1] 标定 P1(%2,%3)").arg(index).arg(point.x()).arg(point.y());
			g_proInfo.undoStack.push(new AddPointCommand("p1", &g_proInfo.frameInfos[index], this, point, g_proInfo.curFrmaeIndex));
			g_logBrowser->append(log);
		}
		else if (INVALID_VALUE_INT == g_proInfo.frameInfos[index].imagePoints[0].x()) {
			g_proInfo.frameInfos[index].imagePoints[0].setX(point.x());
			g_proInfo.frameInfos[index].imagePoints[0].setY(point.y());
			changeFlag = true;
			auto log = QString("帧[%1] 标定 P0(%2,%3)").arg(index).arg(point.x()).arg(point.y());
			g_proInfo.undoStack.push(new AddPointCommand("p0", &g_proInfo.frameInfos[index], this, point, g_proInfo.curFrmaeIndex));
			g_logBrowser->append(log);
		}
	}
	else {
		if (INVALID_VALUE_INT == g_proInfo.frameInfos[index].imagePoints[0].x()) {
			g_proInfo.frameInfos[index].imagePoints[0].setX(point.x());
			g_proInfo.frameInfos[index].imagePoints[0].setY(point.y());
			changeFlag = true;
			auto log = QString("帧[%1] 标定 P0(%2,%3)").arg(index).arg(point.x()).arg(point.y());
			g_logBrowser->append(log);
		}
	}

	//配置帧状态
	if (fixdFirstFrame() &&
		g_proInfo.headIndex == index) {
		g_proInfo.frameInfos[index].frameEditStutas = 2;
		emit sigNotifyStatus(2);
	}

	//配置普通帧状态
	if (imagePointsReady(index) &&
		g_proInfo.headIndex != index)
	{
		g_proInfo.frameInfos[index].frameEditStutas = 1;
		emit sigNotifyStatus(1);
	}

	if (!imagePointsReady(index) &&
		g_proInfo.headIndex != index) {
		g_proInfo.frameInfos[index].frameEditStutas = 0;
		emit sigNotifyStatus(0);
	}

EndHandle:
	//有点改变
	if (changeFlag)
		emit sigPointsChange();

	clearCalc(g_proInfo);
	calcPointP0();
}

void CMovingCamSpace::calcPointP0()
{
	if (!actualPointsReady())
		return;
	if (!fixdFirstFrame())
		return;

	int headIndex = g_proInfo.headIndex;
	float imageP0x = g_proInfo.frameInfos[headIndex].imagePoints[0].x();
	float imageP0y = g_proInfo.frameInfos[headIndex].imagePoints[0].y();
	float imageP1x = g_proInfo.frameInfos[headIndex].imagePoints[1].x();
	float imageP1y = g_proInfo.frameInfos[headIndex].imagePoints[1].y();
	float actualP0X = g_proInfo.actualPoints[0].x();
	float actualP1X = g_proInfo.actualPoints[1].x();

	for (int n = headIndex + 1; n < g_proInfo.frameInfos.size(); ++n) {
		if (INVALID_VALUE_INT == g_proInfo.frameInfos[n].imagePoints[0].x())
			break;

		float curImageP0x = g_proInfo.frameInfos[n].imagePoints[0].x();
		float curImageP0y = g_proInfo.frameInfos[n].imagePoints[0].y();

		float curAx = 0.0, fCurAy = 0.0;
		float num = sqrt((curImageP0x - imageP0x)*(curImageP0x - imageP0x) + (curImageP0y - imageP0y)*(curImageP0y - imageP0y))*(actualP1X - actualP0X);
		float den = sqrt((imageP1x - imageP0x)*(imageP1x - imageP0x) + (imageP1y - imageP0y)*(imageP1y - imageP0y));
		if (den != 0) {
			curAx = (num / den);
			g_proInfo.frameInfos[n].actualP0 = QPointF(curAx, 0);
		}
	}

	calcVelocity();

	notifyVelocity();
}

void CMovingCamSpace::calcVelocity()
{
}

void CMovingCamSpace::notifyVelocity()
{
	if (!actualPointsReady())
		return;
	if (!fixdFirstFrame())
		return;
	int headIndex = g_proInfo.headIndex;

	float imageP0x = g_proInfo.frameInfos[headIndex].imagePoints[0].x();
	float imageP0y = g_proInfo.frameInfos[headIndex].imagePoints[0].y();

	float imageP1x = g_proInfo.frameInfos[headIndex].imagePoints[1].x();
	float imageP1y = g_proInfo.frameInfos[headIndex].imagePoints[1].y();

	float actualP0X = g_proInfo.actualPoints[0].x();
	float actualP0Y = g_proInfo.actualPoints[0].y();

	float actualP1X = g_proInfo.actualPoints[1].x();
	float actualP1Y = g_proInfo.actualPoints[1].y();

	for (int n = g_proInfo.headIndex + 1; n < g_proInfo.frameInfos.size(); ++n) {
		//先判断行车方向
		bool isPass = false;
		bool isEqual = false;
		float t01 = 0.0;
		float t02 = 0.0;
		float t03 = 0.0;
		float t11 = 0.0;
		float t12 = 0.0;
		float t13 = 0.0;
		if (abs(imageP0x - imageP1x) > abs(imageP0y - imageP1y))//左右行驶
		{
			if (imageP0x > imageP1x)//车从屏幕右边向左边行驶
			{
				if (abs(g_proInfo.frameInfos[n].imagePoints[0].x() - imageP1x) <= 2)//相距在两个像素以内
				{
					isEqual = true;
				}
				else if (g_proInfo.frameInfos[n].imagePoints[0].x() < imageP1x)
				{
					isPass = true;
				}
			}
			else {//车从屏幕左边向右边行驶
				if (abs(g_proInfo.frameInfos[n].imagePoints[0].x() - imageP1x) <= 2)//相距在两个像素以内
				{
					isEqual = true;
				}
				else if (g_proInfo.frameInfos[n].imagePoints[0].x() > imageP1x)
				{
					isPass = true;
				}
			}
		}
		else {//上下行驶
			if (imageP0y < imageP1y)//车从屏幕下方向上方行驶
			{
				if (abs(g_proInfo.frameInfos[n].imagePoints[0].x() - imageP1y) <= 2)//相距在两个像素以内
				{
					isEqual = true;
				}
				else if (g_proInfo.frameInfos[n].imagePoints[0].x() > imageP1y)
				{
					isPass = true;
				}
			}
			else {//车从屏幕上方向下方行驶
				if (abs(g_proInfo.frameInfos[n].imagePoints[0].x() - imageP1y) <= 2)//相距在两个像素以内
				{
					isEqual = true;
				}
				else if (g_proInfo.frameInfos[n].imagePoints[0].x() < imageP1y)
				{
					isPass = true;
				}
			}
		}


		if (isPass)
		{
			//视频帧时间戳PTS
			t01 = g_proInfo.frameInfos[n].pts - g_proInfo.frameInfos[headIndex].pts;
			t11 = g_proInfo.frameInfos[n - 1].pts - g_proInfo.frameInfos[headIndex].pts;
			if (t01 > 0)
			{
				g_proInfo.frameInfos[n].v1 = (abs(actualP0X - actualP1X) * 1000 * 1000.0 / t01) * 3.6;
			}

			if (t11 > 0)
			{
				g_proInfo.frameInfos[n - 1].v1 = (abs(actualP0X - actualP1X) * 1000 * 1000.0 / t11) * 3.6;
			}
			//视频平均帧率FPS
			float ds = 0.0;
			float fps = 0.0;
			if (!VTFPS2AverageTime(g_proInfo, ds));
			else
			{
				t02 = (n - headIndex) * ds;
				t12 = (n - 1 - headIndex) * ds;
			}
			if (t02 > 0)
			{
				g_proInfo.frameInfos[n].v2 = (abs(actualP0X - actualP1X) * 1000 * 1000.0 / t02) * 3.6;
			}

			if (t12 > 0)
			{
				g_proInfo.frameInfos[n - 1].v2 = (abs(actualP0X - actualP1X) * 1000 * 1000.0 / t12) * 3.6;
			}
			//视频水印时间
			ds = 0.0;
			if (!VTWT2AverageTime(g_proInfo, ds, fps));
			else
			{
				t03 = (n - headIndex) * ds;
				t13 = (n - 1 - headIndex) * ds;
			}

			if (t03 > 0)
			{
				g_proInfo.frameInfos[n].v3 = (abs(actualP0X - actualP1X) * 1000 * 1000.0 / t03) * 3.6;
			}

			if (t13 > 0)
			{
				g_proInfo.frameInfos[n-1].v3 = (abs(actualP0X - actualP1X) * 1000 * 1000.0 / t13) * 3.6;
			}

			float v0 = 0.0, v1 = 0.0;
			if (g_proInfo.timeMode == TIME_MODE_PTS)
			{
				v0 = g_proInfo.frameInfos[n].v1;
				v1 = g_proInfo.frameInfos[n - 1].v1;
			}
			else if (g_proInfo.timeMode == TIME_MODE_FPS)
			{
				v0 = g_proInfo.frameInfos[n].v2;
				v1 = g_proInfo.frameInfos[n - 1].v2;
			}
			else if (g_proInfo.timeMode == TIME_MODE_WATER_MARK)
			{
				v0 = g_proInfo.frameInfos[n].v3;
				v1 = g_proInfo.frameInfos[n - 1].v3;
			}

			QString strTemp = QString("%1-%2").arg(v0).arg(v1);
			QString text = QString("区间速度: %1 km/h").arg(strTemp);
			auto log = QString("帧[%1]-[%2] 区间速度: %3 km/h").arg(n - 1).arg(n).arg(strTemp);
			g_logBrowser->append(log);
			emit sigNotifyVelocity(text);

			break;
		}

		if (isEqual)
		{
			t01 = 0.0;
			t02 = 0.0;
			t03 = 0.0;
			//视频帧时间戳PTS
			t01 = g_proInfo.frameInfos[n].pts - g_proInfo.frameInfos[headIndex].pts;
			if (t01 > 0)
				g_proInfo.frameInfos[n].v1 = (abs(actualP0X - actualP1X) * 1000 * 1000.0 / t01) * 3.6;

			//视频平均帧率FPS
			float ds = 0.0;
			float fps = 0.0;
			if (!VTFPS2AverageTime(g_proInfo, ds));
			else
			{
				t02 = (n - headIndex) * ds;
			}
			if (t02 > 0)
				g_proInfo.frameInfos[n].v2 = (abs(actualP0X - actualP1X) * 1000 * 1000.0 / t02) * 3.6;

			//视频水印时间
			ds = 0.0;
			if (!VTWT2AverageTime(g_proInfo, ds, fps));
			else
			{
				t03 = (n - headIndex) * ds;
			}

			if (t03 > 0)
				g_proInfo.frameInfos[n].v3 = (abs(actualP0X - actualP1X) * 1000 * 1000.0 / t03) * 3.6;

			float V = 0.0;
			if (g_proInfo.timeMode == TIME_MODE_PTS)
			{
				V = g_proInfo.frameInfos[n].v1;
			}
			else if (g_proInfo.timeMode == TIME_MODE_FPS)
			{
				V = g_proInfo.frameInfos[n].v2;
			}
			else if (g_proInfo.timeMode == TIME_MODE_WATER_MARK)
			{
				V = g_proInfo.frameInfos[n].v3;
			}
			QString text = QString("速度: %1 km/h").arg(V);
			auto log = QString("帧[%1] 速度: %2 km/h").arg(n).arg(V);
			g_logBrowser->append(log);
			emit sigNotifyVelocity(text);

			break;
		}
	}
}

void CMovingCamSpace::initForecast() {
	cv::Mat firstFrame;
	if (!fixdFirstFrame())
		return;
	m_keyTrack.release();
	m_initForecast = false;
	if (0 != m_keyTrack.initKeyTrack(1)) {
		qDebug() << QString("跟踪初始化失败");
		return;
	}
	int headIndex = g_proInfo.headIndex;
	firstFrame = cv::imread(g_proInfo.frameInfos[headIndex].imagePath.toStdString());
	if (firstFrame.data == nullptr)
		return;

	QVector<QPoint> points;
	points.push_back(g_proInfo.frameInfos[headIndex].imagePoints[0]);
	m_keyTrack.addKeyPoint(firstFrame, points);

	m_initForecast = true;
}

void CMovingCamSpace::doForecast(int index)
{
	cv::Mat forecastFrame;

	if (!g_checkKeyTrace)
		return;

	if (!fixdFirstFrame())
		return;

	if (!m_initForecast)
		return;

	int headIndex = g_proInfo.headIndex;
	if (index > g_proInfo.frameInfos.size() - 1)
		return;
	
	forecastFrame = cv::imread(g_proInfo.frameInfos[index].imagePath.toStdString());
	if (forecastFrame.data == nullptr)
		return;

	QVector<QPoint> points;
	m_keyTrack.getCurrentPoint(forecastFrame, points);
	if (points.size() == 0)
		return;
	g_proInfo.frameInfos[index].imagePoints[0] = points[0];
}