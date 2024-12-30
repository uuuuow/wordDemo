#include "OCRThread.h"
#include "ev_ocr_api.h"
#include "Utils.h"
#include <QDebug>
#include "Global.h"
#include <QRegularExpression>
#include <QFile>
#include <QDir>
#include <QElapsedTimer>
COCRThread::COCRThread(QObject *parent)
	: QThread(parent)
{
	connect(this, SIGNAL(sigOCR(std::shared_ptr<OCRMsg>)), this, SLOT(onOCR(std::shared_ptr<OCRMsg>)));
	connect(this, SIGNAL(sigOCRs(QQueue<std::shared_ptr<OCRMsg> >)), this, SLOT(onOCRs(QQueue<std::shared_ptr<OCRMsg> >)));
}

COCRThread::~COCRThread()
{
}

int COCRThread::init()
{
	//qDebug() << "begin ocrthread init";
	auto config = g_appPath + PADDLE_OCR_CONFIG_PATH;
	auto det = config + "/det";
	auto rec = config + "/rec";
	auto keys = config + "/keys.txt";
	if (!QFile::exists(det)) {
		qDebug() << det << "不存在";
		return -1;
	}

	if (!QFile::exists(rec)) {
		qDebug() << rec << "不存在";
		return -1;
	}

	if (!QFile::exists(keys)) {
		qDebug() << keys << "不存在";
		return -1;
	}
	//qDebug() << "begin ocrthread end";

	return EVOcrInit(&ocrHandle, (const char*)det.toStdString().c_str(), (const char*)rec.toStdString().c_str(), (const char*)keys.toStdString().c_str());
}

void COCRThread::exit()
{
	m_exit = true;
	m_semap.release();
}

void COCRThread::onOCR(std::shared_ptr<OCRMsg> ocrMsg)
{
	m_mutex.lock();
	while (!m_queMsg.empty()) {
		auto tmp = m_queMsg.front();
		m_queMsg.pop_front();
	}
	m_queMsg.push_back(ocrMsg);
	m_mutex.unlock();
	m_semap.release();
}

void COCRThread::onOCRs(QQueue<std::shared_ptr<OCRMsg> > queMsg)
{
	auto count = 0;
	m_mutex.lock();
	auto iter = m_queMsg.begin();
	for (; iter != m_queMsg.end();) {
		auto tmp = *iter;
		if (tmp->firstImage)
		{
			iter++;
			continue;
		}
		iter = m_queMsg.erase(iter);
	}

	while (!queMsg.empty()) {
		auto ocrMsg = queMsg.front();
		queMsg.pop_front();
		m_queMsg.push_back(ocrMsg);
		count++;
	}
	m_mutex.unlock();
	qDebug() << QString("OCR队列有 %1 张图片").arg(m_queMsg.size());
	for(auto n = 0; n < count; ++n)
		m_semap.release();
}

