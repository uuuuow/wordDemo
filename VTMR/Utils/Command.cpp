
#include "Command.h"
AddLineCommand::AddLineCommand(const QString& text, VTProjectInfo* info, CPointsManage* pointsManage, 
	QPoint center, QPoint begin, QPoint end, int frameIndex)
	: QUndoCommand(text, nullptr), vtProjectInfo(info), pointsManage(pointsManage), frameIndex(frameIndex) {
	imageLines.append(center); // 中心
	imageLines.append(begin); // 开始
	imageLines.append(end); // 结束
}
AddRectCommand::AddRectCommand(QString type, VTProjectInfo* info, CPointsManage* pointsManage, ZoomPair* zoomPair)
	: type(type), vtProjectInfo(info), pointsManage(pointsManage), zoomPair(zoomPair)
{

}
void AddRectCommand::redo() {
	if (zoomPair->scaleWnd && zoomPair->srcWnd) {
		zoomPair->scaleWnd->show();
		zoomPair->srcWnd->show();
	}
}
void AddRectCommand::undo() {
	//if (zoomPair.scaleWnd && zoomPair.srcWnd) {
	//	CZoom* sc = (CZoom*)zoomPair.scaleWnd;
	//	emit sc->sigClosePartZoom(sc);
	//	CZoom* sr = (CZoom*)zoomPair.srcWnd;
	//	emit sr->sigClosePartZoom(sr);
	//}
}
void AddLineCommand::redo() {
	//1.改变值
	if (text() == "line1") {
		vtProjectInfo->imageLines[0][0] = imageLines[0];
		vtProjectInfo->imageLines[0][1] = imageLines[1];
		vtProjectInfo->imageLines[0][2] = imageLines[2];
	}
	else if (text() == "line2") {
		vtProjectInfo->imageLines[1][0] = imageLines[0];
		vtProjectInfo->imageLines[1][1] = imageLines[1];
		vtProjectInfo->imageLines[1][2] = imageLines[2];
	}
	if (pointsManage) {
		emit pointsManage->sigPointsChange();
	}
}
void AddLineCommand::undo() {
	//1.改变值
	if (text() == "line1") {
		vtProjectInfo->imageLines[0][0] = QPoint(INVALID_VALUE_DOUBLE, INVALID_VALUE_DOUBLE);
		vtProjectInfo->imageLines[0][1] = QPoint(INVALID_VALUE_DOUBLE, INVALID_VALUE_DOUBLE);
		vtProjectInfo->imageLines[0][2] = QPoint(INVALID_VALUE_DOUBLE, INVALID_VALUE_DOUBLE);
	} else if (text() == "line2") {
		vtProjectInfo->imageLines[1][0] = QPoint(INVALID_VALUE_DOUBLE, INVALID_VALUE_DOUBLE);
		vtProjectInfo->imageLines[1][1] = QPoint(INVALID_VALUE_DOUBLE, INVALID_VALUE_DOUBLE);
		vtProjectInfo->imageLines[1][2] = QPoint(INVALID_VALUE_DOUBLE, INVALID_VALUE_DOUBLE);
	}
	if (pointsManage) {
		emit pointsManage->sigPointsChange();
	}
}

AddPointCommand::AddPointCommand(const QString& text, VTFrameInfo* info, CPointsManage* pointsManage, QPoint point, int frameIndex)
	: QUndoCommand(text, nullptr), vtFrameInfo(info), pointsManage(pointsManage), point(point), frameIndex(frameIndex)
{
	pos = this->text().at(1).digitValue();
}

void AddPointCommand::redo()
{
	vtFrameInfo->imagePoints[pos] = point;
	if (pointsManage) {
		emit pointsManage->sigPointsChange();
	}
}

void AddPointCommand::undo()
{
	vtFrameInfo->imagePoints[pos] = QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT);
	if (pointsManage) {
		emit pointsManage->sigPointsChange();
	}
}
