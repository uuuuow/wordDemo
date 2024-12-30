#pragma once

#include <QWidget>
#include "ui_takenControl.h"

/*
*显示编辑结果
*/
class CTakenControl : public QWidget
{
	Q_OBJECT

public:
	CTakenControl(QWidget *parent = Q_NULLPTR);
	~CTakenControl();
	//显示帧信息
	void setFrames(int fps, qint64 pts, int index, int actualIndex);
	//显示鼠标坐标
	void setXY(QPoint point);
	//显示编辑状态
	void setStatus(int status);
	//显示速度
	void setVelocity(QString text);
	//显示水印
	void setWaterMark(QTime time);
	void setSign(bool sign);
signals:
	void sigWaterMarkWnd(QPoint pos);
private:
	Ui::CTakenControl ui;
};
