#pragma once

#include <QPushButton>
#include "Utils.h"
/*
*工程目录节点
*/
class CProjectItem : public QPushButton
{
	Q_OBJECT

public:
	CProjectItem(QString uuid, QString proName, QWidget *parent = nullptr);
	~CProjectItem();
	void setActive(bool status);

private slots:
	void onContextMenu(QPoint pos);

signals:
	void sigActiveProject(QString uuid);
	void sigProjectInfo(QString uuid);
	void sigActivePlayer();
	void sigShowTakenWidge(bool);
	void sigDelProject(QString uuid);
public:
	QMenu *m_popMenu = nullptr; //菜单
	QString m_styleSheet;
	bool m_activeProject = false;
	QAction *m_actionPlayer;
	QAction *m_actionTakenArea;
	QString m_uuid;
};
