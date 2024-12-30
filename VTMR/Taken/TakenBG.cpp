#include "TakenBG.h"
#include <QDebug>
#include "Global.h"
#include "TakenWidget.h"
#include "Utils.h"
#include "Global.h"
#include "InputWidget.h"
#include "ShowTrace.h"
#include "vtmr.h"
#include "ImageEnhance.h"
#include "Zoom.h"
#include "TipsDialog.h"
#include <QDir>
#include "PointsManage.h"
#include <Command.h>

CTakenBG::CTakenBG(CVTMR *vtmr, CImageEnhance *imageEnhance, CShowTrace * trace, QWidget *parent)
	: QWidget(parent)
	, m_traceWidget(trace)
	, m_imageEnhance(imageEnhance)
{
	ui.setupUi(this);
	setMouseTracking(true);
	setAutoFillBackground(true);

	m_takenArea = new CTakenWidget(vtmr, this);
	ui.scrollArea->setWidget(m_takenArea);
	ui.scrollArea->setMouseTracking(true);
	ui.scrollArea->setAutoFillBackground(true);

	m_takenControl = ui.takenControlWidget;

	connect(this, SIGNAL(sigOCRResult(std::shared_ptr<OCRMsg>)), this, SLOT(onOCRResult(std::shared_ptr<OCRMsg>)));
	connect(m_takenArea, &CTakenWidget::sigNextFrame, this, &CTakenBG::onNextFrame);
	connect(m_takenArea, &CTakenWidget::sigPreviousFrame, this, &CTakenBG::onPreviousFrame);

	m_teWaterMark = new QTextEdit(this);
	m_teWaterMark->setMinimumWidth(200);
	m_teWaterMark->setMaximumWidth(200);
	m_teWaterMark->setMinimumHeight(200);
	m_teWaterMark->setMaximumHeight(200);
	m_teWaterMark->hide();
	m_teWaterMark->setReadOnly(true);
}

void CTakenBG::preProcess()
{
	//g_logBrowser->clear();
	if (m_pointsManage && m_pointsManage->getFrameCount(g_proInfo) > 0) {
		m_takenControl->setFrames(g_proInfo.fps, g_proInfo.frameInfos[0].pts, 1, g_proInfo.frameInfos[0].index);
		m_takenArea->drawFrame(g_proInfo.frameInfos[0].imagePath);
		closeAllPartZoom(g_proInfo);
		onPointsChange();
		
		CZoom::rect2Wnd(m_takenArea->getFrame(), g_proInfo.frameInfos[g_proInfo.curFrmaeIndex].zoomRects, m_zoomPair, m_takenArea);
		onPointsChange();
		
		m_imageEnhance->setValue(g_brightness, g_contrast, g_saturation);
		if (g_proInfo.frameInfos[0].pts == g_proInfo.signPTS)
			m_takenControl->setSign(true);
		else
			m_takenControl->setSign(false);
		//显示水印
		showWaterMark();
	}
}

int CTakenBG::initPointsManage(VTProjectInfo &info)
{
	info.curFrmaeIndex = 0;
	if (m_pointsManage) {
		m_pointsManage = nullptr;
		delete m_pointsManage;
	}
	CPointsManage::createPointsManage(this, info.mode, &m_pointsManage);
	preProcess();
	return 0;
}

bool CTakenBG::actualPointsReady()
{
	if(!m_pointsManage)
		initPointsManage(g_proInfo);
	return m_pointsManage ? m_pointsManage->actualPointsReady() : false;
}

void CTakenBG::notifyOCRResult(std::shared_ptr<OCRMsg> msg)
{
	emit sigOCRResult(msg);
}

