#pragma once

#include <QDialog>
#include "ui_MeasuringMethodWnd.h"

class CMeasuringMethodWnd : public QDialog
{
	Q_OBJECT

public:
	CMeasuringMethodWnd(QWidget *parent = Q_NULLPTR);
	~CMeasuringMethodWnd();

private:
	Ui::CMeasuringMethodWnd ui;
};
