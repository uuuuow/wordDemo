#include "vtmr.h"
#include <QtWidgets/QApplication>
#include <QFile>
#include "StartUpDialog.h"
#include <QDebug>
#include "Log.h"
#include <QSharedMemory>
#include "Register/Register.h"
#include "SQLManager.h"
#include "Global.h"'
#include "AppraisalStandardDialog.h"
#include "docx.h"
#include "WordReport.h"


static void setStyle(const QString &style) 
{
	QFile qss(style);
	qss.open(QFile::ReadOnly);
	QByteArray ba = qss.readAll();
	qApp->setStyleSheet(ba);
	qss.close();
}

int main(int argc, char *argv[]) 
{
	QApplication a(argc, argv);

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
	QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);
#endif
	g_appPath = QCoreApplication::applicationDirPath();
	setStyle(":/darkstyle/images/VTMR/darkstyle/style.qss");
#ifndef _DEBUG
	QString sharedMemoryKey = "VTMR-EUTROEYE-2023";
	QSharedMemory sharedMemory(sharedMemoryKey);
	if (!sharedMemory.create(1)) {
		return 0;
	}
	CLog::Instance()->installMessageHandler();
#endif
	CSQLManager::initDataBase();
	//校验是否注册绑定个人电脑
	auto regist = [=]()->bool {
		Register r;
		r.generateMachineCode();
		if (r.checkRegistCode(""))
			return true;
		if (r.exec() != QDialog::Accepted)
			return false;
	};
	if (!regist())
		return 0;

	//加载鉴定标准
	CAppraisalStandardDialog::loadSql();
	CVTMR w;
	//初始化OCR
	if (!w.initOCR()) {
		//qDebug() << "zzq 初始化OCR失败";
		return 0;
	}
	//qDebug() << "zzq 开始加载转圈";
#ifndef _DEBUG
	//启动界面
	[]() {
		auto startUp = CStartUpDialog();
		startUp.exec();
	}();
#endif
	//CWordReport word_report(nullptr);
	//word_report.onSaveReport(g_proInfo, "test");

	//while (1)
	//{
	//	_sleep(1000);
	//}
	w.show();
	return a.exec();
}
