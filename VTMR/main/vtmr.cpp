#include "vtmr.h"
#include "TakenWidget.h"
#include "TakenBG.h"
#include "ProjectWizard.h"
#include <QDateTime>
#include "DecodeVideo.h"
#include "InputWidget.h"
#include <QDir>
#include <InputDialog.h>
#include <QWidgetAction>
#include <QSettings>
#include <QCheckBox>
#include "PointsManage.h"
#include "PlayerBG.h"
#include "ProjectList.h"
#include <QStandardItemModel>
#include "Length2Coordinate.h"
#include "Length4Points.h"
#include "Length2Points.h"
#include "Length3Points.h"
#include "Coordinate2Points.h"
#include "Coordinate3Points.h"
#include "Coordinate4Points.h"
#include "Utils.h"
#include <QMessageBox>
#include <QDockWidget>
#include "Dedistortion.h"
#include <queue>
#include <ShowTrace.h>
#include "ImageEnhance.h"
#include "Magnifier.h"
#include "RulerWidget.h"
#include "WordReport.h"
#include <QFileDialog>
#include "AppraiserDialog.h"
#include <QProgressDialog>
#include <QVector>
#include "SQLManager.h"
#include "OCRThread.h"
#include "TipsDialog.h"
#include "TimeInfoWnd.h"
#include "MeasuringMethodWnd.h"
#include <QActionGroup>
#include <memory>
#include "LogBrowser.h"
#include <DataBase/SerializeData.h>

using std::priority_queue;

CVTMR::CVTMR(QWidget *parent)
	: QMainWindow(parent)
{
	g_vtmr = this;
	ui.setupUi(this);
	ui.actionUndo->setIcon(QIcon(":/DrawTools/images/DrawTools/undogray.png"));
	ui.actionRedo->setIcon(QIcon(":/DrawTools/images/DrawTools/redogray.png"));
	setWindowIcon(QIcon(":/VTMR/images/VTMR/vtmr.ico"));
	auto centralWidget = takeCentralWidget();
	if (centralWidget)
		delete centralWidget;

	setMouseTracking(true);
	setAutoFillBackground(true);

	QFile qss(":/qdarkstyle/style.qss");
	qss.open(QFile::ReadOnly);
	auto ba = qss.readAll();
	setStyleSheet(ba);
	qss.close();
	//qDebug() << "zzq end css";
	g_proDataPath = g_appPath + PROJECT_DATA_PATH;
	QDir dir;
	if (!dir.exists(g_proDataPath))
		dir.mkpath(g_proDataPath);

	connect(this, SIGNAL(sigOCRResult(std::shared_ptr<OCRMsg>)), this, SLOT(onOCRResult(std::shared_ptr<OCRMsg>)));

	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeOut()));
	m_timer->start(100);
	//qDebug() << "zzq end m_timer";
	m_wordReport = new CWordReport(this);
	//connect(m_wordReport, SIGNAL(sigSaveReport(VTProjectInfo, QString)), m_wordReport, SLOT(onSaveReport(VTProjectInfo, QString)));
	connect(m_wordReport, &CWordReport::sigSaveReport, m_wordReport, &CWordReport::onSaveReport);
	connect(m_wordReport, SIGNAL(sigProcessBar(int)), this, SLOT(onProcessBar(int)));
	//qDebug() << "zzq end CVTMR";
}

CVTMR::~CVTMR()
{
	//保存当前项目
	if (!g_proInfo.uuid.isEmpty()) {
		auto file = QString("%1/%2.vtmr").arg(g_proDataPath).arg(g_proInfo.uuid);
		VTWriteFile4Project(file, g_proInfo);
	}

	while (!m_rulers.empty()) {
		auto p = m_rulers.front();
		p->close();
		m_rulers.pop_front();
	}

	CSQLManager::closDataBase();
	if (m_ocr)
		m_ocr->exit();

	//删除多余的项目文件
	QVector<QString> uuids;
	for (auto &proj : g_proInfoList) {
		uuids.append(proj->uuid);
	}
	VTRemoveUuidFolder(g_proDataPath, uuids);
	qApp->exit(0);
}

bool CVTMR::initOCR()
{
	//qDebug() << "begin ocr";
	m_ocr = new COCRThread(this);
	if (0 != m_ocr->init()) {
		//qDebug() << "zzq  CVTMR::initOCR() 初始化OCR失败";
		VTMessageBoxOk(QString("初始化OCR失败"), windowFlags(), QMessageBox::Information);
		return false;
	}
	//qDebug() << "end ocr";
	m_ocr->start();
	//qDebug() << "start ocr";
	//qDebug() << "zzq  CVTMR::initOCR() 初始化成功";
	return true;
}

void CVTMR::onProjectInfo(QString uuid)
{
	auto info = new VTProjectInfo();
	if (uuid == g_proInfo.uuid)
		*info = g_proInfo;
	else {
		for (auto n = 0; n < g_proInfoList.size(); ++n) {
			if (uuid == g_proInfoList[n]->uuid) {
				*info = g_proInfo;
				break;
			}
		}
	}
	auto createProWizard = new CProjectWizard(PROJECT_WIZARD_SHOW, info, QString("工程详情"), this);
	createProWizard->setModal(true);
	createProWizard->exec();
}

