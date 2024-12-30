#include "WordReport.h"
#include "docx.h"
#include <QDateTime>
#include "Global.h"
#include <QFileInfo>
#include "AppraiserDialog.h"
#include <QDebug>
#include "TakenBG.h"
#include <QDir>
#include <QTime>
#include <QStandardPaths>
#include <Windows.h>
#include <QTimer>
#include <QFileDialog>

#include <QProcess>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QString>
#include <set>
#include <QMetaEnum>

#define TAB_X4			"				"
#define SP_X4			"    "
#define SP_X10			"          "
#define SP_X12			"            "
#define SP_X14			"              "
#define SP_X16			"                "
#define SP_X20			"                    "
#define SP_X24			"                        "
#define PIC_SCALSE_SIZE 1000
#define PIC_SCALSE_SIZE2 170

CWorkThread::CWorkThread(QObject *parent)
	: QThread(parent)
{

}

void CWorkThread::run()
{
	while (!m_exit) {
		if (m_proInfoPub.empty() && !m_exit)
			QThread::msleep(200);
		else {
			m_mutex.lock();
			while (!m_proInfoPub.empty() && !m_exit) {
				tagWordInfo wordInfo;
				wordInfo = m_proInfoPub.front();
				m_proInfoPub.pop_front();
				m_proInfoPri.push_back(wordInfo);
			}
			m_mutex.unlock();
		}

		while (!m_proInfoPri.empty()) {
			tagWordInfo wordInfo = m_proInfoPri.front();
			m_proInfoPri.pop_front();
			m_moveSaveAsWnd = true;
			CWordReport::generateReport(wordInfo, (CWordReport *)parent());
			m_moveSaveAsWnd = false;
			if(!m_exit)
				QThread::msleep(50);
		}
		if(!m_exit)
			QThread::msleep(50);
	}
}

void CWorkThread::pushWordReport(VTProjectInfo proInfo, QString savePath)
{
	tagWordInfo wordInfo;
	wordInfo.proInfo = proInfo;
	wordInfo.filePath = savePath;
	m_mutex.lock();
	m_proInfoPub.push_back(wordInfo);
	m_mutex.unlock();
}

QString CWordReport::m_imageTemp;
QString CWordReport::m_songTi = QString("宋体");
QString CWordReport::m_fangSong = QString("仿宋");
QString CWordReport::m_heiTi = QString("黑体");

