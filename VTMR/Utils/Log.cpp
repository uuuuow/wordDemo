#include "Log.h"
#include <QMutex>
#include <iostream>
#include <QDateTime>
#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QTextStream>

//接收调试信息的函数
void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	//{ QtDebugMsg, QtWarningMsg, QtCriticalMsg, QtFatalMsg, QtInfoMsg, QtSystemMsg = QtCriticalMsg };
	static QMutex mutex;
	QMutexLocker lock(&mutex);
	//获取单例
	CLog *instance = CLog::Instance();
	//消息输出位置
	QString context_info; 
	//消息打印时间
	QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

	QString text;
	switch (type)
	{
		//如果是debug信息，那么直接打印至应用程序输出，然后退出本函数
	case QtDebugMsg:
		text = QString("[Debug]");
		context_info = "";
		break;

		//如果是警告，或者是下面的其他消息，则继续执行后面的数据处理
	case QtWarningMsg:
		text = QString("[Warning]");
		context_info = "";
		break;

	case QtCriticalMsg:
		text = QString("[Critical]");
		context_info = "";
		break;

	case QtFatalMsg:
		text = QString("[Fatal]");
		context_info = QString("Func: %1\r\line: %2\r\n")
			.arg(QString(context.function))
			.arg(context.line);
		break;

	case QtInfoMsg:
		text = QString("[Info]");
		context_info = "";
		break;

	default:
		text = QString("[Default]");
		context_info = "";
		break;
	}
	
	//调试信息
	QString message = QString("%1 %2 %3 %4")
		.arg(current_date_time)
		.arg(text)
		.arg(context_info)
		.arg(msg);

	//将调试信息写入文件
	QFile file(instance->logPath() + instance->logName());
	file.open(QIODevice::WriteOnly | QIODevice::Append);
	QTextStream text_stream(&file);
	text_stream << message << "\r\n";
	file.flush();
	file.close();

	//检查文件是否达到了指定大小
	if (file.size() < 1024 * 1024) {
		return;
	}
	//log达到了限制值则将名字更改，防止文件越来越大
	for (int loop = 1; loop < 100; ++loop) {
		QString fileName = QString("%1/log_%2.txt").arg(instance->logPath()).arg(loop);
		QFile file_1(fileName);
		if (file_1.size() < 4) {
			file.rename(fileName);
			return;
		}
	}
}

//CLog单例
CLog* CLog::log = nullptr;
CLog* CLog::Instance()
{
	if (!log) {
		static QMutex muter;
		QMutexLocker clocker(&muter);

		if (!log) {
			log = new CLog(nullptr);
		}
	}
	return log;
}
//安装消息器
void CLog::installMessageHandler()
{
	qInstallMessageHandler(outputMessage);
}
//卸载消息器
void CLog::uninstallMessageHandler()
{
	qInstallMessageHandler(0);
}
//建立文件路径
QString CLog::logPath()
{
	QString current_date_file_name = QDateTime::currentDateTime().toString("yyyy-MM-dd");
	QDir dir(QString("log/%1").arg(current_date_file_name));
	if (!dir.exists()) {
		dir.mkpath("./");
	}
	return dir.path() + "/";
}

QString CLog::logName()
{
	return "log.txt";
}

CLog::CLog(QObject *parent) : QObject(parent)
{
}

CLog::~CLog()
{
	std::cout << "~CLog" << std::endl;
}