void CVTMR::onDelProject(QString uuid)
{
	VTProjectInfo *proInfo;
	auto iter = g_proInfoList.begin();
	for (; iter != g_proInfoList.end(); iter++) {
		proInfo = (VTProjectInfo *)*iter;
		if (proInfo->uuid == uuid)
		{
			g_proInfoList.erase(iter);
			break;
		}
	}
	
	//找出下一个工程
	proInfo = nullptr;
	auto size = g_proInfoList.size();
	if (size > 0)
		proInfo = g_proInfoList[size - 1];

	//删除目录树
	m_projectList->onDelProject(uuid);

	//更新活动工程
	if (g_proInfo.uuid == uuid) {
		if (proInfo)
			onActiveProject(proInfo->uuid);
	}

	if (!proInfo) {
		g_proInfo.uuid = "";
		m_playerBG->ClearPlayer();
	}

	//删除本地数据
	VTDelFileProject(uuid);
}

void CVTMR::onProcessBar(int n)
{
	if (!m_progressDialog)
		return;
	m_progressDialog->setValue(n);
	if (n == 100) {
		m_progressDialog->close();
	}
}

void CVTMR::init()
{
	QDir d(":/");
	qDebug() << "absolutePath: " << d.absolutePath();
	qDebug() << "dirName: " << d.dirName();
	foreach(QFileInfo info, d.entryInfoList()) {
		qDebug() << "absoluteDir: " << info.absoluteDir();
		qDebug() << "absoluteFilePath: " << info.absoluteFilePath();
		qDebug() << "absolutePath: " << info.absolutePath();       //.....其他内容    
	}

	//初始化StatusBar
	QStatusBar* sb = statusBar();
	m_ocrShow = new QLabel(this);
	connect(this, SIGNAL(sigShowOCR(QString)), this, SLOT(onShowOCR(QString)));
	sb->addWidget(m_ocrShow);
	
	//初始化菜单
	initMenu();
	//初始化视图
	initView();
	//初始化视图工具条
	initEditToolBar();
	initViewToolBar();

	connect(m_playerBG, SIGNAL(sigShowTakenWidget(bool)), this, SLOT(onShowTakenWidget(bool)));

	connect(m_takenBG, SIGNAL(sigShowPlayWidget()), this, SLOT(onShowPlayWidget()));
	connect(this, SIGNAL(sigActualPointsChange()), m_takenBG, SLOT(onActualPointsChange()));
	connect(this, SIGNAL(sigDrawImage()), m_takenBG, SLOT(onDrawImage()));
	//////////////////////////////////
	initSqlDatabase();
	//////////////////////////////////
	g_currentWidget = PLAYER_WIDGET;
	switchWidget(g_currentWidget);

	//初始化工程目录
	initProjTree();
}
void CVTMR::initSqlDatabase()
{
	//查询数据
	CAppraiserDialog::loadAppraiser();
	ClinetEditHistorySelectSql();
}

void CVTMR::onTimeOut()
{
	if (m_timer->isActive()) {
		m_timer->stop();
	}
	//qDebug() << "zzq begin init()";
	init();
	installEventFilter(this);
	installEventFilter(m_playerBG);
	installEventFilter(m_takenBG->takenAreaWidget());
	installEventFilter(m_playerBG->getPlayer());
	

	QVector<QWidget *> blackWidgets  = m_playerBG->getBlackWidgets();
	m_blackWidgets = blackWidgets;

	for(auto n = 0; n < m_blackWidgets.size(); ++n)
		installEventFilter(m_blackWidgets[n]);
}

void CVTMR::paintEvent(QPaintEvent *ev)
{
	QWidget::paintEvent(ev);
}