void CTakenBG::onOCRResult(std::shared_ptr<OCRMsg> msg)
{
	auto size = g_proInfo.frameInfos.size();
	if (msg->uuid != g_proInfo.uuid)
		return;
	
	for (auto n = 0; n < size; ++n) {
		if (msg->decodeImage->pts == g_proInfo.frameInfos[n].pts) {
			g_proInfo.frameInfos[n].timeHMS = msg->decodeImage->waterMarkHMS.text;
			if (msg->decodeImage->waterMarks.size()) {
				g_proInfo.frameInfos[n].waterMarks.clear();
				for (int m = 0; m < msg->decodeImage->waterMarks.size(); ++m)
				{
					g_proInfo.frameInfos[n].waterMarks.append(msg->decodeImage->waterMarks[m].text);
				}
			}
			//status bar显示
			auto log = QString("第[%1]帧 PTS[%2] 水印[%3]").arg(g_proInfo.frameInfos[n].index).arg(g_proInfo.frameInfos[n].pts).arg(g_proInfo.frameInfos[n].timeHMS);
			emit g_vtmr->sigShowOCR(log);
			break;
		}

	}
	
	if(msg->last)
		showOCR();
}

void CTakenBG::onWaterMarkChanged(const QTime &time)
{
	if (0 > g_proInfo.curFrmaeIndex || g_proInfo.curFrmaeIndex >= g_proInfo.frameInfos.size())
		return;
	VTCorrectionWT(g_proInfo, g_proInfo.curFrmaeIndex, time);
}

void CTakenBG::showOCR()
{
	//矫正水印
	//VTCorrectWaterMark(g_proInfo);
	//显示时间
	showWaterMark();
	//打印
	for (auto n = 0; n < g_proInfo.frameInfos.size(); ++n)
	{
		auto log = QString("第[%1]帧 PTS[%2] 水印[%3]").arg(g_proInfo.frameInfos[n].index).arg(g_proInfo.frameInfos[n].pts).arg(g_proInfo.frameInfos[n].timeHMS);
		qDebug() << log;
		g_logBrowser ->append(log);
	}
	
	//检查是否有没检测到的水印
	VTCheckWaterMark(g_proInfo, windowFlags());
	//status bar显示 清空
	emit g_vtmr->sigShowOCR("");
}

void CTakenBG::showWaterMark()
{
	if (0 > g_proInfo.curFrmaeIndex || g_proInfo.curFrmaeIndex >= g_proInfo.frameInfos.size())
		return;
	auto waterMarkTime = g_proInfo.frameInfos[g_proInfo.curFrmaeIndex].timeHMS;
	QTime time;
	if (VTIsHHMMSS(waterMarkTime))
		time = QTime::fromString(waterMarkTime, "hh:mm:ss");
	else
		time = QTime::fromString("00:00:00", "hh:mm:ss");
	m_takenControl->setWaterMark(time);
}

CPointsManage *CTakenBG::getPointsManage()
{
	return m_pointsManage;
}

CTakenBG::~CTakenBG()
{
}

QWidget *CTakenBG::takenAreaWidget()
{
	return m_takenArea;
}

