#include "FixedCam1D.h"
#include "MethodApi.h"
#include "Global.h"
#include <QDebug>
#include "Command.h"

CFixedCam1D::CFixedCam1D(VTCalibration mode, QObject *parent)
	: CPointsManage(mode, parent)
{
}

CFixedCam1D::~CFixedCam1D()
{
}

bool CFixedCam1D::fixdFirstFrame()
{
	int headIndex = g_proInfo.headIndex;
	if (headIndex < 0 || g_proInfo.frameInfos.size() == 0)
		return false;
	if (availableImagePoint(g_proInfo.frameInfos[headIndex].imagePoints[0]) &&
		availableImagePoint(g_proInfo.frameInfos[headIndex].imagePoints[1]) &&
		availableImagePoint(g_proInfo.frameInfos[headIndex].imagePoints[2]) &&
		availableImagePoint(g_proInfo.frameInfos[headIndex].imagePoints[3]))
		return true;
	return false;
}

bool CFixedCam1D::imagePointsReady(int index)
{
	if (INVALID_VALUE_INT != g_proInfo.frameInfos[index].imagePoints[0].x() && INVALID_VALUE_INT != g_proInfo.frameInfos[index].imagePoints[0].y()
		&& INVALID_VALUE_INT != g_proInfo.frameInfos[index].imagePoints[1].x() && INVALID_VALUE_INT != g_proInfo.frameInfos[index].imagePoints[1].y()
		&& INVALID_VALUE_INT != g_proInfo.frameInfos[index].imagePoints[2].x() && INVALID_VALUE_INT != g_proInfo.frameInfos[index].imagePoints[2].y()
		&& INVALID_VALUE_INT != g_proInfo.frameInfos[index].imagePoints[3].x() && INVALID_VALUE_INT != g_proInfo.frameInfos[index].imagePoints[3].y())
		return true;
	return false;
}

bool CFixedCam1D::actualPointsReady()
{
	if (INVALID_VALUE_DOUBLE != g_proInfo.actualPoints[1].x() && INVALID_VALUE_DOUBLE != g_proInfo.actualPoints[1].y()
		&& INVALID_VALUE_DOUBLE != g_proInfo.actualPoints[2].x() && INVALID_VALUE_DOUBLE != g_proInfo.actualPoints[2].y()
		&& INVALID_VALUE_DOUBLE != g_proInfo.actualPoints[3].x() && INVALID_VALUE_DOUBLE != g_proInfo.actualPoints[3].y()
		)
		return true;
	return false;
}

void CFixedCam1D::clearImagePoints(int index) {

}

