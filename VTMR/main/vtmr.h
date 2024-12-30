#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_vtmr.h"
#include <QList>
#include "Utils.h"
#include <QProcess>
#include <QTextBrowser>
#include <QCheckBox>
#include <QTimer>
#include <QToolBar>
#include <OCRThread.h>
class CPlayerBG;
class CTakenBG;
class CShowTrace;
class CProjectList;
class CPlayerThread;
class CImageEnhance;
class CMagnifier;
class CRulerWidget;
class CWordReport;
class QProgressDialog;
class CInputDialog;
class CTipsDialog;
class COCRThread;

struct VTProjectInfo;
/*
*主窗口界面
*/
class CVTMR : public QMainWindow, IOCRUser
{
	Q_OBJECT

public:
	CVTMR(QWidget *parent = Q_NULLPTR);
	~CVTMR();
	//初始化
	void init();
	//初始化OCR
	bool initOCR();
	//初始化数据库
	void initSqlDatabase();
	//初始化视图
	void initView();
	//初始化菜单
	void initMenu();
	//文件菜单
	void initFileMenu();
	//视频分析菜单
	void initVideoAnalysisMenu();
	//图像分析菜单
	void initImageAnalysisMenu();
	//数据分析菜单
	void initDataAnalysisMenu();
	//重建结果菜单
	void initReconstructionResultMenu();
	//选项菜单
	void initOptionMenu();
	//帮助菜单
	void initHelpMenu();

	//初始化工程目录
	void initProjTree();
	//初始化上边工具条
	void initEditToolBar();
	//初始化右边工具条
	void initViewToolBar();
	//切换窗口
	void switchWidget(VTCurrentWidget w);
	//读取本地配置
	void readSettings();
	//将配置记录到本地
	void writeSettings();

protected:
	bool eventFilter(QObject *obj, QEvent *ev);
	virtual void paintEvent(QPaintEvent *ev);
	virtual void notifyOCRResult(std::shared_ptr<OCRMsg> msg);

public slots:
	//显示播放窗口
	void onShowPlayWidget();
	//显示编辑窗口
	void onShowTakenWidget(bool doOCR);
	//创建工程
	void onCreateNewProject(VTProjectInfo *proInfo);
	//更新工程
	void onUpdateProject(VTProjectInfo *proInfo);
	//切换工程
	void onSwitchProject(VTProjectInfo &proInfo);
	//设置活动工程
	void onActiveProject(QString uuid);
	//显示运行轨迹
	void onShowTrace();
	//放大镜显示图片
	void onImageScale(int scale, QImage *image, QPoint pos);
	void onTimeOut();
	void onProcessBar(int n);
	//工程详情
	void onProjectInfo(QString uuid);
	//删除工程
	void onDelProject(QString uuid);
	//第一帧图片
	void onFirstImage(std::shared_ptr<VTDecodeImage> decodeImage);
	//OCR结果
	void onOCRResult(std::shared_ptr<OCRMsg> msg);
	//显示OCR结果
	void onShowOCR(QString text);

signals:
	//实际点位更新
	void sigActualPointsChange();
	//显示图片
	void sigDrawImage();
	//OCR结果
	void sigOCRResult(std::shared_ptr<OCRMsg> msg);
	//显示OCR结果
	void sigShowOCR(QString text);
private:
	Ui::VTMR ui;
	//上边竖立工具条
	QToolBar *m_editToolBar = nullptr;
	//右边竖立工具条
	QToolBar *m_viewToolBar = nullptr;
	//播放器播放器背景窗口
	CPlayerBG *m_playerBG = nullptr;
	//编辑背景窗口
	CTakenBG *m_takenBG = nullptr;
	//工程列表
	CProjectList *m_projectList = nullptr;
	//运行轨迹
	CShowTrace *m_traceWidget = nullptr;
	//放大镜
	CMagnifier *m_magnifier = nullptr;

	QDockWidget *m_dockProject = nullptr;
	QDockWidget *m_dockImageEnhance = nullptr;
	QDockWidget *m_dockTrace = nullptr;
	QDockWidget *m_dockLog = nullptr;
	QDockWidget *m_dockMagnifier = nullptr;
	QDockWidget *m_dockPlayBG = nullptr;
	QDockWidget *m_dockTakenBG = nullptr;

	//日志
	QTextBrowser *m_logBrowser = nullptr;
	//图像增强
	CImageEnhance *m_imageEnhance = nullptr;
	//尺子
	QVector<CRulerWidget *>	m_rulers;
	QCheckBox *m_checkBox = nullptr;

	//启动初始化定时器
	QTimer *m_timer;
	//报告
	CWordReport *m_wordReport = nullptr;
	QProgressDialog *m_progressDialog = nullptr;

	CInputDialog *m_inputDialog = nullptr;
	CTipsDialog *m_ocrTipsDialog = nullptr;

	QVector<QWidget *> m_blackWidgets;
	//OCR
	COCRThread* m_ocr = nullptr;
	//QStatusBar 显示ocr结果
	QLabel* m_ocrShow = nullptr;
};
