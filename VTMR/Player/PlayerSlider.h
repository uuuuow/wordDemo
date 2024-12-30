#pragma once

#include <QWidget>
#include <QSlider>
/*
*播放器进度条
*/
class CPlayerSlider : public QSlider
{
	Q_OBJECT

public:
	CPlayerSlider(QWidget *parent);
	~CPlayerSlider();
	void mousePressEvent(QMouseEvent *e);

};
