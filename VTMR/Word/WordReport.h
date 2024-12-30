#pragma once

#include <QObject>
#include <QThread>
#include <QQueue>
#include "Utils.h"
#include <QMutex>
#include <memory>
struct tagWordInfo
{
	VTProjectInfo proInfo;
	QString filePath;
};

class CWorkThread : public QThread
{
	Q_OBJECT

public:
	CWorkThread(QObject *parent);
	void setExit() { m_exit = true; }
	void pushWordReport(VTProjectInfo proInfo, QString filePath);
	bool m_moveSaveAsWnd = false;
private:
	virtual void run();
private:
	volatile bool m_exit = false;
	QMutex m_mutex;
	QQueue<tagWordInfo> m_proInfoPub;
	QQueue<tagWordInfo> m_proInfoPri;
};

class CWordReport : public QObject
{
	Q_OBJECT

public:
	CWordReport(QObject *parent);
	~CWordReport();
	static void generateReport(tagWordInfo &wordInfo, CWordReport *obj);
signals:
	void sigSaveReport(VTProjectInfo &proInfo, QString filePath);
	void sigProcessBar(int n);
public slots:
	void onSaveReport(VTProjectInfo& proInfo, QString filePath);

private:
	CWorkThread *m_workThread;
	QTimer* m_timer;
public:
	static QString m_imageTemp;
	static QString m_songTi;
	static QString m_fangSong;
	static QString m_heiTi;
};
