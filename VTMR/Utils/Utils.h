#pragma once

#include <QString>
#include <QPoint>
#include <QImage>
#include <opencv2/opencv.hpp>
#include <string>
#include <QDateTime>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QMap>
#include <memory>
#include <QUndoStack>
using std::string;
class QDockWidget;
class QToolBar;
class CInputWidget;

#define APP_VERSION "1.0.1.20"

//无效的int值
#define INVALID_VALUE_INT -5000
//无效的float值
#define INVALID_VALUE_FLOAT -5000.0f
//无效的double值
#define INVALID_VALUE_DOUBLE -5000.0
//像片坐标数量
#define IMAGE_POINTS_COUNT 10
//实际坐标数量
#define ACTUAL_POINTS_COUNT 5

//#define ZOOM_LINE_COLOR "#1464A0"
#define ZOOM_LINE_COLOR "#FFFFFF"

//目录宏
#define PROJECT_DATA_PATH "/projectdata"
#define VTMR_DB_PATH "/config/vtmr.db"
#define APPRAISER_PATH "/config/appraiser"
#define PADDLE_OCR_CONFIG_PATH "/config/ocr"
#define CONF_DOCK_PATH "/config/dock.ini"
#define PROJECT_DATA_TEMP_PATH "/projectdata/temp"

////////////////////////////////////////////
//标定方式
enum VTCalibration {
	CALIBRATION_UNKNOWN = -1,
	//固定摄像头
	//车身自标定
	FIXED_CAM_ONESELF,
	//地面参照物
	FIXED_CAM_GROUND_REFERENCE,
	//一维线性
	FIXED_CAM_1D,
	//二维线性
	FIXED_CAM_2D,
	//车载摄像头
	//空间标定
	MOVING_CAM_SPACE,
	//二维本车
	MOVING_CAM_2D_ONESELF,
	//二维目标车
	MOVING_CAM_2D_TARGET_VEHICLE
};

//计算速度所需的时间模式
enum VTTimeMode {
	//视频帧时间戳PTS
	TIME_MODE_PTS,
	//视频平均帧率FPS
	TIME_MODE_FPS,
	//视频水印时间
	TIME_MODE_WATER_MARK
};

//局部放大
struct VTZoomPairRect {
	QPoint srcTopLeft;
	QPoint srcBottomRight;
	QPoint scaleTopLeft;
	float scale = 1.0;
};

//帧信息
struct VTFrameInfo {
	VTFrameInfo() {
		index = -1;
		pts = -1;
		frameEditStutas = 0;
		v1 = INVALID_VALUE_FLOAT;
		v2 = INVALID_VALUE_FLOAT;
		v3 = INVALID_VALUE_FLOAT;
		timeHMS = "";
		for (auto n = 0; n < IMAGE_POINTS_COUNT; ++n)
		{
			imagePoints.push_back(QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT));
		}

		actualP0 = QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT);
	}

	VTFrameInfo(const VTFrameInfo &r) {
		for (auto n = 0; n < IMAGE_POINTS_COUNT; ++n) {
			imagePoints.push_back(r.imagePoints[n]);
		}

		actualP0 = r.actualP0;
		index = r.index;
		pts = r.pts;
		frameEditStutas = r.frameEditStutas;
		imagePath = r.imagePath;
		v1 = r.v1;
		v2 = r.v2;
		v3 = r.v3;
		md5 = r.md5;
		timeHMS = r.timeHMS;
		zoomRects = r.zoomRects;
	}

	VTFrameInfo& operator=(const VTFrameInfo &r) {
		if (this == &r)
			return *this;
		for (auto n = 0; n < IMAGE_POINTS_COUNT; ++n) {
			imagePoints.push_back(r.imagePoints[n]);
		}

		actualP0 = r.actualP0;
		index = r.index;
		pts = r.pts;
		frameEditStutas = r.frameEditStutas;
		imagePath = r.imagePath;
		v1 = r.v1;
		v2 = r.v2;
		v3 = r.v3;
		md5 = r.md5;
		timeHMS = r.timeHMS;
		zoomRects = r.zoomRects;
		return *this;
	}

	QVector<QPoint> imagePoints;		//像片坐标
	QPointF actualP0;					//测速点实际坐标
	int index;							//帧所在完整视频的索引
	qint64 pts;						//播放时间戳
	int frameEditStutas;				//0-未编辑 1-编辑 2-首帧
	QString imagePath;				//图片路径
	float v1;                           //速度（视频帧时间戳PTS）
	float v2;                           //速度（视频平均帧率FPS）
	float v3;                           //速度（视频水印时间）
	QString md5;						//图片MD5
	QString timeHMS;					//水印时分秒
	QStringList waterMarks;				//其他水印
	QVector<VTZoomPairRect> zoomRects;//局部放大窗口
};

