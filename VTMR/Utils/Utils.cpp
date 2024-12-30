#include "Utils.h"
#include <QDir>
#include <QDirIterator>
#include <QCryptographicHash>
#include <QDebug>
#include <QJsonParseError>
#include <QJsonObject>
#include "SerializeData.h"
#include <QMessageBox>
#include <QVector>
#include <QStringConverter>
#include "vtmr.h"
#include <QGuiApplication>
#include <QScreen>
#include <QStandardPaths>
#include <Windows.h>
#include "Global.h"
#undef max
#undef min

string VTQtStr2StdStr(const QString qstr)
{
	QByteArray cdata = qstr.toLocal8Bit();
	return string(cdata);
}

void VTClearFiles(const QString& temp_path)
{
	QDir dir(temp_path);
	if (dir.isEmpty())
	{
		return;
	}
	QStringList filter; //过滤器
	filter.append("*");
	QDirIterator it(temp_path, filter, QDir::Files, QDirIterator::Subdirectories);
	while (it.hasNext()) { //若容器中还有成员，继续执行删除操作
		dir.remove(it.next());
	}
}

int VTGetJsonFiles(QString &strDir, QStringList &list)
{
	QDir dir(strDir);
	if (!dir.exists())
		return -1;
	dir.setFilter(QDir::Files | QDir::NoSymLinks);
	QStringList filters;
	filters << "*.vtmr";
	dir.setNameFilters(filters);
	list = dir.entryList();
	if (list.count() <= 0)
		return -2;
	return 0;
}

int VTReadFile2Project(QString filePath, VTProjectInfo &proInfo)
{
	QFile file(filePath);
	if (!file.open(QFile::ReadOnly | QIODevice::Text))
	{
		qDebug() << "json opened fail";
		return -1;
	}
	QString  val = file.readAll();
	file.close();
	//qDebug() << val;
	QJsonParseError jsonError;
	QJsonDocument doucmentDecode = QJsonDocument::fromJson(val.toUtf8(), &jsonError);  // 转化为 JSON 文档
	if (!doucmentDecode.isNull() && (jsonError.error == QJsonParseError::NoError)) {  // 解析未发生错误
		if (doucmentDecode.isObject()) { // JSON 文档为对象
			QJsonObject object = doucmentDecode.object();  // 转化为对象
			decodeProjectInfo(object, proInfo);
			return 0;
		}
	}
	return -2;
}
int VTWriteFile4Project(QString filePath, VTProjectInfo &proInfo)
{
	QJsonObject obj;
	encodeProjectInfo(obj, proInfo);

	QJsonDocument document;
	document.setObject(obj);
	QByteArray byteArray = document.toJson(QJsonDocument::Compact);
	QString strJson(byteArray);

	QFile file(filePath);
	if (!file.open(QFile::WriteOnly | QIODevice::Text))
		return -1;
	file.write(document.toJson());
	file.close();

	return 0;
}

int VTDelFileProject(QString uuid)
{
	QString filePath = QString("%1/%2.vtmr").arg(g_proDataPath).arg(uuid);
	QString strDir = QString("%1/%2").arg(g_proDataPath).arg(uuid);
	QDir dir;
	dir.setPath(strDir);
	dir.removeRecursively();

	QFile file(filePath);
	file.remove();
	return 0;
}

QMessageBox::StandardButton VTMessageBoxOk(QString text, Qt::WindowFlags flags, QMessageBox::Icon icon, QWidget* parent/* = nullptr*/)
{
	QMessageBox::StandardButton resBtn = QMessageBox::No;
	QMessageBox box(icon, "", text, QMessageBox::NoButton, parent);
	box.setStandardButtons(QMessageBox::Ok);
	box.setButtonText(QMessageBox::Ok, QString("确 定"));
	return (QMessageBox::StandardButton)box.exec();
}

QMessageBox::StandardButton VTMessageBoxYesNo(QString text, QWidget* parent/* = nullptr*/)
{
	QMessageBox box(parent);
	box.setText(text);
	box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	box.setButtonText(QMessageBox::Yes, QString("   是   "));
	box.setButtonText(QMessageBox::No, QString("   否   "));
	return (QMessageBox::StandardButton)box.exec();
}

