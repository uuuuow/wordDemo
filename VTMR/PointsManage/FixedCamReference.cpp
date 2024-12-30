#include "FixedCamReference.h"
#include "TakenWidget.h"
#include "InputWidget.h"
#include "MethodApi.h"
#include "Global.h"
#include <QDebug>
#include "Command.h"

CFixedCamReference::CFixedCamReference(VTCalibration mode, QObject *parent)
	: CPointsManage(mode, parent)
{
}

CFixedCamReference::~CFixedCamReference()
{
}

bool CFixedCamReference::fixdFirstFrame()
{
	int headIndex = g_proInfo.headIndex;
	if (headIndex < 0 || g_proInfo.frameInfos.size() == 0)
		return false;
	if (availableImagePoint(g_proInfo.imageLines[0][0]) &&
		availableImagePoint(g_proInfo.imageLines[0][1]) &&
		availableImagePoint(g_proInfo.imageLines[0][2]) &&
		availableImagePoint(g_proInfo.imageLines[1][0]) &&
		availableImagePoint(g_proInfo.imageLines[1][1]) &&
		availableImagePoint(g_proInfo.imageLines[1][2]) &&
		availableImagePoint(g_proInfo.frameInfos[headIndex].imagePoints[0]))
		return true;
	return false;
}

bool CFixedCamReference::imagePointsReady(int index)
{
	if (availableImagePoint(g_proInfo.frameInfos[index].imagePoints[0]))
		return true;
	return false;
}

bool CFixedCamReference::actualPointsReady()
{
	if (INVALID_VALUE_DOUBLE != g_proInfo.actualPoints[1].x())
		return true;
	return false;
}

void CFixedCamReference::addPoints(int index, QPoint point)
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

	//配置首帧
	if (!fixdFirstFrame())
		g_proInfo.headIndex = index;

	if (!availableImagePoint(g_proInfo.frameInfos[index].imagePoints[0])) {
		g_proInfo.frameInfos[index].imagePoints[0].setX(point.x());
		g_proInfo.frameInfos[index].imagePoints[0].setY(point.y());
		g_proInfo.undoStack.push(new AddPointCommand("p0", &g_proInfo.frameInfos[index], this, point, g_proInfo.curFrmaeIndex));
		changeFlag = true;
		auto log = QString("帧[%1] 标定 P0(%2,%3)").arg(index).arg(point.x()).arg(point.y());
		g_logBrowser->append(log);
	}

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

EndHandle:
	//有点改变
	if (changeFlag) {
		emit sigPointsChange();
		m_nearLineIndex = -1;
		m_farLineIndex = -1;
		clearCalc(g_proInfo);
		notifyVelocity();
	}

}

void CFixedCamReference::addLine(int index, QPoint center, QPoint begin, QPoint end)
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
	else if (!availableImagePoint(g_proInfo.imageLines[1][0]) ||
		!availableImagePoint(g_proInfo.imageLines[1][1]) ||
		!availableImagePoint(g_proInfo.imageLines[1][2])) {
		g_proInfo.imageLines[1][0] = center;
		g_proInfo.imageLines[1][1] = begin;
		g_proInfo.imageLines[1][2] = end;
		g_proInfo.undoStack.push(new AddLineCommand("line2", &g_proInfo,
			this, center, begin, end, g_proInfo.curFrmaeIndex));
		auto log = QString("帧[%1] 标定 Line2[(%2,%3) (%4,%5)]").arg(index).arg(begin.x()).arg(begin.y()).arg(end.y()).arg(end.y());
		g_logBrowser->append(log);
		changeFlag = true;
	}

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

