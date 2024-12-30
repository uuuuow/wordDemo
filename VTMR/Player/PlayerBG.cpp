#include "PlayerBG.h"
#include "Global.h"
#include <QThread>
#include "Utils.h"
#include "vtmr.h"
#include "PlayerWidget.h"
#include <QMessageBox>
#include "DecodeThread.h"
#include <QDebug>
#include "DecodeVideo.h"
#include "Global.h"
#include "TakenBG.h"
#include <QFile>

extern "C"
{
#include "libavutil/time.h"
}

CPlayerBG::CPlayerBG(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.labelCurTime->setFocusPolicy(Qt::NoFocus);
	ui.sliderPlayer->setFocusPolicy(Qt::NoFocus);
	ui.labelDuration->setFocusPolicy(Qt::NoFocus);
	ui.cmboxSpeed->setFocusPolicy(Qt::NoFocus);
	ui.btnBack5s->setFocusPolicy(Qt::NoFocus);
	ui.btnBack->setFocusPolicy(Qt::NoFocus);
	ui.btnPlay->setFocusPolicy(Qt::NoFocus);
	ui.btnForward->setFocusPolicy(Qt::NoFocus);
	ui.btnForward5s->setFocusPolicy(Qt::NoFocus);
	ui.btnTaken->setFocusPolicy(Qt::NoFocus);
	//qDebug() << "player width:" << ui.PlayerWidget->width();
	//qDebug() << "player height:" << ui.PlayerWidget->height();
	QStringList strTemp = { "1X","2X","3X","4X" };
	ui.cmboxSpeed->addItems(strTemp);
	ui.cmboxSpeed->setMaximumWidth(50);
	ui.btnPlay->setIcon(QIcon(QString::fromLatin1(":/Player/images/Player/play.svg")));

	m_decodeThread = new CDecodeThread(ui.PlayerWidget, this);
	m_decodeThread->start();

	connect(m_decodeThread, SIGNAL(sigFPS(int)), this, SLOT(onFPS(int)));
	//上一帧
	connect(ui.btnBack, SIGNAL(clicked()), this, SLOT(onBack()));
	//下一帧
	connect(ui.btnForward, SIGNAL(clicked()), this, SLOT(onForward()));

	//快退5秒
	connect(ui.btnBack5s, SIGNAL(clicked()), this, SLOT(onBack5s()));
	//快进5秒
	connect(ui.btnForward5s, SIGNAL(clicked()), this, SLOT(onForward5s()));

	//显示播放时间
	connect(m_decodeThread, SIGNAL(sigPlayerTime(qint64, qint64)), this, SLOT(onPlayerTime(qint64, qint64)));
	//显示画面
	connect(m_decodeThread, SIGNAL(sigPlayerImage(std::shared_ptr<VTDecodeImage>)), this, SLOT(onPlayerImage(std::shared_ptr<VTDecodeImage>)));
	//播放倍速
	connect(ui.cmboxSpeed, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		[=](int index) {
		if (g_proInfo.uuid == "")
			return;
		m_decodeThread->playSpeed(index + 1); 
	});
	//播放、暂停
	connect(ui.btnPlay, SIGNAL(clicked()), SLOT(onPlayPause()));
	//提取
	connect(ui.btnTaken, SIGNAL(clicked()), SLOT(onTaken()));

	ui.sliderPlayer->installEventFilter(this);
	//显示OCR框
	connect(ui.PlayerWidget, SIGNAL(sigUpdateFirstImage()), ui.PlayerWidget, SLOT(onUpdateFirstImage()));
}

CPlayerBG::~CPlayerBG()
{
	m_decodeThread->exit();
}

bool CPlayerBG::eventFilter(QObject *obj, QEvent *ev)
{
	if (ev->type() == QEvent::KeyPress) {
		QKeyEvent *event = static_cast<QKeyEvent*> (ev);
		if (event->key() == Qt::Key_Space) {
			event->ignore();
		}
	}
	else if (ev->type() == QEvent::KeyRelease) {
		QKeyEvent *event = static_cast<QKeyEvent*> (ev);
		if (event->key() == Qt::Key_Space) {
			onPlayPause();
			event->ignore();
		}
	}
	else if (ev->type() == QEvent::MouseButtonPress) {
		if (obj == ui.sliderPlayer) {
			onSliderPress();
		}
	}
	else if (ev->type() == QEvent::MouseButtonRelease) {
		if (obj == ui.sliderPlayer) {
			onSliderRelease();
		}

	}
	return QWidget::eventFilter(obj, ev);
}

