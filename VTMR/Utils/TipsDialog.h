#pragma once

#include <QDialog>
#include "ui_TipsDialog.h"
#include <QLabel>

class CTipsDialog : public QDialog
{
	Q_OBJECT

public:
	CTipsDialog(QString strTitle, QWidget *parent = Q_NULLPTR);
	~CTipsDialog();
signals:
	void sigClose();
public slots:
	void onClose();

private:
	Ui::CTipsDialog ui;
	QLabel *m_title;
};
