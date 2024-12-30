#include "MovingCam2DTargetVehicle.h"
#include "Global.h"
#include <QDebug>


CMovingCam2DTargetVehicle::CMovingCam2DTargetVehicle(VTCalibration mode, QObject *parent)
	: CPointsManage(mode, parent)
{
}

CMovingCam2DTargetVehicle::~CMovingCam2DTargetVehicle()
{
}

bool CMovingCam2DTargetVehicle::fixdFirstFrame()
{
	return true;
}

void CMovingCam2DTargetVehicle::clearImagePoints(int index)
{

}

bool CMovingCam2DTargetVehicle::imagePointsReady(int index)
{
	if (INVALID_VALUE_INT != g_proInfo.frameInfos[index].imagePoints[0].x() && INVALID_VALUE_INT != g_proInfo.frameInfos[index].imagePoints[0].y()
		&& INVALID_VALUE_INT != g_proInfo.frameInfos[index].imagePoints[1].x() && INVALID_VALUE_INT != g_proInfo.frameInfos[index].imagePoints[1].y()
		&& INVALID_VALUE_INT != g_proInfo.frameInfos[index].imagePoints[2].x() && INVALID_VALUE_INT != g_proInfo.frameInfos[index].imagePoints[2].y()
		&& INVALID_VALUE_INT != g_proInfo.frameInfos[index].imagePoints[3].x() && INVALID_VALUE_INT != g_proInfo.frameInfos[index].imagePoints[3].y()
		&& INVALID_VALUE_INT != g_proInfo.frameInfos[index].imagePoints[4].x() && INVALID_VALUE_INT != g_proInfo.frameInfos[index].imagePoints[4].y()
		)
	{
		return true;
	}
	return false;
}

bool CMovingCam2DTargetVehicle::actualPointsReady()
{
	if (INVALID_VALUE_DOUBLE != g_proInfo.actualPoints[1].x() && INVALID_VALUE_DOUBLE != g_proInfo.actualPoints[1].y()
		&& INVALID_VALUE_DOUBLE != g_proInfo.actualPoints[2].x() && INVALID_VALUE_DOUBLE != g_proInfo.actualPoints[2].y()
		&& INVALID_VALUE_DOUBLE != g_proInfo.actualPoints[3].x() && INVALID_VALUE_DOUBLE != g_proInfo.actualPoints[3].y()
		&& INVALID_VALUE_DOUBLE != g_proInfo.actualPoints[4].x() && INVALID_VALUE_DOUBLE != g_proInfo.actualPoints[4].y()
		)
		return true;
	return false;
}

void CMovingCam2DTargetVehicle::addPoints(int index, QPoint point)
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

	if (INVALID_VALUE_INT == g_proInfo.frameInfos[index].imagePoints[1].x() && INVALID_VALUE_INT == g_proInfo.frameInfos[index].imagePoints[1].y()) {
		g_proInfo.frameInfos[index].imagePoints[1].setX(point.x());
		g_proInfo.frameInfos[index].imagePoints[1].setY(point.y());
		changeFlag = true;
		auto log = QString("帧[%1] 标定 P1(%2,%3)").arg(index).arg(point.x()).arg(point.y());
		g_proInfo.undoStack.push(new AddPointCommand("p1", &g_proInfo.frameInfos[index], this, point, g_proInfo.curFrmaeIndex));
		g_logBrowser->append(log);
	}
	else if (INVALID_VALUE_INT == g_proInfo.frameInfos[index].imagePoints[2].x() && INVALID_VALUE_INT == g_proInfo.frameInfos[index].imagePoints[2].y()) {
		g_proInfo.frameInfos[index].imagePoints[2].setX(point.x());
		g_proInfo.frameInfos[index].imagePoints[2].setY(point.y());
		changeFlag = true;
		auto log = QString("帧[%1] 标定 P2(%2,%3)").arg(index).arg(point.x()).arg(point.y());
		g_proInfo.undoStack.push(new AddPointCommand("p2", &g_proInfo.frameInfos[index], this, point, g_proInfo.curFrmaeIndex));
		g_logBrowser->append(log);
	}
	else if (INVALID_VALUE_INT == g_proInfo.frameInfos[index].imagePoints[3].x() && INVALID_VALUE_INT == g_proInfo.frameInfos[index].imagePoints[3].y()) {
		g_proInfo.frameInfos[index].imagePoints[3].setX(point.x());
		g_proInfo.frameInfos[index].imagePoints[3].setY(point.y());
		changeFlag = true;
		auto log = QString("帧[%1] 标定 P3(%2,%3)").arg(index).arg(point.x()).arg(point.y());
		g_proInfo.undoStack.push(new AddPointCommand("p3", &g_proInfo.frameInfos[index], this, point, g_proInfo.curFrmaeIndex));
		g_logBrowser->append(log);
	}
	else if (INVALID_VALUE_INT == g_proInfo.frameInfos[index].imagePoints[4].x() && INVALID_VALUE_INT == g_proInfo.frameInfos[index].imagePoints[4].y()) {
		g_proInfo.frameInfos[index].imagePoints[4].setX(point.x());
		g_proInfo.frameInfos[index].imagePoints[4].setY(point.y());
		changeFlag = true;
		auto log = QString("帧[%1] 标定 P4(%2,%3)").arg(index).arg(point.x()).arg(point.y());
		g_proInfo.undoStack.push(new AddPointCommand("p4", &g_proInfo.frameInfos[index], this, point, g_proInfo.curFrmaeIndex));
		g_logBrowser->append(log);
	}
	else if (INVALID_VALUE_INT == g_proInfo.frameInfos[index].imagePoints[0].x() && INVALID_VALUE_INT == g_proInfo.frameInfos[index].imagePoints[0].y()) {
		g_proInfo.frameInfos[index].imagePoints[0].setX(point.x());
		g_proInfo.frameInfos[index].imagePoints[0].setY(point.y());
		changeFlag = true;
		auto log = QString("帧[%1] 标定 P0(%2,%3)").arg(index).arg(point.x()).arg(point.y());
		g_proInfo.undoStack.push(new AddPointCommand("p0", &g_proInfo.frameInfos[index], this, point, g_proInfo.curFrmaeIndex));
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
	if (changeFlag)
		emit sigPointsChange();

	clearCalc(g_proInfo);

	calcPointP0();
}

