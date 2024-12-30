#include "TwoPoints.h"
#include "TakenWidget.h"
#include "InputWidget.h"
#include "MethodApi.h"
#include "Global.h"
#include "Command.h"
#include <QDebug>

CTwoPoints::CTwoPoints(VTCalibration mode, QObject *parent)
	: CPointsManage(mode, parent)
{
}

CTwoPoints::~CTwoPoints()
{
}

bool CTwoPoints::fixdFirstFrame()
{
	int headIndex = g_proInfo.headIndex;
	if (headIndex < 0 || g_proInfo.frameInfos.size() == 0)
		return false;
	if (availableImagePoint(g_proInfo.imageLines[0][0]) &&
		availableImagePoint(g_proInfo.imageLines[0][1]) &&
		availableImagePoint(g_proInfo.imageLines[0][2]))
		//availableImagePoint(g_proInfo.imageLines[1][0]) &&
		//availableImagePoint(g_proInfo.imageLines[1][1]) &&
		//availableImagePoint(g_proInfo.imageLines[1][2]))
		return true;
	return false;
}

bool CTwoPoints::imagePointsReady(int index)
{
	if (availableImagePoint(g_proInfo.frameInfos[index].imagePoints[0]))
		return true;
	return false;
}

bool CTwoPoints::actualPointsReady()
{
	if (INVALID_VALUE_DOUBLE != g_proInfo.actualPoints[1].x())
		return true;
	return false;
}

bool CTwoPoints::needNotify()
{
	/*if (fixdFirstFrame() && g_proInfo.fV0 != IV)
		return false;

	if (g_proInfo.fV0 != INVALID_VALUE_FLOAT)
		return false;*/
	return true;
}

void CTwoPoints::addPoints(int index, QPoint point)
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

	//是否配置首帧
	if (!fixdFirstFrame())
		return;
	//是否是普通帧
	if (index <= g_proInfo.headIndex)
		return;

	if (!availableImagePoint(g_proInfo.frameInfos[index].imagePoints[0])) {
		g_proInfo.frameInfos[index].imagePoints[0].setX(point.x());
		g_proInfo.frameInfos[index].imagePoints[0].setY(point.y());
		g_proInfo.undoStack.push(new AddPointCommand("p0", &g_proInfo.frameInfos[index], this, point, g_proInfo.curFrmaeIndex));
		changeFlag = true;
		auto log = QString("帧[%1] 标定 P0(%2,%3)").arg(index).arg(point.x()).arg(point.y());
		g_logBrowser->append(log);
	}

	//配置普通帧状态
	if (imagePointsReady(index)) {
		g_proInfo.frameInfos[index].frameEditStutas = 1;
		emit sigNotifyStatus(1);
	}
	else {
		g_proInfo.frameInfos[index].frameEditStutas = 0;
		emit sigNotifyStatus(0);
	}

EndHandle:
	//有点改变
	if (changeFlag) {
		emit sigPointsChange();
		m_farLineIndex = -1;
		clearCalc(g_proInfo);
		notifyVelocity();
	}
}

void CTwoPoints::addLine(int index, QPoint center, QPoint begin, QPoint end)
{
	bool changeFlag = false;
	if (g_proInfo.frameInfos.size() == 0)
		return;
	if (index >= g_proInfo.frameInfos.size())
		return;

	if (fixdFirstFrame()) {
		emit sigPointsChange();
		return;
	}

	//配置首帧
	g_proInfo.headIndex = index;
	
	if (!availableImagePoint(g_proInfo.imageLines[0][0]) ||
		!availableImagePoint(g_proInfo.imageLines[0][1]) ||
		!availableImagePoint(g_proInfo.imageLines[0][2])) {
		g_proInfo.imageLines[0][0] = center;
		g_proInfo.imageLines[0][1] = begin;
		g_proInfo.imageLines[0][2] = end;
		g_proInfo.undoStack.push(new AddLineCommand("line1", &g_proInfo, this, center, begin, end, g_proInfo.curFrmaeIndex));
		auto log = QString("帧[%1] 标定 Line1[(%2,%3) (%4,%5)]").arg(index).arg(begin.x()).arg(begin.y()).arg(end.y()).arg(end.y());
		g_logBrowser->append(log);
		changeFlag = true;
	}
	/*else if (!availableImagePoint(g_proInfo.imageLines[1][0]) ||
		!availableImagePoint(g_proInfo.imageLines[1][1]) ||
		!availableImagePoint(g_proInfo.imageLines[1][2])) {
		g_proInfo.imageLines[1][0] = center;
		g_proInfo.imageLines[1][1] = begin;
		g_proInfo.imageLines[1][2] = end;
		g_proInfo.undoStack.push(new AddLineCommand("line2", &g_proInfo, this, center, begin, end, g_proInfo.curFrmaeIndex));
		auto log = QString("帧[%1] 标定 Line2[(%2,%3) (%4,%5)]").arg(index).arg(begin.x()).arg(begin.y()).arg(end.y()).arg(end.y());
		g_logBrowser->append(log);
		changeFlag = true;
	}*/

	//配置帧状态
	if (fixdFirstFrame() && 
		g_proInfo.headIndex == index) {
		g_proInfo.frameInfos[index].frameEditStutas = 2;
		emit sigNotifyStatus(2);
	}

	//配置普通帧状态
	if (imagePointsReady(index) &&
		g_proInfo.headIndex != index) {
		g_proInfo.frameInfos[index].frameEditStutas = 1;
		emit sigNotifyStatus(1);
	}

	if (!imagePointsReady(index) &&
		g_proInfo.headIndex != index) {
		g_proInfo.frameInfos[index].frameEditStutas = 0;
		emit sigNotifyStatus(0);
	}

	//有点改变
	emit sigPointsChange();
}

