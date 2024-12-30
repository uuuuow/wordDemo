#pragma once

#include <QDialog>
#include "Global.h"
#include "ui_Register.h"

class Register : public QDialog
{
	Q_OBJECT

public:
	Register(QWidget *parent = nullptr);
	~Register();
	bool checkRegistCode(QString text);
	void generateMachineCode();
	QString loadRegCode();
	void insertSql(QString regCode);

private:
	Ui::RegisterClass ui;
	QString m_machineCode;
};
