#pragma once

#include <QThread>
#include <QSemaphore>
#include <QMutex>
#include <QQueue>
class IOCRUser;
#include <QRect>
#include "Utils.h"
#include <memory>

struct OCRMsg 
{
	QString uuid;
	IOCRUser *user;
	std::shared_ptr<VTDecodeImage> decodeImage;
	bool last = false;
	bool firstImage = false;

	OCRMsg() {
		uuid = "";
		user = nullptr;
		last = false;
	}
	~OCRMsg() {
	}
	OCRMsg(const OCRMsg &r) {
		uuid = r.uuid;
		user = r.user;
		last = r.last;
		if (r.decodeImage) {
			decodeImage = std::make_shared<VTDecodeImage>();
			*decodeImage = *r.decodeImage;
		}
		else {
			decodeImage.reset();
		}
	}
	OCRMsg &operator=(const OCRMsg &r) {
		if (&r == this)
			return *this;

		uuid = r.uuid;
		user = r.user;
		last = r.last;
		decodeImage.reset();
		if (r.decodeImage) {
			decodeImage = std::make_shared<VTDecodeImage>();
			*decodeImage = *r.decodeImage;
		}
		return *this;
	}
};

class IOCRUser 
{
public:
	virtual void notifyOCRResult(std::shared_ptr<OCRMsg> msg) = 0;
};

class COCRThread : public QThread
{
	Q_OBJECT

public:
	COCRThread(QObject *parent);
	~COCRThread();
	int init();
	void exit();
private:
	bool checkHMS(QString ocr, QString &time);
	void doOCR(QImage *image, QString writePath, QVector<VTWaterMark> &waterWarks, bool detect);
signals:
	void sigOCR(std::shared_ptr<OCRMsg> ocrMsg);
	void sigOCRs(QQueue<std::shared_ptr<OCRMsg> > queMsg);
public slots:
	void onOCR(std::shared_ptr<OCRMsg> ocrMsg);
	void onOCRs(QQueue<std::shared_ptr<OCRMsg> > queMsg);
protected:
	virtual void run();
private:
	bool m_exit = false;
	QSemaphore m_semap;
	QMutex m_mutex;
	QQueue<std::shared_ptr<OCRMsg> > m_queMsg;
	void* ocrHandle = nullptr;
};