bool COCRThread::checkHMS(QString ocr, QString &time)
{
	auto index = -1;
	auto reg1 = QRegularExpression("([0-9][0-9])\\:([0-5][0-9])\\:([0-5][0-9])");
	index = ocr.indexOf(reg1);
	if (index != -1) {
		time = ocr.mid(index, QString("00:00:00").size());
		return true;
	}

	auto reg2 = QRegularExpression("([0-9][0-9])\\:([0-5][0-9][0-5][0-9])");
	index = ocr.indexOf(reg2);
	if (index != -1) {
		time = ocr.mid(index, QString("00:0000").size());
		time.insert(5, ":");
		return true;
	}

	auto reg3 = QRegularExpression("([0-9][0-9][0-5][0-9])\\:([0-5][0-9])");
	index = ocr.indexOf(reg3);
	if (index != -1) {
		time = ocr.mid(index, QString("0000:00").size());
		time.insert(2, ":");
		return true;
	}

	auto reg4 = QRegularExpression("([0-9][0-9])\\:([0-5][0-9])\\.([0-5][0-9])");
	index = ocr.indexOf(reg4);
	if (index != -1) {
		time = ocr.mid(index, QString("00:00.00").size());
		index = time.indexOf(".");
		if (index != -1) {
			time.replace(index, 1, ":");
		}
		return true;
	}

	auto reg5 = QRegularExpression("([0-9][0-9])\\.([0-5][0-9])\\:([0-5][0-9])");
	index = ocr.indexOf(reg5);
	if (index != -1) {
		time = ocr.mid(index, QString("00.00:00").size());
		index = time.indexOf(".");
		if (index != -1) {
			time.replace(index, 1, ":");
		}
		return true;
	}

	auto reg6 = QRegularExpression("([0-9][0-9])\\.([0-5][0-9])\\.([0-5][0-9])");
	index = ocr.indexOf(reg6);
	if (index != -1) {
		time = ocr.mid(index, QString("00.00.00").size());
		index = time.indexOf(".");
		if (index != -1) {
			time.replace(index, 1, ":");
		}
		index = time.indexOf(".");
		if (index != -1) {
			time.replace(index, 1, ":");
		}
		return true;
	}

	auto reg7 = QRegularExpression("([0-9][0-9])\\.([0-5][0-9])([0-5][0-9])");
	index = ocr.indexOf(reg7);
	if (index != -1) {
		time = ocr.mid(index, QString("00.0000").size());
		index = time.indexOf(".");
		if (index != -1) {
			time.replace(index, 1, ":");
		}
		time.insert(5, ":");
		return true;
	}

	auto reg8 = QRegularExpression("([0-9][0-9])([0-5][0-9])\\.([0-5][0-9])");
	index = ocr.indexOf(reg8);
	if (index != -1) {
		time = ocr.mid(index, QString("0000.00").size());
		index = time.indexOf(".");
		if (index != -1) {
			time.replace(index, 1, ":");
		}
		time.insert(2, ":");
		return true;
	}

	//2020-01-02 000000
	auto reg9 = QRegularExpression("([0-9][0-9][0-9][0-9])\\-([0-1][0-9])\\-([0-3][0-9]) ([0-2][0-9][0-5][0-9][0-5][0-9])");
	index = ocr.indexOf(reg9);
	if (index != -1) {
		time = ocr.mid(index + 11, QString("000000").size());
		time.insert(2, ":");
		time.insert(5, ":");
		return true;
	}

	//020-01-02 000000
	auto reg10 = QRegularExpression("([0-9][0-9][0-9])\\-([0-1][0-9])\\-([0-3][0-9]) ([0-2][0-9][0-5][0-9][0-5][0-9])");
	index = ocr.indexOf(reg10);
	if (index != -1) {
		time = ocr.mid(index + 10, QString("000000").size());
		time.insert(2, ":");
		time.insert(5, ":");
		return true;
	}

	//20-01-02 000000
	auto reg11 = QRegularExpression("([0-9][0-9])\\-([0-1][0-9])\\-([0-3][0-9]) ([0-2][0-9][0-5][0-9][0-5][0-9])");
	index = ocr.indexOf(reg11);
	if (index != -1) {
		time = ocr.mid(index + 9, QString("000000").size());
		time.insert(2, ":");
		time.insert(5, ":");
		return true;
	}

	//0-01-02 000000
	auto reg12 = QRegularExpression("([0-9])\\-([0-1][0-9])\\-([0-3][0-9]) ([0-2][0-9][0-5][0-9][0-5][0-9])");
	index = ocr.indexOf(reg12);
	if (index != -1) {
		time = ocr.mid(index + 8, QString("000000").size());
		time.insert(2, ":");
		time.insert(5, ":");
		return true;
	}

	//-01-02 000000
	auto reg13 = QRegularExpression("\\-([0-1][0-9])\\-([0-3][0-9]) ([0-2][0-9][0-5][0-9][0-5][0-9])");
	index = ocr.indexOf(reg13);
	if (index != -1) {
		time = ocr.mid(index + 7, QString("000000").size());
		time.insert(2, ":");
		time.insert(5, ":");
		return true;
	}

	//01-02 000000
	auto reg14 = QRegularExpression("([0-1][0-9])\\-([0-3][0-9]) ([0-2][0-9][0-5][0-9][0-5][0-9])");
	index = ocr.indexOf(reg14);
	if (index != -1) {
		time = ocr.mid(index + 6, QString("000000").size());
		time.insert(2, ":");
		time.insert(5, ":");
		return true;
	}

	//1-02 000000
	auto reg15 = QRegularExpression("([0-9])\\-([0-3][0-9]) ([0-2][0-9][0-5][0-9][0-5][0-9])");
	index = ocr.indexOf(reg15);
	if (index != -1) {
		time = ocr.mid(index + 5, QString("000000").size());
		time.insert(2, ":");
		time.insert(5, ":");
		return true;
	}

	//-02 000000
	auto reg16 = QRegularExpression("\\-([0-3][0-9]) ([0-2][0-9][0-5][0-9][0-5][0-9])");
	index = ocr.indexOf(reg16);
	if (index != -1) {
		time = ocr.mid(index + 4, QString("000000").size());
		time.insert(2, ":");
		time.insert(5, ":");
		return true;
	}

	//2020-01-02000000
	auto reg17 = QRegularExpression("([0-9][0-9][0-9][0-9])\\-([0-1][0-9])\\-([0-3][0-9][0-2][0-9][0-5][0-9][0-5][0-9])");
	index = ocr.indexOf(reg17);
	if (index != -1) {
		time = ocr.mid(index + 10, QString("000000").size());
		time.insert(2, ":");
		time.insert(5, ":");
		return true;
	}

	//020-01-02000000
	auto reg18 = QRegularExpression("([0-9][0-9][0-9])\\-([0-1][0-9])\\-([0-3][0-9][0-2][0-9][0-5][0-9][0-5][0-9])");
	index = ocr.indexOf(reg18);
	if (index != -1) {
		time = ocr.mid(index + 9, QString("000000").size());
		time.insert(2, ":");
		time.insert(5, ":");
		return true;
	}

	//20-01-02000000
	auto reg19 = QRegularExpression("([0-9][0-9])\\-([0-1][0-9])\\-([0-3][0-9][0-2][0-9][0-5][0-9][0-5][0-9])");
	index = ocr.indexOf(reg19);
	if (index != -1) {
		time = ocr.mid(index + 8, QString("000000").size());
		time.insert(2, ":");
		time.insert(5, ":");
		return true;
	}

	//0-01-02000000
	auto reg20 = QRegularExpression("([0-9])\\-([0-1][0-9])\\-([0-3][0-9][0-2][0-9][0-5][0-9][0-5][0-9])");
	index = ocr.indexOf(reg20);
	if (index != -1) {
		time = ocr.mid(index + 7, QString("000000").size());
		time.insert(2, ":");
		time.insert(5, ":");
		return true;
	}

	//-01-02000000
	auto reg21 = QRegularExpression("\\-([0-1][0-9])\\-([0-3][0-9][0-2][0-9][0-5][0-9][0-5][0-9])");
	index = ocr.indexOf(reg21);
	if (index != -1) {
		time = ocr.mid(index + 6, QString("000000").size());
		time.insert(2, ":");
		time.insert(5, ":");
		return true;
	}

	//01-02000000
	auto reg22 = QRegularExpression("([0-1][0-9])\\-([0-3][0-9][0-2][0-9][0-5][0-9][0-5][0-9])");
	index = ocr.indexOf(reg22);
	if (index != -1) {
		time = ocr.mid(index + 5, QString("000000").size());
		time.insert(2, ":");
		time.insert(5, ":");
		return true;
	}

	//1-02000000
	auto reg23 = QRegularExpression("([0-9])\\-([0-3][0-9][0-2][0-9][0-5][0-9][0-5][0-9])");
	index = ocr.indexOf(reg23);
	if (index != -1) {
		time = ocr.mid(index + 4, QString("000000").size());
		time.insert(2, ":");
		time.insert(5, ":");
		return true;
	}
	return false;
}