void CTakenBG::onHeadFrame() {
	if (!m_pointsManage)
	{
		qDebug() << "m_pointsManage == NULL";
		return;
	}
	if (0 == m_pointsManage->getFrameCount(g_proInfo))
		return;
	if (0 > g_proInfo.curFrmaeIndex)
		return;

	//若没有固定首帧，当前帧的点位清除
	m_pointsManage->clearImagePoints(g_proInfo, g_proInfo.curFrmaeIndex);


	auto index = g_proInfo.signPTS;
	m_takenArea->drawFrame(m_pointsManage->getImagePath(g_proInfo, index));
	m_imageEnhance->setValue(g_brightness, g_contrast, g_saturation);
	m_pointsManage->addPoints(index, QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT));

	//显示状态
	qint64 pts = 0;
	pts = m_pointsManage->getPTS(g_proInfo, g_proInfo.curFrmaeIndex);
	m_takenControl->setFrames(g_proInfo.fps, pts, g_proInfo.curFrmaeIndex + 1, g_proInfo.frameInfos[g_proInfo.curFrmaeIndex].index);

	closeAllPartZoom(g_proInfo);
	onPointsChange();

	CZoom::rect2Wnd(m_takenArea->getFrame(), g_proInfo.frameInfos[g_proInfo.curFrmaeIndex].zoomRects, m_zoomPair, m_takenArea);
	onPointsChange();

	m_pointsManage->clearCalc(g_proInfo);
	m_pointsManage->calcPointP0();
	if (pts == g_proInfo.signPTS)
		m_takenControl->setSign(true);
	else
		m_takenControl->setSign(false);

	//显示水印
	showWaterMark();
}
void CTakenBG::onPreviousFrame()
{
	if (!m_pointsManage)
	{
		qDebug() << "m_pointsManage == NULL";
		return;
	}
	if (0 == m_pointsManage->getFrameCount(g_proInfo))
		return;
	if (0 >= g_proInfo.curFrmaeIndex)
		return;

	//若没有固定首帧，当前帧的点位清除
	m_pointsManage->clearImagePoints(g_proInfo, g_proInfo.curFrmaeIndex);

	g_proInfo.curFrmaeIndex--;
	
	auto index = g_proInfo.curFrmaeIndex;
	m_takenArea->drawFrame(m_pointsManage->getImagePath(g_proInfo, index));
	m_imageEnhance->setValue(g_brightness, g_contrast, g_saturation);
	m_pointsManage->addPoints(index, QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT));

	//显示状态
	qint64 pts = 0;
	pts = m_pointsManage->getPTS(g_proInfo, g_proInfo.curFrmaeIndex);
	m_takenControl->setFrames(g_proInfo.fps, pts, g_proInfo.curFrmaeIndex+1, g_proInfo.frameInfos[g_proInfo.curFrmaeIndex].index);

	closeAllPartZoom(g_proInfo);
	onPointsChange();

	CZoom::rect2Wnd(m_takenArea->getFrame(), g_proInfo.frameInfos[g_proInfo.curFrmaeIndex].zoomRects, m_zoomPair, m_takenArea);
	onPointsChange();

	m_pointsManage->clearCalc(g_proInfo);
	m_pointsManage->calcPointP0();
	if (pts == g_proInfo.signPTS)
		m_takenControl->setSign(true);
	else
		m_takenControl->setSign(false);

	//显示水印
	showWaterMark();
}

void CTakenBG::onNextFrame()
{
	//qDebug() << "asd";
	if (!m_pointsManage) {
		qDebug() << "m_pointsManage == NULL";
		return;
	}
	if (0 == m_pointsManage->getFrameCount(g_proInfo))
		return;
	if (g_proInfo.curFrmaeIndex >= m_pointsManage->getFrameCount(g_proInfo) - 1)
		return;

	//若没有固定首帧，当前帧的点位清除
	m_pointsManage->clearImagePoints(g_proInfo, g_proInfo.curFrmaeIndex);
	g_proInfo.curFrmaeIndex++;

	auto index = g_proInfo.curFrmaeIndex;
	m_takenArea->drawFrame(m_pointsManage->getImagePath(g_proInfo, index));
	m_imageEnhance->setValue(g_brightness, g_contrast, g_saturation);
	m_pointsManage->addPoints(g_proInfo.curFrmaeIndex, QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT));

	m_pointsManage->doForecast(g_proInfo.curFrmaeIndex);
	qint64 pts = 0;
	pts = m_pointsManage->getPTS(g_proInfo, g_proInfo.curFrmaeIndex);
	m_takenControl->setFrames(g_proInfo.fps, pts, g_proInfo.curFrmaeIndex + 1, g_proInfo.frameInfos[g_proInfo.curFrmaeIndex].index);
	m_takenControl->setStatus(m_pointsManage->getFrameEditStutas(g_proInfo, g_proInfo.curFrmaeIndex));

	closeAllPartZoom(g_proInfo);
	onPointsChange();

	CZoom::rect2Wnd(m_takenArea->getFrame(), g_proInfo.frameInfos[g_proInfo.curFrmaeIndex].zoomRects, m_zoomPair, m_takenArea);
	onPointsChange();

	m_pointsManage->clearCalc(g_proInfo);
	m_pointsManage->calcPointP0();

	if (pts == g_proInfo.signPTS)
		m_takenControl->setSign(true);
	else
		m_takenControl->setSign(false);

	//显示水印
	showWaterMark();
}

