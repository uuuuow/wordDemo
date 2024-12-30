#pragma once
#include <QJsonObject>
#include <QJsonArray>
#include <QPointF>
#include "Utils.h"
/*
*数据序列化与反序列化
*/
void encodeRect(QJsonObject &objRect, QRect &rc);
void decodeRect(QJsonObject &objRect, QRect &rc);

void encodeWaterTime(QJsonObject &obj, VTWaterMark &waterMark);
void decodeWaterTime(QJsonObject &obj, VTWaterMark &waterMark);

void encodeWaterTimes(QJsonArray &objArray, QVector<VTWaterMark> &waterMarks);
void decodeWaterTimes(QJsonArray &objArray, QVector<VTWaterMark> &waterMarks);

void encodeOCR(QJsonObject &obj, std::shared_ptr<VTDecodeImage> decodeImage);
void decodeOCR(QJsonObject &obj, std::shared_ptr<VTDecodeImage> decodeImage);

int encodeActualPoint(QJsonObject &obj, QPointF &point);
int decodeActualPoint(QJsonObject &obj, QPointF &point);

int encodeActualPoints(QJsonArray &objArray, QVector<QPointF> &points);
int decodeActualPoints(QJsonArray &objArray, QVector<QPointF> &points);

int encodeImagePoint(QJsonObject &obj, QPoint &point, int &index);
int decodeImagePoint(QJsonObject &obj, QPoint &point, int &index);

int encodeImagePoints(QJsonArray &objArray, QVector<QPoint> &point);
int decodeImagePoints(QJsonArray &objArray, QVector<QPoint> &point);

int encodeFrameInfo(QJsonObject &objFrameInfo, VTFrameInfo &frameInfo);
int decodeFrameInfo(QJsonObject &objFrameInfo, VTFrameInfo &frameInfo);

int encodeFrameInfos(QJsonArray &objArray, QVector<VTFrameInfo> &frameInfos);
int decodeFrameInfos(QJsonArray &objArray, QVector<VTFrameInfo> &frameInfos);

int encodeProjectInfo(QJsonObject &objProjectInfo, VTProjectInfo &proInfo);
int decodeProjectInfo(QJsonObject &objProjectInfo, VTProjectInfo &proInfo);

QString actualPointstoJsonString(QVector<QPointF> &actualPoints);
QString actualPointtoJsonString(QPointF point);

int decodeLogs(QString logs);