void COCRThread::doOCR(QImage *image, QString writePath, QVector<VTWaterMark> &waterWarks, bool detect)
{
	cv::Mat mat = cv::Mat(image->height(), image->width(), CV_8UC3, (void*)image->constBits(), image->bytesPerLine());
	EVOCRResult results;
	memset(&results, 0, sizeof(EVOCRResult));
	//QElapsedTimer ElapsedTimer;
	//ElapsedTimer.start();
	EVOcrProcess(ocrHandle, mat.rows, mat.cols, mat.type(), (char*)mat.data, mat.step[0], writePath == "" ? nullptr : (char*)writePath.toStdString().c_str(), detect, &results);
	//qDebug() << "===cost " << ElapsedTimer.elapsed() << " ms";

	for (auto n = 0; n < results.boxNum; ++n) {
		auto box = results.box[n];
		VTWaterMark waterMark;
		waterMark.text = QString::fromLocal8Bit(box.text);

		waterMark.rect.setX(box.x);
		waterMark.rect.setY(box.y);
		waterMark.rect.setWidth(box.w);
		waterMark.rect.setHeight(box.h);
		waterWarks.push_back(waterMark);
	}
}

void COCRThread::run()
{
	while (!m_exit) {
		std::shared_ptr<VTDecodeImage>decodeImage = nullptr;
		std::shared_ptr<OCRMsg> ocrMsg;
		m_semap.acquire();

		if (m_queMsg.size() == 0)
			continue;

		m_mutex.lock();
		ocrMsg = m_queMsg.front();
		m_queMsg.pop_front();
		m_mutex.unlock();

		if (ocrMsg->uuid != g_proInfo.uuid && !ocrMsg->firstImage)
			continue;
		decodeImage = ocrMsg->decodeImage;
		if (!decodeImage->image && decodeImage->imagePath != "") {
			QImage imageTmp;
			if (!imageTmp.load(decodeImage->imagePath)) {
				ocrMsg->user->notifyOCRResult(ocrMsg);
				continue;
			}
			
			decodeImage->image = new QImage;
			if (imageTmp.format() != QImage::Format_RGB888) {
				*decodeImage->image = imageTmp.convertToFormat(QImage::Format_RGB888);
			}
		}
		
		if (decodeImage->image) {
			QImage image;
			QRect expand(0, 0, 0, 0);
			auto detect = false;
			if (decodeImage->waterMarkHMS.rect != QRect(0, 0, 0, 0)) {
				VTExpandingEdge(decodeImage->waterMarkHMS.rect, decodeImage->image->rect(), 10, expand);
			}
			else
				detect = true;

			if (expand != QRect(0, 0, 0, 0)) {
				image = decodeImage->image->copy(expand);
				detect = false;
			}
			else
				image = *decodeImage->image;

			auto imageDir = QString("%1/%2/WaterMark").arg(g_proDataPath).arg(ocrMsg->uuid);
			QDir dir;
			if (!dir.exists(imageDir))
				dir.mkpath(imageDir);
			auto writePath = QString("%1/%2.jpg").arg(imageDir).arg(ocrMsg->decodeImage->pts);
			if (ocrMsg->firstImage)
				writePath = "";
			QElapsedTimer tm;
			tm.start();
			doOCR(&image, writePath, decodeImage->waterMarks, detect);
			qDebug() << "ocr cost " << tm.elapsed() << "ms";
			auto iter = decodeImage->waterMarks.begin();
			for (; iter != decodeImage->waterMarks.end(); ) {
				QString timeHMS = "";
				VTWaterMark wm = *iter;
				if (!detect)
					wm.rect = expand;

				if (wm.rect.width() <= 20 || wm.rect.height() <= 20) {
					qDebug() << wm.rect << QString(" 太小抛弃[%1]").arg(wm.text);
					iter = decodeImage->waterMarks.erase(iter);
					continue;
				}

				if (checkHMS(wm.text, timeHMS)) {
					//中心区域不是时间水印
					if (!VTIsCenterRect4WaterTime(wm.rect, decodeImage->image->rect())) {
						decodeImage->waterMarkHMS.text = timeHMS;
						if (decodeImage->waterMarkHMS.rect == QRect(0, 0, 0, 0)) {
							decodeImage->waterMarkHMS.rect = wm.rect;
						}
					}
					else {
						qDebug() << QString("在图像中心位置的时间戳不认为是水印时间 time rect ") << wm.rect << " image rect" << decodeImage->image->rect() << QString("内容[%1]").arg(wm.text);
					}
				}
				iter++;
			}

			ocrMsg->user->notifyOCRResult(ocrMsg);
		}
		else {
			ocrMsg->user->notifyOCRResult(ocrMsg);
		}
	}
}