void CPlayerBG::onDecodeStatus(DecodeResponse *response)
{
	if (g_proInfo.uuid != response->uuid) {
		delete response;
		return;
	}
	switch (response->msg)
	{
	case DECODE_MSG_INIT:
	{
		if (response->code != 0) {
			ui.PlayerWidget->setInitVideo(false);
			if (!QFile::exists(g_proInfo.videoPath))
				VTMessageBoxOk(QString("视频不存在！%1").arg(g_proInfo.videoPath), windowFlags(), QMessageBox::Information, this);
			else
				VTMessageBoxOk(QString("视频初始化失败！%1").arg(g_proInfo.videoPath), windowFlags(), QMessageBox::Information, this);
		}
		else {
			ui.PlayerWidget->initVideoInfo(g_proInfo.videoPath,
				response->fps, response->duration, response->countFrames);
			ui.PlayerWidget->setInitVideo(true);
			g_proInfo.fps = response->fps;
			g_proInfo.startTime = response->startTime;
			g_proInfo.duration = response->duration;
			g_proInfo.packetDTSs = response->packetDTSs;
		}
		break;
	}
	case DECODE_MSG_PLAY:
	{
		m_lockBtn = false;
		break;
	}
	case DECODE_MSG_PAUSE:
	{
		m_lockBtn = false;
		break;
	}
	case DECODE_MSG_TAKEN:
	{
		m_lockBtn = false;
		break;
	}
	case DECODE_MSG_FINISH_TAKEN:
	{
		m_lockBtn = false;
		emit sigShowTakenWidget(true);
		break;
	}
	case DECODE_MSG_BACK:
	{
		m_lockBtn = false;
		break;
	}
	case DECODE_MSG_FORWARD:
	{
		m_lockBtn = false;
		break;
	}
	case DECODE_MSG_BACK5S:
	{
		m_lockBtn = false;
		break;
	}
	case DECODE_MSG_FORWARD5S:
	{
		m_lockBtn = false;
		break;
	}
	default:
		break;
	}

	delete response;
}

void CPlayerBG::ClearPlayer()
{
	ui.btnPlay->setIcon(QIcon(QString::fromLatin1(":/Player/images/Player/play.svg")));
	ui.btnTaken->setText(QString("开始提取"));
	ui.PlayerWidget->setStatus(PLAYER_STATUS_PAUSE);
	ui.sliderPlayer->setValue(0);
	m_decodeThread->pause(g_proInfo.uuid);
}

void CPlayerBG::onChangeProject()
{
	ui.PlayerWidget->setInitVideo(false);
	m_decodeThread->initVideo(g_proInfo.uuid, g_proInfo.videoPath);
	
	ui.btnPlay->setIcon(QIcon(QString::fromLatin1(":/Player/images/Player/play.svg")));
	if (g_proInfo.frameInfos.size() > 0)
		ui.btnTaken->setText(QString("重新提取"));
	else {
		ui.btnTaken->setText(QString("开始提取"));
	}
	ui.PlayerWidget->setStatus(PLAYER_STATUS_PAUSE);
	// 设置模式为正常模式（可以框选）
	ui.PlayerWidget->setMode(NORMAL);
	ui.sliderPlayer->setValue(0);
}

void CPlayerBG::onFPS(int fps)
{
	g_proInfo.fps = fps;
}

