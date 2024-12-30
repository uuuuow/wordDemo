#pragma once

#include <QObject>
#include "Utils.h"
#include "KeyTrack.h"
class CKeyTrack;
/*
*标记点位管理
*/
class CPointsManage : public QObject, public CKeyTrack
{
	Q_OBJECT

public:
	CPointsManage(VTCalibration mode, QObject *parent);
	~CPointsManage();
	static void createPointsManage(QObject *obj, VTCalibration mode, CPointsManage **manage);
	// 获取帧数量
	static int getFrameCount(VTProjectInfo &info);
	// 获取图像所有点位
	static void getImagePoints(VTProjectInfo &info, int index, QVector<QPoint>& points);
	// 获取帧PTS
	static qint64 getPTS(VTProjectInfo &info, int index);
	// 获取图片路径
	static QString getImagePath(VTProjectInfo &info, int index);
	// 获取编辑状态
	static int getFrameEditStutas(VTProjectInfo &info, int index);
	// 判断是否为同一个点
	static bool theSamePoint(QPoint pos, QPoint imagePoint);
	static int getPreFrameInfo(VTProjectInfo &info, int index, VTFrameInfo &preFrameInfo);
	static int getFrameV(VTProjectInfo &info, int index, float &v);
	// 获取所有速度点
	static void getVPoints(VTProjectInfo &info, QPolygonF &points, QStringList &strHMSList);
	// 获取所有P0点
	static int getMPoints(VTProjectInfo &info, QVector<QPointF> &points, QVector<int64_t> &ptss);
	//是否为有效点位
	static bool availableImagePoint(QPoint &point);
	static void clearCalc(VTProjectInfo &info);
	//点是否在直线上
	static double pointOnLine(QPoint point, QLine line, bool about = true);
	// 处理当前帧点位
	void clearImagePoints(VTProjectInfo &info, int index);
	void delPoints(VTProjectInfo &info, int index, QPoint point);
	VTCalibration getMode() { return m_mode; };

public:
	//是否固定首帧
	virtual bool fixdFirstFrame() { return false; };
	//是否准备好标定点
	virtual bool imagePointsReady(int index) { return false; };
	//是否准备好实际坐标
	virtual bool actualPointsReady() { return false; };
	//添加图像坐标点
	virtual void addPoints(int index, QPoint point) {};
	//添加直线
	virtual void addLine(int index, QPoint center, QPoint begin, QPoint end) { emit sigPointsChange(); };
	virtual void addEllipse() { emit sigPointsChange(); };
	virtual void addRectangle() { emit sigPointsChange(); };
	//计算待测点P0的实际坐标
	virtual void calcPointP0() {};
	//计算行驶速度
	virtual void calcVelocity() {};
	//通知速度计算结果
	virtual void notifyVelocity() {};
	//点预测初始化
	virtual void initForecast() {};
	//点预测
	virtual void doForecast(int index) {};
	virtual bool needNotify() { return true; };

signals:
	void sigPointsChange();
	void sigNotifyStatus(int status);
	void sigNotifyVelocity(QString text);
	void sigNotifyVelocity();

protected:
	//点位跟踪
	CKeyTrack m_keyTrack;
	bool m_initForecast = false;
	VTCalibration m_mode = CALIBRATION_UNKNOWN;
};
