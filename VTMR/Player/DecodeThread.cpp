#include "DecodeThread.h"
#include "DecodeVideo.h"
#include "Utils.h"
#include "Global.h"
#include <QDir>
#include <QDebug>
#include "OCRThread.h"

#define AV_TIME_BASE_Q				av_make_q(1, AV_TIME_BASE)

CDecodeThread::CDecodeThread(CPlayerWidget *playerWidget, QObject *parent)
	: QThread(parent), m_playerWidget(playerWidget)
{
	connect(this, SIGNAL(sigDecodeStatus(DecodeResponse *)), parent, SLOT(onDecodeStatus(DecodeResponse *)));
	connect(this, SIGNAL(sigEOF(int)), parent, SLOT(onEOF(int)));
	connect(this, SIGNAL(sigFirstImage(std::shared_ptr<VTDecodeImage>)), (QObject *)g_vtmr, SLOT(onFirstImage(std::shared_ptr<VTDecodeImage>)));
}

CDecodeThread::~CDecodeThread()
{
}

void CDecodeThread::initVideo(QString uuid, QString videoPath)
{
	auto header = new DecodeHeader;
	header->uuid = uuid;
	header->msg = DECODE_MSG_INIT;
	header->videoPath = videoPath;
	m_mutx.lock();
	m_decodeMsgs.push_back(header);
	m_mutx.unlock();
}

void CDecodeThread::play(QString uuid)
{
	auto header = new DecodeHeader;
	header->uuid = uuid;
	header->msg = DECODE_MSG_PLAY;
	m_mutx.lock();
	m_decodeMsgs.push_back(header);
	m_mutx.unlock();
}

void CDecodeThread::pause(QString uuid)
{
	auto header = new DecodeHeader;
	header->uuid = uuid;
	header->msg = DECODE_MSG_PAUSE;
	m_mutx.lock();
	m_decodeMsgs.push_back(header);
	m_mutx.unlock();
}

void CDecodeThread::taken(QString uuid, VTTimeMode mode)
{
	auto header = new DecodeHeader;
	header->uuid = uuid;
	header->msg = DECODE_MSG_TAKEN;
	header->timeMode = mode;
	m_mutx.lock();
	m_decodeMsgs.push_back(header);
	m_mutx.unlock();
}

void CDecodeThread::finishTaken(QString uuid)
{
	auto header = new DecodeHeader;
	header->uuid = uuid;
	header->msg = DECODE_MSG_FINISH_TAKEN;
	m_mutx.lock();
	m_decodeMsgs.push_back(header);
	m_mutx.unlock();
}

void CDecodeThread::seek(QString uuid, float pos)
{
	auto header = new DecodeHeader;
	header->uuid = uuid;
	header->msg = DECODE_MSG_SEEK;
	header->pos = pos;
	m_mutx.lock();
	m_decodeMsgs.push_back(header);
	m_mutx.unlock();
}

void CDecodeThread::playSpeed(int speed)
{
	auto header = new DecodeHeader;
	header->nSpeed = speed;
	header->msg = DECODE_MSG_SPEED;

	m_mutx.lock();
	m_decodeMsgs.push_back(header);
	m_mutx.unlock();
}

void CDecodeThread::back(QString uuid)
{
	auto header = new DecodeHeader;
	header->uuid = uuid;
	header->msg = DECODE_MSG_BACK;

	m_mutx.lock();
	m_decodeMsgs.push_back(header);
	m_mutx.unlock();
}

void CDecodeThread::forward(QString uuid)
{
	auto header = new DecodeHeader;
	header->uuid = uuid;
	header->msg = DECODE_MSG_FORWARD;

	m_mutx.lock();
	m_decodeMsgs.push_back(header);
	m_mutx.unlock();
}

void CDecodeThread::back5s(QString uuid)
{
	auto header = new DecodeHeader;
	header->uuid = uuid;
	header->msg = DECODE_MSG_BACK5S;

	m_mutx.lock();
	m_decodeMsgs.push_back(header);
	m_mutx.unlock();
}

void CDecodeThread::forward5s(QString uuid)
{
	auto header = new DecodeHeader;
	header->uuid = uuid;
	header->msg = DECODE_MSG_FORWARD5S;

	m_mutx.lock();
	m_decodeMsgs.push_back(header);
	m_mutx.unlock();
}