QMessageBox::StandardButton VTMessageBoxOkCancel(QString text, Qt::WindowFlags flags, QMessageBox::Icon icon)
{
	QMessageBox::StandardButton resBtn = QMessageBox::No;
	QMessageBox box(icon, "", text);
	box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
	box.setButtonText(QMessageBox::Ok, QString("确 定"));
	box.setButtonText(QMessageBox::Cancel, QString("取 消"));
	box.setWindowFlags(flags | Qt::FramelessWindowHint);
	return (QMessageBox::StandardButton)box.exec();
}

// 对比度调节
void VTAdjustContrast(QImage &Img, int nContrast) {
	if (0 == nContrast)
		return;
	int pixels = Img.width() * Img.height();
	unsigned int *data = (unsigned int *)Img.bits();
	int red, green, blue, nRed, nGreen, nBlue;
	if (nContrast > 0 && nContrast < 100) {
		float param = 1 / (1 - nContrast / 100.0) - 1;
		for (int i = 0; i < pixels; ++i) {
			nRed = qRed(data[i]);
			nGreen = qGreen(data[i]);
			nBlue = qBlue(data[i]);
			red = nRed + (nRed - 127) * param;
			red = (red < 0x00) ? 0x00 : (red > 0xff) ? 0xff : red;
			green = nGreen + (nGreen - 127) * param;
			green = (green < 0x00) ? 0x00 : (green > 0xff) ? 0xff : green;
			blue = nBlue + (nBlue - 127) * param;
			blue = (blue < 0x00) ? 0x00 : (blue > 0xff) ? 0xff : blue;
			data[i] = qRgba(red, green, blue, qAlpha(data[i]));
		}
	}
	else {
		for (int i = 0; i < pixels; ++i) {
			nRed = qRed(data[i]);
			nGreen = qGreen(data[i]);
			nBlue = qBlue(data[i]);
			red = nRed + (nRed - 127) * nContrast / 100.0;
			red = (red < 0x00) ? 0x00 : (red > 0xff) ? 0xff : red;
			green = nGreen + (nGreen - 127) * nContrast / 100.0;
			green = (green < 0x00) ? 0x00 : (green > 0xff) ? 0xff : green;
			blue = nBlue + (nBlue - 127) * nContrast / 100.0;
			blue = (blue < 0x00) ? 0x00 : (blue > 0xff) ? 0xff : blue;
			data[i] = qRgba(red, green, blue, qAlpha(data[i]));
		}
	}
}

// 饱和度调节
void VTAdjustSaturation(QImage &Img, int nSaturation) {
	if (0 == nSaturation)
		return;
	int red, green, blue, nRed, nGreen, nBlue;
	int pixels = Img.width() * Img.height();
	unsigned int *data = (unsigned int *)Img.bits();
	float Increment = nSaturation / 100.0;
	float delta = 0;
	float minVal, maxVal;
	float L, S;
	float alpha;
	for (int i = 0; i < pixels; ++i) {
		nRed = qRed(data[i]);
		nGreen = qGreen(data[i]);
		nBlue = qBlue(data[i]);
		minVal = std::min(std::min(nRed, nGreen), nBlue);
		maxVal = std::max(std::max(nRed, nGreen), nBlue);
		delta = (maxVal - minVal) / 255.0;
		L = 0.5*(maxVal + minVal) / 255.0;
		S = std::max(0.5*delta / L, 0.5*delta / (1 - L));
		if (Increment > 0) {
			alpha = std::max(S, 1 - Increment);
			alpha = 1.0 / alpha - 1;
			red = nRed + (nRed - L*255.0)*alpha;
			red = (red < 0x00) ? 0x00 : (red > 0xff) ? 0xff : red;
			green = nGreen + (nGreen - L*255.0)*alpha;
			green = (green < 0x00) ? 0x00 : (green > 0xff) ? 0xff : green;
			blue = nBlue + (nBlue - L*255.0)*alpha;
			blue = (blue < 0x00) ? 0x00 : (blue > 0xff) ? 0xff : blue;
		}
		else {
			alpha = Increment;
			red = L*255.0 + (nRed - L * 255.0)*(1 + alpha);
			red = (red < 0x00) ? 0x00 : (red > 0xff) ? 0xff : red;
			green = L*255.0 + (nGreen - L * 255.0)*(1 + alpha);
			green = (green < 0x00) ? 0x00 : (green > 0xff) ? 0xff : green;
			blue = L*255.0 + (nBlue - L * 255.0)*(1 + alpha);
			blue = (blue < 0x00) ? 0x00 : (blue > 0xff) ? 0xff : blue;
		}
		data[i] = qRgba(red, green, blue, qAlpha(data[i]));
	}
}

