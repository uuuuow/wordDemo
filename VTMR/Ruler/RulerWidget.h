#pragma once

#include <QWidget>
#include "ui_RulerWidget.h"
#include <QMenu>
#include <QAction>
/*
*直角尺
*/
class CRulerWidget : public QWidget
{
	Q_OBJECT

public:
	CRulerWidget(QWidget *parent = Q_NULLPTR);
	~CRulerWidget();

	private slots:
	void onMove(QPoint);
	void onOrn();
	void onAlign();
	void onClose();

private:
	void keyReleaseEvent(QKeyEvent *e);

private:
	Ui::CRulerWidget ui;
	QMenu *m_Menu;
	QAction *m_actOrn, *m_actAlign, *m_actClose;
};
