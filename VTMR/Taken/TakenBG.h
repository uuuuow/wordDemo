#pragma once

#include <QWidget>
#include "ui_TakenBG.h"
#include "Utils.h"
#include <QVector>

QT_BEGIN_NAMESPACE
class QTextEdit;
QT_END_NAMESPACE


class CTakenWidget;
class CTakenControl;
class CDecodeVideo;
class CPointsManage;
class CShowTrace;
class CVTMR;
class CImageEnhance;
struct ZoomPair;

#include <QTimerEvent>
#include "OCRThread.h"
/*
*编辑背景窗口
*/
class CTakenBG : public QWidget, public IOCRUser
{
	Q_OBJECT

public:
	CTakenBG(CVTMR *vtmr, CImageEnhance *imageEnhance, CShowTrace * trace, QWidget *parent = Q_NULLPTR);
	~CTakenBG();
	//初始化标记点
	int initPointsManage(VTProjectInfo &info);
	//返回编辑窗口
	QWidget *takenAreaWidget();
	//返回标记点管理句柄
	CPointsManage *getPointsManage();
	//显示第一帧
	void preProcess();
	void closeAllPartZoom(VTProjectInfo &info);
	//水印识别
	void showOCR();
	void showWaterMark();
	bool actualPointsReady();
	static void drawResoutImage(QImage &image, VTProjectInfo &info, int index);
	void modeChanged();
	//重做
	void reDoClick();
protected:
	virtual void notifyOCRResult(std::shared_ptr<OCRMsg> msg);

public slots:
	//显示首帧
	void onHeadFrame();
	//显示上一帧
	void onPreviousFrame();
	//显示下一帧
	void onNextFrame();
	//画点动作切换
	void onActionPoint(bool);
	//橡皮差动作切换
	void onActionEraser(bool);
	//线动作切换
	void onActionLine(bool);
	//重做功能
	void onActionRedo();
	//矩形动作切换
	void onActionRectangle(bool);
	//椭圆动作切换
	void onActionEllipse(bool);
	//局部放大图片
	void onActionPartZoom(bool checked);
	// 撤销操作
	void onActionUndo();
	//标记点位变化
	void onPointsChange();
	//添加标记点
	void onAddPoint(QPoint point);
	//添加线段
	void onAddLine(QPoint center, QPoint begin, QPoint end);
	//添加椭圆
	void onAddEllipse();
	//添加矩形
	void onAddRectangl();
	//删除标记点
	void onDelPoint(QPoint point);
	//通知鼠标位置
	void onNotifyMousePos(QPoint point);
	//通知编辑状态
	void onNotifyStatus(int status);
	//通知计算速度
	void onNotifyVelocity(QString text);
	//实际点位变化
	void onActualPointsChange();
	//通知计算速度
	void onNotifyVelocity();
	//显示图片
	void onDrawImage();
	//局部缩放显示
	void onAddPartZoom(QImage *image, QPoint begin, QPoint end);
	//局部缩放窗口移动或者缩放
	void onPartZoomChange(QWidget *widget);
	//局部缩放窗口移动
	void onPartZoomMove(QWidget *widget);
	//关闭局部缩放窗口
	void onClosePartZoom(QWidget *widget);
	//OCR结果
	void onOCRResult(std::shared_ptr<OCRMsg> msg);
	//水印
	void onWaterMarkChanged(const QTime &time);
	void onWaterMarkWnd(QPoint pos);

signals:
	//切换为播放界面
	void sigShowPlayWidget();
	//通知编辑工具切换
	void sigNotifyTool(VTToolType type);
	//OCR结果
	void sigOCRResult(std::shared_ptr<OCRMsg> msg);
	//关闭OCR进度框
	void sigClose();
private:
	Ui::TakenBG ui;
	//图形
	//编辑点位句柄
	CPointsManage *m_pointsManage = nullptr;
	//编辑窗口
	CTakenWidget *m_takenArea = nullptr;
	//编辑显示结果
	CTakenControl *m_takenControl = nullptr;
	//运行轨迹
	CShowTrace *m_traceWidget = nullptr;
	//图像增强
	CImageEnhance *m_imageEnhance = nullptr;
	//局部放大窗口管理
	QVector<ZoomPair> m_zoomPair;

	QTextEdit *m_teWaterMark;
};
