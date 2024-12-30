#pragma once

#include <QDialog>
#include "ui_Dedistortion.h"
#include <QStandardItemModel>
#include <string>
#include <set>
#include "opencv2/opencv.hpp"
#include <QString>
#include <QTimer>
#include <QThread>
#include "Utils.h"
/*
* 去畸变
*/
class CDedistortionProThread :public QThread
{
	Q_OBJECT
public:
	CDedistortionProThread(QObject * parent = nullptr);
	int init(QStringList listImage, int objectSizeW, int objectSizeH, int squareSize);
	int setExit() { m_exit = true; return 0; }
protected:
	void run();
signals:
	void sigFinished(int status, VTDedistortion*dedistortion);
private:
	QStringList m_listImage;
	int m_objectSizeW;
	int m_objectSizeH;
	int m_squareSize;
	bool m_exit = false;
};

class CDedistortion : public QDialog
{
	Q_OBJECT

public:
	CDedistortion(QWidget *parent = Q_NULLPTR);
	~CDedistortion();
	void drawImage();
	void getData();
	void setData();
	void showResult();
	static int doDedistortion(QImage &image);

protected:
	void closeEvent(QCloseEvent *event);

public slots:
	void onAdd();
	void onDel();
	void onClose();
	void onDedistortion();
	void onObjWChanged(const QString &text);
	void onObjHChanged(const QString &text);
	void onTimeout();
	void onFinished(int status, VTDedistortion* dedistortion);

private:
	Ui::Dedistortion ui;

	int m_objectSizeW;
	int m_objectSizeH;

	int m_squareSize;

	QImage m_imageBlack;
	QImage m_imageWhite;

	std::vector<std::string> m_vecImagePath;
	std::vector<std::string> m_vecImageName;

	QTimer *m_processTimer = nullptr;
	int m_currentProcess = 0;
	CDedistortionProThread *m_proThread = nullptr;
	QStringList m_listImage;
	bool m_valueChanged = false;
	VTDedistortion m_dedistortion;
};