//畸变参数
struct VTDedistortion {
	VTDedistortion() {
		has = 0;
		useFlag = false;
		objW = 0;
		objH = 0;
		squareSize = 0;
		for (auto i = 0; i < 3; i++)
			for (auto j = 0; j < 3; j++)
				matrixs[i][j] = INVALID_VALUE_DOUBLE;
		for (auto n = 0; n < 5; n++)
			distCoeffs[n] = INVALID_VALUE_DOUBLE;
	}

	VTDedistortion(const VTDedistortion&r) {
		has = r.has;
		useFlag = r.useFlag;
		objW = r.objW;
		objH = r.objH;
		squareSize = r.squareSize;

		for (auto i = 0; i < 3; i++)
			for (auto j = 0; j < 3; j++)
				matrixs[i][j] = r.matrixs[i][j];
		for (auto n = 0; n < 5; n++)
			distCoeffs[n] = r.distCoeffs[n];
		listImage = r.listImage;
	}

	VTDedistortion& operator=(const VTDedistortion&r) {
		if (this == &r)
			return *this;
		has = r.has;
		useFlag = r.useFlag;
		objW = r.objW;
		objH = r.objH;
		squareSize = r.squareSize;

		for (auto i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				matrixs[i][j] = r.matrixs[i][j];
		for (auto n = 0; n < 5; n++)
			distCoeffs[n] = r.distCoeffs[n];

		listImage = r.listImage;
		return *this;
	}

	int has;
	bool useFlag;
	int objW;
	int objH;
	int squareSize;
	double matrixs[3][3];
	double distCoeffs[5];
	QStringList listImage;
};

struct VTAppraiserInfo {
	QString id;
	QString name;
	QString photoPath;
	QString photoPathTmp;
};

struct VTAppraisalStandard {
	QString id;
	QString name;
	bool operator<(const VTAppraisalStandard& other) const {
		return id < other.id;
	}
};
//uint qHash(const VTAppraisalStandard& v) {
//	return qHash(v.id) ^ qHash(v.name);
//}
struct VTWaterMark {
	QRect rect = QRect(0, 0, 0, 0);
	QString text = "";
};

struct VTDecodeImage {
	int64_t pts;
	int index;
	QString uuid;
	QString imagePath;
	QImage *image;
	VTWaterMark waterMarkHMS;//时分秒水印
	QVector<VTWaterMark> waterMarks;  //所有水印框
	VTDecodeImage() {
		uuid = "";
		index = -1;
		pts = 0;
		image = nullptr;
		imagePath = "";
		waterMarks.clear();
	}
	~VTDecodeImage() {
		if (image)
			delete image;
	}
	VTDecodeImage(const VTDecodeImage & r) {
		pts = r.pts;
		index = r.index;

		image = nullptr;

		if (r.image) {
			image = new QImage;
			*image = r.image->copy();
		}
		imagePath = r.imagePath;
		waterMarks = r.waterMarks;
		waterMarkHMS = r.waterMarkHMS;
	}

