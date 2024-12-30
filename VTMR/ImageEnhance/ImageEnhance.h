#pragma once

#include <QWidget>
#include "ui_ImageEnhance.h"
/*
*图像增强界面
*调节亮度、对比度、饱和度
*/
class CImageEnhance : public QWidget
{
	Q_OBJECT

public:
	CImageEnhance(QWidget *parent = Q_NULLPTR);
	~CImageEnhance();
	void setValue(int brightness, int contrast, int saturation);

signals:
	void sigBrightnessValueChanged(int value);
	void sigContrastValueChanged(int value);
	void sigSaturationValueChanged(int value);

private:
	Ui::CImageEnhance ui;
};