void CTwoPoints::calcPointP0()
{
	if (!actualPointsReady())
		return;
	if (!fixdFirstFrame())
		return;

	QPoint nearP0 = g_proInfo.imageLines[1][0];
	QPoint farP0 = g_proInfo.imageLines[0][0];
	float L = abs(g_proInfo.actualPoints[1].x() - g_proInfo.actualPoints[0].x());
	float actualP0X = g_proInfo.actualPoints[0].x();
	float imageL = sqrt(nearP0.x() - farP0.x()) * (nearP0.x() - farP0.x()) + (nearP0.y() - farP0.y()) * (nearP0.y() - farP0.y());
	for (int n = g_proInfo.headIndex; n <= m_farLineIndex; ++n) {
		if (!availableImagePoint(g_proInfo.frameInfos[n].imagePoints[0]))
			continue;
		QPoint P0 = g_proInfo.frameInfos[n].imagePoints[0];
		float imageLP0 = sqrt(nearP0.x() - P0.x()) * (nearP0.x() - P0.x()) + (nearP0.y() - P0.y()) * (nearP0.y() - P0.y());
		float curAx = 0.0;
		if (imageL != 0) {
			curAx = (imageLP0 * L / imageL) + actualP0X;
			g_proInfo.frameInfos[n].actualP0 = QPointF(curAx, 0);
		}

		float t1 = 0.0;
		float t2 = 0.0;
		float t3 = 0.0;
		//视频帧时间戳PTS
		t1 = g_proInfo.frameInfos[n].pts - g_proInfo.frameInfos[g_proInfo.headIndex].pts;
		if (t1 > 0)
			g_proInfo.frameInfos[n].v1 = (curAx * 1000.0 * 1000.0 / t1) * 3.6;
		//视频平均帧率FPS
		float ds = 0.0;
		float fps = 0.0;
		if (!VTFPS2AverageTime(g_proInfo, ds))
			t2 = 0.0;
		else
			t2 = (n - g_proInfo.headIndex) * ds;
		if (t2 > 0)
			g_proInfo.frameInfos[n].v2 = (curAx * 1000.0 * 1000.0 / t2) * 3.6;
		//视频水印时间
		ds = 0.0;
		if (!VTWT2AverageTime(g_proInfo, ds, fps))
			t3 = 0.0;
		else
			t3 = (n - g_proInfo.headIndex) * ds;

		if (t3 > 0)
			g_proInfo.frameInfos[n].v3 = (curAx * 1000.0 * 1000.0 / t3) * 3.6;
	}
}

