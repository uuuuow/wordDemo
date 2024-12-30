#pragma once

#include <QWidget>
#include "ui_Magnifier.h"
#include <QPoint>
#include <QDateTime>
#include <QLabel>
/*
*放大镜界面
*/
class CMagnifier : public QWidget
{
	Q_OBJECT

public:
	CMagnifier(QWidget *parent = Q_NULLPTR);
	~CMagnifier();
	void setMultiple(int n);
	void setLabelPixmap(QImage *image,QPoint &pos);
protected:
	void timerEvent(QTimerEvent *e);
	void paintEvent(QPaintEvent *);

private:
	Ui::Magnifier ui;
	float m_multiple = 1.0f;
	QLabel *m_labelShowMultiple = nullptr;
	QDateTime m_lastTime;
	QImage m_showImage;
	QRect m_showRect;
};
