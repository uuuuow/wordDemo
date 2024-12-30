#pragma once
extern "C"
{
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/avutil.h"
#include "libavutil/time.h"
#include "libavutil/imgutils.h"
}

#include <QImage>

/*
*视频解码
*/
class CDecodeVideo
{
public:
	CDecodeVideo();
	CDecodeVideo(QString uuid);
	~CDecodeVideo();

	static void initFFmpeg();

	int openInput(QString videoPath);
	int initDecodeContext(AVStream *inputStream);
	int decode(AVPacket* packet, AVFrame *frame, int *got_frame);
	bool seek(float pos);
	bool seek(int64_t dts, int streamIndex);
	int getVideoStreamIndex();
	int readPacket(AVPacket &pkt);
	int getFPS() { return m_fps; }
	QString getUID() { return m_uuid; }
	int64_t getDuration() { return m_duration; }
	int64_t getStartTime() { return m_startTime; }

	int AVFrame2QImage(AVFrame * pFrame, QImage **imageOut);
public:
	AVRational m_timebase;

private:
	AVFormatContext *m_inputContext = nullptr;
	int m_videoStream = -1;
	int m_fps = 25;
	QString m_uuid;
	struct SwsContext *m_swsContext = nullptr;
	int64_t m_duration = 0;
	int64_t m_startTime = 0;
};

