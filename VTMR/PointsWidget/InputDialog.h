#pragma once

#include <QDialog>
#include "ui_InputDialog.h"
#include <QDialog>
#include <QImage>
#include "Utils.h"

class COCRWidget;

class CInputDialog : public QDialog
{
	Q_OBJECT

public:
	CInputDialog(int type, VTTimeMode timeMode, QWidget *parent = Q_NULLPTR);
	~CInputDialog();
	void setWidget(QWidget * w);

public slots:
	void onOK();
private:
	Ui::InputDialog ui;
};