// 亮度调节
void VTAdjustBrightness(QImage &Img, int nBrightness) {
	if (0 == nBrightness)
		return;
	int red, green, blue;
	int pixels = Img.width() * Img.height();
	unsigned int *data = (unsigned int *)Img.bits();
	for (int i = 0; i < pixels; ++i) {
		red = qRed(data[i]) + nBrightness;
		red = (red < 0x00) ? 0x00 : (red > 0xff) ? 0xff : red;
		green = qGreen(data[i]) + nBrightness;
		green = (green < 0x00) ? 0x00 : (green > 0xff) ? 0xff : green;
		blue = qBlue(data[i]) + nBrightness;
		blue = (blue  < 0x00) ? 0x00 : (blue  > 0xff) ? 0xff : blue;
		data[i] = qRgba(red, green, blue, qAlpha(data[i]));
	}
}

QImage VTMat2QImage(cv::Mat mtx)
{
	switch (mtx.type())
	{
	case CV_8UC1:
	{
		QImage img((const unsigned char *)(mtx.data), mtx.cols, mtx.rows, mtx.cols, QImage::Format_Grayscale8);
		return img;
	}
	break;
	case CV_8UC3:
	{
		QImage img((const unsigned char *)(mtx.data), mtx.cols, mtx.rows, mtx.cols * 3, QImage::Format_RGB888);
		return img.rgbSwapped();
	}
	break;
	case CV_8UC4:
	{
		QImage img((const unsigned char *)(mtx.data), mtx.cols, mtx.rows, mtx.cols * 4, QImage::Format_ARGB32);
		return img;
	}
	break;
	default:
	{
		QImage img;
		return img;
	}
	break;
	}
}

cv::Mat VTQImage2Mat(QImage &image)
{
	cv::Mat mat;
	switch (image.format())
	{
	case QImage::Format_ARGB32:
	case QImage::Format_RGB32:
	case QImage::Format_ARGB32_Premultiplied:
		mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
		break;
	case QImage::Format_RGB888:
		mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
		cv::cvtColor(mat, mat, CV_BGR2RGB);
		break;
	case QImage::Format_Indexed8:
		mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
		break;
	}
	return mat;
}

bool VTIsHHMMSS(QString time)
{
	QRegularExpression reg = QRegularExpression("([0-9][0-9])\\:([0-5][0-9])\\:([0-5][0-9])");
	int index = time.indexOf(reg);
	return (index != -1);
}

bool VTTransRect(QRect src, QRect srcBg, QRect dstBg, QRect &dst)
{
	if (dstBg.width() == 0 || dstBg.height() == 0)
		return false;
	double dw = srcBg.width() * 1.0 / dstBg.width();
	double dh = srcBg.height() * 1.0 / dstBg.height();
	if (dw == 0.0 || dh == 0.0)
		return false;

	double x = src.x() * 1.0 / dw;
	double y = src.y() * 1.0 / dh;
	double w = src.width() * 1.0 / dw;
	double h = src.height() * 1.0 / dh;

	dst.setX(x);
	dst.setY(y);
	dst.setWidth(w);
	dst.setHeight(h);
	return true;
}

bool VTExpandingEdge(QRect src, QRect srcBg, int expand, QRect &dst)
{
	if (srcBg.width() < src.width() || srcBg.height() < src.height())
		return false;
	int x = 0, y = 0, w = 0, h = 0;

	x = src.x() - expand;
	if (x < 0)
		x = 0;

	y = src.y() - expand;
	if (y < 0)
		y = 0;

	w = src.width() + expand + (src.x() - x);
	if (x + w > srcBg.width())
		w = srcBg.width() - x;

	h = src.height() + expand + (src.y() - y);
	if (y + h > srcBg.height())
		h = srcBg.height() - y;

	dst.setX(x);
	dst.setY(y);
	dst.setWidth(w);
	dst.setHeight(h);
	return true;
}