void CMovingCam2DTargetVehicle::calcPointP0()
{
	if (!actualPointsReady())
		return;

	for (int n = 0; n < g_proInfo.frameInfos.size(); ++n)
	{
		if (!imagePointsReady(n))
			continue;

		vector<cv::Point2f> corners(4);
		corners[0] = cv::Point2f(g_proInfo.frameInfos[n].imagePoints[1].x(), g_proInfo.frameInfos[n].imagePoints[1].y());
		corners[1] = cv::Point2f(g_proInfo.frameInfos[n].imagePoints[2].x(), g_proInfo.frameInfos[n].imagePoints[2].y());
		corners[2] = cv::Point2f(g_proInfo.frameInfos[n].imagePoints[3].x(), g_proInfo.frameInfos[n].imagePoints[3].y());
		corners[3] = cv::Point2f(g_proInfo.frameInfos[n].imagePoints[4].x(), g_proInfo.frameInfos[n].imagePoints[4].y());

		vector<cv::Point2f> corners_trans(4);
		corners_trans[0] = cv::Point2f(g_proInfo.actualPoints[1].x(), g_proInfo.actualPoints[1].y());
		corners_trans[1] = cv::Point2f(g_proInfo.actualPoints[2].x(), g_proInfo.actualPoints[2].y());
		corners_trans[2] = cv::Point2f(g_proInfo.actualPoints[3].x(), g_proInfo.actualPoints[3].y());
		corners_trans[3] = cv::Point2f(g_proInfo.actualPoints[4].x(), g_proInfo.actualPoints[4].y());

		cv::Mat transform = getPerspectiveTransform(corners, corners_trans);
		vector<cv::Point2f> ponits, points_trans;
		ponits.push_back(cv::Point2f(g_proInfo.frameInfos[n].imagePoints[0].x(), g_proInfo.frameInfos[n].imagePoints[0].y()));
		perspectiveTransform(ponits, points_trans, transform);
		if (points_trans.size() > 0)
			g_proInfo.frameInfos[n].actualP0 = QPointF(points_trans[0].x, points_trans[0].y);
	}

	calcVelocity();
	notifyVelocity();
}

void CMovingCam2DTargetVehicle::calcVelocity()
{
	if (!actualPointsReady())
		return;

	for (int n = 1; n < g_proInfo.frameInfos.size(); ++n)
	{
		if (!imagePointsReady(n))
			continue;
		VTFrameInfo preFrameInfo;
		if (getPreFrameInfo(g_proInfo, n, preFrameInfo))
			continue;

		qint64 prePts = preFrameInfo.pts;
		qint64 curPts = g_proInfo.frameInfos[n].pts;

		QPointF actualPreP0 = preFrameInfo.actualP0;
		QPointF actualCurP0 = g_proInfo.frameInfos[n].actualP0;

		//计算速度
		float x1 = actualPreP0.x();
		float y1 = actualPreP0.y();

		float x2 = actualCurP0.x();
		float y2 = actualCurP0.y();

		float s = sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
		float t1 = 0.0;
		float t2 = 0.0;
		float t3 = 0.0;
		//视频帧时间戳PTS
		t1 = curPts - prePts;
		if (t1)
			g_proInfo.frameInfos[n].v1 = (s * 1000 * 1000.0 / t1) * 3.6;
		//视频平均帧率FPS
		float ds = 0.0;
		float fps = 0.0;
		if (!VTFPS2AverageTime(g_proInfo, ds))
			t2 = 0.0;
		else
			t2 = (n - preFrameInfo.index) * ds;
		if (t2)
			g_proInfo.frameInfos[n].v2 = (s * 1000 * 1000.0 / t2) * 3.6;
		//视频水印时间
		ds = 0.0;
		if (!VTWT2AverageTime(g_proInfo, ds, fps))
			t3 = 0.0;
		else
			t3 = (n - preFrameInfo.index) * ds;
		if (t3)
			g_proInfo.frameInfos[n].v3 = (s * 1000 * 1000.0 / t3) * 3.6;
	}
}

void CMovingCam2DTargetVehicle::notifyVelocity()
{
	emit sigNotifyVelocity();
}

void CMovingCam2DTargetVehicle::initForecast() {
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
	points.push_back(g_proInfo.frameInfos[headIndex].imagePoints[4]);
	m_keyTrack.addKeyPoint(firstFrame, points);

	m_initForecast = true;
}

void CMovingCam2DTargetVehicle::doForecast(int index)
{
	cv::Mat firstFrame;
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
	if (points.size() < 5)
		return;
	g_proInfo.frameInfos[index].imagePoints[0] = points[0];
	g_proInfo.frameInfos[index].imagePoints[1] = points[1];
	g_proInfo.frameInfos[index].imagePoints[2] = points[2];
	g_proInfo.frameInfos[index].imagePoints[3] = points[3];
	g_proInfo.frameInfos[index].imagePoints[3] = points[4];
}