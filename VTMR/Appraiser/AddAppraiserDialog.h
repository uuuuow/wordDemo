#pragma once

#include <QDialog>
#include "ui_AddAppraiserDialog.h"
#include "Utils.h"

class CAddAppraiserDialog : public QDialog
{
	Q_OBJECT

public:
	CAddAppraiserDialog(QWidget *parent = Q_NULLPTR);
	~CAddAppraiserDialog();

	VTAppraiserInfo getInfo();

protected:
	virtual bool eventFilter(QObject *watched, QEvent *event) override;
public:
	VTAppraiserInfo m_info;
	QImage m_photo;

private:
	Ui::AddAppraiserDialog ui;
};
