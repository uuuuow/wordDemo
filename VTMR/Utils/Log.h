#pragma once

#include <QObject>

class CLog : public QObject
{
	Q_OBJECT
public:
	static CLog* Instance();
	static CLog* log;

	void installMessageHandler();
	void uninstallMessageHandler();
	QString logPath();
	QString logName();
public:
	CLog(QObject *parent);
	~CLog();
};