void CTakenBG::onActionPoint(bool checked)
{
	if (checked)
		m_takenArea->setShape(TOOL_POINT);
	else
		m_takenArea->setShape(TOOL_NONE);
}

void CTakenBG::onActionEraser(bool checked)
{
	if (checked)
		m_takenArea->setShape(TOOL_ERASER);
	else
		m_takenArea->setShape(TOOL_NONE);
}

void CTakenBG::onActionLine(bool checked)
{
	if (checked) {
		//qDebug() << "press line true";

		m_takenArea->setShape(TOOL_LINE);
	}

	else {
		//qDebug() << "press line false";
		m_takenArea->setShape(TOOL_NONE);
	}

}



void CTakenBG::onActionRectangle(bool checked)
{
	if (checked)
		m_takenArea->setShape(TOOL_RECTANGLE);
	else
		m_takenArea->setShape(TOOL_NONE);
}

void CTakenBG::onActionEllipse(bool checked)
{
	if (checked)
		m_takenArea->setShape(TOOL_ELLIPSE);
	else
		m_takenArea->setShape(TOOL_NONE);
}

void CTakenBG::onActionPartZoom(bool checked)
{
	if (checked)
		m_takenArea->setShape(TOOL_PARTZOOM);
	else
		m_takenArea->setShape(TOOL_NONE);
}
void CTakenBG::onActionRedo()
{

	m_takenArea->setShape(TOOL_REDO);
	int i = g_proInfo.undoStack.index();
	auto command = (g_proInfo.undoStack.command(i));
	if (command != nullptr) {
		if (command->text() == "line1" || command->text() == "line2") {
			AddLineCommand* addLineCommand = (AddLineCommand*)command;
			if (addLineCommand && addLineCommand->vtProjectInfo && !addLineCommand->vtProjectInfo->imageLines.empty() && addLineCommand->frameIndex == g_proInfo.curFrmaeIndex) {
				g_proInfo.undoStack.redo();
			}
		}
		else if (command->text().contains("p")) {
			AddPointCommand* addPointCommand = (AddPointCommand*)command;
			if (addPointCommand && addPointCommand->vtFrameInfo && !addPointCommand->vtFrameInfo->imagePoints.empty() && addPointCommand->frameIndex == g_proInfo.curFrmaeIndex) {
				g_proInfo.undoStack.redo();
			}
		}
	}
}
void CTakenBG::onActionUndo()
{
	
	m_takenArea->setShape(TOOL_UNDO);
	int i = g_proInfo.undoStack.index();
	//调用栈的undo方法
	auto command = (g_proInfo.undoStack.command(i - 1));
	if (command != nullptr) {
		if (command->text() == "line1" || command->text() == "line2") {
			AddLineCommand* addLineCommand = (AddLineCommand*)command;
			if (addLineCommand && addLineCommand->vtProjectInfo && !addLineCommand->vtProjectInfo->imageLines.empty() && addLineCommand->frameIndex == g_proInfo.curFrmaeIndex) {
				g_proInfo.undoStack.undo();
			}
		}
		else if (command->text().contains("p")) {
			AddPointCommand* addPointCommand = (AddPointCommand*)command;
			if (addPointCommand && addPointCommand->vtFrameInfo && !addPointCommand->vtFrameInfo->imagePoints.empty() && addPointCommand->frameIndex == g_proInfo.curFrmaeIndex) {
				g_proInfo.undoStack.undo();
			}
		}
	}
}

