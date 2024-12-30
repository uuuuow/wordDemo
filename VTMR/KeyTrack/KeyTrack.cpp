#include "KeyTrack.h"
#include "EvKeyptTrackAPI.h"
#include "Utils.h"
#include <QDebug>

CKeyTrack::CKeyTrack()
{
}


CKeyTrack::~CKeyTrack()
{
}

int CKeyTrack::initKeyTrack(int flag)
{
	EV_SCENE type;
	if (0 == flag)
		type = STREET_CAMERA;
	else
		type = CAR_CAMERA;
	initEvKeyTrackAPI(&m_trackHandle, type);
	if (!m_trackHandle)
		return -1;
	return 0;
}

int CKeyTrack::addKeyPoint(cv::Mat &image, QVector<QPoint> &points)
{
	if (!m_trackHandle)
		return -1;
	if (m_addPoint)
		return 0;
	//Mat dst = VTQImage2Mat(image);
	for(int n = 0; n < points.size(); n++)
		addPointEvKeyTrackAPI(m_trackHandle, (char*)image.data, image.cols, image.rows, image.step[0], EvPoint(points[n].x(), points[n].y()));
	m_addPoint = true;
	return 0;
}

int CKeyTrack::getCurrentPoint(cv::Mat &image,QVector<QPoint> &points)
{
	vector<vector<EvPoint>> pointsRes;
	if (!m_trackHandle)
		return -1;

	runEvKeyTrackAPI(m_trackHandle, (char*)image.data, image.cols, image.rows, image.step[0]);
	
	if (!getTrackingRstAPI(m_trackHandle, pointsRes))
		return -2;

	for (auto i = 0; i < pointsRes.size(); i++) {
		auto j = pointsRes[i].size() - 1;
		QPoint point;
		point.setX(pointsRes[i][j].x);
		point.setY(pointsRes[i][j].y);
		points.push_back(point);
	}

	return 0;
}

void CKeyTrack::release()
{
	releaseEvKeyTrackAPI(m_trackHandle);
}