void CVTMR::initView()
{
	QWidget* titleBar = nullptr;
	QWidget* emptyWidget = nullptr;
	setDockNestingEnabled(true);

	//工程目录
	m_projectList = new CProjectList(this);
	m_dockProject = new QDockWidget("工程目录", this);
	m_dockProject->setMinimumWidth(280);
	m_dockProject->setMaximumWidth(280);
	m_dockProject->setWidget(m_projectList);
	m_dockProject->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);

	//播放器
	m_playerBG = new CPlayerBG(this);
	g_playerBG = m_playerBG;
	m_dockPlayBG = new QDockWidget(QString("播放器"), this);
	m_dockPlayBG->setWidget(m_playerBG);
	m_dockPlayBG->setFeatures(QDockWidget::NoDockWidgetFeatures);
	titleBar = m_dockPlayBG->titleBarWidget();
	emptyWidget = new QWidget();
	m_dockPlayBG->setTitleBarWidget(emptyWidget);
	delete titleBar;

	//放大镜
	m_magnifier = new CMagnifier(this);
	m_dockMagnifier = new QDockWidget(QString("放大镜"), this);
	m_dockMagnifier->setFixedSize(280, 280);
	m_dockMagnifier->setWidget(m_magnifier);
	m_dockMagnifier->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);

	//图像增强
	m_imageEnhance = new CImageEnhance(this);
	m_dockImageEnhance = new QDockWidget(QString("图像增强"), this);
	m_dockImageEnhance->setFixedWidth(280);
	m_dockImageEnhance->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
	m_dockImageEnhance->setWidget(m_imageEnhance);
	m_dockImageEnhance->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);

	//运行状态
	m_traceWidget = new CShowTrace(this);
	m_traceWidget->setFocusPolicy(Qt::ClickFocus);
	m_dockTrace = new QDockWidget(QString("运行状态"), this);
	m_dockTrace->setWidget(m_traceWidget);
	//m_dockTrace->setFixedWidth(500);
	m_dockTrace->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);

	//提取界面
	m_takenBG = new CTakenBG(this, m_imageEnhance, m_traceWidget, this);
	m_dockTakenBG = new QDockWidget(QString("提取界面"), this);
	m_dockTakenBG->setWidget(m_takenBG);
	m_dockTakenBG->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
	titleBar = m_dockTakenBG->titleBarWidget();
	emptyWidget = new QWidget();
	m_dockTakenBG->setTitleBarWidget(emptyWidget);
	delete titleBar;
	g_takenBG = m_takenBG;

	//日志
	m_logBrowser = new CLogBrowser(this);
	m_dockLog = new QDockWidget(QString("日志"), this);
	m_dockLog->setWidget(m_logBrowser);
	m_dockLog->setFixedWidth(280);
	m_dockLog->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
	g_logBrowser = m_logBrowser;

	//连接信号
	connect(m_projectList, SIGNAL(sigActiveProject(QString)), this, SLOT(onActiveProject(QString)));
	connect(m_projectList, SIGNAL(sigActivePlayer()), this, SLOT(onShowPlayWidget()));

	connect(m_imageEnhance, &CImageEnhance::sigBrightnessValueChanged, [&](int value) {
		if (!m_takenBG)
			return;
		if (g_proInfo.frameInfos.size() == 0 || g_proInfo.curFrmaeIndex > g_proInfo.frameInfos.size() - 1)
			return;
		if (g_brightness == value)
			return;
		g_brightness = value;
		emit sigDrawImage();
	});

	connect(m_imageEnhance, &CImageEnhance::sigContrastValueChanged, [&](int value) {
		if (!m_takenBG)
			return;
		if (g_proInfo.frameInfos.size() == 0 || g_proInfo.curFrmaeIndex > g_proInfo.frameInfos.size() - 1)
			return;
		if (g_contrast == value)
			return;
		g_contrast = value;
		emit sigDrawImage();
	});

	connect(m_imageEnhance, &CImageEnhance::sigSaturationValueChanged, [&](int value) {
		if (!m_takenBG)
			return;
		if (g_proInfo.frameInfos.size() == 0 || g_proInfo.curFrmaeIndex > g_proInfo.frameInfos.size() - 1)
			return;
		if (g_saturation == value)
			return;
		g_saturation = value;
		emit sigDrawImage();
	});
}

//文件菜单
void CVTMR::initFileMenu()
{
	//新建工程
	connect(ui.actionNewProject, &QAction::triggered, 
		[=]() {
		if (CAppraiserDialog::getInfo().size() == 0) {
			VTMessageBoxOk(QString("请先录入鉴定主体信息！"), windowFlags(), QMessageBox::Information, this);
			return;
		}

		auto createProWizard = new CProjectWizard(PROJECT_WIZARD_CREATE, nullptr, QString("创建工程向导"), this);
		createProWizard->setModal(true);
		createProWizard->exec();
	});

	//保存
	connect(ui.actionSave, &QAction::triggered,
		[=]() {
		if (!g_proInfo.uuid.isEmpty()) {
			auto file = QString("%1/%2.vtmr").arg(g_proDataPath).arg(g_proInfo.uuid);
			VTWriteFile4Project(file, g_proInfo);
		}

	});

	connect(ui.actionRuler, &QAction::triggered,
		[=]() {
		CRulerWidget *process = new CRulerWidget();
		process->show();
		m_rulers.push_back(process);
	});

	//退出
	connect(ui.actionExit, &QAction::triggered,
		[=]()
	{
		close();
	});
	
	//未实现灰色处理
	ui.actionSaveAs->setDisabled(true);
	ui.actionOpenProject->setDisabled(true);
}

//视频分析菜单
void CVTMR::initVideoAnalysisMenu()
{
	//选择标定方式
	connect(ui.actionCalibrationMethod, &QAction::triggered,
		[=]() {
		auto measuringMethodWnd = new CMeasuringMethodWnd(this);
		measuringMethodWnd->exec();
		m_takenBG->modeChanged();
	});

	//启用光溜跟踪
	auto subMenu = new QMenu("特征跟踪");
	ui.menuVideoAnalysis->insertMenu(ui.actionCalibrationMethod, subMenu);
	auto wa = new QWidgetAction(0);
	m_checkBox = new QCheckBox("启用/禁用");
	wa->setDefaultWidget(m_checkBox);
	subMenu->addAction(wa);
	connect(m_checkBox, &QCheckBox::stateChanged,
		[=](int state)
	{
		g_checkKeyTrace = state;
	});

	//未实现灰色处理
	ui.actionWaterMark->setDisabled(true);
}

