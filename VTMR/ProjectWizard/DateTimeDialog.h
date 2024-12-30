#pragma once

#include <QDialog>
#include "ui_DateTimeDialog.h"
/*
*日期窗口
*/
class CDateTimeDialog : public QDialog
{
	Q_OBJECT

public:
	CDateTimeDialog(QDateTime datetime, bool show = true, QWidget *parent = Q_NULLPTR);
	~CDateTimeDialog();

public slots:
	virtual void accept();

signals:
	void sigSelectedDate(QDateTime);
private:
	Ui::DateTimeDialog ui;
};
