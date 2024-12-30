#pragma once

#include <QListWidget>
#include "Utils.h"
#include <QMap>
class CProjectItem;
/*
*工程目录
*/
class CProjectList : public QListWidget
{
	Q_OBJECT

public:
	CProjectList(QWidget *parent);
	~CProjectList();
	void addProject(VTProjectInfo &proInfo);

	virtual bool eventFilter(QObject *obj, QEvent *ev);

signals:
	void sigActiveProject(QString uuid);
	void sigActivePlayer();
	void sigUpdateProject(VTProjectInfo *info);
public slots:
	void onActiveProject(QString uuid);
	void onDelProject(QString uuid);

	void onActivePlayer();
private:
	VTProjectInfo m_tagProjectInfo;
};
