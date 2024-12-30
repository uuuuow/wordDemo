#pragma once

#include <QWidget>
#include "ui_WaterMarkWnd.h"

class CWaterMarkWnd : public QWidget
{
	Q_OBJECT

public:
	CWaterMarkWnd(QColor color, QWidget *parent = Q_NULLPTR);
	~CWaterMarkWnd();
	QColor getColor() { return m_color; }
	QRect getRect() { return m_waterMarkRect; }
	void setRect(QRect wm, QRect image);
	void moveWnd(QRect r);
signals:
	void sigClose(QWidget* pThis);
	void sigHide(QWidget *pThis);

protected:
	void paintEvent(QPaintEvent *ev);
	void resizeEvent(QResizeEvent *ev);

private:
	Ui::CWaterMarkWnd ui;
	QColor m_color;
	QRect m_waterMarkRect;
	QRect m_imageRect;
};
