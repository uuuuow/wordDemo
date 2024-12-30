#pragma once

#include <QWidget>
#include "ui_PlayerBG.h"
#include "Utils.h"
#include <QTimerEvent>
#include "DecodeThread.h"

class CVTMR;
class CPlayerThread;
class CPlayerWidget;
class CDecodeThread;

/*
*播放器界面
*/
class CPlayerBG : public QWidget
{
	Q_OBJECT

public:
	CPlayerBG(QWidget *parent = Q_NULLPTR);
	~CPlayerBG();
	QVector<QWidget *> getBlackWidgets()
	{
		QVector<QWidget *> widgets;
		widgets.push_back(ui.cmboxSpeed);
		widgets.push_back(ui.btnBack);
		widgets.push_back(ui.btnForward);
		widgets.push_back(ui.btnBack5s);
		widgets.push_back(ui.btnForward5s);
		widgets.push_back(ui.btnPlay);
		widgets.push_back(ui.btnTaken);
		return  widgets;
	}

	QWidget *getPlayer() { return ui.PlayerWidget; }
	//切换工程
	void onChangeProject();
	CDecodeThread *decodeThread() { return m_decodeThread; }
	//清空
	void ClearPlayer();
	//更新OCR
	void updateOCRRect();
public:
	virtual bool eventFilter(QObject *obj, QEvent *ev);
private:
	bool canOperate();
	//按压进度条
	void onSliderPress();
	//松开进度条
	void onSliderRelease();

public slots:
	//状态
	void onDecodeStatus(DecodeResponse *response);
	//播放暂停按钮
	void onPlayPause();
	//帧率
	void onFPS(int fps);
	//上一帧
	void onBack();
	//下一帧
	void onForward();
	//快退5秒
	void onBack5s();
	//快进5秒
	void onForward5s();
	//提取按钮
	void onTaken();
	//显示时间
	void onPlayerTime(qint64 pts, qint64 totalMS);
	//解码后的图片
	void onPlayerImage(std::shared_ptr<VTDecodeImage> decodeImage);
	//重置提取
	void onReTaken();
	//播放完毕
	void onEOF(int cmd);
signals:
	//切换为编辑窗口
	void sigShowTakenWidget(bool doOCR);
private:
	Ui::PlayerBG ui;
	//解码线程
	CDecodeThread *m_decodeThread = nullptr;
	int m_fps;
	//是否按压进度条
	bool m_pressSlider = false;
	CVTMR *m_vtmr;

	bool m_lockBtn = false;
	//当前解码状态
	DecodeMessage m_decodeMsg = DECODE_MSG_PAUSE;
};