	VTDecodeImage& operator=(const VTDecodeImage & r) {
		if (this == &r)
			return *this;

		pts = r.pts;
		index = r.index;
		if (image)
			delete image;
		image = nullptr;

		if (r.image) {
			image = new QImage;
			*image = r.image->copy();
		}
		imagePath = r.imagePath;
		waterMarkHMS = r.waterMarkHMS;
		waterMarks = r.waterMarks;
		return *this;
	}
};

//工程信息
struct VTProjectInfo {
	VTProjectInfo() {
		uuid;
		projectName;
		videoPath;
		mode = CALIBRATION_UNKNOWN;
		createTime = 0;

		fps = 0;
		v01 = INVALID_VALUE_FLOAT;
		v02 = INVALID_VALUE_FLOAT;
		v03 = INVALID_VALUE_FLOAT;
		v11 = INVALID_VALUE_FLOAT;
		v12 = INVALID_VALUE_FLOAT;
		v13 = INVALID_VALUE_FLOAT;
		headIndex = -1;
		tailIndex0 = -1;
		tailIndex1 = -1;
		curFrmaeIndex = 0;

		frameInfos.clear();
		imageLines.clear();
		actualPoints = {
			QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
			QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
			QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
			QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
			QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT)
		};

		for (size_t n = 0; n < 2; ++n) {
			QVector<QPoint> points;
			points.push_back(QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT));
			points.push_back(QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT));
			points.push_back(QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT));
			imageLines.push_back(points);
		}

		timeMode = TIME_MODE_PTS;
	}

	VTProjectInfo(const VTProjectInfo &r) {
		uuid = r.uuid;
		projectName = r.projectName;
		videoPath = r.videoPath;
		mode = r.mode;

		accidentTime = r.accidentTime;
		accidentLocation = r.accidentLocation;
		licensePlateNumber = r.licensePlateNumber;
		vehicleBrand = r.vehicleBrand;
		vehicleType = r.vehicleType;
		vehicleVIN = r.vehicleVIN;
		referenceDesFrom = r.referenceDesFrom;
		referenceDesTo = r.referenceDesTo;

		client = r.client;
		commitments = r.commitments;
		dateOfCommission = r.dateOfCommission;
		appraisalDate = r.appraisalDate;
		appraisalLocation = r.appraisalLocation;
		appraisalMaterial = r.appraisalMaterial;
		appraisalStandards = r.appraisalStandards;
		appraiser1 = r.appraiser1;
		appraiser2 = r.appraiser2;

		createTime = r.createTime;
		fps = r.fps;
		headIndex = r.headIndex;
		tailIndex0 = r.tailIndex0;
		tailIndex1 = r.tailIndex1;
		v01 = r.v01;
		v02 = r.v02;
		v03 = r.v03;
		v11 = r.v11;
		v12 = r.v12;
		v13 = r.v13;

		dedistortion = r.dedistortion;
		frameInfos = r.frameInfos;
		imageLines = r.imageLines;
		actualPoints = r.actualPoints;
		
		referenceLen1 = r.referenceLen1;
		referenceLen2 = r.referenceLen2;
		
		timeMode = r.timeMode;
		hashAlgo = r.hashAlgo;
		
		duration = r.duration;
		signPTS = r.signPTS;
		
		curFrmaeIndex = r.curFrmaeIndex;

		if (firstImage) {
			firstImage = std::make_shared<VTDecodeImage>();
			*firstImage = *(r.firstImage);
		}
		
		packetDTSs = r.packetDTSs;
		mapHashInfo = r.mapHashInfo;
		fileSize = r.fileSize;
	}

	VTProjectInfo &operator=(const VTProjectInfo &r) {
		if (this == &r)
			return *this;
		uuid = r.uuid;
		projectName = r.projectName;
		videoPath = r.videoPath;
		mode = r.mode;

		accidentTime = r.accidentTime;
		accidentLocation = r.accidentLocation;
		licensePlateNumber = r.licensePlateNumber;
		vehicleBrand = r.vehicleBrand;
		vehicleType = r.vehicleType;
		vehicleVIN = r.vehicleVIN;
		referenceDesFrom = r.referenceDesFrom;
		referenceDesTo = r.referenceDesTo;

		client = r.client;
		commitments = r.commitments;
		dateOfCommission = r.dateOfCommission;
		appraisalDate = r.appraisalDate;
		appraisalLocation = r.appraisalLocation;
		appraisalMaterial = r.appraisalMaterial;
		appraisalStandards = r.appraisalStandards;
		appraiser1 = r.appraiser1;
		appraiser2 = r.appraiser2;

		createTime = r.createTime;
		fps = r.fps;
		headIndex = r.headIndex;
		tailIndex0 = r.tailIndex0;
		tailIndex1 = r.tailIndex1;
		v01 = r.v01;
		v02 = r.v02;
		v03 = r.v03;
		v11 = r.v11;
		v12 = r.v12;
		v13 = r.v13;

		dedistortion = r.dedistortion;
		frameInfos = r.frameInfos;
		imageLines = r.imageLines;
		actualPoints = r.actualPoints;

		referenceLen1 = r.referenceLen1;
		referenceLen2 = r.referenceLen2;

		timeMode = r.timeMode;
		hashAlgo = r.hashAlgo;

		duration = r.duration;
		signPTS = r.signPTS;

		curFrmaeIndex = r.curFrmaeIndex;
		
		firstImage.reset();
		if (r.firstImage) {
			firstImage = std::make_shared<VTDecodeImage>();
			*firstImage = *(r.firstImage);
		}

		packetDTSs = r.packetDTSs;
		mapHashInfo = r.mapHashInfo;
		fileSize = r.fileSize;
		return *this;
	}

	//清空解算数据
	void clearCalcData() {
		frameInfos.clear();
		imageLines.clear();
		for (size_t n = 0; n < 2; ++n)
		{
			QVector<QPoint> points;
			points.push_back(QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT));
			points.push_back(QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT));
			points.push_back(QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT));
			imageLines.push_back(points);
		}
		headIndex = -1;
		tailIndex0 = -1;
		tailIndex1 = -1;
	}

	QString uuid;							//项目唯一ID
	
	QString projectName;					//项目名称
	QString videoPath;						//视频路径
	VTCalibration mode;						//标定方式

	QString accidentTime;					//事故发生时间
	QString accidentLocation;				//事故地点
	QString licensePlateNumber;				//车辆号牌
	QString vehicleBrand;					//车辆品牌
	QString vehicleType;					//车辆类型
	QString vehicleVIN;						//车辆识别代码VIN
	QString referenceDesFrom;				//参照物描述
	QString referenceDesTo;					//参照物描述

	QString client;							//委托人
	QString commitments;					//委托事项
	QString dateOfCommission;				//委托日期
	QString appraisalDate;					//鉴定日期
	QString appraisalLocation;				//鉴定地点
	QString appraisalMaterial;				//鉴定材料
	QVector<VTAppraisalStandard> appraisalStandards;//适应标准
	VTAppraiserInfo appraiser1;				//第一鉴定人
	VTAppraiserInfo appraiser2;				//第二鉴定人

	int createTime;							//工程创建时间
	int fps;
	int headIndex;							//前端参考点帧的索引
	int tailIndex0;							//后端接近参考点帧的索引
	int tailIndex1;							//后端超过参考点帧的索引
	float v01;								//速度（视频帧时间戳PTS）
	float v02;								//速度（视频平均帧率FPS）
	float v03;								//速度（视频水印时间）
	float v11;								//速度（视频帧时间戳PTS）
	float v12;								//速度（视频平均帧率FPS）
	float v13;								//速度（视频水印时间）

	VTDedistortion dedistortion;			//去畸变参数
	QVector<VTFrameInfo> frameInfos;		//帧参数
	QVector<QVector<QPoint> > imageLines;	//直线
	QVector<QPointF> actualPoints;			//实际点位坐标
	//参照物长度
	double referenceLen1 = 0.0;
	double referenceLen2 = 0.0;

	////////////////////////////////////////
	//计算速度的时间方式 
	VTTimeMode timeMode;
	QCryptographicHash::Algorithm hashAlgo = QCryptographicHash::Algorithm::Sha256;
	//总时长
	int64_t duration = 0;//单位秒
	int64_t startTime = 0;//开始时间
	int64_t signPTS;//初始解算帧
	////////////////////////////////////////

	//以下参数不用保存
	int curFrmaeIndex;					//当前帧索引
	std::shared_ptr<VTDecodeImage> firstImage;		//第一帧图片
	//所有DTS
	QVector<int64_t> packetDTSs;
	QMap<QCryptographicHash::Algorithm, QByteArray> mapHashInfo;
	int fileSize = 0;
	//QTextBrowser* logBrowser; // 日志
	//redo undo栈
	QUndoStack undoStack;
};