//图像分析菜单
void CVTMR::initImageAnalysisMenu()
{
	//时间信息
	connect(ui.actionTimeInfo, &QAction::triggered,
		[=]() {
		auto timeInfoWnd = new CTimeInfoWnd(this);
		timeInfoWnd->exec();
	});

	//相机标定
	connect(ui.actionCameraCalibration, &QAction::triggered,
		[=]() {
		auto dedistortion = new CDedistortion(this);
		dedistortion->exec();
	});

	//参照物的距离输入
	connect(ui.actionLength, &QAction::triggered,
		[=]() {
		if (g_proInfo.mode == CALIBRATION_UNKNOWN) {
			VTMessageBoxOk(QString("请先选择标定方式！\r\n视频分析  >选择标定方式"), windowFlags(), QMessageBox::Information, this);
			return;
		}
		m_inputDialog = new CInputDialog(0, g_proInfo.timeMode, this);
		CLength2Coordinate *length2Coordinates = nullptr;
		if (FIXED_CAM_ONESELF == g_proInfo.mode || MOVING_CAM_SPACE == g_proInfo.mode || FIXED_CAM_GROUND_REFERENCE == g_proInfo.mode)
			length2Coordinates = new CLength2Points(m_inputDialog);
		else if (FIXED_CAM_1D == g_proInfo.mode)
			length2Coordinates = new CLength3Points(m_inputDialog);
		else
			length2Coordinates = new CLength4Points(m_inputDialog);

		length2Coordinates->setReferenceDes(g_proInfo.referenceDesFrom, g_proInfo.referenceDesTo);
		length2Coordinates->init(g_proInfo.actualPoints);
		m_inputDialog->setWidget(length2Coordinates);

		auto res = m_inputDialog->exec();
		if (res == QDialog::Accepted) {
			g_proInfo.actualPoints = length2Coordinates->getActualPoints();
			g_proInfo.referenceLen1 = sqrtf((g_proInfo.actualPoints[0].x() - g_proInfo.actualPoints[1].x())*(g_proInfo.actualPoints[0].x() - g_proInfo.actualPoints[1].x())
				- (g_proInfo.actualPoints[0].y() - g_proInfo.actualPoints[1].x())*(g_proInfo.actualPoints[0].x() - g_proInfo.actualPoints[1].y()));
			length2Coordinates->getReferenceDes(g_proInfo.referenceDesFrom, g_proInfo.referenceDesTo);
		}
	});

	//参照物坐标输入
	connect(ui.actionCoordinate, &QAction::triggered,
		[=]() {
		if (g_proInfo.mode == CALIBRATION_UNKNOWN) {
			VTMessageBoxOk(QString("请先选择标定方式！\r\n视频分析  >选择标定方式"), windowFlags(), QMessageBox::Information, this);
			return;
		}
		m_inputDialog = new CInputDialog(1, g_proInfo.timeMode, this);
		CInputWidget * actualCoordinate = nullptr;
		if (FIXED_CAM_ONESELF == g_proInfo.mode || MOVING_CAM_SPACE == g_proInfo.mode || FIXED_CAM_GROUND_REFERENCE == g_proInfo.mode)
			actualCoordinate = new CCoordinate2Points(this);
		else if (FIXED_CAM_1D == g_proInfo.mode)
			actualCoordinate = new CCoordinate3Points(this);
		else
			actualCoordinate = new CCoordinate4Points(this);
		actualCoordinate->initActual(g_proInfo.actualPoints);

		m_inputDialog->setWidget(actualCoordinate);
		auto res = m_inputDialog->exec();
		if (res == QDialog::Accepted) {
			g_proInfo.actualPoints = actualCoordinate->getActualPoints();
			if (m_takenBG)
				emit sigActualPointsChange();
		}
	});

	//未实现灰色处理
	ui.actionImageFormat->setDisabled(true);
	ui.actionDistortionParameter->setDisabled(true);
}

//数据分析菜单
void CVTMR::initDataAnalysisMenu()
{
	//未实现灰色处理
	ui.actionFeatureTrackingCoordinates->setDisabled(true);
	ui.actionAbnormalPointDetection->setDisabled(true);
}

//重建结果菜单
void CVTMR::initReconstructionResultMenu()
{
	//文书报告
	connect(ui.actionWordReport, &QAction::triggered,
		[=]() {
		QString fileName;
		switch (g_proInfo.mode) {
		case FIXED_CAM_ONESELF: {
			break;
		}
		case MOVING_CAM_SPACE: {
			break;
		}
		default: {
			VTMessageBoxOk(QString("没有可用的模板"), windowFlags(), QMessageBox::Information, this);
			return;
		}
		}
		QString defaultFileName = QDir::homePath() + "/司法鉴定科学研究院司法鉴定意见书.docx";
		QString savePath = QFileDialog::getSaveFileName(
			this,
			"另存为",
			defaultFileName,
			"Word 文档 (*.docx);;所有文件 (*.*)"
		);

		if (savePath.isEmpty()) {
			qDebug() << "用户取消了保存操作。";
			return;
		}


		m_progressDialog = new QProgressDialog(QString("正在生成报告..."), QString("取消"), 0, 100, this, windowFlags() | Qt::FramelessWindowHint);

		emit m_wordReport->sigSaveReport(g_proInfo, savePath);
		m_progressDialog->setWindowModality(Qt::WindowModal);
		m_progressDialog->setMinimumDuration(5);
		m_progressDialog->setWindowTitle(QString("请稍后"));
		m_progressDialog->setCancelButtonText(NULL);
		m_progressDialog->show();
	});

	//未实现灰色处理
	ui.actionResultReport->setDisabled(true);
}