void CTakenBG::onPointsChange()
{
	if (!m_pointsManage)
		return;

	if (m_pointsManage->getFrameCount(g_proInfo) <= 0)
		return;
	
	QVector<QPoint> qtPoints;
	QVector<VTPoint> vtPoints;
	QVector<VTLine> vtLines;
	VTPoint tPoint;
	VTLine tLine;
	m_pointsManage->getImagePoints(g_proInfo, g_proInfo.curFrmaeIndex, qtPoints);
	if (g_proInfo.mode == FIXED_CAM_ONESELF ||
		g_proInfo.mode == MOVING_CAM_SPACE) {
		if(!m_pointsManage->fixdFirstFrame()) {
			tLine.ponit1 = g_proInfo.imageLines[0][1];
			tLine.ponit2 = g_proInfo.imageLines[0][2];
			tLine.color = QColor(0, 255, 0);
			vtLines.push_back(tLine);

			tLine.ponit1 = g_proInfo.imageLines[1][1];
			tLine.ponit2 = g_proInfo.imageLines[1][2];
			tLine.color = QColor(255, 0, 0);
			vtLines.push_back(tLine);

			tPoint.point = g_proInfo.imageLines[0][0];
			tPoint.color = QColor(255, 0, 0);
			vtPoints.push_back(tPoint);

			tPoint.point = g_proInfo.imageLines[1][0];
			tPoint.color = QColor(0, 255, 0);
			vtPoints.push_back(tPoint);
		}
		else if (g_proInfo.headIndex == g_proInfo.curFrmaeIndex) {
			tLine.ponit1 = g_proInfo.imageLines[0][1];
			tLine.ponit2 = g_proInfo.imageLines[0][2];
			tLine.color = QColor(0, 255, 0);
			vtLines.push_back(tLine);

			tLine.ponit1 = g_proInfo.imageLines[1][1];
			tLine.ponit2 = g_proInfo.imageLines[1][2];
			tLine.color = QColor(255, 0, 0);
			vtLines.push_back(tLine);

			tPoint.point = g_proInfo.imageLines[0][0];
			tPoint.color = QColor(255, 0, 0);
			vtPoints.push_back(tPoint);

			tPoint.point = g_proInfo.imageLines[1][0];
			tPoint.color = QColor(0, 255, 0);
			vtPoints.push_back(tPoint);
		}
		else if (g_proInfo.headIndex < g_proInfo.curFrmaeIndex) {
			tPoint.point = g_proInfo.imageLines[0][0];
			tPoint.color = QColor(255, 0, 0);
			vtPoints.push_back(tPoint);

			tLine.ponit1 = g_proInfo.imageLines[0][1];
			tLine.ponit2 = g_proInfo.imageLines[0][2];
			tLine.color = QColor(0, 255, 0);
			vtLines.push_back(tLine);
		}
	}
	else if (g_proInfo.mode == FIXED_CAM_GROUND_REFERENCE) {
		tLine.ponit1 = g_proInfo.imageLines[0][1];
		tLine.ponit2 = g_proInfo.imageLines[0][2];
		tLine.color = QColor(0, 255, 0);
		vtLines.push_back(tLine);

		tLine.ponit1 = g_proInfo.imageLines[1][1];
		tLine.ponit2 = g_proInfo.imageLines[1][2];
		tLine.color = QColor(0, 255, 0);
		vtLines.push_back(tLine);
	}

	//局部放大框与源图框之间的连线
	QVector<VTLine> lines;
	for (auto & zoomPair : m_zoomPair) {
		VTLine tLine1, tLine2, tLine3;
		QVector<VTLine> zoomLines;
		QPoint p1 = zoomPair.srcWnd->pos();
		QPoint p2 = zoomPair.scaleWnd->pos();
		QRect r1 = QRect(p1, zoomPair.srcWnd->size());
		QRect r2 = QRect(p2, zoomPair.scaleWnd->size());
		CZoom::getLines(r1, r2, zoomLines);
		for(auto &vtLine : zoomLines)
			lines.push_back(vtLine);
	}

	m_takenArea->adjustDrawImage(
		g_contrast,
		g_saturation,
		g_brightness,
		qtPoints,
		vtLines,
		vtPoints,
		lines);
}