void CFixedCamReference::calcPointP0()
{
	if (!actualPointsReady())
		return;
	if (!fixdFirstFrame())
		return;
	if (m_nearLineIndex < 0 || m_farLineIndex < 0)
		return;

	QPoint nearP0 = g_proInfo.frameInfos[m_nearLineIndex].imagePoints[0];
	QPoint farP0 = g_proInfo.frameInfos[m_farLineIndex].imagePoints[0];
	float L = abs(g_proInfo.actualPoints[1].x() - g_proInfo.actualPoints[0].x());
	float actualP0X = g_proInfo.actualPoints[0].x();
	float imageL = sqrt(nearP0.x() - farP0.x()) * (nearP0.x() - farP0.x()) + (nearP0.y() - farP0.y()) * (nearP0.y() - farP0.y());
	for (int n = m_nearLineIndex; n <= m_farLineIndex; ++n) {
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
		float ds = 0.0;
		float fps = 0.0;
		//视频帧时间戳PTS
		t1 = g_proInfo.frameInfos[n].pts - g_proInfo.frameInfos[m_nearLineIndex].pts;
		if (t1 > 0)
			g_proInfo.frameInfos[n].v1 = (curAx * 1000.0 * 1000.0 / t1) * 3.6;
		
		//视频平均帧率FPS
		ds = 0.0;
		if (!VTFPS2AverageTime(g_proInfo, ds))
			t2 = 0.0;
		else
			t2 = (n - m_nearLineIndex) * ds;
		if (t2 > 0)
			g_proInfo.frameInfos[n].v2 = (curAx * 1000.0 * 1000.0 / t2) * 3.6;
		//视频水印时间
		ds = 0.0;
		if (!VTWT2AverageTime(g_proInfo, ds, fps))
			t3 = 0.0;
		else
			t3 = (n - m_nearLineIndex) * ds;
		if (t3 > 0)
			g_proInfo.frameInfos[n].v3 = (curAx * 1000.0 * 1000.0 / t3) * 3.6;
	}
}