//选项菜单
void CVTMR::initOptionMenu()
{
	//录入鉴定主体信息
	connect(ui.actionAppraiser, &QAction::triggered,
		[=]() {
		auto appraiserDialog = new CAppraiserDialog(this);
		appraiserDialog->exec();
	});

	//未实现灰色处理
	ui.actionAccountSettings->setDisabled(true);
	ui.actionBackgroundColor->setDisabled(true); 
	ui.actionNumberOfHistoricalProject->setDisabled(true);
	ui.actionWaterMarkParameters->setDisabled(true);
	ui.actionApplicableStandards->setDisabled(true);
	ui.actionWordReportStyle->setDisabled(true);
}

//帮助菜单
void CVTMR::initHelpMenu()
{
	connect(ui.actionAbout, &QAction::triggered,
		[=]() {
		qint64 timestamp = g_authDate.toLongLong();  // 将字符串转换为 qint64 类型的时间戳
		QDateTime dateTime = QDateTime::fromSecsSinceEpoch(timestamp);  // 将时间戳转换为 QDateTime 对象
		QString formattedDateTime = dateTime.toString("yyyy-MM-dd hh:mm:ss");
		int days = g_authTime.toInt() / 24;
		// 获取当前日期时间
		QDateTime currentDateTime = QDateTime::currentDateTime();
		// 获取日期差距
		QDate timestampDate = dateTime.date();
		QDate currentDate = currentDateTime.date();
		int daysDifference = timestampDate.daysTo(currentDate);
		VTMessageBoxOk(QString("视频速度重建系统\n(Video Traffic Moment Reconstruct)\n\n单位：司法鉴定科学研究院\n技术支持：上海弘目智能科技有限公司\n\n版本%1\n\n授权日期：%2\n\n剩余天数：%3天").arg(APP_VERSION).arg(formattedDateTime).arg(days - daysDifference), windowFlags(), QMessageBox::NoIcon, this);
	});
}

void CVTMR::initMenu()
{
	//菜单栏

	 //文件菜单
	initFileMenu();
	//视频分析菜单
	initVideoAnalysisMenu();
	//图像分析菜单
	initImageAnalysisMenu();
	//数据分析菜单
	initDataAnalysisMenu();
	//重建结果菜单
	initReconstructionResultMenu();
	//选项菜单
	initOptionMenu();
	//帮助菜单
	initHelpMenu();
}

static void sort_project(QVector<VTProjectInfo *> &proInfo)
{
	int i, j;
	VTProjectInfo *tmp;
	int len = proInfo.size();
	for (i = 0; i < len - 1; ++i) {
		for (j = 0; j < len - 1 - i; ++j) {
			if (proInfo[j]->createTime > proInfo[j + 1]->createTime) {
				tmp = proInfo[j];
				proInfo[j] = proInfo[j + 1];
				proInfo[j + 1] = tmp;
			}
		}
	}
}

void CVTMR::initProjTree()
{
	QStringList strList;
	VTGetJsonFiles(g_proDataPath, strList);
	priority_queue<VTUuid> queUID;
	QVector<QString> uuids;
	auto size = 0;

	for (auto n = 0; n < strList.size(); n++) {
		QString file = g_proDataPath + "/" + strList[n];
		VTProjectInfo *projTemp = new VTProjectInfo;
		if (0 == VTReadFile2Project(file, *projTemp)) {
			g_proInfoList.push_back(projTemp);
			VTUuid temp;
			temp.createTime = projTemp->createTime;
			temp.uuid = projTemp->uuid;
			temp.proName = projTemp->projectName;
			queUID.push(temp);
			uuids.push_back(temp.uuid);
		}
	}

	sort_project(g_proInfoList);
	size = g_proInfoList.size();
	for (auto n = 0; n < size; ++n)
		m_projectList->addProject(*g_proInfoList[n]);
	if (size > 0)
		m_projectList->onActiveProject(g_proInfoList[size - 1]->uuid);
}