void CTakenBG::drawResoutImage(QImage &image, VTProjectInfo &info, int index)
{
	if (CPointsManage::getFrameCount(info) <= 0)
		return;

	QVector<QPoint> points;
	QVector<VTPoint> tPoints;
	QVector<VTLine> tLines;
	VTPoint tPoint;
	VTLine tLine;
	CPointsManage *pPointsManage = nullptr;

	CPointsManage::createPointsManage(nullptr, info.mode, &pPointsManage);
	if (!pPointsManage)
	{
		qDebug() << "pPointsManage == null";
		return;
	}
	CPointsManage::getImagePoints(info, index, points);
	if (info.mode == FIXED_CAM_ONESELF ||
		info.mode == MOVING_CAM_SPACE)
	{
		if (info.headIndex == index)
		{
			tLine.ponit1 = info.imageLines[0][1];
			tLine.ponit2 = info.imageLines[0][2];
			tLine.color = QColor(0, 255, 0);
			tLines.push_back(tLine);

			tLine.ponit1 = info.imageLines[1][1];
			tLine.ponit2 = info.imageLines[1][2];
			tLine.color = QColor(255, 0, 0);
			tLines.push_back(tLine);

			tPoint.point = info.imageLines[0][0];
			tPoint.color = QColor(255, 0, 0);
			tPoints.push_back(tPoint);
			tPoint.point = info.imageLines[1][0];
			tPoint.color = QColor(0, 255, 0);
			tPoints.push_back(tPoint);
		}
		else if (info.headIndex < index)
		{
			tPoint.point = info.imageLines[0][0];
			tPoint.color = QColor(255, 0, 0);
			tPoints.push_back(tPoint);
			tLine.ponit1 = info.imageLines[0][1];
			tLine.ponit2 = info.imageLines[0][2];
			tLine.color = QColor(0, 255, 0);
			tLines.push_back(tLine);
		}
	}
	else if (info.mode == FIXED_CAM_GROUND_REFERENCE)
	{
		tLine.ponit1 = info.imageLines[0][1];
		tLine.ponit2 = info.imageLines[0][2];
		tLine.color = QColor(0, 255, 0);
		tLines.push_back(tLine);

		tLine.ponit1 = info.imageLines[1][1];
		tLine.ponit2 = info.imageLines[1][2];
		tLine.color = QColor(0, 255, 0);
		tLines.push_back(tLine);
	}

	//局部放大
	QVector<VTLine> zoomRectTmp;
	for (auto & zoomRect : info.frameInfos[index].zoomRects) {
		QPoint begin = zoomRect.srcTopLeft;
		QPoint end = zoomRect.srcBottomRight;
		tLine.ponit1 = begin;
		tLine.ponit2 = end;
		tLine.color = QColor(ZOOM_LINE_COLOR);
		zoomRectTmp.push_back(tLine);
		QRect r1 = QRect(begin, end);

		begin = zoomRect.scaleTopLeft;
		int w = abs(zoomRect.srcBottomRight.x() - zoomRect.srcTopLeft.x()) * zoomRect.scale;
		int h = abs(zoomRect.srcBottomRight.y() - zoomRect.srcTopLeft.y()) * zoomRect.scale;
		end = zoomRect.scaleTopLeft + QPoint(w, h);
		tLine.ponit1 = begin;
		tLine.ponit2 = end;
		tLine.color = QColor(ZOOM_LINE_COLOR);
		zoomRectTmp.push_back(tLine);

		QVector<VTLine> zoomLines;
		QRect r2 = QRect(begin, end);
		CZoom::getLines(r1, r2, zoomLines);
		for (auto &line : zoomLines) {
			line.color = QColor(ZOOM_LINE_COLOR);
			tLines.push_back(line);
		}
	}

	CTakenWidget::adjustDrawImage(image, points, tLines, tPoints, zoomRectTmp, info.frameInfos[index].zoomRects);
}

void CTakenBG::onAddPoint(QPoint point)
{
	if(m_pointsManage)
		m_pointsManage->addPoints(g_proInfo.curFrmaeIndex,point);
}