bool CPlayerBG::canOperate()
{
	if (g_proInfo.uuid == "")
		return false;

	//未初始化不可操作
	if (!ui.PlayerWidget->initVideo()) {
		VTMessageBoxOk(QString("视频初始化失败！"), windowFlags(), QMessageBox::Information, this);
	}

	//首帧不存在不可操作
	if (!g_proInfo.firstImage)
		return false;

	//未定位到时分秒水印位置不可操作
	if (g_proInfo.firstImage->waterMarkHMS.rect == QRect(0, 0, 0, 0)) {
		auto text = QString("请在播放器上用鼠标左键框出时分秒水印位置");
		VTMessageBoxOk(text, windowFlags(), QMessageBox::Information, this);
		return false;
	}

	if (m_lockBtn) {
		qDebug() << QString("操作过于频发！");
		return false;
	}
	return true;
}

//播放按钮
void CPlayerBG::onPlayPause()
{
	if (!canOperate())
		return;

	if (ui.PlayerWidget->status() == PLAYER_STATUS_TAKEN)
		return;

	if (ui.PlayerWidget->status() == PLAYER_STATUS_PLAY) {
		m_lockBtn = true;
		m_decodeThread->pause(g_proInfo.uuid);
		ui.btnPlay->setIcon(QIcon(QString::fromLatin1(":/Player/images/Player/play.svg")));
		ui.PlayerWidget->setStatus(PLAYER_STATUS_PAUSE);
	}
	else {
		m_lockBtn = true;
		m_decodeThread->play(g_proInfo.uuid);
		ui.btnPlay->setIcon(QIcon(QString::fromLatin1(":/Player/images/Player/pause.svg")));
		ui.PlayerWidget->setStatus(PLAYER_STATUS_PLAY);
	}
}

//上一帧
void CPlayerBG::onBack()
{
	if (!canOperate())
		return;

	if (ui.PlayerWidget->status() != PLAYER_STATUS_PAUSE)
		return;

	m_decodeThread->back(g_proInfo.uuid);
}

//下一帧
void CPlayerBG::onForward()
{
	if (!canOperate())
		return;

	if (ui.PlayerWidget->status() != PLAYER_STATUS_PAUSE)
		return;

	m_decodeThread->forward(g_proInfo.uuid);
}

//快退5秒
void CPlayerBG::onBack5s()
{
	if (!canOperate())
		return;

	if (ui.PlayerWidget->status() != PLAYER_STATUS_PAUSE && ui.PlayerWidget->status() != PLAYER_STATUS_PLAY)
		return;

	m_decodeThread->back5s(g_proInfo.uuid);
}

//快进5秒
void CPlayerBG::onForward5s()
{
	if (!canOperate())
		return;

	if (ui.PlayerWidget->status() != PLAYER_STATUS_PAUSE && ui.PlayerWidget->status() != PLAYER_STATUS_PLAY)
		return;

	m_decodeThread->forward5s(g_proInfo.uuid);
}

//重置提取
void CPlayerBG::onReTaken()
{
	if (!canOperate())
		return;

	g_proInfo.clearCalcData();

	//从头开始播放
	m_decodeThread->seek(g_proInfo.uuid, 0);
	ui.btnTaken->setText(QString("开始提取"));

	//开始播放
	m_decodeThread->pause(g_proInfo.uuid);
	ui.btnPlay->setIcon(QIcon(QString::fromLatin1(":/Player/images/Player/play.svg")));
	ui.PlayerWidget->setStatus(PLAYER_STATUS_PAUSE);
}