void CVTMR::initEditToolBar()
{
	m_editToolBar = new QToolBar(QString("工具"), this);
	m_editToolBar->setAllowedAreas(Qt::RightToolBarArea | Qt::TopToolBarArea);
	m_editToolBar->addAction(ui.actionPoint);
	m_editToolBar->addAction(ui.actionLine);
	m_editToolBar->addAction(ui.actionEllipse);
	m_editToolBar->addAction(ui.actionRectangle);
	m_editToolBar->addAction(ui.actionEraser);
	m_editToolBar->addAction(ui.actionPartZoom);
	
	m_editToolBar->addAction(ui.actionUndo);
	m_editToolBar->addAction(ui.actionRedo);

	auto group = new QActionGroup(this);
	group->addAction(ui.actionPoint);
	group->addAction(ui.actionLine);
	group->addAction(ui.actionEllipse);
	group->addAction(ui.actionRectangle);
	group->addAction(ui.actionEraser);
	group->addAction(ui.actionPartZoom);
	group->addAction(ui.actionUndo);
	group->addAction(ui.actionRedo);

	connect(ui.actionPoint, SIGNAL(toggled(bool)), m_takenBG, SLOT(onActionPoint(bool)));
	connect(ui.actionEraser, SIGNAL(toggled(bool)), m_takenBG, SLOT(onActionEraser(bool)));
	connect(ui.actionLine, SIGNAL(toggled(bool)), m_takenBG, SLOT(onActionLine(bool)));
	connect(ui.actionRectangle, SIGNAL(toggled(bool)), m_takenBG, SLOT(onActionRectangle(bool)));
	connect(ui.actionEllipse, SIGNAL(toggled(bool)), m_takenBG, SLOT(onActionEllipse(bool)));
	connect(ui.actionPartZoom, SIGNAL(toggled(bool)), m_takenBG, SLOT(onActionPartZoom(bool)));
	//connect(ui.actionUndo, &QAction::triggered, m_takenBG, SLOT(onActionUndo()));
	connect(ui.actionUndo, &QAction::triggered, [=]() {
		m_takenBG->onActionUndo();
	});
	connect(ui.actionRedo, &QAction::triggered, [=]() {
		m_takenBG->onActionRedo();
	});
}

void CVTMR::initViewToolBar()
{
	m_viewToolBar = new QToolBar(QString("视图"), this);
	m_viewToolBar->setAllowedAreas(Qt::RightToolBarArea | Qt::BottomToolBarArea);

	m_viewToolBar->addAction(ui.actionPlayer);
	m_viewToolBar->addAction(ui.actionTrace);
	m_viewToolBar->addAction(ui.actionMagnifier);
	m_viewToolBar->addAction(ui.actionImageEnhance);
	m_viewToolBar->addAction(ui.actionFlowLog);

	//连接
	connect(ui.actionCatalog, &QAction::triggered,
		[=]() {
		if (m_dockProject->isVisible())
			m_dockProject->hide();
		else
			m_dockProject->show();
		update();
	});

	connect(ui.actionPlayer, &QAction::triggered,
		[=]() {
		onShowPlayWidget();
		update();
	});

	connect(ui.actionTrace, SIGNAL(triggered()), this, SLOT(onShowTrace()));

	connect(ui.actionFlowLog, &QAction::triggered,
		[=]() {
		if (m_dockLog->isVisible())
			m_dockLog->hide();
		else
			m_dockLog->show();
		update();
	});

	connect(ui.actionMagnifier, &QAction::triggered,
		[=]() {
		if (m_dockMagnifier->isVisible())
			m_dockMagnifier->hide();
		else
			m_dockMagnifier->show();
		update();
	});

	connect(ui.actionImageEnhance, &QAction::triggered,
		[=]() {
		if (m_dockImageEnhance->isVisible())
			m_dockImageEnhance->hide();
		else
			m_dockImageEnhance->show();
		update();
	});
}

void CVTMR::switchWidget(VTCurrentWidget w)
{
	/*if (w == PLAYER_WIDGET) {
		QSettings settings(g_appPath + CONF_DOCK_PATH, QSettings::IniFormat);
		settings.beginGroup("taken_window");
		settings.setValue("state", saveState());
		settings.setValue("geometry", saveGeometry());
		settings.endGroup();
	}*/

	removeToolBar(m_editToolBar);
	removeToolBar(m_viewToolBar);

	removeDockWidget(m_dockProject);
	removeDockWidget(m_dockPlayBG);

	removeDockWidget(m_dockMagnifier);
	removeDockWidget(m_dockTakenBG);
	removeDockWidget(m_dockTrace);
	removeDockWidget(m_dockImageEnhance);
	removeDockWidget(m_dockLog);

	if (w == PLAYER_WIDGET) {

		m_editToolBar->hide();
		m_viewToolBar->hide();

		m_dockTakenBG->hide();

		m_dockMagnifier->hide();
		m_dockTrace->hide();
		m_dockImageEnhance->hide();
		m_dockLog->hide();

		m_dockProject->show();
		m_dockPlayBG->show();

		addDockWidget(Qt::LeftDockWidgetArea, m_dockProject);
		addDockWidget(Qt::RightDockWidgetArea, m_dockPlayBG);
	}
	else if (w == TAKEN_WIDGET) {
		m_editToolBar->show();
		m_viewToolBar->show();

		addToolBar(Qt::TopToolBarArea, m_editToolBar);
		addToolBar(Qt::RightToolBarArea, m_viewToolBar);

		m_dockProject->hide();
		m_dockPlayBG->hide();

		m_dockTakenBG->show();

		m_dockMagnifier->show();
		m_dockTrace->show();
		m_dockImageEnhance->show();
		m_dockLog->show();

		addDockWidget(Qt::LeftDockWidgetArea, m_dockMagnifier);
		splitDockWidget(m_dockMagnifier, m_dockImageEnhance, Qt::Vertical);
		splitDockWidget(m_dockImageEnhance, m_dockLog, Qt::Vertical);

		addDockWidget(Qt::RightDockWidgetArea, m_dockTakenBG);
		splitDockWidget(m_dockTakenBG, m_dockTrace, Qt::Horizontal);

		/*QSettings settings(g_appPath + CONF_DOCK_PATH, QSettings::IniFormat);
		settings.beginGroup("taken_window");
		QByteArray windowState = settings.value("state").toByteArray();
		QByteArray windowGeometry = settings.value("geometry").toByteArray();
		settings.endGroup();

		if (!windowState.isEmpty())
			restoreState(windowState);

		if (!windowGeometry.isEmpty())
			restoreGeometry(windowGeometry);*/
	}
}