void CDecodeThread::doMsg()
{
	int ret = 0;
	DecodeHeader * header = nullptr;
	int64_t currentTime = av_gettime();
	if (m_decodeMsgs.size() == 0)
		return;

	m_mutx.lock();
	header = m_decodeMsgs.front();
	m_decodeMsgs.pop_front();
	m_mutx.unlock();

	m_lastMsgTime = currentTime;

	if (!header)
		return;

	DecodeResponse *response = new DecodeResponse();
	response->uuid = header->uuid;
	response->msg = header->msg;
	switch (header->msg)
	{
	case DECODE_MSG_INIT:
	{
		m_message = DECODE_MSG_INIT;
		m_uuid = "";
		reset();
		response->code = initDecode(header->uuid, header->videoPath);
		response->fps = m_fps;
		response->frameInterval = m_frameInterval;
		response->countFrames = m_countFrames;
		response->duration = m_duration;
		response->startTime = m_startTime;
		response->packetDTSs = m_packetDTSs;
		emit sigDecodeStatus(response);
		break;
	}
	case DECODE_MSG_PLAY:
	{
		m_message = DECODE_MSG_PLAY;
		if (m_decodeVideo)
		{
			int64_t pts = -1;
			if (m_decodeImages.size() > 0)
			{
				if (m_curPlayIndex != -1)
				{
					pts = m_decodeImages[m_curPlayIndex]->pts;
				}
			}
			if (m_eof && pts == m_tailFramePTS)
			{
				m_decodeVideo->seek(0, -1);
				m_curPlayIndex = -1;
				while (!m_decodeImages.empty())
				{
					auto &tmp = m_decodeImages.front();
					m_decodeImages.pop_front();
				}
			}
		}
		emit sigDecodeStatus(response);
		break;
	}
	case DECODE_MSG_PAUSE:
	{
		m_message = DECODE_MSG_PAUSE;
		emit sigDecodeStatus(response);
		break;
	}
	case DECODE_MSG_SPEED:
	{
		m_speed = header->nSpeed;
		break;
	}
	case DECODE_MSG_SEEK:
	{
		if (m_decodeVideo)
		{
			m_decodeVideo->seek(header->pos);
			m_curPlayIndex = -1;
			while (!m_decodeImages.empty())
			{
				auto &tmp = m_decodeImages.front();
				m_decodeImages.pop_front();
			}
			m_playOnce = true;
		}

		break;
	}
	case DECODE_MSG_TAKEN:
	{
		m_message = DECODE_MSG_TAKEN;
		emit sigDecodeStatus(response);
		doFirstTaken();
		break;
	}
	case DECODE_MSG_FINISH_TAKEN:
	{
		m_message = DECODE_MSG_PAUSE;
		emit sigDecodeStatus(response);

		break;
	}
	case DECODE_MSG_BACK:
	{
		m_message = DECODE_MSG_BACK;
		ret = doBack();
		emit sigDecodeStatus(response);
		break;
	}
	case DECODE_MSG_FORWARD:
	{
		m_message = DECODE_MSG_FORWARD;
		ret = doForward();
		emit sigDecodeStatus(response);
		break;
	}
	case DECODE_MSG_BACK5S:
	{
		if (m_decodeVideo)
		{
			do {
				int64_t pts = 0;
				if (m_decodeImages.size() > 0) {
					if (m_curPlayIndex != -1) {
						pts = m_decodeImages[m_curPlayIndex]->pts;
					}
				}
				if (pts <= 0)
					break;
				pts -= 5 * 1000000;
				if (pts < 0)
					pts = 0;
				m_decodeVideo->seek(pts, -1);
				m_curPlayIndex = -1;
				while (!m_decodeImages.empty())
				{
					auto &tmp = m_decodeImages.front();
					m_decodeImages.pop_front();
				}
			} while (0);
			m_playOnce = true;
		}
		emit sigDecodeStatus(response);
		break;
	}
	case DECODE_MSG_FORWARD5S:
	{
		if (m_decodeVideo) {
			do {
				int64_t pts = 0;
				if (m_decodeImages.size() > 0) {
					if (m_curPlayIndex != -1) {
						pts = m_decodeImages[m_curPlayIndex]->pts;
					}
				}

				pts += 5 * 1000000;
				if (pts > m_decodeVideo->getDuration())
					break;
				m_decodeVideo->seek(pts, -1);
				m_curPlayIndex = -1;
				while (!m_decodeImages.empty()) {
					auto &tmp = m_decodeImages.front();
					m_decodeImages.pop_front();
				}
			} while (0);
			m_playOnce = true;
		}
		emit sigDecodeStatus(response);
		break;
	}
	default:
	{
		qDebug() << QString("未知命令 %1").arg(header->msg);
		break;
	}
	}
	delete header;
}

