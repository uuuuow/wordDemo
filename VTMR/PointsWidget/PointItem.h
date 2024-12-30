#pragma once

#include <QWidget>
#include "ui_PointItem.h"

class CPointItem : public QWidget
{
	Q_OBJECT

public:
	CPointItem(int px, QWidget *parent = nullptr);
	~CPointItem();
	void setEnableXY(bool enableX, bool enableY);
	QPoint getVelue();
	void setVelue(QPoint point);
	QPointF getVeluF();
	void setVelueF(QPointF point);
	void setVelueFX(float x);
	void setVelueFY(float y);
	void paintEvent(QPaintEvent *e);
signals:
	void onValueFChanged(QPointF point);
private:
	Ui::PointItem ui;
};