void CVTMR::onSwitchProject(VTProjectInfo & proInfo)
{
	if (!g_proInfo.uuid.isEmpty() && g_proInfo.uuid == proInfo.uuid)
		return;
	m_takenBG->initPointsManage(proInfo);

	g_brightness = 0;
	g_contrast = 0;
	g_saturation = 0;
	//保存当前项目
	if (!g_proInfo.uuid.isEmpty()) {
		auto file = QString("%1/%2.vtmr").arg(g_proDataPath).arg(g_proInfo.uuid);
		VTWriteFile4Project(file, g_proInfo);
	}
	
	for (auto info : g_proInfoList) {
		if (info->uuid == g_proInfo.uuid) {
			*info = g_proInfo;
			break;
		}
	}
	// 清空g_log
	g_logBrowser->clear();
	//更换当前项目
	auto has = false;
	g_proInfo = proInfo;
	for (auto info : g_proInfoList) {
		if (info->uuid == g_proInfo.uuid)
		{
			*info = g_proInfo;
			has = true;
			break;
		}
	}

	//通知工程目录
	if (!has) {
		m_projectList->addProject(g_proInfo);
		m_projectList->onActiveProject(g_proInfo.uuid);
	}
	else {
		//已经存在的项目,就是为了读取log到g_log
		auto file = QString("%1/%2.vtmr").arg(g_proDataPath).arg(g_proInfo.uuid);
		VTReadFile2Project(file, g_proInfo);
	}
	qDebug() << g_logBrowser->size();
	//通知播放器
	m_playerBG->onChangeProject();
	m_checkBox->setChecked(false);
}

void CVTMR::onShowTakenWidget(bool doOCR)
{
	m_takenBG->initPointsManage(g_proInfo);
	g_currentWidget = TAKEN_WIDGET;
	
	switchWidget(g_currentWidget);

	if (doOCR && g_proInfo.frameInfos.size() > 0 && g_proInfo.firstImage) {
		QQueue<std::shared_ptr<OCRMsg> > queMsg;
		auto size = g_proInfo.frameInfos.size();
		for (auto n = 0; n < size; ++n) {
			std::shared_ptr<OCRMsg> ocrMsg(new OCRMsg());
			ocrMsg->uuid = g_proInfo.uuid;
			ocrMsg->user = m_takenBG;
			
			std::shared_ptr<VTDecodeImage> decodeImage(new VTDecodeImage());
			decodeImage->uuid = g_proInfo.uuid;
			decodeImage->waterMarkHMS.rect = g_proInfo.firstImage->waterMarkHMS.rect;
			decodeImage->pts = g_proInfo.frameInfos[n].pts;
			decodeImage->imagePath = g_proInfo.frameInfos[n].imagePath;

			ocrMsg->decodeImage = decodeImage;
			if (n == size - 1)
				ocrMsg->last = true;
			queMsg.push_back(ocrMsg);
		}
		size = queMsg.size();
		qDebug() << QString("即将检测OCR %1 张图片").arg(queMsg.size());
		m_ocr->sigOCRs(queMsg);
	}
}

void CVTMR::onShowPlayWidget()
{
	g_currentWidget = PLAYER_WIDGET;
	switchWidget(g_currentWidget);
}

void CVTMR::onCreateNewProject(VTProjectInfo *proInfo)
{

	onSwitchProject(*proInfo);
	g_proInfoList.push_back(proInfo);
	//delete proInfo;
}

void CVTMR::onUpdateProject(VTProjectInfo *proInfo)
{
	if (g_proInfo.uuid == proInfo->uuid)
		g_proInfo = *proInfo;
	for (auto info : g_proInfoList) {
		if (info->uuid == g_proInfo.uuid) {
			*info = *proInfo;
			break;
		}
	}
	emit m_projectList->sigUpdateProject(proInfo);
}

void CVTMR::writeSettings()
{
	QSettings settings("Software Inc.", "Icon Editor");
	settings.beginGroup("CVTMR");
	settings.setValue("size", size());
	settings.setValue("state", saveState());
	settings.endGroup();
}