void CDecodeThread::reset()
{
	m_curPlayIndex = -1;
	m_lastPlayTime = 0;
	m_eof = false;

	for (auto &image : m_decodeImages)
		image.reset();
	m_decodeImages.clear();

	m_keyPTSs.clear();
}

int CDecodeThread::initDecode(QString uuid, QString videoPath)
{
	int ret = 0;

	m_countFrames = 0;
	m_duration = 0;
	m_initVideo = false;
	m_eof = false;
	m_fps = 0;
	m_fistPlayPTS = AV_NOPTS_VALUE;
	m_lastPlayPTS = AV_NOPTS_VALUE;
	m_tailFramePTS = AV_NOPTS_VALUE;
	m_keyPTSs.clear();
	m_packetDTSs.clear();

	m_imageDir = QString("%1/%2").arg(g_proDataPath).arg(uuid);
	QDir dir2;
	if (!dir2.exists(m_imageDir))
		dir2.mkpath(m_imageDir);

	if (m_decodeVideo)
	{
		delete m_decodeVideo;
		m_decodeVideo = nullptr;
	}

	auto decodeVideo = new CDecodeVideo(uuid);


	ret = decodeVideo->openInput(videoPath);
	if (ret != 0)
		goto End;
	m_fps = decodeVideo->getFPS();
	m_frameInterval = 1000000LL / (m_fps > 0 ? m_fps : 25);
	m_duration = decodeVideo->getDuration();
	m_startTime = decodeVideo->getStartTime() > 0 ? decodeVideo->getStartTime() : 0;

	AVPacket pkt;
	while (1)
	{
		av_init_packet(&pkt);
		ret = decodeVideo->readPacket(pkt);
		if (ret < 0)
		{
			break;
		}
		else if (pkt.stream_index == decodeVideo->getVideoStreamIndex())
		{
			m_countFrames++;
			int64_t dts = av_rescale_q(pkt.dts, decodeVideo->m_timebase, AV_TIME_BASE_Q);
			if (pkt.flags & AV_PKT_FLAG_KEY)
			{
				m_keyPTSs.push_back(pkt.dts);
			}
			m_packetDTSs.push_back(dts);
		}
		av_packet_unref(&pkt);
		av_usleep(1);
	}
	decodeVideo->seek(0);

	ret = 0;
	m_decodeVideo = decodeVideo;
	m_initVideo = true;
End:
	if (ret != 0)
	{
		if (decodeVideo)
			delete decodeVideo;
	}
	return ret;
}

int CDecodeThread::doDecode()
{
	int ret = 0;
	int got_frame = 0;
	int64_t dts;
	int countFrames;
	int n;
	AVFrame *videoFrame = nullptr;
	int64_t pts;
	AVPacket pkt;
	av_init_packet(&pkt);

	if (!m_decodeVideo || !m_initVideo)
		goto End;

	if (m_decodeImages.size() >= DECODE_BUFF_TOTAL_SIZE)
		goto End;
	ret = m_decodeVideo->readPacket(pkt);
	if (ret < 0) {
		if (ret == AVERROR_EOF)
			m_eof = true;
		goto End;
	}
	else if (pkt.stream_index != m_decodeVideo->getVideoStreamIndex())
		goto End;

	dts = av_rescale_q(pkt.dts, m_decodeVideo->m_timebase, AV_TIME_BASE_Q);
	if (m_tailFramePTS == AV_NOPTS_VALUE)
		m_tailFramePTS = dts;
	else if (m_tailFramePTS < dts)
		m_tailFramePTS = dts;
	videoFrame = av_frame_alloc();
	m_decodeVideo->decode(&pkt, videoFrame, &got_frame);
	if(got_frame) {
		if (videoFrame->pts == AV_NOPTS_VALUE)
			videoFrame->pts = pkt.dts;

		pts = av_rescale_q(videoFrame->pts, m_decodeVideo->m_timebase, AV_TIME_BASE_Q);

		std::shared_ptr<VTDecodeImage> decodeImage(new VTDecodeImage);
		decodeImage->uuid = m_decodeVideo->getUID();
		decodeImage->pts = pts;

		countFrames = m_packetDTSs.size();
		for (n = 0; n < countFrames; n++) {
			if (pts == m_packetDTSs[n]) {
				decodeImage->index = n + 1;
				break;
			}
		}
		
		m_decodeVideo->AVFrame2QImage(videoFrame, &decodeImage->image);
		m_decodeImages.push_back(decodeImage);
	}

End:
	av_packet_unref(&pkt);
	av_frame_free(&videoFrame);
	return ret;
}