CWordReport::CWordReport(QObject *parent)
	: QObject(parent)
{

	m_imageTemp = QString("%1%2").arg(g_appPath).arg(PROJECT_DATA_TEMP_PATH);
	QDir dir;
	if (!dir.exists(m_imageTemp))
		dir.mkpath(m_imageTemp);

	m_workThread = new CWorkThread(this);
	m_workThread->start();

	m_timer = new QTimer();
	connect(m_timer, &QTimer::timeout, [=]() {
		if (!m_workThread->m_moveSaveAsWnd)
			return;
		HWND hwnd = VTFindWindow("#32770", "另存为");
		if (hwnd)
		{
			QPoint pos = VTDeskPosition();
			int x = pos.x();
			int y = pos.y();

			SetWindowPos(hwnd, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
			ShowWindow(hwnd, SW_SHOW);
			qDebug() << "找到另存为窗口(" << hwnd << ")，移动到" << x << "," << y;
		}
	});
	m_timer->start(200);
}

CWordReport::~CWordReport()
{
	m_workThread->setExit();
	m_workThread->wait();
}

void CWordReport::onSaveReport(VTProjectInfo& proInfo, QString savePath)
{
	m_workThread->pushWordReport(proInfo, savePath);
}

static void GenerateHeaderFooter(VTProjectInfo &proInfo, CDocx &docx)
{
	docx.setHeaderFooter(QString("黑体"), 18, QString("司法鉴定科学研究院司法鉴定意见书"));
}

static void GenerateTitle(VTProjectInfo& proInfo, CDocx& docx)
{
	QString str;
	QDateTime curDateTime = QDateTime::currentDateTime();

	docx.setTextAlign(CDocx::AliginCenter);
	str = QString("司法鉴定科学研究院司法鉴定意见书");
	docx.setContext(str, CWordReport::m_heiTi, QColor(0, 0, 0), 18);

	docx.newLine(1);
	docx.setContext("", CWordReport::m_fangSong, QColor(0, 0, 0), 14);
	docx.newLine(1);

	docx.setTextAlign(CDocx::AliginRight);
	str = QString("司鉴院[%1]交鉴字第   号").arg(curDateTime.toString("yyyy"));
	docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
}

static void Generate1(VTProjectInfo& proInfo, CDocx& docx)
{
	QString str;
	docx.m_numFormatSeq1 = 1;
	docx.m_numFormatSeq2 = 1;

	docx.newLine(1);
	docx.setTextAlign(CDocx::AliginLeftAndRight);
	docx.ParagraphFormat(0, 0, 0, CDocx::AliginLeftAndRight);
	docx.ApplyListTemplate(1, docx.m_numFormatSeq1++);

	str = QString("基本情况");
	docx.setContext(str, CWordReport::m_heiTi, QColor(0, 0, 0), 14);

	docx.ParagraphFormat(0, 0, 0, CDocx::AliginLeftAndRight);
	docx.newLine(1);
	docx.setTextStyle(45);
	docx.ParagraphFormat(2, 0, -5, CDocx::AliginLeftAndRight);

	str = QString("委 托 人：");
	docx.setContext(str, CWordReport::m_heiTi, QColor(0, 0, 0), 14);

	str = QString("%1").arg(proInfo.client);
	docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

	docx.newLine(1);

	str = QString("委托事项：");
	docx.setContext(str, CWordReport::m_heiTi, QColor(0, 0, 0), 14);

	str = QString("根据案件调查需要，对");
	docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

	str = QString("%1%2%3")
		.arg(proInfo.licensePlateNumber)
		.arg(proInfo.vehicleBrand)
		.arg(proInfo.vehicleType);
	docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

	str = QString("事发时的速度进行鉴定。");
	docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

	docx.newLine(1);

	str = QString("委托日期：");
	docx.setContext(str, CWordReport::m_heiTi, QColor(0, 0, 0), 14);

	QDateTime DateOfCommission = QDateTime::fromString(proInfo.dateOfCommission, "yyyy-MM-dd");
	str = DateOfCommission.toString(QString("yyyy年MM月dd日"));
	docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

	docx.newLine(1);

	str = QString("鉴定材料：");
	docx.setContext(str, CWordReport::m_heiTi, QColor(0, 0, 0), 14);

	if (proInfo.mode == FIXED_CAM_ONESELF) {
		str = QString("1、被鉴定车辆");
		docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
		docx.newLine(1);
		
		str = QString("%1").arg(SP_X10);
		docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

		str = QString("2、事发时现场附近监控视频（复制件）");
		docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
	}
	else if (proInfo.mode == MOVING_CAM_SPACE) {
		str = QString("被鉴定车辆车载行车记录仪拍摄的视频（复制件）");
		docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
	}

	docx.newLine(1);

	str = QString("鉴定日期：");
	docx.setContext(str, CWordReport::m_heiTi, QColor(0, 0, 0), 14);
	QDateTime AppraisalDate = QDateTime::fromString(proInfo.appraisalDate, "yyyy-MM-dd");
	str = AppraisalDate.toString(QString("yyyy年MM月dd日"));
	docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

	docx.newLine(1);

	str = QString("鉴定地点：");
	docx.setContext(str, CWordReport::m_heiTi, QColor(0, 0, 0), 14);
	str = QString("%1").arg(proInfo.appraisalLocation);
	docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
}

static void Generate2(VTProjectInfo &proInfo, CDocx &docx)
{
	QString str;

	docx.newLine(1);
	docx.ParagraphFormat(0, 0, 0, CDocx::AliginLeftAndRight);

	docx.newLine(1);
	docx.ParagraphFormat(0, 0, 0, CDocx::AliginLeftAndRight);
	docx.ApplyListTemplate(1, docx.m_numFormatSeq1++);

	str = QString("基本案情");
	docx.setContext(str, CWordReport::m_heiTi, QColor(0, 0, 0), 14);

	docx.ParagraphFormat(0, 0, 0, CDocx::AliginLeftAndRight);
	docx.newLine(1);
	docx.setTextStyle(45);
	docx.ParagraphFormat(0, 0, 2, CDocx::AliginLeftAndRight);

	QDateTime accidentTime = QDateTime::fromString(proInfo.accidentTime, "yyyy-MM-dd hh:mm:ss");
	str = accidentTime.toString(QString("yyyy年MM月dd日hh时mm分"));
	docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
	str = QString("许，被鉴定车辆在");
	docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
	str = QString("%1").arg(proInfo.accidentLocation);
	docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
	str = QString("处发生交通事故。");
	docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
}

static void Generate3_1(VTProjectInfo &proInfo, CDocx &docx)
{
	QString str;
	
	docx.newLine(1);
	docx.ParagraphFormat(0, 0, 0, CDocx::AliginLeftAndRight);

	docx.newLine(1);
	docx.ParagraphFormat(0, 0, 0, CDocx::AliginLeftAndRight);
	docx.ApplyListTemplate(1, docx.m_numFormatSeq1++);

	str = QString("鉴定过程");
	docx.setContext(str, CWordReport::m_heiTi, QColor(0, 0, 0), 14);

	docx.ParagraphFormat(0, 0, 0, CDocx::AliginLeftAndRight);
	docx.newLine(1);
	docx.setTextStyle(45);
	docx.ApplyListTemplate(2, docx.m_numFormatSeq2++);

	str = QString("鉴定方法");
	docx.setContext(str, CWordReport::m_heiTi, QColor(0, 0, 0), 14);

	docx.ParagraphFormat(0, 0, 0, CDocx::AliginLeftAndRight);
	docx.newLine(1);
	docx.setTextStyle(45);
	docx.ParagraphFormat(0, 0, 2, CDocx::AliginLeftAndRight);
	QString strStandard;
	{
		std::set<VTAppraisalStandard> tmp(proInfo.appraisalStandards.begin(), proInfo.appraisalStandards.end());
		int nStandard = proInfo.appraisalStandards.size();
		for (auto& standard : tmp) {
			if (!strStandard.isEmpty())
				strStandard.append(QString("、"));
			strStandard.append(standard.id);
			strStandard.append(standard.name);
		}
	}
	QString des1;
	QString des2;
	if (proInfo.mode == FIXED_CAM_ONESELF) {
		des1 = QString("被鉴定车辆");
		des2 = QString("事发时现场附近监控视频");
	}
	else if (proInfo.mode == MOVING_CAM_SPACE) {
		des1 = QString("事故现场");
		des2 = QString("被鉴定车辆车载行车记录仪拍摄的视频");
	}
	str = QString("参照%1有关条款及检验方法，结合监控视频所示内容及%2相关数据，对委托事项作出鉴定意见。")
		.arg(strStandard)
		.arg(des1);
	docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
}
QString formatTime(int64_t microseconds) {
	int hours = microseconds / (1000 * 1000 * 60 * 60);  // 微秒转小时
	int minutes = (microseconds / (1000 * 1000 * 60)) % 60;  // 微秒转分钟
	int seconds = (microseconds / (1000 * 1000)) % 60;  // 微秒转秒
	int ms = (microseconds / 1000) % 1000;  // 微秒转毫秒

	QString timeString = QString("%1:%2:%3:%4")
		.arg(hours, 2, 10, QLatin1Char('0'))
		.arg(minutes, 2, 10, QLatin1Char('0'))
		.arg(seconds, 2, 10, QLatin1Char('0'))
		.arg(ms, 3, 10, QLatin1Char('0'));

	return timeString;
}

static void Generate3_2(VTProjectInfo &proInfo, CDocx &docx)
{	
	QString str;
	QString des1;
	QString des2;
	if (proInfo.mode == FIXED_CAM_ONESELF) {
		des1 = QString("被鉴定车辆");
		des2 = QString("事发时现场附近监控视频");
	}
	else if (proInfo.mode == MOVING_CAM_SPACE) {
		des1 = QString("事故现场");
		des2 = QString("被鉴定车辆车载行车记录仪拍摄的视频");
	}

	docx.newLine(1);
	docx.setTextStyle(45);
	docx.ApplyListTemplate(2, docx.m_numFormatSeq2++);

	str = QString("检验所见");
	docx.setContext(str, CWordReport::m_heiTi, QColor(0, 0, 0), 14);

	docx.ParagraphFormat(0, 0, 0, CDocx::AliginLeftAndRight);
	docx.newLine(1);
	docx.setTextStyle(45);
	docx.ParagraphFormat(0, 0, 2, CDocx::AliginLeftAndRight);

	QFileInfo VideoInfo(proInfo.videoPath);

	str = QString("1、%2").arg(des2);
	docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
	docx.newLine(1);
	QString strOCRText = "";
	if (proInfo.firstImage != nullptr) {
		for (auto& waterMark : proInfo.firstImage->waterMarks) {
			if (waterMark.text.isEmpty())
				continue;
			if (!strOCRText.isEmpty())
				strOCRText += QString("、");
			strOCRText += QString("“");
			strOCRText += waterMark.text;
			strOCRText += QString("”");
		}
	}
	if (strOCRText.isEmpty())
		strOCRText = QString("“”");

	QString hashAlgo;
	QString hashName;
	switch (proInfo.hashAlgo)
	{
	case QCryptographicHash::Algorithm::Md5:
		hashName = "MD5";
		break;
	case QCryptographicHash::Algorithm::Sha1:
		hashName = "SHA1";
		break;
	case QCryptographicHash::Algorithm::Sha256:
		hashName = "SHA256";
		break;
	case QCryptographicHash::Algorithm::Sha512:
		hashName = "SHA512";
		break;
	default:
		hashName = "MD5";
		break;
	}
	float fileSize = 0;
	if (proInfo.mapHashInfo.find(proInfo.hashAlgo) != proInfo.mapHashInfo.end()) {
		QByteArray data = proInfo.mapHashInfo[proInfo.hashAlgo];
		if (!data.isEmpty())
			hashAlgo = QString(data.toHex().toStdString().c_str());
		fileSize = proInfo.fileSize * 1.0 / (1024 * 1024);
	}
	else {
		QByteArray data;
		if (VTFileHash(proInfo.videoPath, proInfo.fileSize, proInfo.hashAlgo, data)) {
			if (!data.isEmpty()) {
				hashAlgo = QString(data.toHex().toStdString().c_str());
				proInfo.mapHashInfo[proInfo.hashAlgo] = data;
			}
			fileSize = proInfo.fileSize * 1.0 / (1024 * 1024);
		}
	}
	
	str = QString("视频文件名为%1，文件大小为%2MB，哈希值（%3）%4。监控视频显示%5等字样。视频画面连续，按时间顺序，视频画面依次显示以下内容：")
		.arg(VideoInfo.fileName()).arg(QString::number(fileSize, 'f', 2)).arg(hashName).arg(hashAlgo).arg(strOCRText);
	docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
	docx.newLine(1);

	qint64 headPTS = 0;
	qint64 tailPTS0 = 0;
	qint64 tailPTS1 = 0;
	int headIndex = 0;
	int tailIndex0 = 0;
	int tailIndex1 = 0;

	QString headHMS;
	QString tailHMS0;
	QString tailHMS1;
	int headHMSIndex = 0;
	int tailHMS0Index = 0;
	int tailHMS1Index = 0;

	if (proInfo.headIndex >= 0 && proInfo.headIndex < proInfo.frameInfos.size()) {
		headIndex = proInfo.frameInfos[proInfo.headIndex].index;
		headPTS = proInfo.frameInfos[proInfo.headIndex].pts;

		headHMS = proInfo.frameInfos[proInfo.headIndex].timeHMS;
		for (int m = proInfo.headIndex; m >= 0; m--) {
			headHMSIndex++;
			if (headHMS != proInfo.frameInfos[m].timeHMS)
				break;
		}
	}

	if (proInfo.tailIndex0 >= 0 && proInfo.tailIndex0 < proInfo.frameInfos.size()) {
		tailIndex0 = proInfo.frameInfos[proInfo.tailIndex0].index;
		tailPTS0 = proInfo.frameInfos[proInfo.tailIndex0].pts;

		tailHMS0 = proInfo.frameInfos[proInfo.tailIndex0].timeHMS;
		for (int m = proInfo.tailIndex0; m >= 0; m--) {
			tailHMS0Index++;
			if (tailHMS0 != proInfo.frameInfos[m].timeHMS)
				break;
		}
	}

	if (proInfo.tailIndex1 >= 0 && proInfo.tailIndex1 < proInfo.frameInfos.size()) {
		tailIndex1 = proInfo.frameInfos[proInfo.tailIndex1].index;
		tailPTS1 = proInfo.frameInfos[proInfo.tailIndex1].pts;

		tailHMS1 = proInfo.frameInfos[proInfo.tailIndex1].timeHMS;
		for (int m = proInfo.tailIndex1; m >= 0; m--) {
			tailHMS1Index++;
			if (tailHMS1 != proInfo.frameInfos[m].timeHMS)
				break;
		}
	}
	if (proInfo.timeMode == TIME_MODE_PTS) {
		str = QString("在总第%1帧图像上，设与被鉴定车辆%2重合的空间中的一点为参考点A，该图像的显示时间戳(PTS)为%3；")
			.arg(headIndex).arg(proInfo.referenceDesFrom).arg(formatTime(headPTS));
		docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

		if (proInfo.tailIndex0 > 0 && proInfo.tailIndex1 > 0) {
			docx.newLine(1);
			str = QString("在总第%1帧图像上，被鉴定车辆%2尚未达参考点A所在位置，该图像的显示时间戳(PTS)为%3；")
				.arg(tailIndex1).arg(proInfo.referenceDesTo).arg(formatTime(tailPTS1));
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

			docx.newLine(1);
			str = QString("在总第%1帧图像上，被鉴定车辆%2超过参考点A所在位置，该图像的显示时间戳(PTS)为%3；")
				.arg(tailIndex0).arg(proInfo.referenceDesTo).arg(formatTime(tailPTS0));
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
		}
		else {
			docx.newLine(1);
			str = QString("在总第%1帧图像上，被鉴定车辆%2近似到达参考点A所在位置，该图像的显示时间戳(PTS)为%3；")
				.arg(tailIndex0).arg(proInfo.referenceDesTo).arg(formatTime(tailPTS0));
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
		}
	}
	else if (proInfo.timeMode == TIME_MODE_FPS) {
		str = QString("在总第%1帧图像上，设与被鉴定车辆%2重合的空间中的一点为参考点A；")
			.arg(headIndex).arg(proInfo.referenceDesFrom);
		docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
		if (proInfo.tailIndex0 > 0 && proInfo.tailIndex1 > 0) {
			docx.newLine(1);
			str = QString("在总第%1帧图像上，被鉴定车辆%2尚未达参考点A所在位置；")
				.arg(tailIndex1).arg(proInfo.referenceDesTo);
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

			docx.newLine(1);
			str = QString("在总第%1帧图像上，被鉴定车辆%2超过参考点A所在位置；")
				.arg(tailIndex0).arg(proInfo.referenceDesTo);
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
		}
		else {
			docx.newLine(1);
			str = QString("在总第%1帧图像上，被鉴定车辆%2近似到达参考点A所在位置；")
				.arg(tailIndex0).arg(proInfo.referenceDesTo);
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
		}

		docx.newLine(1);
		str = QString("视频帧率为%1fps。")
			.arg(proInfo.fps);
		docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
	}
	else if (proInfo.timeMode == TIME_MODE_WATER_MARK) {
		str = QString("在%1第%2帧图像上，设与被鉴定车辆%3重合的空间中的一点为参考点A；")
			.arg(headHMS).arg(headHMSIndex).arg(proInfo.referenceDesFrom);
		docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

		int nTailCount0 = 0, nTailCount1 = 0;
		for (auto &frameInfo : proInfo.frameInfos) {
			if (tailHMS0 != "" && tailHMS0 == frameInfo.timeHMS)
				nTailCount0++;
			else if (tailHMS1 != "" && tailHMS0 != tailHMS1 && tailHMS1 == frameInfo.timeHMS)
				nTailCount1++;
		}

		if (proInfo.tailIndex0 > 0 && proInfo.tailIndex1 > 0) {
			docx.newLine(1);
			str = QString("在%1第%2帧图像上，被鉴定车辆%3尚未达参考点A所在位置；")
				.arg(tailHMS1).arg(tailHMS1Index).arg(proInfo.referenceDesTo);
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

			docx.newLine(1);
			str = QString("在%1第%2帧图像上，被鉴定车辆%3超过参考点A所在位置；")
				.arg(tailHMS0).arg(tailHMS0Index).arg(proInfo.referenceDesTo);
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

			if (nTailCount1 > 0) {
				docx.newLine(1);
				str = QString("%1共有%2帧图像；")
					.arg(tailHMS0).arg(nTailCount1);
				docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
			}

			if (nTailCount0 > 0) {
				docx.newLine(1);
				str = QString("%1共有%2帧图像；")
					.arg(tailHMS0).arg(nTailCount0);
				docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
			}
		}
		else {
			docx.newLine(1);
			str = QString("在%1第%2帧图像上，被鉴定车辆%3近似到达参考点A所在位置；")
				.arg(tailHMS0).arg(tailHMS0Index).arg(proInfo.referenceDesTo);
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

			docx.newLine(1);
			str = QString("%1共有%2帧图像；")
				.arg(tailHMS0).arg(nTailCount0);
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
		}
	}

	docx.newLine(1);
	str = QString("2、被鉴定车辆");
	docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);


	docx.newLine(1);
	float L = sqrtf((proInfo.actualPoints[0].x() - proInfo.actualPoints[1].x())*(proInfo.actualPoints[0].x() - proInfo.actualPoints[1].x())
		- (proInfo.actualPoints[0].y() - proInfo.actualPoints[1].x())*(proInfo.actualPoints[0].x() - proInfo.actualPoints[1].y()));
	str = QString("车辆识别代号为%1。经测量，被鉴定车辆%2与%3之间的距离约为%4cm。")
		.arg(proInfo.vehicleVIN)
		.arg(proInfo.referenceDesFrom)
		.arg(proInfo.referenceDesTo)
		.arg(L*100);
	docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
}
static void Generate4(VTProjectInfo &proInfo, CDocx &docx)
{
	QString str;
	QString des1;
	QString des2;
	if (proInfo.mode == FIXED_CAM_ONESELF) {
		des1 = QString("被鉴定车辆");
		des2 = QString("事发时现场附近监控视频");
	}
	else if (proInfo.mode == MOVING_CAM_SPACE) {
		des1 = QString("事故现场");
		des2 = QString("被鉴定车辆车载行车记录仪拍摄的视频");
	}

	qint64 headPTS = 0;
	qint64 tailPTS0 = 0;
	qint64 tailPTS1 = 0;
	int headIndex = 0;
	int tailIndex0 = 0;
	int tailIndex1 = 0;

	QString headHMS;
	QString tailHMS0;
	QString tailHMS1;
	int headHMSIndex = 0;
	int tailHMS0Index = 0;
	int tailHMS1Index = 0;

	docx.newLine(1);
	docx.ParagraphFormat(0, 0, 0, CDocx::AliginLeftAndRight);

	if (proInfo.headIndex >= 0 && proInfo.headIndex < proInfo.frameInfos.size()) {
		headIndex = proInfo.frameInfos[proInfo.headIndex].index;
		headPTS = proInfo.frameInfos[proInfo.headIndex].pts;

		headHMS = proInfo.frameInfos[proInfo.headIndex].timeHMS;
		for (int m = proInfo.headIndex; m >= 0; m--) {
			headHMSIndex++;
			if (headHMS != proInfo.frameInfos[m].timeHMS)
				break;
		}
	}

	if (proInfo.tailIndex0 >= 0 && proInfo.tailIndex0 < proInfo.frameInfos.size())
	{
		tailIndex0 = proInfo.frameInfos[proInfo.tailIndex0].index;
		tailPTS0 = proInfo.frameInfos[proInfo.tailIndex0].pts;

		tailHMS0 = proInfo.frameInfos[proInfo.tailIndex0].timeHMS;
		for (int m = 0; m >= 0; m--)
		{
			tailHMS0Index++;
			if (tailHMS0 != proInfo.frameInfos[m].timeHMS)
				break;
		}
	}

	if (proInfo.tailIndex1 >= 0 && proInfo.tailIndex1 < proInfo.frameInfos.size()) {
		tailIndex1 = proInfo.frameInfos[proInfo.tailIndex1].index;
		tailPTS1 = proInfo.frameInfos[proInfo.tailIndex1].pts;

		tailHMS1 = proInfo.frameInfos[proInfo.tailIndex1].timeHMS;
		for (int m = 0; m >= 0; m--) {
			tailHMS1Index++;
			if (tailHMS1 != proInfo.frameInfos[m].timeHMS)
				break;
		}
	}

	docx.ParagraphFormat(0, 0, 0, CDocx::AliginLeftAndRight);
	docx.newLine(1);
	docx.setTextStyle(45);
	docx.ApplyListTemplate(1, docx.m_numFormatSeq1++);

	str = QString("分析计算");
	docx.setContext(str, CWordReport::m_heiTi, QColor(0, 0, 0), 14);

	docx.ParagraphFormat(0, 0, 0, CDocx::AliginLeftAndRight);
	docx.newLine(1);
	docx.setTextStyle(45);
	docx.ParagraphFormat(0, 0, 2, CDocx::AliginLeftAndRight);

	double dValue = 0.04;
	dValue = 1.0 / proInfo.fps;
	
	str = QString("利用速度－时间－位移关系式，求解被鉴定车辆通过参考点A时的速度，公式如下:");
	docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

	docx.newLine(1);
	docx.setTextAlign(CDocx::AliginCenter);
	str = QString("v=l/t");
	docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

	float L = sqrtf((proInfo.actualPoints[0].x() - proInfo.actualPoints[1].x())*(proInfo.actualPoints[0].x() - proInfo.actualPoints[1].x())
		- (proInfo.actualPoints[0].y() - proInfo.actualPoints[1].x())*(proInfo.actualPoints[0].x() - proInfo.actualPoints[1].y()));
	docx.newLine(1);
	docx.setTextAlign(CDocx::AliginLeftAndRight);
	str = QString("经测量，被鉴定车辆通过参考点A所经过的距离l约为%1cm，所经过的时间t为：").arg(L * 100.0);
	docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);


	float v0 = 0.0;
	float v1 = 0.0;
	float t0 = 0;
	float t1 = 0;
	float frameRate = 0.0;
	float frameDuration = 0.0;
	if (proInfo.timeMode == TIME_MODE_PTS) {
		v0 = proInfo.v01;
		v1 = proInfo.v11;
		t0 = (tailPTS0 - headPTS) / 1000000.0;
		t1 = (tailPTS1 - headPTS) / 1000000.0;
		
		if (proInfo.tailIndex0 > 0 && proInfo.tailIndex1 > 0) {
			docx.newLine(1);
			docx.setTextAlign(CDocx::AliginCenter);
			
			str = QString("t=%1-%2=%3 s").arg(tailPTS0 / 1000000.0).arg(headPTS / 1000000.0).arg((tailPTS0 - headPTS) / 1000000.0);
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
			docx.newLine(1);	
			str = QString("t=%1-%2=%3 s").arg(tailPTS1 / 1000000.0).arg(headPTS / 1000000.0).arg((tailPTS1 - headPTS) / 1000000.0);
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

			docx.newLine(1);
			docx.setTextAlign(CDocx::AliginLeftAndRight);
			str = QString("将参数代入上式整理计算得：");
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

			docx.newLine(1);
			docx.setTextAlign(CDocx::AliginCenter);
			
			str = QString("v= l/t > %1/%2 = %3 m/s ≈ %4 km/h").arg(L).arg(t0).arg(v0 / 3.6).arg(QString::number(v0, 'f', 2));
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

			docx.newLine(1);
			str = QString("v= l/t < %1/%2 = %3 m/s ≈ %4 km/h").arg(L).arg(t1).arg(v1 / 3.6).arg(QString::number(v1, 'f', 2));
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

			docx.newLine(1);
			docx.setTextAlign(CDocx::AliginLeftAndRight);
			str = QString("根据以上分析计算，被鉴定车辆通过上述参考点A时的速度约为%1-%2km/h。").arg(QString::number(v0, 'f', 2)).arg(QString::number(v1, 'f', 2));
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
		}
		else {
			docx.newLine(1);
			docx.setTextAlign(CDocx::AliginCenter);
			str = QString("t=%1-%2=%3 s").arg(tailPTS0 / 1000000.0).arg(headPTS / 1000000.0).arg((tailPTS0 - headPTS) / 1000000.0);
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

			docx.newLine(1);
			str = QString("v= l/t = %1/%2 = %3 m/s ≈ %4 km/h").arg(L).arg(t0).arg(v0 / 3.6).arg(QString::number(v0, 'f', 2));
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

			docx.newLine(1);
			docx.setTextAlign(CDocx::AliginLeftAndRight);
			str = QString("根据以上分析计算，被鉴定车辆通过上述参考点A时的速度约为%1km/h。").arg(QString::number(v0, 'f', 2));
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
		}
	}
	else if (proInfo.timeMode == TIME_MODE_FPS) {
		v0 = proInfo.v02;
		v1 = proInfo.v12;
		
		VTFPS2AverageTime(proInfo, frameDuration);
		frameDuration /= 1000000.0;
		t0 = (frameDuration > 0) ? ((proInfo.tailIndex0 - proInfo.headIndex) * frameDuration) : 0;
		t1 = (frameDuration > 0) ? ((proInfo.tailIndex1 - proInfo.headIndex) * frameDuration) : 0;
		frameRate = proInfo.fps;

		if (proInfo.tailIndex0 > 0 && proInfo.tailIndex1 > 0) {
			docx.newLine(1);
			docx.setTextAlign(CDocx::AliginCenter);
			str = QString("t =(%1-%2)/%3=%4 s").arg(tailIndex0).arg(headIndex).arg(frameRate).arg(t0);
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

			docx.newLine(1);
			str = QString("t =(%1-%2)/%3=%4 s").arg(tailIndex1).arg(headIndex).arg(frameRate).arg(t1);
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

			docx.newLine(1);
			docx.setTextAlign(CDocx::AliginLeftAndRight);
			str = QString("将参数代入上式整理计算得：");
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

			docx.newLine(1);
			docx.setTextAlign(CDocx::AliginCenter);
			str = QString("v= l/t > %1/%2 = %3 m/s ≈ %4 km/h").arg(L).arg(t0).arg(v0 / 3.6).arg(QString::number(v0, 'f', 2));
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

			docx.newLine(1);
			str = QString("v= l/t < %1/%2 = %3 m/s ≈ %4 km/h").arg(L).arg(t1).arg(v1 / 3.6).arg(QString::number(v1, 'f', 2));
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

			docx.newLine(1);
			docx.setTextAlign(CDocx::AliginLeftAndRight);
			str = QString("根据以上分析计算，被鉴定车辆通过上述参考点A时的速度约为%1-%2km/h。").arg(QString::number(v0, 'f', 2)).arg(QString::number(v1, 'f', 2));
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
		}
		else {
			docx.newLine(1);
			docx.setTextAlign(CDocx::AliginCenter);
			str = QString("t =(%1-%2)/%3=%4 s").arg(tailIndex0).arg(headIndex).arg(frameRate).arg(t0);
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

			docx.newLine(1);
			str = QString("v= l/t = %1/%2 = %3 m/s ≈ %4 km/h").arg(L).arg(t0).arg(v0 / 3.6).arg(v0);
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

			docx.newLine(1);
			docx.setTextAlign(CDocx::AliginLeftAndRight);
			str = QString("根据以上分析计算，被鉴定车辆通过上述参考点A时的速度约为%1km/h。").arg(QString::number(v0, 'f', 2));
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
		}
	}
	else if (proInfo.timeMode == TIME_MODE_WATER_MARK) {
		v0 = proInfo.v03;
		v1 = proInfo.v13;

		VTWT2AverageTime(proInfo, frameDuration, frameRate);
		frameDuration /= 1000000.0;
		t0 = (frameDuration > 0) ? ((proInfo.tailIndex0 - proInfo.headIndex) * frameDuration) : 0;
		t1 = (frameDuration > 0) ? ((proInfo.tailIndex1 - proInfo.headIndex) * frameDuration) : 0;

		QString frameOfSec0;//25+25+7=57
		QString frameOfSec1;//25+25+7=57
		QString tailHMS;
		qDebug() << "proInfo.tailIndex0 - proInfo.headIndex=" << proInfo.tailIndex0 - proInfo.headIndex;
		qDebug() << "proInfo.tailIndex1 - proInfo.headIndex=" << proInfo.tailIndex1 - proInfo.headIndex;

		if (proInfo.headIndex >= 0 && proInfo.tailIndex0 >= 0 && proInfo.tailIndex0 < proInfo.frameInfos.size()) {
			int count = 0;
			tailHMS = proInfo.frameInfos[proInfo.headIndex].timeHMS;
			for (int m = proInfo.headIndex; m < proInfo.tailIndex0; m++) {
				count++;
				if (tailHMS != proInfo.frameInfos[m].timeHMS) {
					if(!frameOfSec0.isEmpty())
						frameOfSec0 += "+";
					frameOfSec0 += QString::number(count);
					tailHMS = proInfo.frameInfos[m].timeHMS;
					count = 0;
				}
			}
			if (count > 0) {
				if (!frameOfSec0.isEmpty())
					frameOfSec0 += "+";
				frameOfSec0 += QString::number(count);
			}
		}

		if (proInfo.headIndex >= 0 && proInfo.tailIndex1 >= 0 && proInfo.tailIndex1 < proInfo.frameInfos.size()) {
			int count = 0;
			tailHMS = proInfo.frameInfos[proInfo.headIndex].timeHMS;
			for (int m = proInfo.headIndex; m < proInfo.tailIndex1; m++) {
				count++;
				if (tailHMS != proInfo.frameInfos[m].timeHMS) {
					if (!frameOfSec1.isEmpty())
						frameOfSec1 += "+";
					frameOfSec1 += QString::number(count);
					count = 0;
					tailHMS = proInfo.frameInfos[m].timeHMS;
				}
			}
			if (count > 0) {
				if (!frameOfSec1.isEmpty())
					frameOfSec1 += "+";
				frameOfSec1 += QString::number(count);
			}
		}

		if (proInfo.tailIndex0 > 0 && proInfo.tailIndex1 > 0) {
			docx.newLine(1);
			docx.setTextAlign(CDocx::AliginCenter);
			if(-1 != frameOfSec1.lastIndexOf("+"))
				str = QString("t =(%1)/%2=%3 s").arg(frameOfSec0).arg(frameRate).arg(t0);
			else
				str = QString("t =%1/%2=%3 s").arg(frameOfSec0).arg(frameRate).arg(t0);

			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

			docx.newLine(1);
			if (-1 != frameOfSec1.lastIndexOf("+"))
				str = QString("t =(%1)/%2=%3 s").arg(frameOfSec1).arg(frameRate).arg(t1);
			else
				str = QString("t =%1/%2=%3 s").arg(frameOfSec1).arg(frameRate).arg(t1);

			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

			docx.newLine(1);
			docx.setTextAlign(CDocx::AliginLeftAndRight);
			str = QString("将参数代入上式整理计算得：");
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

			docx.newLine(1);
			docx.setTextAlign(CDocx::AliginCenter);
			str = QString("v= l/t > %1/%2 = %3 m/s ≈ %4 km/h").arg(L).arg(t0).arg(v0 / 3.6).arg(QString::number(v0, 'f', 2));
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

			docx.newLine(1);
			str = QString("v= l/t < %1/%2 = %3 m/s ≈ %4 km/h").arg(L).arg(t1).arg(v1 / 3.6).arg(QString::number(v1, 'f', 2));
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

			docx.newLine(1);
			docx.setTextAlign(CDocx::AliginLeftAndRight);
			str = QString("根据以上分析计算，被鉴定车辆通过上述参考点A时的速度约为%1-%2km/h。").arg(QString::number(v0, 'f', 2)).arg(QString::number(v1, 'f', 2));
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
		}
		else {
			docx.newLine(1);
			docx.setTextAlign(CDocx::AliginCenter);
			if (-1 != frameOfSec1.lastIndexOf("+"))
				str = QString("t =(%1)/%2=%3 s").arg(frameOfSec0).arg(frameRate).arg(t0);
			else
				str = QString("t =%1/%2=%3 s").arg(frameOfSec0).arg(frameRate).arg(t0);
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

			docx.newLine(1);
			str = QString("v= l/t = %1/%2 = %3 m/s ≈ %4 km/h").arg(L).arg(t0).arg(v0 / 3.6).arg(QString::number(v0, 'f', 2));
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

			docx.newLine(1);
			docx.setTextAlign(CDocx::AliginLeftAndRight);
			str = QString("根据以上分析计算，被鉴定车辆通过上述参考点A时的速度约为%1km/h。").arg(QString::number(v0, 'f', 2));
			docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
		}
	}
}