bool VTWT2AverageTime(VTProjectInfo &info, float &ds, float &fps)
{
	int size = info.frameInfos.size();
	if (size == 0)
		return false;
	int nBeginSecs = -1;
	int nEndSecs = -1;
	int n = 0, m = 0;
	for (n = 0; n < size; ++n)
	{
		QString timeHMS = info.frameInfos[n].timeHMS;
		if (!VTIsHHMMSS(timeHMS))
			return false;
		QTime time = QTime::fromString(timeHMS, "hh:mm:ss");
		int nSecs = (time.hour() * 60 + time.minute()) * 60 + time.second();
		if (nBeginSecs == -1)
			nBeginSecs = nSecs;
		else
		{
			if(nBeginSecs != nSecs)
			{
				nBeginSecs = nSecs;
				break;
			}
		}
	}

	for (m = size - 1; m > n; --m)
	{
		QString strWaterMarkTime = info.frameInfos[m].timeHMS;
		if (!VTIsHHMMSS(strWaterMarkTime))
			return false;
		QTime time = QTime::fromString(strWaterMarkTime, "hh:mm:ss");
		int nSecs = (time.hour() * 60 + time.minute()) * 60 + time.second();
		if (nEndSecs == -1)
			nEndSecs = nSecs;
		else
		{
			if (nEndSecs != nSecs)
			{
				m += 1;
				break;
			}
		}
	}

	if (nEndSecs <= nBeginSecs || m <= n)
		return false;

	ds = 1000000.0 * (nEndSecs - nBeginSecs) / (m - n);
	fps = (m - n) / (nEndSecs - nBeginSecs);
	return true;
}

bool VTFPS2AverageTime(VTProjectInfo &info, float &ds)
{
	if (info.fps <= 0)
		return false;
	ds = 1000000.0 / info.fps;
	return true;
}

void VTCorrectWaterMark(VTProjectInfo &info)
{
	QString str1;
	QString str2;
	int index1 = -1;
	int index2 = -1;
	for (int n = 0; n < info.frameInfos.size() - 2; n++)
	{
		QString strTime1 = info.frameInfos[n].timeHMS;
		QString strTime2 = info.frameInfos[n + 1].timeHMS;
		if (VTIsHHMMSS(strTime1) && !VTIsHHMMSS(strTime2))
		{
			if (index1 == -1)
			{
				str1 = strTime1;
				index1 = n;
			}
		}

		if (!VTIsHHMMSS(strTime1) && VTIsHHMMSS(strTime2))
		{
			if (index1 != -1 && index2 == -1)
			{
				str2 = strTime2;
				index2 = n + 1;
			}
		}

		if (index1 != -1 && index2 != -1 && str1 == str2)
		{
			for (int m = index1 + 1; m < index2; ++m)
			{
				info.frameInfos[m].timeHMS = str1;
			}
			index1 = -1;
			index2 = -1;
		}
		else if (index1 != -1 && index2 != -1 && str1 != str2)
		{
			index1 = -1;
			index2 = -1;
		}
	}
}

void VTCheckWaterMark(VTProjectInfo &info, Qt::WindowFlags flags)
{
	QString log = "";
	for (int n = 0; n < info.frameInfos.size(); ++n)
	{
		QString timeHMS = info.frameInfos[n].timeHMS;
		if (!VTIsHHMMSS(timeHMS))
		{
			if (log != "")
				log += ",";
			log += QString::number(n + 1);
			log += "(";
			log += QString::number(g_proInfo.frameInfos[n].index);
			log += ")";
		}
	}
	if (log != "")
	{
		VTMessageBoxOk(QString("第 %1 帧识别水印识别有误，请手动矫正。").arg(log), flags, QMessageBox::Information);
	}
}

bool VTIsCenterRect4WaterTime(QRect src, QRect bg)
{
	if (src.width() > bg.width() || src.height() > bg.height())
		return false;

	if (src.y() < bg.height() * 2 / 3 && src.y() + src.height() > bg.height() / 3)
		return true;

	return false;
}
bool VTFileHash(QString filePath, int &fileSize, QCryptographicHash::Algorithm algo, QByteArray &data)
{
	QFile file(filePath);
	QCryptographicHash hash(algo);
	if (file.open(QIODevice::ReadOnly)) {
		while (file.atEnd() == false) {
			QByteArray message = file.readAll();
			hash.addData(message);
		}
		fileSize = file.size();
		file.close();
	}
	else {
		qDebug() << "file open failed" << filePath;
		return false;
	}
	data = hash.result();
	return true;
}