void CDecodeThread::playFirstFrame()
{
	std::shared_ptr<VTDecodeImage> decodeImage;
	if (!m_decodeVideo || m_fistPlayPTS != AV_NOPTS_VALUE)
		return;
	if (m_decodeImages.size() == 0 || m_message != DECODE_MSG_INIT)
		return;

	m_curPlayIndex = 0;
	
	decodeImage = std::make_shared<VTDecodeImage>();
	*decodeImage = *(m_decodeImages[m_curPlayIndex]);
	int fps = m_decodeVideo->getFPS();
	emit sigPlayerImage(decodeImage);
	int64_t pts = (m_decodeImages[m_curPlayIndex]->pts - m_startTime);
	emit sigPlayerTime(pts / 1000, m_decodeVideo->getDuration() / 1000);
	emit sigFPS(fps);
	m_fistPlayPTS = m_decodeImages[m_curPlayIndex]->pts;
	m_lastPlayPTS = m_fistPlayPTS;
	m_lastPlayTime = av_gettime();

	decodeImage = std::make_shared<VTDecodeImage>();
	*decodeImage = *(m_decodeImages[m_curPlayIndex]);
	emit sigFirstImage(decodeImage);
}

void CDecodeThread::doPlay()
{
	int64_t currentTime;
	if (!m_decodeVideo)
		return;
	if (DECODE_MSG_PLAY != m_message && DECODE_MSG_TAKEN != m_message && !m_playOnce)
		return;
	if (m_curPlayIndex + 1 >= m_decodeImages.size())
		return;

	if (m_curPlayIndex + DECODE_BUFF_TAIL_SIZE >= DECODE_BUFF_TOTAL_SIZE)
	{
		auto &tmp = m_decodeImages.front();
		m_decodeImages.pop_front();
		m_curPlayIndex--;
	}

	currentTime = av_gettime();
	if ((currentTime - m_lastPlayTime < m_frameInterval / m_speed) && DECODE_MSG_TAKEN != m_message)
		return;
	m_curPlayIndex++;
	
	std::shared_ptr<VTDecodeImage> decodeImage(new VTDecodeImage);
	*decodeImage = *(m_decodeImages[m_curPlayIndex]);
	emit sigPlayerImage(decodeImage);

	m_lastPlayPTS = m_decodeImages[m_curPlayIndex]->pts;

	int64_t pts = (m_decodeImages[m_curPlayIndex]->pts - m_startTime);
	emit sigPlayerTime(pts / 1000, m_decodeVideo->getDuration() / 1000);
	m_lastPlayTime = currentTime;

	doTaken(m_decodeImages[m_curPlayIndex]);

	if (m_eof && m_decodeImages[m_curPlayIndex]->pts == m_tailFramePTS)
		emit sigEOF(m_message);
	m_playOnce = false;
}

void CDecodeThread::doFirstTaken()
{
	if (!m_decodeVideo)
		return;
	if (m_curPlayIndex >= m_decodeImages.size() || m_curPlayIndex < 0)
		return;

	int64_t lastPTS = m_decodeImages[m_curPlayIndex]->pts;
	std::shared_ptr<VTDecodeImage>decodeImage = nullptr;
	int size = m_decodeImages.size();
	for (int n = 0; n <= m_curPlayIndex; ++n)
	{
		if (m_decodeImages[n]->pts + 1100000 < lastPTS)
			continue;

		decodeImage = m_decodeImages[n];

		auto imagePath = QString("%1/%2.jpg").arg(m_imageDir).arg(decodeImage->index);
		int x = decodeImage->image->width();
		int y = decodeImage->image->height();
		decodeImage->image->save(imagePath, nullptr, 100);

		//应该做成回调给主线程
		VTFrameInfo frameinfo;
		frameinfo.imagePath = imagePath;
		frameinfo.pts = decodeImage->pts;
		frameinfo.index = decodeImage->index;
		g_proInfo.frameInfos.push_back(frameinfo);
	}
}

void CDecodeThread::doTaken(std::shared_ptr<VTDecodeImage>decodeImage)
{
	if (DECODE_MSG_TAKEN != m_message)
		return;
	auto imagePath = QString("%1/%2.jpg").arg(m_imageDir).arg(decodeImage->index);
	decodeImage->image->save(imagePath, nullptr, 100);

	//应该做成回调给主线程
	VTFrameInfo frameinfo;
	frameinfo.imagePath = imagePath;
	frameinfo.pts = decodeImage->pts;
	frameinfo.index = decodeImage->index;
	g_proInfo.frameInfos.push_back(frameinfo);
}