void CFixedCam1D::addPoints(int index, QPoint point)
{
	auto changeFlag = false;
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
	}

	if (INVALID_VALUE_INT == g_proInfo.frameInfos[index].imagePoints[1].x() 
		&& INVALID_VALUE_INT == g_proInfo.frameInfos[index].imagePoints[1].y()) {
		g_proInfo.frameInfos[index].imagePoints[1].setX(point.x());
		g_proInfo.frameInfos[index].imagePoints[1].setY(point.y());
		changeFlag = true;
		auto log = QString("帧[%1] 标定 P1(%2,%3)").arg(index).arg(point.x()).arg(point.y());
		g_proInfo.undoStack.push(new AddPointCommand("p1", &g_proInfo.frameInfos[index], this, point, g_proInfo.curFrmaeIndex));
		g_logBrowser->append(log);
	}
	else if (INVALID_VALUE_INT == g_proInfo.frameInfos[index].imagePoints[2].x() 
		&& INVALID_VALUE_INT == g_proInfo.frameInfos[index].imagePoints[2].y()) {
		g_proInfo.frameInfos[index].imagePoints[2].setX(point.x());
		g_proInfo.frameInfos[index].imagePoints[2].setY(point.y());
		changeFlag = true;
		auto log = QString("帧[%1] 标定 P2(%2,%3)").arg(index).arg(point.x()).arg(point.y());
		g_proInfo.undoStack.push(new AddPointCommand("p2", &g_proInfo.frameInfos[index], this, point, g_proInfo.curFrmaeIndex));
		g_logBrowser->append(log);
	}
	else if (INVALID_VALUE_INT == g_proInfo.frameInfos[index].imagePoints[3].x() 
		&& INVALID_VALUE_INT == g_proInfo.frameInfos[index].imagePoints[3].y()) {
		g_proInfo.frameInfos[index].imagePoints[3].setX(point.x());
		g_proInfo.frameInfos[index].imagePoints[3].setY(point.y());
		changeFlag = true;
		auto log = QString("帧[%1] 标定 P3(%2,%3)").arg(index).arg(point.x()).arg(point.y());
		g_proInfo.undoStack.push(new AddPointCommand("p3", &g_proInfo.frameInfos[index], this, point, g_proInfo.curFrmaeIndex));
		g_logBrowser->append(log);
	}
	else if (INVALID_VALUE_INT == g_proInfo.frameInfos[index].imagePoints[0].x() 
		&& INVALID_VALUE_INT == g_proInfo.frameInfos[index].imagePoints[0].y()) {
		g_proInfo.frameInfos[index].imagePoints[0].setX(point.x());
		g_proInfo.frameInfos[index].imagePoints[0].setY(point.y());
		changeFlag = true;
		auto log = QString("帧[%1] 标定 P0(%2,%3)").arg(index).arg(point.x()).arg(point.y());
		g_proInfo.undoStack.push(new AddPointCommand("p0", &g_proInfo.frameInfos[index], this, point, g_proInfo.curFrmaeIndex));
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
	
	if(!imagePointsReady(index) &&
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

void CFixedCam1D::calcPointP0()
{
	if (!actualPointsReady())
		return;
	for (auto n = 1; n < g_proInfo.frameInfos.size(); ++n) {
		if (!imagePointsReady(n-1))
			continue;
		if (INVALID_VALUE_INT == g_proInfo.frameInfos[n].imagePoints[0].x())
			continue;

		//计算当前点在前一帧的位置
		double imageP1X = g_proInfo.frameInfos[n-1].imagePoints[1].x();
		double imageP2X = g_proInfo.frameInfos[n-1].imagePoints[2].x();
		double imageP3X = g_proInfo.frameInfos[n-1].imagePoints[3].x();

		double actualP1X = g_proInfo.actualPoints[1].x();
		double actualP2X = g_proInfo.actualPoints[2].x();
		double actualP3X = g_proInfo.actualPoints[3].x();

		float preDLT[8] = { 0 };
		double f02 = imageP1X * actualP1X;
		double f12 = imageP2X * actualP2X;
		double f22 = imageP3X * actualP3X;
		double szfCoefficient[9] = {
			actualP1X, 1, f02,
			actualP2X, 1, f12,
			actualP3X, 1, f22 };

		float nValue0 = (-1) * (imageP1X);
		float nValue1 = (-1) * (imageP2X);
		float nValue2 = (-1) * (imageP3X);

		float szfValue[3] = { nValue0, nValue1, nValue2 };

		MatrixCalculation(szfCoefficient, szfValue, preDLT, 3, 3);
		double preImageP0X = g_proInfo.frameInfos[n - 1].imagePoints[0].x();
		float preP0x = -1 * (preDLT[1] + preImageP0X) / (preDLT[0] + preDLT[2] * preImageP0X);

		double curImageP0X = g_proInfo.frameInfos[n].imagePoints[0].x();
		float curP0x = -1 * (preDLT[1] + curImageP0X) / (preDLT[0] + preDLT[2] * curImageP0X);

		float p0x = fabs(curP0x - preP0x);
		g_proInfo.frameInfos[n].actualP0 = QPointF(p0x, 0);
	}
	calcVelocity();
	notifyVelocity();
}

void CFixedCam1D::calcVelocity()
{
	if (!actualPointsReady())
		return;

	for (auto n = 2; n < g_proInfo.frameInfos.size(); ++n) {
		if (!imagePointsReady(n))
			continue;
		if (!imagePointsReady(n-1))
			continue;
		VTFrameInfo preFrameInfo;
		if (getPreFrameInfo(g_proInfo, n, preFrameInfo))
			continue;

		qint64 prePts = preFrameInfo.pts;
		qint64 curPts = g_proInfo.frameInfos[n].pts;
		
		QPointF actualCurP0 = g_proInfo.frameInfos[n].actualP0;

		//计算速度

		float x2 = actualCurP0.x();
		float s = sqrt(x2*x2);
		float t1 = 0.0;
		float t2 = 0.0;
		float t3 = 0.0;
		float ds = 0.0;
		float fps = 0.0;
		//视频帧时间戳PTS
		t1 = curPts - prePts;
		//视频平均帧率FPS
		if (!VTFPS2AverageTime(g_proInfo, ds))
			t2 = 0.0;
		else
			t2 = (n - preFrameInfo.index) * ds;
		//视频水印时间
		ds = 0.0;
		if (!VTWT2AverageTime(g_proInfo, ds, fps))
			t3 = 0.0;
		else
			t3 = (n - preFrameInfo.index) * ds;
		if(t1 > 0)
			g_proInfo.frameInfos[n].v1 = (s * 1000.0 * 1000.0 / t1) * 3.6;
		if (t2 > 0)
			g_proInfo.frameInfos[n].v1 = (s * 1000.0 * 1000.0 / t2) * 3.6;
		if (t3 > 0)
			g_proInfo.frameInfos[n].v1 = (s * 1000.0 * 1000.0 / t3) * 3.6;
	}
}

void CFixedCam1D::notifyVelocity()
{
	emit sigNotifyVelocity();
}

void CFixedCam1D::initForecast() {
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
	points.push_back(g_proInfo.frameInfos[headIndex].imagePoints[1]);
	points.push_back(g_proInfo.frameInfos[headIndex].imagePoints[2]);
	points.push_back(g_proInfo.frameInfos[headIndex].imagePoints[3]);
	m_keyTrack.addKeyPoint(firstFrame, points);

	m_initForecast = true;
}

void CFixedCam1D::doForecast(int index)
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
	if(forecastFrame.data == nullptr)
		return;

	QVector<QPoint> points;
	m_keyTrack.getCurrentPoint(forecastFrame, points);
	if (points.size() < 4)
		return;
	g_proInfo.frameInfos[index].imagePoints[0] = points[0];
	g_proInfo.frameInfos[index].imagePoints[1] = points[1];
	g_proInfo.frameInfos[index].imagePoints[2] = points[2];
	g_proInfo.frameInfos[index].imagePoints[3] = points[3];
}