void VTCorrectionWT(VTProjectInfo &info, int curIndex, const QTime &time)
{
	int size = info.frameInfos.size();
	if (size == 0)
		return;
	if (curIndex < 0 || curIndex >= size)
		return;
	info.frameInfos[curIndex].timeHMS = time.toString("hh:mm:ss");
}

QString VTNumToChinese(QString num) 
{
	QString result;
	for (int i = 0; i < num.length(); i++) {
		int n = num.mid(i, 1).toInt();
		result += QString("零一二三四五六七八九").mid(n, 1);
	}
	return result;
}

static QString Num2Chinese(int number)
{
	// 定义中文数字字符
	const QStringList chineseNumberChars = { QString("零"), QString("一"), QString("二"), QString("三"), QString("四" ), QString("五"), 
		QString("六"), QString("七"), QString("八"), QString("九"), QString("十")};

	if (number <= 10) {
		// 如果数字在 10 以内，直接返回对应的中文字符
		return chineseNumberChars.at(number);
	}
	else if (number < 20) {
		// 如果数字在 11 ~ 19 之间，返回 "十X" 的形式
		return QString("十") + chineseNumberChars.at(number - 10);
	}
	else if (number < 100) {
		// 如果数字在 20 ~ 99 之间，返回 "X十Y" 的形式
		int tens = number / 10;
		int ones = number % 10;
		QString chineseNumber = chineseNumberChars.at(tens) + QString("十");
		if (ones != 0) {
			chineseNumber += chineseNumberChars.at(ones);
		}
		return chineseNumber;
	}
	else {
		// 如果数字不在 1 ~ 99 之间，返回空字符串
		return "";
	}
}


QString VTDate2Chinese(QDate date)
{
	QString str;

	str += VTNumToChinese(QString::number(date.year()));
	str += QString("年");

	str += Num2Chinese(date.month());
	str += QString("月");

	str += Num2Chinese(date.day());
	str += QString("日");
	return str;
}

void VTRemoveUuidFolder(QString dirPath, QVector<QString>& uuids)
{
	QVector<QString> folders;
	QDir dir(dirPath);
	dir.setFilter(QDir::Dirs);
	if (uuids.size() == 0)
		return;

	foreach(QFileInfo fullDir, dir.entryInfoList()) {
		if (fullDir.fileName() == "." || fullDir.fileName() == "..")
			continue;
		bool find = false;
		for (auto uuid : uuids) {
			if (uuid == fullDir.fileName()) {
				find = true;
				break;
			}
		}
		if (!find && fullDir.isDir() && fullDir.exists()) {
			qDebug() << fullDir.absoluteFilePath();
			QDir dirTmp(fullDir.absoluteFilePath());
			dirTmp.removeRecursively();
		}
	}
}

QPoint VTDeskPosition()
{
	QPoint windowPos = g_vtmr->mapToGlobal(QPoint(0, 0));
	QRect desktopGeometry = QGuiApplication::primaryScreen()->geometry();
	QPoint desktopPos = desktopGeometry.topLeft();
	return windowPos - desktopPos;
}

HWND VTFindWindow(QString className, QString windowName)
{
	return FindWindow(
		className == "" ? nullptr : reinterpret_cast<LPCSTR>(className.toLocal8Bit().data()),
		windowName == "" ? nullptr : reinterpret_cast<LPCSTR>(windowName.toLocal8Bit().data()));
}

QString VTGetDesktopPath()
{
	return QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
}

QString VTMillisecondsToTimeFormat(int milliseconds) 
{
	int hours = milliseconds / 3600000;
	int minutes = (milliseconds % 3600000) / 60000;
	int seconds = ((milliseconds % 3600000) % 60000) / 1000;
	int remainingMilliseconds = ((milliseconds % 3600000) % 60000) % 1000;

	return QString("%1:%2:%3.%4")
		.arg(hours, 2, 10, QChar('0'))
		.arg(minutes, 2, 10, QChar('0'))
		.arg(seconds, 2, 10, QChar('0'))
		.arg(remainingMilliseconds, 3, 10, QChar('0'));
}

QString VTSecondsToTimeFormat(int seconds) 
{
	int hours = seconds / 3600;
	int minutes = (seconds % 3600) / 60;
	int remainingSeconds = seconds % 60;

	return QString("%1:%2:%3")
		.arg(hours, 2, 10, QChar('0'))
		.arg(minutes, 2, 10, QChar('0'))
		.arg(remainingSeconds, 2, 10, QChar('0'));
}