static void Generate5(VTProjectInfo &proInfo, CDocx &docx)
{
	QString str;

	qint64 headPTS = 0;
	qint64 tailPTS0 = 0;
	qint64 tailPTS1 = 0;
	int headIndex = 0;
	int tailIndex0 = 0;
	int tailIndex1 = 0;

	QString headHMS;
	QString tailHMS0;
	QString tailHMS1;
	int headHMSIndex = 0;
	int tailHMS0Index = 0;
	int tailHMS1Index = 0;
	QString imageTmpPath = QString("%1/0.jpg").arg(CWordReport::m_imageTemp);

	docx.newLine(1);
	docx.ParagraphFormat(0, 0, 0, CDocx::AliginLeftAndRight);

	if (proInfo.headIndex >= 0 && proInfo.headIndex < proInfo.frameInfos.size()) {
		headIndex = proInfo.frameInfos[proInfo.headIndex].index;
		headPTS = proInfo.frameInfos[proInfo.headIndex].pts;

		headHMS = proInfo.frameInfos[proInfo.headIndex].timeHMS;
		for (int m = proInfo.headIndex; m >= 0; m--) {
			headHMSIndex++;
			if (headHMS != proInfo.frameInfos[m].timeHMS)
				break;
		}
	}

	if (proInfo.tailIndex0 >= 0 && proInfo.tailIndex0 < proInfo.frameInfos.size()) {
		tailIndex0 = proInfo.frameInfos[proInfo.tailIndex0].index;
		tailPTS0 = proInfo.frameInfos[proInfo.tailIndex0].pts;

		tailHMS0 = proInfo.frameInfos[proInfo.tailIndex0].timeHMS;
		for (int m = proInfo.tailIndex0; m >= 0; m--) {
			tailHMS0Index++;
			if (tailHMS0 != proInfo.frameInfos[m].timeHMS)
				break;
		}
	}

	if (proInfo.tailIndex1 >= 0 && proInfo.tailIndex1 < proInfo.frameInfos.size()) {
		tailIndex1 = proInfo.frameInfos[proInfo.tailIndex1].index;
		tailPTS1 = proInfo.frameInfos[proInfo.tailIndex1].pts;

		tailHMS1 = proInfo.frameInfos[proInfo.tailIndex1].timeHMS;
		for (int m = proInfo.tailIndex1; m >= 0; m--) {
			tailHMS1Index++;
			if (tailHMS1 != proInfo.frameInfos[m].timeHMS)
				break;
		}
	}

	docx.ParagraphFormat(0, 0, 0, CDocx::AliginLeftAndRight);
	docx.newLine(1);
	docx.setTextStyle(45);
	docx.ApplyListTemplate(1, docx.m_numFormatSeq1++);

	str = QString("鉴定意见");
	docx.setContext(str, CWordReport::m_heiTi, QColor(0, 0, 0), 14);

	docx.ParagraphFormat(0, 0, 0, CDocx::AliginLeftAndRight);
	docx.newLine(1);
	docx.setTextStyle(45);
	docx.ParagraphFormat(0, 0, 2, CDocx::AliginLeftAndRight);

	float v0 = 0.0;
	float v1 = 0.0;
	if (proInfo.timeMode == TIME_MODE_PTS) {
		v0 = proInfo.v01;
		v1 = proInfo.v11;
	}
	else if (proInfo.timeMode == TIME_MODE_FPS) {
		v0 = proInfo.v02;
		v1 = proInfo.v12;
	}
	else if (proInfo.timeMode == TIME_MODE_WATER_MARK) {
		v0 = proInfo.v03;
		v1 = proInfo.v13;
	}

	if (v1 <= 0.0) {
		str = QString("%1%2牌%3轿车事发时的速度约为%4km/h可以成立。")
			.arg(proInfo.licensePlateNumber)
			.arg(proInfo.vehicleBrand)
			.arg(proInfo.vehicleType)
			.arg(QString::number(v0, 'f', 0));
	}
	else {
		str = QString("%1%2牌%3轿车事发时的速度约为%4-%5km/h可以成立。")
			.arg(proInfo.licensePlateNumber)
			.arg(proInfo.vehicleBrand)
			.arg(proInfo.vehicleType)
			.arg(QString::number(v0, 'f', 0))
			.arg(QString::number(v1, 'f', 0));
	}

	docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

	QString strappraiserName = proInfo.appraiser1.name;
	if (strappraiserName.size() == 2)
		strappraiserName.insert(1, "  ");
	
	docx.newLine(1);
	docx.setTextAlign(CDocx::AliginRight);
	docx.ParagraphFormat(0, 5, 0, CDocx::AliginRight);
	str = QString("鉴定人：%1").arg(strappraiserName);
	docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

	docx.newLine(1);
	docx.setTextAlign(CDocx::AliginRight);
	str = QString("《司法鉴定人执业证》证号：%1").arg(proInfo.appraiser1.id);
	docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

	strappraiserName = proInfo.appraiser2.name;
	if (strappraiserName.size() == 2)
		strappraiserName.insert(1, "  ");
	
	docx.newLine(1);
	docx.setTextAlign(CDocx::AliginRight);
	str = QString("鉴定人：%1").arg(strappraiserName);
	docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

	docx.newLine(1);
	docx.setTextAlign(CDocx::AliginRight);
	
	str = QString("《司法鉴定人执业证》证号：%1").arg(proInfo.appraiser2.id);
	docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

	docx.newLine(2);
	docx.setTextAlign(CDocx::AliginRight);

	QDate currentDate = QDate::currentDate();
	str = VTDate2Chinese(currentDate);
	docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);

	docx.newLine(1);
	docx.ParagraphFormat(0, 0, 0, CDocx::AliginLeftAndRight);

	docx.changePage();
	docx.ParagraphFormat(0, 0, 0, CDocx::AliginLeftAndRight);


	int index = -1;
	index = proInfo.headIndex;

	VTClearFiles(CWordReport::m_imageTemp);

	if (index >= 0) {
		docx.newLine(1);
		docx.ParagraphFormat(0, 0, 0, CDocx::AliginLeftAndRight);
		QImage image;
		if (image.load(proInfo.frameInfos[index].imagePath)) {
			CTakenBG::drawResoutImage(image, proInfo, index);
			image.save(imageTmpPath, nullptr, 100);
			//docx.addPic(imageTmpPath, 340.157471);
			docx.addPic(imageTmpPath, 11 * 28.3465, 6 * 28.3465);
		}
		else
			qDebug() << "word report load image " << proInfo.frameInfos[index].imagePath << " faild";

		if (proInfo.timeMode == TIME_MODE_WATER_MARK)
			str = QString("%1第%2帧图像").arg(headHMS).arg(headHMSIndex);
		else
			str = QString("第%1帧图像").arg(proInfo.frameInfos[index].index);
		
		docx.newLine(1);
		docx.setTextAlign(CDocx::AliginCenter);
		docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
	}


	index = proInfo.tailIndex1;
	if (index >= 0) {
		docx.newLine(1);
		docx.ParagraphFormat(0, 0, 0, CDocx::AliginLeftAndRight);
		QImage image;
		if (image.load(proInfo.frameInfos[index].imagePath)) {
			CTakenBG::drawResoutImage(image, proInfo, index);
			image.save(imageTmpPath, nullptr, 100);
			//docx.addPic(imageTmpPath, 340.157471);
			docx.addPic(imageTmpPath, 11 * 28.3465, 6 * 28.3465);
		}
		else
			qDebug() << "word report load image " << proInfo.frameInfos[index].imagePath << " faild";

		if (proInfo.timeMode == TIME_MODE_WATER_MARK)
			str = QString("%1第%2帧图像").arg(tailHMS1).arg(tailHMS1Index);
		else
			str = QString("第%1帧图像").arg(proInfo.frameInfos[index].index);

		docx.newLine(1);
		docx.setTextAlign(CDocx::AliginCenter);
		docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
	}


	index = proInfo.tailIndex0;
	if (index >= 0) {
		docx.newLine(1);
		docx.ParagraphFormat(0, 0, 0, CDocx::AliginLeftAndRight);
		QImage image;
		if (image.load(proInfo.frameInfos[index].imagePath)) {
			CTakenBG::drawResoutImage(image, proInfo, index);
			image.save(imageTmpPath, nullptr, 100);
			//docx.addPic(imageTmpPath, 340.157471);
			docx.addPic(imageTmpPath, 11 * 28.3465, 6 * 28.3465);
		}
		else
			qDebug() << "word report load image " << proInfo.frameInfos[index].imagePath << " faild";

		if (proInfo.timeMode == TIME_MODE_WATER_MARK)
			str = QString("%1第%2帧图像").arg(tailHMS0).arg(tailHMS0Index);
		else
			str = QString("第%1帧图像").arg(proInfo.frameInfos[index].index);

		docx.newLine(1);
		docx.setTextAlign(CDocx::AliginCenter);
		docx.setContext(str, CWordReport::m_fangSong, QColor(0, 0, 0), 14);
	}

	docx.changePage();
	docx.ParagraphFormat(0, 0, 0, CDocx::AliginLeftAndRight);

	QMap<QString, VTAppraiserInfo> info = CAppraiserDialog::getInfo();
	QMap<QString, VTAppraiserInfo>::iterator iter = info.find(proInfo.appraiser1.id);
	if (iter != info.end())
		docx.addPic(iter.value().photoPathTmp, 7 * 28.3465, 9.5 * 28.3465);
	else
		qDebug() << QString("WordReport:找不到 鉴定人%1 鉴定执业证号%2").arg(proInfo.appraiser1.name).arg(proInfo.appraiser1.id);

	iter = info.find(proInfo.appraiser2.id);
	if (iter != info.end())
		docx.addPic(iter.value().photoPathTmp, 7 * 28.3465, 9.5 * 28.3465);
	else
		qDebug() << QString("WordReport:找不到 鉴定人%1 鉴定执业证号%2").arg(proInfo.appraiser2.name).arg(proInfo.appraiser2.id);
}

