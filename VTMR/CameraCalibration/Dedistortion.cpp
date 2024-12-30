#include "Dedistortion.h"
#include <QFileDialog>
#include <QDEBUG>
#include <QStandardItem>
#include <iostream>
#include <fstream>
#include "opencv2/opencv.hpp"
#include <QDir>
#include<iostream>
#include <vector>
#include <string>
#include <QPainter>
#include "Global.h"
#include "Utils.h"
#include <QImage>
#include <QMessageBox>

#define DEBUG_OUTPUT_INFO
#define IMAGE_SQUARE_SIZE 20

using namespace std;
using namespace cv;

cv::Mat QImageToMat(QImage &image)
{
	cv::Mat mat;
	switch (image.format()) {
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

CDedistortionProThread::CDedistortionProThread(QObject * parent)
{

}

int CDedistortionProThread::init(QStringList listImage, int objectSizeW, int objectSizeH, int squareSize)
{
	m_listImage = listImage;
	m_objectSizeW = objectSizeW;
	m_objectSizeH = objectSizeH;
	m_squareSize = squareSize;
	return 0;
}

void CDedistortionProThread::run()
{
	std::string imageFormat = "jpg";
	cout << "开始提取角点………………";
	Size image_size;
	auto image_count = 0;  /* 图像数量 */
	Size board_size = Size(m_objectSizeW - 1, m_objectSizeH - 1);    /* 标定板上每行、列的角点数 */
	vector<Point2f> image_points_buf;  /* 缓存每幅图像上检测到的角点 */
	vector<vector<Point2f>> image_points_seq; /* 保存检测到的所有角点 */
	std::string filename;
	auto count = -1;//用于存储角点个数。
	auto downsize = 1;
	for (auto n = 0; n < m_listImage.size(); n++) {
		filename = m_listImage[n].toStdString();
		
		// 用于观察检验输出
		cout << "image_count = " << image_count << endl;
		/* 输出检验*/
		cout << "-->count = " << count;
		auto imageInput = imread(filename);
		if (image_count == 0) {//读入第一张图片时获取图像宽高信息
			image_size.width = imageInput.cols;
			image_size.height = imageInput.rows;
			cout << "image_size.width = " << image_size.width << endl;
			cout << "image_size.height = " << image_size.height << endl;
			downsize = image_size.width / 1000;
			if (downsize == 0)
			{
				downsize = 1;
			}
		}

		cv::Size msize(image_size.width / downsize, image_size.height / downsize);
		Mat small;
		cv::resize(imageInput, small, msize);
		/* 提取角点 */
		if (0 == findChessboardCorners(small, board_size, image_points_buf, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE)) {
			cout << "can not find chessboard corners!\n"; //找不到角点
			continue;
		}
		else {
			for (size_t j = 0; j<image_points_buf.size(); j++) {
				image_points_buf[j].x = image_points_buf[j].x*downsize;
				image_points_buf[j].y = image_points_buf[j].y*downsize;
			}

			image_count++;
			Mat view_gray;
			cvtColor(imageInput, view_gray, CV_RGB2GRAY);
			/* 亚像素精确化 */
			find4QuadCornerSubpix(view_gray, image_points_buf, Size(5, 5)); //对粗提取的角点进行精确化
			image_points_seq.push_back(image_points_buf);  //保存亚像素角点

			vector<Point2f> pointTmp = image_points_buf;
			for (size_t j = 0; j<pointTmp.size(); j++) {
				pointTmp[j].x = pointTmp[j].x / downsize;
				pointTmp[j].y = pointTmp[j].y / downsize;
			}
			/* 在图像上显示角点位置 */
			drawChessboardCorners(small, board_size, pointTmp, true); //用于在图片中标记角点
		}
	}
	if (image_count <= 0) {
		emit sigFinished(-1, nullptr);
		return;
	}

	//以下是摄像机标定
	cout << "开始标定………………";
	/*棋盘三维信息*/
	Size square_size = Size(m_squareSize, m_squareSize);  /* 实际测量得到的标定板上每个棋盘格的大小 */
	vector<vector<Point3f>> object_points; /* 保存标定板上角点的三维坐标 */
										   /*内外参数*/
	Mat cameraMatrix = Mat(3, 3, CV_64F, Scalar::all(0)); /* 摄像机内参数矩阵 */
	vector<int> point_counts;  // 每幅图像中角点的数量
	Mat distCoeffs = Mat(1, 5, CV_64F, Scalar::all(0)); /* 摄像机的5个畸变系数：k1,k2,p1,p2,k3 */
	vector<Mat> tvecsMat;  /* 每幅图像的旋转向量 */
	vector<Mat> rvecsMat; /* 每幅图像的平移向量 */
						  /* 初始化标定板上角点的三维坐标 */
	int i, j, t;
	for (t = 0; t<image_count; t++) {
		vector<Point3f> tempPointSet;
		for (i = 0; i<board_size.height; i++) {
			for (j = 0; j<board_size.width; j++) {
				Point3f realPoint;
				/* 假设标定板放在世界坐标系中z=0的平面上 */
				realPoint.x = i*square_size.width;
				realPoint.y = j*square_size.height;
				realPoint.z = 0;
				tempPointSet.push_back(realPoint);
			}
		}
		object_points.push_back(tempPointSet);
	}
	/* 初始化每幅图像中的角点数量，假定每幅图像中都可以看到完整的标定板 */
	for (i = 0; i<image_count; i++) {
		point_counts.push_back(board_size.width*board_size.height);
	}
	/* 开始标定 */
	calibrateCamera(object_points, image_points_seq, image_size, cameraMatrix, distCoeffs, rvecsMat, tvecsMat, 0);
	//对标定结果进行评价
	double total_err = 0.0; /* 所有图像的平均误差的总和 */
	double err = 0.0; /* 每幅图像的平均误差 */
	vector<Point2f> image_points2; /* 保存重新计算得到的投影点 */
	for (i = 0; i<image_count; i++) {
		vector<Point3f> tempPointSet = object_points[i];
		/* 通过得到的摄像机内外参数，对空间的三维点进行重新投影计算，得到新的投影点 */
		projectPoints(tempPointSet, rvecsMat[i], tvecsMat[i], cameraMatrix, distCoeffs, image_points2);
		/* 计算新的投影点和旧的投影点之间的误差*/
		vector<Point2f> tempImagePoint = image_points_seq[i];
		Mat tempImagePointMat = Mat(1, tempImagePoint.size(), CV_32FC2);
		Mat image_points2Mat = Mat(1, image_points2.size(), CV_32FC2);
		for (int j = 0; j < tempImagePoint.size(); j++) {
			image_points2Mat.at<Vec2f>(0, j) = Vec2f(image_points2[j].x, image_points2[j].y);
			tempImagePointMat.at<Vec2f>(0, j) = Vec2f(tempImagePoint[j].x, tempImagePoint[j].y);
		}
		err = norm(image_points2Mat, tempImagePointMat, NORM_L2);
		total_err += err /= point_counts[i];
	}
	auto dedistortion = new VTDedistortion;
	for (auto i = 0; i < 3; i++)
		for (auto j = 0; j < 3; j++)
			dedistortion->matrixs[i][j] = cameraMatrix.at<double>(i, j);

	for (auto i = 0; i < 5; i++) {
		dedistortion->distCoeffs[i] = distCoeffs.at<double>(0, i);
	}

	dedistortion->has = 1;
	emit sigFinished(0, dedistortion);
}

CDedistortion::CDedistortion(QWidget *parent)
	: QDialog(parent), m_imageBlack(IMAGE_SQUARE_SIZE, IMAGE_SQUARE_SIZE, QImage::Format_RGB32), m_imageWhite(IMAGE_SQUARE_SIZE, IMAGE_SQUARE_SIZE, QImage::Format_RGB32)
{
	//初始化界面
	ui.setupUi(this);
	setMaximumSize(QSize(500, 500));
	for (int x = 0; x < m_imageBlack.width(); x++) {
		for (int y = 0; y< m_imageBlack.height(); y++) {
			m_imageBlack.setPixel(x, y, qRgb(0, 0, 0));
		}
	}
	for (int x = 0; x < m_imageWhite.width(); x++) {
		for (int y = 0; y< m_imageWhite.height(); y++) {
			m_imageWhite.setPixel(x, y, qRgb(255, 255, 255));
		}
	}
	drawImage();
	connect(ui.btnAdd, SIGNAL(clicked()), this, SLOT(onAdd()));
	connect(ui.btnDel, SIGNAL(clicked()), this, SLOT(onDel()));
	connect(ui.btnClose, SIGNAL(clicked()), this, SLOT(onClose()));
	connect(ui.btnDedistortion, SIGNAL(clicked()), this, SLOT(onDedistortion()));
	connect(ui.obj_w, SIGNAL(textChanged(const QString &)), this, SLOT(onObjWChanged(const QString &)));
	connect(ui.obj_h, SIGNAL(textChanged(const QString &)), this, SLOT(onObjHChanged(const QString &)));

	//初始化数据
	setData();
}

CDedistortion::~CDedistortion()
{
}

void CDedistortion::drawImage()
{
	if (m_objectSizeW <= 0 || m_objectSizeH <= 0)
		return;
	QImage image(20 * m_objectSizeW, 20 * m_objectSizeH, QImage::Format_RGB32);
	for (int x = 0; x < image.width(); x++) {
		for (int y = 0; y< image.height(); y++) {
			image.setPixel(x, y, qRgb(255, 255, 255));
		}
	}

	bool flagH = true;
	bool flagW = true;

	QPainter painter(&image);
	for (int i = 0; i < m_objectSizeH; i++) //行
	{
		for (int j = 0; j < m_objectSizeW; j++) //列
		{
			QPoint point = QPoint(j*IMAGE_SQUARE_SIZE, i*IMAGE_SQUARE_SIZE);
			if (flagW)
			{
				painter.drawImage(point, m_imageBlack);
				flagW = false;
			}
			else
			{
				painter.drawImage(point, m_imageWhite);
				flagW = true;
			}
		}

		flagW = !flagH;
		flagH = flagW;
	}
	painter.end();

	ui.label->setPixmap(QPixmap::fromImage(image));
}

void CDedistortion::onObjWChanged(const QString &text)
{
	m_objectSizeW = text.toInt();
	drawImage();
}

void CDedistortion::onObjHChanged(const QString &text)
{
	m_objectSizeH = text.toInt();
	drawImage();
}

void CDedistortion::onAdd()
{
	QFileDialog fileDialog(this);
	fileDialog.setWindowTitle(QString("请选择相机标定图片"));
	QStringList filters;
	filters << "Image files (*.png *.jpg *.bmp)";
	fileDialog.setNameFilters(filters);
	fileDialog.setFileMode(QFileDialog::ExistingFiles);
	//弹出对话框
	if (fileDialog.exec() == QDialog::Accepted) {
		QString strAppPath = QCoreApplication::applicationDirPath();
		string stdstrAppPath = strAppPath.toStdString();
		//strPathList  返回值是一个list，如果是单个文件选择的话，只要取出第一个来就行了。
		QStringList strPathList = fileDialog.selectedFiles();
		int count = strPathList.size();
		for (int i = 0; i < count; i++)
		{
			QString str = strPathList.at(i);
			new QListWidgetItem(str, ui.listWidget);
		}
	}
}

void CDedistortion::onDel()
{
	ui.listWidget->takeItem(ui.listWidget->currentRow());
}

void CDedistortion::onClose()
{
	close();
}

void CDedistortion::getData()
{
	m_objectSizeW = ui.obj_w->text().toInt();
	m_objectSizeH = ui.obj_h->text().toInt();
	m_squareSize = ui.squaresize->text().toInt();

	m_listImage.clear();

	QString strImage;
	for (int row = 0; row < ui.listWidget->count(); row++)
	{
		strImage = ui.listWidget->item(row)->text();
		m_listImage << strImage;
	}
}

void CDedistortion::setData()
{
	ui.cbUse->setChecked(g_proInfo.dedistortion.useFlag);
	if (g_proInfo.dedistortion.has)
	{
		m_objectSizeW = g_proInfo.dedistortion.objW;
		m_objectSizeH = g_proInfo.dedistortion.objH;
		m_squareSize = g_proInfo.dedistortion.squareSize;
		m_listImage = g_proInfo.dedistortion.listImage;
	}
	else
	{
		m_objectSizeW = 10;
		m_objectSizeH = 7;
		m_squareSize = 10;
	}

	QString strTemp;
	strTemp = QString("%1").arg(m_objectSizeW);
	ui.obj_w->setText(strTemp);
	strTemp = QString("%1").arg(m_objectSizeH);
	ui.obj_h->setText(strTemp);
	strTemp = QString("%1").arg(m_squareSize);
	ui.squaresize->setText(strTemp);

	m_listImage = g_proInfo.dedistortion.listImage;
	for (int n = 0; n < m_listImage.size(); n++)
	{
		QString str = m_listImage.at(n);
		new QListWidgetItem(str, ui.listWidget);
	}

	m_dedistortion = g_proInfo.dedistortion;
	showResult();
}

void CDedistortion::showResult()
{
	if (m_dedistortion.matrixs[0][0] == INVALID_VALUE_DOUBLE
		|| m_dedistortion.matrixs[0][1] == INVALID_VALUE_DOUBLE
		|| m_dedistortion.matrixs[0][2] == INVALID_VALUE_DOUBLE
		|| m_dedistortion.matrixs[1][0] == INVALID_VALUE_DOUBLE
		|| m_dedistortion.matrixs[1][1] == INVALID_VALUE_DOUBLE
		|| m_dedistortion.matrixs[1][2] == INVALID_VALUE_DOUBLE
		|| m_dedistortion.matrixs[2][0] == INVALID_VALUE_DOUBLE
		|| m_dedistortion.matrixs[2][1] == INVALID_VALUE_DOUBLE
		|| m_dedistortion.matrixs[2][2] == INVALID_VALUE_DOUBLE)
		return;
	QString strMatrix00 = QString::number(m_dedistortion.matrixs[0][0], 'f', 4);
	QString strMatrix01 = QString::number(m_dedistortion.matrixs[0][1], 'f', 4);
	QString strMatrix02 = QString::number(m_dedistortion.matrixs[0][2], 'f', 4);
	QString strMatrix10 = QString::number(m_dedistortion.matrixs[1][0], 'f', 4);
	QString strMatrix11 = QString::number(m_dedistortion.matrixs[1][1], 'f', 4);
	QString strMatrix12 = QString::number(m_dedistortion.matrixs[1][2], 'f', 4);
	QString strMatrix20 = QString::number(m_dedistortion.matrixs[2][0], 'f', 4);
	QString strMatrix21 = QString::number(m_dedistortion.matrixs[2][1], 'f', 4);
	QString strMatrix22 = QString::number(m_dedistortion.matrixs[2][2], 'f', 4);
	QString strResult = QString("IntrinsicMatrix:\n%1 %2 %3\n%4 %5 %6\n%7 %8 %9")
		.arg(strMatrix00).arg(strMatrix01).arg(strMatrix02)
		.arg(strMatrix10).arg(strMatrix11).arg(strMatrix12)
		.arg(strMatrix20).arg(strMatrix21).arg(strMatrix22);
	ui.showResult->setText(strResult);
}

void CDedistortion::onDedistortion()
{
	if (m_processTimer)
		m_processTimer->stop();
	else
	{
		m_processTimer = new QTimer(this);
		connect(m_processTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
	}
	m_currentProcess = 0;
	ui.progressBar->setValue(m_currentProcess);

	getData();

	if (m_listImage.size() == 0)
	{
		VTMessageBoxOk(QString("未设置相机标定图片！"), windowFlags(), QMessageBox::Information, this);
		return;
	}
	ui.btnDedistortion->setEnabled(false);
	m_processTimer->start(1000);
	if (!m_proThread)
	{
		m_proThread = new CDedistortionProThread(this);
	}
	else {
		disconnect(m_proThread, SIGNAL(sigFinished(int, VTDedistortion*)), this, SLOT(onFinished(int, VTDedistortion*)));
		m_proThread->setExit();
		m_proThread = new CDedistortionProThread(this);
	}

	connect(m_proThread, SIGNAL(sigFinished(int, VTDedistortion*)), this, SLOT(onFinished(int, VTDedistortion*)));
	m_proThread->init(m_listImage, m_objectSizeW, m_objectSizeH, m_squareSize);
	m_proThread->start();
}

int CDedistortion::doDedistortion(QImage &image)
{
	if (!g_proInfo.dedistortion.has || !g_proInfo.dedistortion.useFlag)
		return -1;
	Mat cameraMatrix = Mat(3, 3, CV_64F, Scalar::all(0)); /* 摄像机内参数矩阵 */
	Mat distCoeffs = Mat(1, 5, CV_64F, Scalar::all(0)); /* 摄像机的5个畸变系数：k1,k2,p1,p2,k3 */

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			cameraMatrix.at<double>(i, j) = g_proInfo.dedistortion.matrixs[i][j];

	for (int i = 0; i < 5; i++)
	{
		distCoeffs.at<double>(0, i) = g_proInfo.dedistortion.distCoeffs[i];
	}

	Mat imageSource = QImageToMat(image);
	Size image_size;
	image_size.width = imageSource.cols;
	image_size.height = imageSource.rows;

	int downsize = 1;
	downsize = image_size.width / 1000;
	if (downsize == 0)
	{
		downsize = 1;
	}
	/************************************************************************
	显示定标结果
	*************************************************************************/
	Mat mapx = Mat(image_size, CV_32FC1);
	Mat mapy = Mat(image_size, CV_32FC1);
	Mat R = Mat::eye(3, 3, CV_32F);
	initUndistortRectifyMap(cameraMatrix, distCoeffs, R, cameraMatrix, image_size, CV_32FC1, mapx, mapy);
	
	
	Mat newimage = imageSource.clone();
	//另一种不需要转换矩阵的方式
	//undistort(imageSource,newimage,cameraMatrix,distCoeffs);
	remap(imageSource, newimage, mapx, mapy, INTER_LINEAR);

	image = VTMat2QImage(newimage);
	Mat small;
	cv::Size msize(image_size.width / downsize, image_size.height / downsize);
	cv::resize(newimage, small, msize);
	//imshow(strImageName.c_str(), small);
	//waitKey(1);
	return 0;
}

void CDedistortion::onTimeout()
{
	m_currentProcess = m_currentProcess + (100 - m_currentProcess) / 5;
	ui.progressBar->setValue(m_currentProcess);
}

void CDedistortion::onFinished(int status, VTDedistortion* dedistortion)
{
	ui.btnDedistortion->setEnabled(true);
	m_processTimer->stop();
	ui.progressBar->setValue(100);
	if (status)
	{
		VTMessageBoxOk(QString("提取角点失败！"), windowFlags(), QMessageBox::Information, this);
	}
	else
	{
		m_dedistortion = *dedistortion;
		showResult();
		m_valueChanged = true;
	}
}

void CDedistortion::closeEvent(QCloseEvent *event)
{
	g_proInfo.dedistortion.useFlag = ui.cbUse->isChecked();
	if (!m_valueChanged)
		return;
	QMessageBox::StandardButton resBtn = VTMessageBoxYesNo(QString("是否保存数据！"), this);
	switch (resBtn)
	{
	case QMessageBox::Yes:	
	{
		if (g_proInfo.uuid == "")
		{
			qDebug() << "uid is empty.";
			break;
		}
			
		QDir dir;
		QString strPath = g_proDataPath + "/" + g_proInfo.uuid + "/CameraCalibration/";
		if (!dir.exists(strPath))
		{
			dir.mkpath(strPath);
		}

		getData();
		if(m_listImage.size() > 0)
		{
			m_dedistortion.listImage.clear();
			if (!m_listImage[0].contains("/CameraCalibration") && !m_listImage[0].contains("\\CameraCalibration"))
			{
				VTClearFiles(strPath);
				//将图片转移到存储文件夹下CameraCalibration
				for (int n = 0; n < m_listImage.size(); n++)
				{
					QFileInfo fileInfo(m_listImage[n]);
					QString tmp = strPath + QString("%1").arg(n+1) + "." + fileInfo.suffix();

					QFile file(m_listImage[n]);
					file.copy(tmp);
					m_dedistortion.listImage << tmp;
				}
			}
			else
			{
				m_dedistortion.listImage = m_listImage;
			}
		}
		//保存数据
		m_dedistortion.objW = m_objectSizeW;
		m_dedistortion.objH = m_objectSizeH;
		m_dedistortion.squareSize = m_squareSize;
		g_proInfo.dedistortion = m_dedistortion;
	}
		break;
	default:
		break;
	}
}
