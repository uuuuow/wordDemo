#pragma once

#include <QDialog>
#include "ui_StartUpDialog.h"

//程序启动窗口
class CStartUpDialog : public QDialog
{
	Q_OBJECT

public:
	CStartUpDialog(QWidget *parent = Q_NULLPTR);
	~CStartUpDialog();

protected:
	virtual void paintEvent(QPaintEvent *ev);

	public slots:
	void updateProgressbar();

private:
	Ui::StartUpDialog ui;
	QTimer *m_timer;
	int m_persent;
	QPixmap m_pixmap;
	RingsMapProgressbar *m_progressbar2;
};