void CWordReport::generateReport(tagWordInfo& wordInfo, CWordReport* obj) {
	VTProjectInfo proInfo = wordInfo.proInfo;
	QString savePath = wordInfo.filePath;
	QJsonObject json;

	json["accidentTime"] = proInfo.accidentTime;                 //事故发生时间
	json["accidentLocation"] = proInfo.accidentLocation;		//事故地点
	json["licensePlateNumber"] = proInfo.licensePlateNumber;	//车辆号牌
	json["vehicleBrand"] = proInfo.vehicleBrand;				//车辆品牌
	json["vehicleType"] = proInfo.vehicleType;					//车辆类型
	json["vehicleVIN"] = proInfo.vehicleVIN;					//车辆识别代码VIN
	json["referenceDesFrom"] = proInfo.referenceDesFrom;		//参照物描述
	json["referenceDesTo"] = proInfo.referenceDesTo;				//参照物描述
	json["fps"] = proInfo.fps;
	json["client"] = proInfo.client;								//委托人
	json["dateOfCommission"] = proInfo.dateOfCommission;			//委托日期
	json["commitments"] = proInfo.commitments;						//委托事项
	json["appraisalDate"] = proInfo.appraisalDate;					// 鉴定日期
	json["appraisalLocation"] = proInfo.appraisalLocation;			//鉴定地点
	json["appraisalMaterial"] = proInfo.appraisalMaterial;			//鉴定材料
	QCryptographicHash::Algorithm hashAlgo = proInfo.hashAlgo;
	QMetaEnum metaEnum = QMetaEnum::fromType<QCryptographicHash::Algorithm>();
	QString hashAlgoString = metaEnum.valueToKey(hashAlgo);
	if (hashAlgoString == "Md5") {
		hashAlgoString = "MD5";
	}
	else if(hashAlgoString == "Sha1") {
		hashAlgoString = "SHA1";
	}
	else if (hashAlgoString == "Sha256") {
		hashAlgoString = "SHA256";
	}
	else if (hashAlgoString == "Sha512") {
		hashAlgoString = "SHA512";
	}
	else {
		hashAlgoString = "SHA256";
	}
	
	json["hashName"] = hashAlgoString;								//hash名
	QString hashAlgoVal;
	if (proInfo.mapHashInfo.find(proInfo.hashAlgo) != proInfo.mapHashInfo.end()) {
		QByteArray data = proInfo.mapHashInfo[proInfo.hashAlgo];
		if (!data.isEmpty())
			hashAlgoVal = QString(data.toHex().toStdString().c_str());
	}
	else {
		QByteArray data;
		if (VTFileHash(proInfo.videoPath, proInfo.fileSize, proInfo.hashAlgo, data)) {
			if (!data.isEmpty()) {
				hashAlgoVal = QString(data.toHex().toStdString().c_str());
				proInfo.mapHashInfo[proInfo.hashAlgo] = data;
			}
		}
	}
	json["hashVal"] = hashAlgoVal;									 //hash值
	json["appraiser1"] = proInfo.appraiser1.name;					//第一鉴定人
	json["idOfAppraiser1"] = proInfo.appraiser1.id;					//第一鉴定人号码
	json["appraiser2"] = proInfo.appraiser2.name;					//第二鉴定人
	json["idOfAppraiser2"] = proInfo.appraiser2.id;					//第二鉴定人号码
	json["v01"] = proInfo.v01;							//速度（视频帧时间戳PTS）
	json["v02"] = proInfo.v02;							//速度（视频平均帧率FPS）
	json["v03"] = proInfo.v03;							//速度（视频水印时间）
	json["v11"] = proInfo.v11;							//速度（视频帧时间戳PTS）
	json["v12"] = proInfo.v12;							//速度（视频平均帧率FPS）
	json["v13"] = proInfo.v13;							//速度（视频水印时间）
	json["fileName"] = QFileInfo(proInfo.videoPath).fileName();			// 文件名
	json["timeMode"] = proInfo.timeMode;
	json["tailIndex0"] = proInfo.tailIndex0;
	json["tailIndex1"] = proInfo.tailIndex1;

	float L = sqrtf((proInfo.actualPoints[0].x() - proInfo.actualPoints[1].x()) * (proInfo.actualPoints[0].x() - proInfo.actualPoints[1].x())
		- (proInfo.actualPoints[0].y() - proInfo.actualPoints[1].x()) * (proInfo.actualPoints[0].x() - proInfo.actualPoints[1].y()));
	json["distance"] = L*100;

	QString strOCRText = "";
	if (proInfo.firstImage != nullptr) {
		for (auto& waterMark : proInfo.firstImage->waterMarks) {
			if (waterMark.text.isEmpty())
				continue;
			if (!strOCRText.isEmpty())
				strOCRText += QString("、");
			strOCRText += QString("“");
			strOCRText += waterMark.text;
			strOCRText += QString("”");
		}
	}
	json["OCRText"] = strOCRText;

	QString strStandard;
	std::set<VTAppraisalStandard> tmp(proInfo.appraisalStandards.begin(), proInfo.appraisalStandards.end());
	for (auto& standard : tmp) {
		if (!strStandard.isEmpty())
			strStandard.append(QString("、"));
		strStandard.append(standard.id);
		strStandard.append(standard.name);
	}
	json["standard"] = strStandard;    // 标准

	json["savePath"] = savePath;

	int index = -1;
	QString imagePath;
	index = proInfo.headIndex;
	VTClearFiles(CWordReport::m_imageTemp);
	if (index >= 0) {
		imagePath = proInfo.frameInfos[index].imagePath;
	}
	QImage image;
	QString mod_imagePath;
	if (image.load(imagePath)) {
		CTakenBG::drawResoutImage(image, proInfo, index);
		QFileInfo fileInfo(imagePath);            // 提取路径信息
		mod_imagePath = QString("%1/%2_mod.%3")
			.arg(fileInfo.path())           // 文件所在目录
			.arg(fileInfo.baseName())       // 不带后缀的文件名
			.arg(fileInfo.suffix());
		image.save(mod_imagePath, nullptr, 100);
	}
	json["image1Path"] = mod_imagePath;

	index = proInfo.tailIndex1;
	if (index >= 0) {
		imagePath = proInfo.frameInfos[index].imagePath;
	}
	if (image.load(imagePath)) {
		CTakenBG::drawResoutImage(image, proInfo, index);
		QFileInfo fileInfo(imagePath);            // 提取路径信息
		mod_imagePath = QString("%1/%2_mod.%3")
			.arg(fileInfo.path())           // 文件所在目录
			.arg(fileInfo.baseName())       // 不带后缀的文件名
			.arg(fileInfo.suffix());
		image.save(mod_imagePath, nullptr, 100);
	}
	json["image2Path"] = mod_imagePath;

	index = proInfo.tailIndex0;
	if (index >= 0) {
		imagePath = proInfo.frameInfos[index].imagePath;
	}
	if (image.load(imagePath)) {
		CTakenBG::drawResoutImage(image, proInfo, index);
		QFileInfo fileInfo(imagePath);            // 提取路径信息
		mod_imagePath = QString("%1/%2_mod.%3")
			.arg(fileInfo.path())           // 文件所在目录
			.arg(fileInfo.baseName())       // 不带后缀的文件名
			.arg(fileInfo.suffix());
		image.save(mod_imagePath, nullptr, 100);
	}
	json["image3Path"] = mod_imagePath;

	QString headHMS;
	int headHMSIndex = 0;
	if (proInfo.headIndex >= 0 && proInfo.headIndex < proInfo.frameInfos.size()) {
		headHMS = proInfo.frameInfos[proInfo.headIndex].timeHMS;
		for (int m = proInfo.headIndex; m >= 0; m--) {
			headHMSIndex++;
			if (headHMS != proInfo.frameInfos[m].timeHMS)
				break;
		}
	}
	json["image1HMS"] = headHMS;
	json["image1HMSIndex"] = headHMSIndex;

	QString tailHMS1;
	int tailHMS1Index = 0;
	if (proInfo.tailIndex1 >= 0 && proInfo.tailIndex1 < proInfo.frameInfos.size()) {
		tailHMS1 = proInfo.frameInfos[proInfo.tailIndex1].timeHMS;
		for (int m = proInfo.tailIndex1; m >= 0; m--) {
			tailHMS1Index++;
			if (tailHMS1 != proInfo.frameInfos[m].timeHMS)
				break;
		}
	}
	json["image2HMS"] = tailHMS1;
	json["image2HMSIndex"] = tailHMS1Index;

	QString tailHMS0;
	int tailHMS0Index = 0;
	if (proInfo.tailIndex0 >= 0 && proInfo.tailIndex0 < proInfo.frameInfos.size()) {
		tailHMS0 = proInfo.frameInfos[proInfo.tailIndex0].timeHMS;
		for (int m = proInfo.tailIndex0; m >= 0; m--) {
			tailHMS0Index++;
			if (tailHMS0 != proInfo.frameInfos[m].timeHMS)
				break;
		}
	}
	json["image3HMS"] = tailHMS0;
	json["image3HMSIndex"] = tailHMS0Index;

	int headIndex = 0;
	int headPTS = 0;
	int tailIndex1 = 0;
	int tailPTS1 = 0;
	int tailIndex0 = 0;
	int tailPTS0 = 0;
	headIndex = proInfo.frameInfos[proInfo.headIndex].index;
	headPTS = proInfo.frameInfos[proInfo.headIndex].pts;
	tailIndex1 = proInfo.frameInfos[proInfo.tailIndex1].index;
	tailPTS1 = proInfo.frameInfos[proInfo.tailIndex1].pts;
	tailIndex0 = proInfo.frameInfos[proInfo.tailIndex0].index;
	tailPTS0 = proInfo.frameInfos[proInfo.tailIndex0].pts;

	QString str;

	if (proInfo.timeMode == TIME_MODE_PTS) {
		str = QString("在总第%1帧图像上，设与被鉴定车辆%2重合的空间中的一点为参考点A，该图像的显示时间戳(PTS)为%3；")
			.arg(headIndex).arg(proInfo.referenceDesFrom).arg(formatTime(headPTS));
		json["content1"] = str;

		if (proInfo.tailIndex0 > 0 && proInfo.tailIndex1 > 0) {
			str = QString("在总第%1帧图像上，被鉴定车辆%2尚未达参考点A所在位置，该图像的显示时间戳(PTS)为%3；")
				.arg(tailIndex1).arg(proInfo.referenceDesTo).arg(formatTime(tailPTS1));
			json["content2"] = str;

			str = QString("在总第%1帧图像上，被鉴定车辆%2超过参考点A所在位置，该图像的显示时间戳(PTS)为%3；")
				.arg(tailIndex0).arg(proInfo.referenceDesTo).arg(formatTime(tailPTS0));
			json["content3"] = str;
			json["content4"] = "";
			json["content5"] = "";
		}
		else {
			str = QString("在总第%1帧图像上，被鉴定车辆%2近似到达参考点A所在位置，该图像的显示时间戳(PTS)为%3；")
				.arg(tailIndex0).arg(proInfo.referenceDesTo).arg(formatTime(tailPTS0));
			json["content2"] = str;
			json["content3"] = "";
			json["content4"] = "";
			json["content5"] = "";
		}
	}
	else if (proInfo.timeMode == TIME_MODE_FPS) {
		str = QString("在总第%1帧图像上，设与被鉴定车辆%2重合的空间中的一点为参考点A；")
			.arg(headIndex).arg(proInfo.referenceDesFrom);
		json["content1"] = str;
		if (proInfo.tailIndex0 > 0 && proInfo.tailIndex1 > 0) {
			str = QString("在总第%1帧图像上，被鉴定车辆%2尚未达参考点A所在位置；")
				.arg(tailIndex1).arg(proInfo.referenceDesTo);
			json["content2"] = str;

			str = QString("在总第%1帧图像上，被鉴定车辆%2超过参考点A所在位置；")
				.arg(tailIndex0).arg(proInfo.referenceDesTo);
			json["content3"] = str;
			json["content4"] = "";
			json["content5"] = "";
		}
		else {
			str = QString("在总第%1帧图像上，被鉴定车辆%2近似到达参考点A所在位置；")
				.arg(tailIndex0).arg(proInfo.referenceDesTo);
			json["content2"] = str;
		}

		str = QString("视频帧率为%1fps。")
			.arg(proInfo.fps);
		json["content3"] = str;
		json["content4"] = "";
		json["content5"] = "";
	}
	else if (proInfo.timeMode == TIME_MODE_WATER_MARK) {
		str = QString("在%1第%2帧图像上，设与被鉴定车辆%3重合的空间中的一点为参考点A；")
			.arg(headHMS).arg(headHMSIndex).arg(proInfo.referenceDesFrom);
		json["content1"] = str;

		int nTailCount0 = 0, nTailCount1 = 0;
		for (auto& frameInfo : proInfo.frameInfos) {
			if (tailHMS0 != "" && tailHMS0 == frameInfo.timeHMS)
				nTailCount0++;
			else if (tailHMS1 != "" && tailHMS0 != tailHMS1 && tailHMS1 == frameInfo.timeHMS)
				nTailCount1++;
		}

		if (proInfo.tailIndex0 > 0 && proInfo.tailIndex1 > 0) {
			str = QString("在%1第%2帧图像上，被鉴定车辆%3尚未达参考点A所在位置；")
				.arg(tailHMS1).arg(tailHMS1Index).arg(proInfo.referenceDesTo);
			json["content2"] = str;

			str = QString("在%1第%2帧图像上，被鉴定车辆%3超过参考点A所在位置；")
				.arg(tailHMS0).arg(tailHMS0Index).arg(proInfo.referenceDesTo);
			json["content3"] = str;

			if (nTailCount1 > 0) {
				str = QString("%1共有%2帧图像；")
					.arg(tailHMS0).arg(nTailCount1);
				json["content4"] = str;
			}

			if (nTailCount0 > 0) {
				str = QString("%1共有%2帧图像；")
					.arg(tailHMS0).arg(nTailCount0);
				json["content5"] = str;
			}
		}
		else {
			str = QString("在%1第%2帧图像上，被鉴定车辆%3近似到达参考点A所在位置；")
				.arg(tailHMS0).arg(tailHMS0Index).arg(proInfo.referenceDesTo);
			json["content2"] = str;

			str = QString("%1共有%2帧图像；")
				.arg(tailHMS0).arg(nTailCount0);
			json["content3"] = str;
			json["content4"] = "";
			json["content5"] = "";
		}
	}

	QString des1 = "";
	QString des2 = "";
	if (proInfo.mode == FIXED_CAM_ONESELF) {
		des1 = QString("被鉴定车辆");
		des2 = QString("车辆识别代号为%1。经测量，被鉴定车辆%2至%3的距离约为%4cm。").arg(proInfo.vehicleVIN).arg(proInfo.referenceDesFrom).arg(proInfo.referenceDesTo).arg(L*100);
	}
	else if (proInfo.mode == FIXED_CAM_GROUND_REFERENCE) {
		des1 = QString("事故地点复勘");
		des2 = QString("根据上述视频检验内容，经在事故地点测量得到,%1至%2的距离约为%3cm。").arg(proInfo.referenceDesFrom).arg(proInfo.referenceDesTo).arg(L * 100);
	}
	json["eval_mode"] = des1;
	json["content6"] = des2;

	QJsonDocument doc(json);
	QString docToJson = doc.toJson(QJsonDocument::Compact);

	QProcess process;
	QStringList arguments;
	//QString program = "python";

	// 获取 Qt 程序所在目录，确保路径一致
	QString appDir = QCoreApplication::applicationDirPath();
	QString program = QDir::cleanPath(appDir + QDir::separator() + "wordMod.exe");
	//QString pythonPath = "C:/Users/fecwo/Desktop/wordMod.py";

	//arguments << pythonPath << docToJson;
	arguments  << docToJson;  // Python 脚本和json

	// 启动 Python 脚本
	process.start(program, arguments);
	emit obj->sigProcessBar(50);
 
	// 等待脚本完成
	process.waitForFinished();
	emit obj->sigProcessBar(100);

	// 获取输出和错误信息
	QString output = process.readAllStandardOutput();
	QString error = process.readAllStandardError();

	qDebug() << "Output:" << output;
	qDebug() << "Error:" << error;

}

 /*void CWordReport::generateReport(tagWordInfo& wordInfo, CWordReport* obj)
{
	QString str;
	CDocx docx(obj);
	VTProjectInfo proInfo = wordInfo.proInfo;
	if (!docx.openNewWord(false)) {
		emit obj->sigProcessBar(100);
		VTMessageBoxOk(QString("生成报告文档失败！"), windowFlags(), QMessageBox::Information);
		return;
	}
	int num = 0;

	GenerateTitle(proInfo, docx);
	emit obj->sigProcessBar(5);

	Generate1(proInfo, docx);
	num = rand() % 10;
	emit obj->sigProcessBar(10 + num);

	Generate2(proInfo, docx);
	num = rand() % 10;
	emit obj->sigProcessBar(20 + num);

	Generate3_1(proInfo, docx);
	num = rand() % 10;
	emit obj->sigProcessBar(40 + num);
	
	Generate3_2(proInfo, docx);
	num = rand() % 10;
	emit obj->sigProcessBar(60 + num);

	Generate4(proInfo, docx);
	num = rand() % 10;
	emit obj->sigProcessBar(80 + num);

	Generate5(proInfo, docx);
	GenerateHeaderFooter(proInfo, docx);
	emit obj->sigProcessBar(100);
	
	docx.quitWord();
} */