void CTakenBG::onAddLine(QPoint center, QPoint begin, QPoint end)
{
	if(m_pointsManage)
		m_pointsManage->addLine(g_proInfo.curFrmaeIndex, center, begin, end);
}

void CTakenBG::onAddEllipse()
{
	if(m_pointsManage)
		m_pointsManage->addEllipse();
}

void CTakenBG::onAddRectangl()
{
	if(m_pointsManage)
		m_pointsManage->addRectangle();
}

void CTakenBG::onDelPoint(QPoint point)
{
	if(m_pointsManage)
		m_pointsManage->delPoints(g_proInfo, g_proInfo.curFrmaeIndex, point);
}

void CTakenBG::onAddPartZoom(QImage *image, QPoint begin, QPoint end)
{
	auto zoomSrc = new CZoom(0, 1.0, image, begin, end, m_takenArea);
	zoomSrc->show();
	
	auto zoomScale = new CZoom(1, 2.0, image, begin, end, m_takenArea);
	zoomScale->show();

	ZoomPair zoomPair;
	zoomPair.srcWnd = zoomSrc;
	zoomPair.scaleWnd = zoomScale;
	m_zoomPair.push_back(zoomPair);

	VTZoomPairRect zoomRect;
	zoomRect.srcTopLeft = begin;
	zoomRect.srcBottomRight = end;
	zoomRect.scaleTopLeft = zoomScale->pos();
	zoomRect.scale = 2.0;
	g_proInfo.frameInfos[g_proInfo.curFrmaeIndex].zoomRects.push_back(zoomRect);

	onPointsChange();

	delete image;
}

void CTakenBG::onNotifyMousePos(QPoint point)
{
	m_takenControl->setXY(point);
}

void CTakenBG::onNotifyStatus(int status)
{
	m_takenControl->setStatus(status);
	if (2 == status) {
		if(m_pointsManage)
			m_pointsManage->initForecast();
	}
}