void CTwoPoints::notifyVelocity()
{
	if (!actualPointsReady())
		return;
	if (!fixdFirstFrame())
		return;

	float L = abs(g_proInfo.actualPoints[0].x() - g_proInfo.actualPoints[1].x());
	int headIndex = g_proInfo.headIndex;
	int index = headIndex;
	QPoint firstFrameP0 = g_proInfo.imageLines[0][0];
	QLine nearLine,farLine;
	nearLine.setP1(g_proInfo.imageLines[1][1]);
	nearLine.setP2(g_proInfo.imageLines[1][2]);
	farLine.setP1(g_proInfo.imageLines[0][1]);
	farLine.setP2(g_proInfo.imageLines[0][2]);
	double near = pointOnLine(firstFrameP0, nearLine, false);
	double far = pointOnLine(firstFrameP0, farLine, false);
	int farLineIndex = -1;
	int farLinePreIndex = -1;
	int preValue = 0;
	for (; index < g_proInfo.frameInfos.size(); ++index) {
		double value = 0;
		if (!imagePointsReady(index))
			continue;
		QPoint P0 = g_proInfo.frameInfos[index].imagePoints[0];
		value = pointOnLine(P0, farLine);
		if (value == 0) //在线上
			farLineIndex = index;
		else if ((preValue < 0 && value > 0) || (preValue > 0 && value < 0)) {//前后帧分布在线的两侧 
			farLinePreIndex = index - 1;
			farLineIndex = index;
		}
		preValue = value;

		if (farLineIndex != -1)
			break;
	}
	
	if (farLineIndex >= 0) {
		m_farLineIndex = farLineIndex;
		float t01 = 0.0;
		float t02 = 0.0;
		float t03 = 0.0;
		float t11 = 0.0;
		float t12 = 0.0;
		float t13 = 0.0;
		QString text = "";
		if (farLinePreIndex < 0) {
			g_proInfo.tailIndex0 = farLineIndex;
			//视频帧时间戳PTS
			t01 = g_proInfo.frameInfos[farLineIndex].pts - g_proInfo.frameInfos[headIndex].pts;
			if (t01 > 0)
				g_proInfo.v01 = (L * 1000.0 * 1000.0 / t01) * 3.6;
			//视频平均帧率FPS
			float ds = 0.0;
			float fps = 0.0;
			if (!VTFPS2AverageTime(g_proInfo, ds))
				t02 = 0.0;
			else
				t02 = (farLineIndex - headIndex) * ds;
			if (t02 > 0)
				g_proInfo.v02 = (L * 1000.0 * 1000.0 / t02) * 3.6;
			//视频水印时间
			ds = 0.0;
			if (!VTWT2AverageTime(g_proInfo, ds, fps))
				t03 = 0.0;
			else
				t03 = (farLineIndex - headIndex) * ds;
			if (t03 > 0)
				g_proInfo.v03 = (L * 1000.0 * 1000.0 / t03) * 3.6;

			float V = 0.0;
			if (g_proInfo.timeMode == TIME_MODE_PTS)
				V = g_proInfo.v01;
			else if (g_proInfo.timeMode == TIME_MODE_FPS)
				V = g_proInfo.v02;
			else if (g_proInfo.timeMode == TIME_MODE_WATER_MARK)
				V = g_proInfo.v03;

			text = QString("速度: %1 km/h").arg(V);
			if (g_proInfo.curFrmaeIndex == farLineIndex) {
				g_logBrowser->append(QString("实际线段间距 %1").arg(L));
				g_logBrowser->append(text);
			}
		}
		else {
			g_proInfo.tailIndex0 = farLineIndex;
			g_proInfo.tailIndex1 = farLinePreIndex;

			t01 = 0.0;
			t02 = 0.0;
			t03 = 0.0;
			t11 = 0.0;
			t12 = 0.0;
			t13 = 0.0;
			//视频帧时间戳PTS
			t01 = g_proInfo.frameInfos[farLineIndex].pts - g_proInfo.frameInfos[headIndex].pts;
			t11 = g_proInfo.frameInfos[farLinePreIndex].pts - g_proInfo.frameInfos[headIndex].pts;
			if (t01 > 0)
				g_proInfo.v01 = (L * 1000.0 * 1000.0 / t01)* 3.6;

			if (t11 > 0)
				g_proInfo.v11 = (L * 1000.0 * 1000.0 / t11) * 3.6;
			//视频平均帧率FPS
			float ds = 0.0;
			float fps = 0.0;
			if (!VTFPS2AverageTime(g_proInfo, ds)) {
				t02 = 0.0;
				t12 = 0.0;
			}
			else {
				t02 = (farLineIndex - headIndex) * ds;
				t12 = (farLinePreIndex - headIndex) * ds;
			}
			if (t02 > 0)
				g_proInfo.v02 = (L * 1000.0 * 1000.0 / t02)* 3.6;

			if (t12 > 0)
				g_proInfo.v12 = (L * 1000.0 * 1000.0 / t12) * 3.6;
			//视频水印时间
			ds = 0.0;
			if (!VTWT2AverageTime(g_proInfo, ds, fps)) {
				t03 = 0.0;
				t13 = 0.0;
			}
			else {
				t03 = (farLineIndex - headIndex) * ds;
				t13 = (farLinePreIndex - headIndex) * ds;
			}

			if (t03 > 0)
				g_proInfo.v03 = (L * 1000.0 * 1000.0 / t03)* 3.6;

			if (t13 > 0)
				g_proInfo.v13 = (L * 1000.0 * 1000.0 / t13) * 3.6;

			float v0 = 0.0, v1 = 0.0;
			if (g_proInfo.timeMode == TIME_MODE_PTS) {
				v0 = g_proInfo.v01;
				v1 = g_proInfo.v11;
			}
			else if (g_proInfo.timeMode == TIME_MODE_FPS) {
				v0 = g_proInfo.v02;
				v1 = g_proInfo.v12;
			}
			else if (g_proInfo.timeMode == TIME_MODE_WATER_MARK) {
				v0 = g_proInfo.v03;
				v1 = g_proInfo.v13;
			}

			QString strTemp = QString("%1-%2").arg(v0).arg(v1);
			text = QString("区间速度: %1 km/h").arg(strTemp);
			if (g_proInfo.curFrmaeIndex == farLineIndex) {
				g_logBrowser->append(QString("实际线段间距 %1").arg(L));
				g_logBrowser->append(text);
			}
		}
		calcPointP0();
		if (g_proInfo.curFrmaeIndex == farLineIndex)
				emit sigNotifyVelocity(text);
	}
}

void CTwoPoints::initForecast() {
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
	points.push_back(g_proInfo.imageLines[1][0]);
	m_keyTrack.addKeyPoint(firstFrame, points);

	m_initForecast = true;
}

void CTwoPoints::doForecast(int index)
{
	cv::Mat forecastFrame;

	if (!g_checkKeyTrace)
		return;

	if (!fixdFirstFrame())
		return;

	if (!m_initForecast)
		return;

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