//开始提取按钮
void CPlayerBG::onTaken()
{
	if (!canOperate())
		return;

	if (ui.btnTaken->text() == QString("重新提取")) {
		m_decodeMsg = m_decodeThread->decodeMsg();
		m_decodeThread->pause(g_proInfo.uuid);
		auto resBtn = VTMessageBoxYesNo(QString("是否重新提取？"), this);
		if (resBtn != QMessageBox::Yes) {
			if (m_decodeMsg == DECODE_MSG_PLAY)
				m_decodeThread->play(g_proInfo.uuid);
			return;
		}
		//清空数据
		g_proInfo.clearCalcData();
		g_logBrowser->clear();
		//从头开始播放
		m_decodeThread->seek(g_proInfo.uuid, 0);
		ui.btnTaken->setText(QString("开始提取"));

		//开始播放
		m_lockBtn = true;
		m_decodeThread->play(g_proInfo.uuid);
		ui.btnPlay->setIcon(QIcon(QString::fromLatin1(":/Player/images/Player/pause.svg")));
		ui.PlayerWidget->setStatus(PLAYER_STATUS_PLAY);
	}
	else if (ui.btnTaken->text() == QString("开始提取")) {
		//暂停
		m_lockBtn = true;
		m_decodeThread->pause(g_proInfo.uuid);
		ui.btnPlay->setIcon(QIcon(QString::fromLatin1(":/Player/images/Player/play.svg")));
		ui.PlayerWidget->setStatus(PLAYER_STATUS_PAUSE);
		if (!g_takenBG->actualPointsReady()) {
			VTMessageBoxOk(QString("请输入参照物长度（坐标）：\r\n图像分析  >特征尺寸  >参照物长度输入(参照物坐标输入)"), windowFlags(), QMessageBox::Information, this);
			return;
		}
		g_proInfo.clearCalcData();

		m_decodeThread->taken(g_proInfo.uuid, g_proInfo.timeMode);
		ui.btnTaken->setText(QString("结束提取"));
		
		ui.btnPlay->setIcon(QIcon(QString::fromLatin1(":/Player/images/Player/pause.svg")));
		ui.PlayerWidget->setStatus(PLAYER_STATUS_TAKEN);
	}
	else if (ui.btnTaken->text() == QString("结束提取")) {
		m_decodeThread->finishTaken(g_proInfo.uuid);
		ui.btnPlay->setIcon(QIcon(QString::fromLatin1(":/Player/images/Player/play.svg")));
		ui.btnTaken->setText(QString("重新提取"));
		ui.PlayerWidget->setStatus(PLAYER_STATUS_PAUSE);
	}
}

void CPlayerBG::onEOF(int cmd)
{
	if (cmd == DECODE_MSG_PLAY)
		onPlayPause();
	else if (cmd == DECODE_MSG_TAKEN)
		onTaken();
}

void CPlayerBG::onSliderPress()
{
	if (!canOperate())
		return;

	m_pressSlider = true;
	m_decodeMsg = m_decodeThread->decodeMsg();
	m_decodeThread->pause(g_proInfo.uuid);
}

void CPlayerBG::onSliderRelease()
{
	if (!canOperate())
		return;

	float pos = 0;
	pos = (float)ui.sliderPlayer->value() / (float)(ui.sliderPlayer->maximum() + 1);

	if (m_decodeThread->decodeMsg() == DECODE_MSG_TAKEN)
		return;
	m_decodeThread->seek(g_proInfo.uuid, pos);
	if (m_decodeMsg == DECODE_MSG_PLAY)
		m_decodeThread->play(g_proInfo.uuid);
	m_pressSlider = false;
}

void CPlayerBG::onPlayerTime(qint64 pts, qint64 totalMS)
{
	float rate = 0;
	if(totalMS > 0) 
		rate = (float)pts / (float)totalMS;
	int value = rate * 1000;
	if (!m_pressSlider) {
		if (value != ui.sliderPlayer->value()) {
			ui.sliderPlayer->setValue(value);
		}
	}

	//显示播放时间
	int num = (pts / 1000);
	int hour = num / 3600;
	num = num - hour*(3600);
	int min = num / 60;
	num = num - min*(60);
	int sec = num;
	char buf[1024] = { 0 };
	sprintf(buf, "%02d:%02d:%02d", hour, min, sec);
	ui.labelCurTime->setText(buf);

	memset(buf, 0, sizeof(buf) / sizeof(char));
	num = (totalMS / 1000);
	hour = num / 3600;
	num = num - hour*(3600);
	min = num / 60;
	num = num - min*(60);
	sec = num;
	sprintf(buf, "%02d:%02d:%02d", hour, min, sec);
	ui.labelDuration->setText(buf);
}

void CPlayerBG::onPlayerImage(std::shared_ptr<VTDecodeImage>decodeImage)
{
	ui.PlayerWidget->onPlayerImage(decodeImage);
}

void CPlayerBG::updateOCRRect()
{
	emit ui.PlayerWidget->sigUpdateFirstImage();
}