void CTakenBG::onNotifyVelocity(QString text)
{
	if (!m_pointsManage)
		return;
	m_takenControl->setVelocity(text);
	
	m_traceWidget->drawVPoints();

	QVector<QPointF> p0s;
	QVector<int64_t> ptss;
	CPointsManage::getMPoints(g_proInfo, p0s, ptss);

	int width = 1920, height = 1080;
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

void CTakenBG::onNotifyVelocity()
{
	float v = 0.0;
	if (0 == m_pointsManage->getFrameV(g_proInfo, g_proInfo.curFrmaeIndex, v)) {
		auto text = QString("速度: %1 km/h").arg(v);
		m_takenControl->setVelocity(text);
	}

	m_traceWidget->drawVPoints();

	QVector<QPointF> p0s;
	QVector<int64_t> ptss;
	CPointsManage::getMPoints(g_proInfo, p0s, ptss);

	int width = 1920, height = 1080;
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

void CTakenBG::onActualPointsChange()
{
	if (!m_pointsManage)
		return;
	m_pointsManage->clearCalc(g_proInfo);
	m_pointsManage->calcPointP0();
}

void CTakenBG::onDrawImage()
{
	if (!m_pointsManage)
		return;

	if (0 == m_pointsManage->getFrameCount(g_proInfo))
		return;
	if (g_proInfo.curFrmaeIndex > m_pointsManage->getFrameCount(g_proInfo) - 1)
		return;
	if (g_proInfo.curFrmaeIndex < 0)
		return;
	if (m_takenArea) {
		int index = g_proInfo.curFrmaeIndex;
		m_takenArea->drawFrame(m_pointsManage->getImagePath(g_proInfo, index));
	}
}

void CTakenBG::onPartZoomChange(QWidget *widget)
{
	if (g_proInfo.curFrmaeIndex < 0 || g_proInfo.curFrmaeIndex >= g_proInfo.frameInfos.size())
		return;
	for (auto &iterZoomRect : g_proInfo.frameInfos[g_proInfo.curFrmaeIndex].zoomRects) {
		for (auto &iterZoomPair : m_zoomPair) {
			auto zoom = (CZoom *)iterZoomPair.scaleWnd;
			if (iterZoomRect.srcTopLeft == zoom->getBeginPos() && iterZoomRect.srcBottomRight == zoom->getEndPos()) {
				iterZoomRect.scaleTopLeft = zoom->pos();
				iterZoomRect.scale = zoom->getScale();
				break;
			}
		}
	}

	onPointsChange();
}

void CTakenBG::onPartZoomMove(QWidget *widget)
{
	if (g_proInfo.curFrmaeIndex < 0 || g_proInfo.curFrmaeIndex > g_proInfo.frameInfos.size())
		return;

	for (auto &zoomRect : g_proInfo.frameInfos[g_proInfo.curFrmaeIndex].zoomRects) {
		auto zoom = (CZoom *)widget;
		if (zoomRect.srcTopLeft == zoom->getBeginPos() && zoomRect.srcBottomRight == zoom->getEndPos()) {
			zoomRect.scaleTopLeft = zoom->pos();
		}
	}
	onPointsChange();
}

void CTakenBG::onClosePartZoom(QWidget *widget)
{
	auto iter = m_zoomPair.begin();
	for (; iter != m_zoomPair.end();) {
		ZoomPair zoomPair = *iter;
		if (zoomPair.srcWnd == widget || zoomPair.scaleWnd == widget) {
			auto iterZoomRect = g_proInfo.frameInfos[g_proInfo.curFrmaeIndex].zoomRects.begin();
			for (; iterZoomRect != g_proInfo.frameInfos[g_proInfo.curFrmaeIndex].zoomRects.end(); ) {
				CZoom * zoom = (CZoom *)widget;
				if ((*iterZoomRect).srcTopLeft == zoom->getBeginPos() && (*iterZoomRect).srcBottomRight == zoom->getEndPos()) {
					iterZoomRect = g_proInfo.frameInfos[g_proInfo.curFrmaeIndex].zoomRects.erase(iterZoomRect);
				}
				else
					iterZoomRect++;
			}

			iter = m_zoomPair.erase(iter);
			zoomPair.srcWnd->close();
			zoomPair.scaleWnd->close();
		}
		else
			iter++;
	}
	onPointsChange();
}

void CTakenBG::closeAllPartZoom(VTProjectInfo &info)
{
	auto iter = m_zoomPair.begin();
	for (; iter != m_zoomPair.end();) {
		ZoomPair zoomPair = *iter;
		zoomPair.srcWnd->close();
		zoomPair.scaleWnd->close();
		iter = m_zoomPair.erase(iter);
	}
}

void CTakenBG::onWaterMarkWnd(QPoint pos)
{
	if (m_teWaterMark->isHidden()) {
		m_teWaterMark->clear();
		if (g_proInfo.curFrmaeIndex >= 0 && g_proInfo.curFrmaeIndex < g_proInfo.frameInfos.size()) {
			for(auto &waterMark : g_proInfo.frameInfos[g_proInfo.curFrmaeIndex].waterMarks)
				m_teWaterMark->append(waterMark);
		}
		int x = m_takenControl->pos().x() + pos.x();
		int y = m_takenControl->pos().y() - 200;

		m_teWaterMark->move(x, y);
		m_teWaterMark->show();
	}
	else {
		m_teWaterMark->hide();
	}
}

void CTakenBG::modeChanged()
{
	if(m_pointsManage && m_pointsManage->getMode() != g_proInfo.mode)
		CPointsManage::createPointsManage(this, g_proInfo.mode, &m_pointsManage);
}

void CTakenBG::reDoClick()
{
	qDebug() << "true";
	QVector<QPoint> points;
	QVector<VTLine> tLines;
	QVector<VTPoint> tPoints;
	QVector<VTLine> zoom;
	m_takenArea->adjustDrawImage(
		g_contrast,
		g_saturation,
		g_brightness,
		points,
		tLines,
		tPoints,
		zoom
	);
}
