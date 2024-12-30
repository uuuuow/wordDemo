#pragma once

#include <QScrollArea>
#include "ui_CustomScrollArea.h"
/*
*滚动条
*/
class CustomScrollArea : public QScrollArea
{
	Q_OBJECT

public:
	CustomScrollArea(QWidget *parent = Q_NULLPTR);
	~CustomScrollArea();
protected:
	virtual void wheelEvent(QWheelEvent *ev);
	virtual void keyPressEvent(QKeyEvent *ev);
	virtual void keyReleaseEvent(QKeyEvent *ev);

private:
	Ui::CustomScrollArea ui;
};