void CVTMR::readSettings()
{
	QSettings settings("Software Inc.", "Icon Editor");
	settings.beginGroup("CVTMR");
	resize(settings.value("size").toSize());
	restoreState(settings.value("state").toByteArray());
	settings.endGroup();
}

void CVTMR::onActiveProject(QString uuid)
{
	auto find = -1;
	for (auto n = 0; n < g_proInfoList.size(); ++n) {
		if (uuid == g_proInfoList[n]->uuid) {
			find = n;
			break;
		}
	}
	if (find == -1)
		return;
	onSwitchProject(*g_proInfoList[find]);
}

void CVTMR::onShowTrace()
{
	if (m_dockTrace->isVisible())
		m_dockTrace->hide();
	else
	{
		m_dockTrace->show();
		QPolygonF points;
		int64_t pts = 0;
		m_traceWidget->drawVPoints();

		QVector<QPointF> p0s;
		QVector<int64_t> ptss;
		CPointsManage::getMPoints(g_proInfo, p0s, ptss);

		auto width = 1920, height = 1080;
		if (g_proInfo.frameInfos.size() > 0) {
			if (g_proInfo.firstImage && g_proInfo.firstImage->image) {
				width = g_proInfo.firstImage->image->width();
				height = g_proInfo.firstImage->image->height();
			}
		}

		QVector<QPointF> actualPoints;
		if (g_proInfo.mode == FIXED_CAM_ONESELF || g_proInfo.mode == FIXED_CAM_GROUND_REFERENCE || g_proInfo.mode == MOVING_CAM_SPACE)
			m_traceWidget->drawMPixPoints(width, height, p0s, actualPoints, ptss);
		else
			m_traceWidget->drawMPoints(p0s, g_proInfo.actualPoints, ptss);
	}
}

void CVTMR::onImageScale(int scale, QImage *image, QPoint pos)
{
	m_magnifier->setMultiple(scale);
	m_magnifier->setLabelPixmap(image, pos);
	delete image;
}

bool CVTMR::eventFilter(QObject *obj, QEvent *ev)
{
	QKeyEvent *event = nullptr;

	if (ev->type() == QEvent::KeyPress) {
		event = static_cast<QKeyEvent*> (ev);
		for (auto n = 0; n < m_blackWidgets.size(); ++n) {
			if (obj == m_blackWidgets[n]) {
				event->ignore();
				return QMainWindow::eventFilter(obj, ev);
			}
		}
	}
	else if (ev->type() == QEvent::KeyRelease) {
		event = static_cast<QKeyEvent*> (ev);
		for (auto n = 0; n < m_blackWidgets.size(); ++n) {
			if (obj == m_blackWidgets[n]) {
				event->ignore();
				return QMainWindow::eventFilter(obj, ev);
			}
		}
	}

	return QMainWindow::eventFilter(obj, ev);
}

void CVTMR::notifyOCRResult(std::shared_ptr<OCRMsg> msg)
{
	emit sigOCRResult(msg);
}

void CVTMR::onOCRResult(std::shared_ptr<OCRMsg> msg)
{
	g_proInfo.firstImage = msg->decodeImage;

	//关闭提示框
	if (m_ocrTipsDialog)
		emit m_ocrTipsDialog->sigClose();
	//更新水印
	m_playerBG->updateOCRRect();
	//status bar显示
	auto log = QString("首帧水印[%1]").arg(g_proInfo.firstImage->waterMarkHMS.text);
	emit sigShowOCR(log);
}

void CVTMR::onShowOCR(QString text)
{
	m_ocrShow->setText(text);
}

void CVTMR::onFirstImage(std::shared_ptr<VTDecodeImage> decodeImage)
{
	VTWaterMark waterMarkHMS;					//水印时分秒
	QRect rectHMS = QRect(0, 0, 0, 0);		//时分秒水印框
	QVector<VTWaterMark> waterMarks;  //所有水印框
	if (g_proInfo.firstImage) {
		waterMarkHMS = g_proInfo.firstImage->waterMarkHMS;
		waterMarks = g_proInfo.firstImage->waterMarks;
		g_proInfo.firstImage.reset();
	}
	g_proInfo.firstImage = decodeImage;
	g_proInfo.firstImage->waterMarkHMS = waterMarkHMS;
	g_proInfo.firstImage->waterMarks = waterMarks;
	//画水印框
	m_playerBG->updateOCRRect();

	if (waterMarkHMS.rect != QRect(0, 0, 0, 0))
		return;

	std::shared_ptr<OCRMsg> ocrMsg(new OCRMsg());
	ocrMsg->uuid = g_proInfo.uuid;
	ocrMsg->user = this;

	ocrMsg->decodeImage = std::make_shared<VTDecodeImage>();
	*ocrMsg->decodeImage = *decodeImage;
	ocrMsg->firstImage = true;

	m_ocr->sigOCR(ocrMsg);
	m_ocrTipsDialog = new CTipsDialog(QString("正在提取水印..."), this);
	m_ocrTipsDialog->exec();
}
