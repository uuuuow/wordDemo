#include "DecodeVideo.h"
#include <QDebug>
#include "Utils.h"

CDecodeVideo::CDecodeVideo()
{
}

CDecodeVideo::CDecodeVideo(QString uuid)
{
	m_uuid = uuid;
}


CDecodeVideo::~CDecodeVideo()
{
	if (m_inputContext != nullptr) {
		avformat_close_input(&m_inputContext);
		m_inputContext = nullptr;
	}
}

void CDecodeVideo::initFFmpeg()
{
	av_register_all();
	av_log_set_level(AV_LOG_WARNING);
}

int CDecodeVideo::openInput(QString videoPath)
{
	string inputUrl = VTQtStr2StdStr(videoPath);

	int ret = avformat_open_input(&m_inputContext, inputUrl.c_str(), nullptr, nullptr);
	if (ret < 0) {
		qDebug() << QString("avformat_open_input failed");
		return  ret;
	}
	ret = avformat_find_stream_info(m_inputContext, nullptr);
	if (ret < 0)
		qDebug() << QString("avformat_find_stream_info failed");

	for (int i = 0; i < m_inputContext->nb_streams; i++) {
		AVCodecContext *enc = m_inputContext->streams[i]->codec;
		if (enc->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			AVRational rational{ 1, AV_TIME_BASE };
			m_videoStream = i;
			m_fps = av_q2d(m_inputContext->streams[i]->avg_frame_rate);
			
			m_duration = av_rescale_q(m_inputContext->streams[i]->duration, m_inputContext->streams[i]->time_base, rational);
			
			m_timebase = m_inputContext->streams[i]->time_base;
			enc->pkt_timebase = m_inputContext->streams[i]->time_base;
			m_startTime = av_rescale_q(m_inputContext->streams[i]->start_time, m_inputContext->streams[i]->time_base, rational);
		}
	}
	if (m_videoStream < 0) {
		qDebug() << QString("do not find vedio,%1").arg(videoPath);
		return -1;
	}
	if (m_duration <= 0) {
		m_duration = m_inputContext->duration;
	}
	initDecodeContext(m_inputContext->streams[m_videoStream]);
	return ret;
}

int CDecodeVideo::readPacket(AVPacket &pkt)
{
	return av_read_frame(m_inputContext, &pkt);
}

int CDecodeVideo::initDecodeContext(AVStream *inputStream)
{
	AVCodecID codecId = inputStream->codec->codec_id;
	AVCodec *codec = avcodec_find_decoder(codecId);
	if (!codec)
		return -1;
	AVDictionary *pOption = nullptr;
	av_dict_set_int(&pOption, "threads", 8, 0);
	return avcodec_open2(inputStream->codec, codec, nullptr);
}

int CDecodeVideo::decode(AVPacket* packet, AVFrame *frame, int *got_frame)
{
	int ret;
	
	AVStream* inputStream = m_inputContext->streams[m_videoStream];
	AVCodecContext *avctx = inputStream->codec;

	if (packet) {
		ret = avcodec_send_packet(avctx, packet);
		if (ret < 0 && ret != AVERROR_EOF)
			return ret;
	}

	ret = avcodec_receive_frame(avctx, frame);
	if (ret < 0 && ret != AVERROR(EAGAIN))
		return ret;
	if (ret >= 0)
		*got_frame = 1;
	return 0;
}

int CDecodeVideo::getVideoStreamIndex()
{
	return m_videoStream;
}

bool CDecodeVideo::seek(float pos)
{
	if (!m_inputContext)
		return false;
	int64_t stamp = 0;
	stamp = pos * m_duration + m_startTime;
	int re = av_seek_frame(m_inputContext, -1, stamp,
		AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
	if (re >= 0)
		return true;
	return false;
}

bool CDecodeVideo::seek(int64_t dts, int streamIndex)
{
	if (!m_inputContext)
		return false;
	int re = av_seek_frame(m_inputContext, streamIndex, dts,
		AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
	if (re >= 0)
		return true;
	return false;
}

int CDecodeVideo::AVFrame2QImage(AVFrame * pFrame, QImage **imageOut)
{
	int ret = 0;
	QImage *img = new QImage(pFrame->width, pFrame->height, QImage::Format_RGB888);
	if (!m_swsContext) {
		m_swsContext = sws_getContext(pFrame->width, pFrame->height, (AVPixelFormat)pFrame->format,
			pFrame->width, pFrame->height, AV_PIX_FMT_RGB24, SWS_BICUBIC, nullptr, nullptr, nullptr);

		if (!m_swsContext) {
			qDebug() << "sws_getContext failed.";
			ret = AVERROR(EINVAL);
			return ret;
		}
	}

	uint8_t* dst[] = { img->bits() };
	int dstStride[4] = {0};
	av_image_fill_linesizes(dstStride, AV_PIX_FMT_RGB24, pFrame->width);
	sws_scale(m_swsContext, pFrame->data, (const int*)pFrame->linesize,
		0, pFrame->height, dst, dstStride);

	*imageOut = img;
	return  0;
}