enum VTToolType   //枚举变量，几个图形的选择
{
	TOOL_NONE,      //没有图形
	TOOL_POINT,     //点
	TOOL_LINE,      //直线
	TOOL_ELLIPSE,   //椭圆
	TOOL_RECTANGLE, //矩形
	TOOL_ERASER,    //橡皮擦
	TOOL_PARTZOOM,  //局部放大图片
	TOOL_UNDO,		//撤销
	TOOL_REDO		//重做
};

enum VTCurrentWidget
{
	PLAYER_WIDGET, //播放窗口
	TAKEN_WIDGET   //编辑窗口
};

struct VTUuid
{
	int createTime;
	QString uuid;
	QString proName;
	bool operator<(const VTUuid& a) const
	{
		return a.createTime < createTime;
	}
};

enum VTPalyerStatus {
	PLAYER_STATUS_PLAY,
	PLAYER_STATUS_PAUSE,
	PLAYER_STATUS_TAKEN,
};

struct VTPoint {
	QPoint point;
	QColor color;
};

struct VTLine {
	QPoint ponit1;
	QPoint ponit2;
	QColor color;
};

// QString转换成std::string
string VTQtStr2StdStr(const QString qstr);

// 清空目录
void VTClearFiles(const QString& temp_path);

// 获取目录下所有.vtmr文件
int VTGetJsonFiles(QString &strDir, QStringList &list);