int CDecodeThread::doBack()
{
	int ret = 0;
	int64_t currentTime;
	bool findFrame = false;
	int64_t startTime, pts;
	int index;
	int n = 0;
	std::shared_ptr<VTDecodeImage> decodeImage;
	if (!m_decodeVideo)
		return -1;

	if (m_lastPlayPTS <= m_fistPlayPTS)
	{
		ret = -1;
		goto End;
	}

	for (; m_curPlayIndex >= 0; m_curPlayIndex--)
	{
		if (m_decodeImages[m_curPlayIndex]->pts < m_lastPlayPTS)
		{
			findFrame = true;
			break;
		}
	}

	if (!findFrame)
	{
		int64_t nKeyPTS = AV_NOPTS_VALUE;
		for (n = m_keyPTSs.size() - 1; n >= 0; --n)
		{
			pts = av_rescale_q(m_keyPTSs[n], m_decodeVideo->m_timebase, AV_TIME_BASE_Q);
			if (pts < m_lastPlayPTS)
			{
				nKeyPTS = m_keyPTSs[n];
				break;
			}
		}
		if (n > 0)
		{
			nKeyPTS = m_keyPTSs[n - 1];
		}
		if (nKeyPTS == AV_NOPTS_VALUE)
		{
			ret = -1;
			goto End;
		}
		m_decodeVideo->seek(nKeyPTS, m_decodeVideo->getVideoStreamIndex());
		for (auto &image : m_decodeImages)
			image.reset();
		m_decodeImages.clear();

		while (true)
		{
			for (index = 0; index < m_decodeImages.size(); ++index)
			{
				if(m_lastPlayPTS == m_decodeImages[index]->pts)
				{
					if (index > 0)
					{
						m_curPlayIndex = index - 1;
						findFrame = true;
					}
					break;
				}
			}
			if (findFrame)
				break;
			if(AVERROR_EOF == doDecode())
				break;
		}
	}

	if (!findFrame)
	{
		ret = -1;
		goto End;
	}
	currentTime = av_gettime();
	m_lastPlayPTS = m_decodeImages[m_curPlayIndex]->pts;

	decodeImage = std::make_shared<VTDecodeImage>();
	*decodeImage = *(m_decodeImages[m_curPlayIndex]);
	emit sigPlayerImage(decodeImage);

	pts = (m_decodeImages[m_curPlayIndex]->pts - m_startTime);
	emit sigPlayerTime(pts / 1000, m_decodeVideo->getDuration() / 1000);
	m_lastPlayTime = currentTime;
End:
	m_message = DECODE_MSG_PAUSE;
	return ret;
}

int CDecodeThread::doForward()
{
	int ret = 0;
	int64_t currentTime;
	std::shared_ptr<VTDecodeImage> decodeImage;
	if (!m_decodeVideo)
		return -1;
	bool findFrame = false;
	int64_t startTime, pts;

	while (true)
	{
		if (m_eof == AVERROR_EOF && m_curPlayIndex < m_decodeImages.size() && m_curPlayIndex >= 0)
		{
			if (m_tailFramePTS == m_decodeImages[m_curPlayIndex]->pts)
			{
				goto End;
			}
		}

		doDecode();

		if (m_curPlayIndex + DECODE_BUFF_TAIL_SIZE >= DECODE_BUFF_TOTAL_SIZE)
		{
			auto &tmp = m_decodeImages.front();
			m_decodeImages.pop_front();
			m_curPlayIndex--;
		}

		if (m_curPlayIndex + 1 >= m_decodeImages.size())
			continue;
		findFrame = true;
		break;
	}

	if (!findFrame)
		goto End;
	currentTime = av_gettime();

	m_curPlayIndex++;

	decodeImage = std::make_shared<VTDecodeImage>();
	*decodeImage = *(m_decodeImages[m_curPlayIndex]);
	emit sigPlayerImage(decodeImage);

	pts = (m_decodeImages[m_curPlayIndex]->pts - m_startTime);
	emit sigPlayerTime(pts / 1000, m_decodeVideo->getDuration() / 1000);
	m_lastPlayTime = currentTime;
	m_lastPlayPTS = m_decodeImages[m_curPlayIndex]->pts;
End:
	m_message = DECODE_MSG_PAUSE;

	return ret;
}

void CDecodeThread::run()
{
	CDecodeVideo::initFFmpeg();
	while (!m_exit)
	{
		doMsg();
		doDecode();
		playFirstFrame();
		doPlay();
		if(!m_exit)
			av_usleep(1000);
	}
}