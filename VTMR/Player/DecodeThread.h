#pragma once

#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QVector>
#include "Utils.h"
extern "C"
{
#include "libavutil/avutil.h"
}

class CDecodeVideo;
enum DecodeMessage
{
	DECODE_MSG_INIT,//初始化
	DECODE_MSG_PLAY,//播放
	DECODE_MSG_PAUSE,//暂停
	DECODE_MSG_TAKEN,//提取
	DECODE_MSG_FINISH_TAKEN,//停止提取
	DECODE_MSG_SEEK,//拖拽播放
	DECODE_MSG_SPEED,//倍速
	DECODE_MSG_BACK,//上一帧
	DECODE_MSG_FORWARD,//下一帧
	DECODE_MSG_BACK5S,//快退5秒
	DECODE_MSG_FORWARD5S,//快进5秒
};

struct DecodeHeader
{
	DecodeMessage msg;
	QString uuid;
	QString videoPath;
	VTTimeMode timeMode;
	float pos;
	int nSpeed = 1;
};

struct DecodeResponse
{
	QString uuid = "";
	DecodeMessage msg = DECODE_MSG_PAUSE;
	int code = 0;
	QString reason = "";
	int fps = 0;
	int frameInterval = 0;
	int64_t countFrames = 0;
	int64_t duration = 0;
	int64_t startTime = 0;
	QVector<int64_t> packetDTSs;
};

class CPlayerWidget;

class CDecodeThread : public QThread
{
	Q_OBJECT

public:
	CDecodeThread(CPlayerWidget *playerWidget, QObject *parent);
	~CDecodeThread();
	DecodeMessage decodeMsg() { return m_message; }
	void exit() { m_exit = true; }
	void initVideo(QString uuid, QString videoPath);
	void play(QString uuid);
	void pause(QString uuid);
	void taken(QString uuid, VTTimeMode mode);
	void seek(QString uuid, float pos);
	void playSpeed(int speed);
	void back(QString uuid);
	void forward(QString uuid);
	void back5s(QString uuid);
	void forward5s(QString uuid);
	void finishTaken(QString uuid);

private:
	int initDecode(QString uuid, QString videoPath);
	void playFirstFrame();
	void reset();
	void doMsg();
	int doDecode();
	void doPlay();
	void doTaken(std::shared_ptr<VTDecodeImage>decodeImage);
	int doBack();
	int doForward();
	void doFirstTaken();

protected:
	void run();
signals:
	void sigFPS(int fps);
	void sigPlayerImage(std::shared_ptr<VTDecodeImage>decodeImage);
	void sigPlayerTime(qint64 pts, qint64 duration);
	void sigDecodeStatus(DecodeResponse *response);
	void sigFirstImage(std::shared_ptr<VTDecodeImage> decodeImage);
	void sigEOF(int cmd);
private:
	CDecodeVideo *m_decodeVideo = nullptr;
	bool m_exit = false;
	QQueue<DecodeHeader *> m_decodeMsgs;
	QMutex m_mutx;
	int64_t m_lastMsgTime = 0;
	QVector<int64_t> m_keyPTSs;
	QVector<int64_t> m_packetDTSs;
	DecodeMessage m_message = DECODE_MSG_PAUSE;
	//回调
	CPlayerWidget *m_playerWidget;
	//播放放控制
	QString m_uuid;
	bool m_initVideo = false;
	bool m_eof = false;
	QVector<std::shared_ptr<VTDecodeImage>> m_decodeImages;
	int m_curPlayIndex = -1;
	int64_t m_duration = 0;
	int64_t m_startTime = 0;
	int64_t m_frameInterval = 0;
	int64_t m_countFrames = 0;
	int m_fps = 0;
	int64_t m_lastPlayTime = 0;
	int64_t m_lastPlayPTS = AV_NOPTS_VALUE;
	int64_t m_fistPlayPTS = AV_NOPTS_VALUE;
	int64_t m_tailFramePTS = AV_NOPTS_VALUE;
	int m_speed = 1;
	//截取图片
	QString m_imageDir;
	bool m_playOnce = false;
	
#define DECODE_BUFF_TOTAL_SIZE 150 //总大小
#define DECODE_BUFF_TAIL_SIZE 50  //预加载大小
};
