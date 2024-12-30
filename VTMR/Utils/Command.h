#pragma once
#include <QUndoStack>
#include <PointsManage/PointsManage.h>
#include <Zoom.h>
class AddLineCommand : public QUndoCommand {
public:
	AddLineCommand(const QString& text, VTProjectInfo* info, CPointsManage* pointsManage, QPoint center, QPoint begin, QPoint end, int frameIndex);
	void redo() override;
	void undo() override;
	int frameIndex;
	VTProjectInfo* vtProjectInfo;
private:

	
	CPointsManage* pointsManage;
	QVector<QPoint> imageLines;	//Ö±Ïß
};
class AddPointCommand : public QUndoCommand {
public:
	AddPointCommand(const QString& text, VTFrameInfo* info, CPointsManage* pointsManage, QPoint point, int frameIndex);
	void redo() override;
	void undo() override;
	VTFrameInfo* vtFrameInfo;
	CPointsManage* pointsManage;
	int frameIndex;
private:
	int pos = 0;
	QPoint point;
};
class AddRectCommand : public QUndoCommand {
public:
	AddRectCommand(QString type, VTProjectInfo* info, CPointsManage* pointsManage, ZoomPair* zoomPair);
	void redo() override;
	void undo() override;
private:
	QString type;
	VTProjectInfo* vtProjectInfo;
	CPointsManage* pointsManage;
	ZoomPair* zoomPair;
};