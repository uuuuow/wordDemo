#pragma once
#include "opencv2/opencv.hpp"
#include <iostream>
#include <QImage>

/*
*点跟踪
*/
class CKeyTrack
{
public:
	CKeyTrack();
	~CKeyTrack();
	//初始化
	int initKeyTrack(int flag);
	//添加点
	int addKeyPoint(cv::Mat &image, QVector<QPoint> &points);
	//获取跟踪点
	int getCurrentPoint(cv::Mat &image, QVector<QPoint> &pointss);
	void release();
public:
	void *m_trackHandle = nullptr;
	bool m_addPoint = false;
};