void CFixedCamReference::notifyVelocity()
{
	if (!actualPointsReady())
		return;
	if (!fixdFirstFrame())
		return;

	float L = abs(g_proInfo.actualPoints[0].x() - g_proInfo.actualPoints[1].x());
	int headIndex = g_proInfo.headIndex;
	int index = headIndex;
	QPoint firstFrameP0 = g_proInfo.frameInfos[headIndex].imagePoints[0];
	QLine line1, line2;
	QLine nearLine,farLine;
	line1.setP1(g_proInfo.imageLines[0][1]);
	line1.setP2(g_proInfo.imageLines[0][2]);
	line2.setP1(g_proInfo.imageLines[1][1]);
	line2.setP2(g_proInfo.imageLines[1][2]);
	double onLine1 = pointOnLine(firstFrameP0, line1, false);
	double onLine2 = pointOnLine(firstFrameP0, line2, false);
	int nearLineIndex = -1;
	int nearLinePreIndex = -1;
	int farLineIndex = -1;
	int farLinePreIndex = -1;
	double near = abs(onLine1) < abs(onLine2) ? onLine1 : onLine2; //判断最近的那条线
	double far = abs(onLine1) > abs(onLine2) ? onLine1 : onLine2; //判断最远的那条线
	if (near == onLine1) {
		nearLine = line1;
		farLine = line2;
	}
	else {
		nearLine = line2;
		farLine = line1;
	}
	int preValue1 = 0;
	int preValue2 = 0;
	for (; index < g_proInfo.frameInfos.size(); ++index) {
		double value = 0;
		if (!imagePointsReady(index))
			continue;
		QPoint P0 = g_proInfo.frameInfos[index].imagePoints[0];
		if (nearLineIndex == -1) {
			value = pointOnLine(P0, nearLine);
			if (value == 0) //在线上
				nearLineIndex = index;
			else if ((preValue1 < 0 && value > 0) || (preValue1 > 0 && value < 0)) {//前后帧分布在线的两侧
				nearLinePreIndex = index - 1;
				nearLineIndex = index;
			}
			preValue1 = value;
		}
		else {
			value = pointOnLine(P0, farLine);
			if(value == 0) //在线上
				farLineIndex = index;
			else if ((preValue2 < 0 && value > 0) || (preValue2 > 0 && value < 0)) {//前后帧分布在线的两侧
				farLinePreIndex = index - 1;
				farLineIndex = index;
			}
			preValue2 = value;
		}

		if (nearLineIndex != -1 && farLineIndex != -1)
			break;
	}
	
	if (nearLineIndex >= 0 && farLineIndex >= 0) {
		float t01 = 0.0;
		float t02 = 0.0;
		float t03 = 0.0;
		float t11 = 0.0;
		float t12 = 0.0;
		float t13 = 0.0;
		float ds = 0.0;
		float fps = 0.0;
		QString text;

		if (nearLinePreIndex < 0 && farLinePreIndex < 0) {
			//视频帧时间戳PTS
			t01 = g_proInfo.frameInfos[farLineIndex].pts - g_proInfo.frameInfos[nearLineIndex].pts;
			if (t01 > 0)
				g_proInfo.v01 = (L * 1000.0 * 1000.0 / t01) * 3.6;
			//视频平均帧率FPS
			ds = 0.0;
			if (!VTFPS2AverageTime(g_proInfo, ds))
				t02 = 0.0;
			else
				t02 = (farLineIndex - m_nearLineIndex) * ds;
			if (t02 > 0)
				g_proInfo.v02 = (L * 1000.0 * 1000.0 / t02) * 3.6;
			//视频水印时间
			float ds = 0.0;
			if (!VTWT2AverageTime(g_proInfo, ds, fps))
				t03 = 0.0;
			else
				t03 = (farLineIndex - m_nearLineIndex) * ds;
			if (t03 > 0)
				g_proInfo.v03 = (L * 1000.0 * 1000.0 / t03) * 3.6;

			float v = 0.0;
			if (g_proInfo.timeMode == TIME_MODE_PTS)
				v = g_proInfo.v01;
			else if (g_proInfo.timeMode == TIME_MODE_FPS)
				v = g_proInfo.v02;
			else if (g_proInfo.timeMode == TIME_MODE_WATER_MARK)
				v = g_proInfo.v03;

			text = QString("速度: %1 km/h").arg(v);
			g_logBrowser->append(QString("实际线段间距 %1").arg(L));
			g_logBrowser->append(text);
			m_nearLineIndex = nearLineIndex;
			m_farLineIndex = farLineIndex;
		}
		else if (nearLinePreIndex >= 0 && farLinePreIndex < 0) {
			t01 = 0.0;
			t02 = 0.0;
			t03 = 0.0;
			t11 = 0.0;
			t12 = 0.0;
			t13 = 0.0;
			ds = 0.0;
			//视频帧时间戳PTS
			t01 = g_proInfo.frameInfos[farLineIndex].pts - g_proInfo.frameInfos[nearLinePreIndex].pts;
			t11 = g_proInfo.frameInfos[farLineIndex].pts - g_proInfo.frameInfos[nearLineIndex].pts;
			if (t01 > 0)
				g_proInfo.v01 = (L * 1000.0 * 1000.0 / t01) * 3.6;

			if (t11 > 0)
				g_proInfo.v11 = (L * 1000.0 * 1000.0 / t11) * 3.6;
			//视频平均帧率FPS
			ds = 0.0;
			if (!VTFPS2AverageTime(g_proInfo, ds))
				t02 = t12 = 0.0;
			else {
				t02 = (farLineIndex - nearLinePreIndex) * ds;
				t12 = (farLineIndex - m_nearLineIndex) * ds;
			}
			if (t02 > 0)
				g_proInfo.v02 = (L * 1000.0 * 1000.0 / t02) * 3.6;

			if (t12 > 0)
				g_proInfo.v12 = (L * 1000.0 * 1000.0 / t12) * 3.6;
			//视频水印时间
			ds = 0.0;
			if (!VTWT2AverageTime(g_proInfo, ds, fps))
				t03 = t13 = 0.0;
			else {
				t03 = (farLineIndex - nearLinePreIndex) * ds;
				t13 = (farLineIndex - m_nearLineIndex) * ds;
			}
			
			if (t03 > 0)
				g_proInfo.v03 = (L * 1000.0 * 1000.0 / t03) * 3.6;

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
			g_logBrowser->append(QString("实际线段间距 %1").arg(L));
			g_logBrowser->append(text);
			m_nearLineIndex = nearLinePreIndex;
			m_farLineIndex = farLineIndex;
		}
		else if (nearLinePreIndex < 0 && farLinePreIndex > 0) {
			t01 = 0.0;
			t02 = 0.0;
			t03 = 0.0;
			t11 = 0.0;
			t12 = 0.0;
			t13 = 0.0;
			ds = 0.0;
			//视频帧时间戳PTS
			t01 = g_proInfo.frameInfos[farLineIndex].pts - g_proInfo.frameInfos[nearLineIndex].pts;
			t11 = g_proInfo.frameInfos[farLinePreIndex].pts - g_proInfo.frameInfos[nearLineIndex].pts;
			if (t01 > 0)
				g_proInfo.v01 = (L * 1000.0 * 1000.0 / t01) * 3.6;
			if (t11 > 0)
				g_proInfo.v11 = (L * 1000.0 * 1000.0 / t11) * 3.6;
			//视频平均帧率FPS
			ds = 0.0;
			if (!VTFPS2AverageTime(g_proInfo, ds))
				t02 = t12 = 0.0;
			else {
				t02 = (farLineIndex - nearLinePreIndex) * ds;
				t12 = (farLineIndex - m_nearLineIndex) * ds;
			}
			if (t02 > 0)
				g_proInfo.v02 = (L * 1000.0 * 1000.0 / t02) * 3.6;
			if (t12 > 0)
				g_proInfo.v12 = (L * 1000.0 * 1000.0 / t12) * 3.6;
			//视频水印时间
			ds = 0.0;
			if (!VTWT2AverageTime(g_proInfo, ds, fps))
				t03 = t13 = 0.0;
			else {
				t03 = (farLineIndex - nearLineIndex) * ds;
				t13 = (farLinePreIndex - m_nearLineIndex) * ds;
			}

			if (t03 > 0)
				g_proInfo.v03 = (L * 1000.0 * 1000.0 / t03) * 3.6;

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
			g_logBrowser->append(QString("实际线段间距 %1").arg(L));
			g_logBrowser->append(text);
			m_nearLineIndex = nearLineIndex;
			m_farLineIndex = farLineIndex;
		}
		else if (nearLinePreIndex >= 0 && farLinePreIndex > 0) {
			//视频帧时间戳PTS
			t01 = g_proInfo.frameInfos[farLineIndex].pts - g_proInfo.frameInfos[nearLinePreIndex].pts;
			t11 = g_proInfo.frameInfos[farLinePreIndex].pts - g_proInfo.frameInfos[nearLineIndex].pts;
			if (t01 > 0)
				g_proInfo.v01 = (L * 1000.0 * 1000.0 / t01)* 3.6;

			if (t11 > 0)
				g_proInfo.v11 = (L * 1000.0 * 1000.0 / t11) * 3.6;
			//视频平均帧率FPS
			ds = 0.0;
			if (!VTFPS2AverageTime(g_proInfo, ds))
				t02 = t12 = 0.0;
			else {
				t02 = (farLineIndex - nearLinePreIndex) * ds;
				t12 = (farLinePreIndex - nearLineIndex) * ds;
			}
			if (t02 > 0)
				g_proInfo.v02 = (L * 1000.0 * 1000.0 / t02)* 3.6;

			if (t12 > 0)
				g_proInfo.v12 = (L * 1000.0 * 1000.0 / t12) * 3.6;
			//视频水印时间
			ds = 0.0;
			if (!VTWT2AverageTime(g_proInfo, ds, fps))
				t03 = t13 = 0.0;
			else {
				t03 = (farLineIndex - nearLinePreIndex) * ds;
				t13 = (farLinePreIndex - nearLineIndex) * ds;
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
			g_logBrowser->append(QString("实际线段间距 %1").arg(L));
			g_logBrowser->append(text);
			m_nearLineIndex = nearLinePreIndex;
			m_farLineIndex = farLineIndex;
		}
		calcPointP0();
		emit sigNotifyVelocity(text);
	}
}

void CFixedCamReference::initForecast() {
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

void CFixedCamReference::doForecast(int index)
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