// 读取工程文件
int VTReadFile2Project(QString file, VTProjectInfo &proInfo);

// 将工程记录到本地
int VTWriteFile4Project(QString file, VTProjectInfo &proInfo);

//删除本地工程文件
int VTDelFileProject(QString uuid);

// 消息弹出框
QMessageBox::StandardButton VTMessageBoxOk(QString text, Qt::WindowFlags flags, QMessageBox::Icon icon, QWidget* parent = nullptr);
QMessageBox::StandardButton VTMessageBoxYesNo(QString text, QWidget* parent = nullptr);
QMessageBox::StandardButton VTMessageBoxOkCancel(QString text, Qt::WindowFlags flags, QMessageBox::Icon icon);

// 对比度调节
void VTAdjustContrast(QImage &Img, int nContrast);

// 饱和度调节
void VTAdjustSaturation(QImage &Img, int nSaturation);

// 亮度调节
void VTAdjustBrightness(QImage &Img, int nBrightness);

QImage VTMat2QImage(cv::Mat mtx);

cv::Mat VTQImage2Mat(QImage &image);

QString VTDate2Chinese(QDate date);

//判断是不是hh:mm:ss
bool VTIsHHMMSS(QString time);
//矩形转换
bool VTTransRect(QRect src, QRect srcBg, QRect dstBg, QRect &dst);
//扩边
bool VTExpandingEdge(QRect src, QRect srcBg, int expand, QRect &dst);
//计算两帧之间时间间隔
bool VTWT2AverageTime(VTProjectInfo &info, float &ds, float &fps);
bool VTFPS2AverageTime(VTProjectInfo &info, float &ds);
//矫正水印
void VTCorrectWaterMark(VTProjectInfo &info);
//检查是否有没检测到的水印
void VTCheckWaterMark(VTProjectInfo &info, Qt::WindowFlags flags);
//判断水印是否在中间位置
bool VTIsCenterRect4WaterTime(QRect src, QRect bg);
bool VTFileHash(QString file, int &fileSize, QCryptographicHash::Algorithm algo,QByteArray &data);
//矫正当前帧水印时间
void VTCorrectionWT(VTProjectInfo &info, int curIndex, const QTime &time);
//数字转中文
QString VTNumToChinese(QString num);
//删除工程目录下的文件夹
void VTRemoveUuidFolder(QString dirPath, QVector<QString>& uuids);
// 获取窗口相对于桌面的位置
QPoint VTDeskPosition();
// 获取窗口句柄
HWND VTFindWindow(QString className, QString windowName);
// 获取桌面路径
QString VTGetDesktopPath();
//毫秒转时分秒.毫秒
QString VTMillisecondsToTimeFormat(int milliseconds);
//秒转时分秒
QString VTSecondsToTimeFormat(int seconds);
