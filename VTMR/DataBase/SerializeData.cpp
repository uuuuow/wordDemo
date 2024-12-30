#include "SerializeData.h"
#include <QJsonDocument>
#include <QVector>
#include <QVariant>
#include <Utils/Global.h>
#include <Utils/Utils.h>
#include <QtCore/qfile.h>

int encodeActualPoint(QJsonObject &obj, QPointF &point)
{
	obj.insert("x", point.rx());
	obj.insert("y", point.ry());
	return 0;
}

int decodeActualPoint(QJsonObject &obj, QPointF &point)
{
	auto x = INVALID_VALUE_FLOAT;
	auto y = INVALID_VALUE_FLOAT;
	if (obj.contains("x")) {
		auto value = obj.value("x");
		if (value.isDouble()) {
			x = value.toVariant().toFloat();
		}
	}

	if (obj.contains("y")) {
		auto value = obj.value("y");
		if (value.isDouble()) {
			y = value.toVariant().toFloat();
		}
	}
	point = QPointF(x, y);
	return 0;
}

int encodeActualPoints(QJsonArray &objArray, QVector<QPointF> &points)
{
	for (auto n = 0; n < points.size(); n++) {
		QJsonObject objPoint;
		encodeActualPoint(objPoint, points[n]);
		objArray.append(objPoint);
	}
	return 0;
}

int decodeActualPoints(QJsonArray &objArray, QVector<QPointF> & points)
{
	auto size = objArray.size();
	for (auto i = 0; i < size; ++i) {
		auto value = objArray.at(i);
		if (value.isObject()) {
			auto obj = value.toObject();
			QPointF point;
			decodeActualPoint(obj, point);
			points[i] = point;
		}
	}
	return 0;
}

int encodeImagePoint(QJsonObject &obj, QPoint &point, int &index)
{
	obj.insert("i", index);
	obj.insert("x", point.rx());
	obj.insert("y", point.ry());
	return 0;
}

int decodeImagePoint(QJsonObject &obj, QPoint &point, int &index)
{
	auto x = INVALID_VALUE_INT;
	auto y = INVALID_VALUE_INT;
	if (obj.contains("i")) {
		auto value = obj.value("i");
		if (value.isDouble()) {
			index = value.toInt();
		}
	}

	if (obj.contains("x")) {
		auto value = obj.value("x");
		if (value.isDouble()) {
			x = value.toInt();
		}
	}

	if (obj.contains("y")) {
		auto value = obj.value("y");
		if (value.isDouble()) {
			y = value.toInt();
		}
	}
	point = QPoint(x, y);
	return 0;
}

int encodeImagePoints(QJsonArray &objArray, QVector<QPoint> &point)
{
	for (auto n = 0; n < IMAGE_POINTS_COUNT; n++) {
		QJsonObject objPoint;
		encodeImagePoint(objPoint, point[n], n);
		objArray.append(objPoint);
	}
	return 0;
}

int decodeImagePoints(QJsonArray &objArray, QVector<QPoint> &point)
{
	auto size = objArray.size();
	for (auto i = 0; i < size; ++i) {
		if (i > IMAGE_POINTS_COUNT - 1)
			return -1;
		auto value = objArray.at(i);
		if (value.isObject()) {
			auto obj = value.toObject();
			auto index = -1;
			QPoint tmp(INVALID_VALUE_INT, INVALID_VALUE_INT);
			decodeImagePoint(obj, tmp, index);
			if (index >= 0 && index < point.size()) {
				point[index] = tmp;
			}
		}
	}
	return 0;
}

static int encodeZoomPair(QJsonObject &objZoomPair, VTZoomPairRect &zoom)
{
	auto index = 0;
	objZoomPair.insert("scale", zoom.scale);
	QJsonObject objPoint;
	encodeImagePoint(objPoint, zoom.srcTopLeft, index);
	objZoomPair.insert("src_top_left", objPoint);

	encodeImagePoint(objPoint, zoom.srcBottomRight, index);
	objZoomPair.insert("src_bottom_right", objPoint);

	encodeImagePoint(objPoint, zoom.scaleTopLeft, index);
	objZoomPair.insert("scale_top_left", objPoint);
	return 0;
}

static int decodeZoomPair(QJsonObject &objZoomPair, VTZoomPairRect &zoom)
{
	auto index = 0;
	if (objZoomPair.contains("scale")) {
		auto value = objZoomPair.value("scale");
		if (value.isDouble()) {
			zoom.scale = value.toDouble();
		}
	}

	if (objZoomPair.contains("src_top_left")) {
		auto value = objZoomPair.value("src_top_left");
		if (value.isObject()) {
			auto obj = value.toObject();
			decodeImagePoint(obj, zoom.srcTopLeft, index);
		}
	}

	if (objZoomPair.contains("src_bottom_right")) {
		auto value = objZoomPair.value("src_bottom_right");
		if (value.isObject()) {
			auto obj = value.toObject();
			decodeImagePoint(obj, zoom.srcBottomRight, index);
		}
	}

	if (objZoomPair.contains("scale_top_left")) {
		auto value = objZoomPair.value("scale_top_left");
		if (value.isObject()) {
			auto obj = value.toObject();
			decodeImagePoint(obj, zoom.scaleTopLeft, index);
		}
	}
	return 0;
}

static int encodeZoomPairs(QJsonArray &objZoomPairs, QVector<VTZoomPairRect> &zooms)
{
	for (auto n = 0; n < zooms.size(); ++n) {
		QJsonObject objZoomPair;
		encodeZoomPair(objZoomPair, zooms[n]);
		objZoomPairs.append(objZoomPair);
	}
	return 0;
}

static int decodeZoomPairs(QJsonArray &objZoomPairs, QVector<VTZoomPairRect> &zooms)
{
	auto size = objZoomPairs.size();
	for (auto i = 0; i < size; ++i) {		
		auto value = objZoomPairs.at(i);
		if (value.isObject()) {
			auto obj = value.toObject();
			VTZoomPairRect zoom;
			decodeZoomPair(obj, zoom);
			zooms.push_back(zoom);
		}
	}
	return 0;
}

int encodeFrameInfo(QJsonObject &objFrameInfo, VTFrameInfo &frameInfo)
{
	objFrameInfo.insert("index", frameInfo.index);
	objFrameInfo.insert("pts", frameInfo.pts);
	objFrameInfo.insert("frame_edit_stutas", frameInfo.frameEditStutas);
	objFrameInfo.insert("v1", frameInfo.v1);
	objFrameInfo.insert("v2", frameInfo.v2);
	objFrameInfo.insert("v3", frameInfo.v3);
	objFrameInfo.insert("image_path", frameInfo.imagePath);
	objFrameInfo.insert("md5", frameInfo.md5);
	objFrameInfo.insert("water_time_hhmmss", frameInfo.timeHMS);

	QJsonObject objActualP0;
	encodeActualPoint(objActualP0, frameInfo.actualP0);
	objFrameInfo.insert("actual_p0", objActualP0);

	QJsonArray objImagePoints;
	encodeImagePoints(objImagePoints, frameInfo.imagePoints);
	objFrameInfo.insert("image_points", objImagePoints);

	QJsonArray objZoomPairs;
	encodeZoomPairs(objZoomPairs, frameInfo.zoomRects);
	objFrameInfo.insert("zoom_pairs", objZoomPairs);
	return 0;
}

int decodeFrameInfo(QJsonObject &objFrameInfo, VTFrameInfo &frameInfo)
{
	if (objFrameInfo.contains("index")) {
		auto value = objFrameInfo.value("index");
		if (value.isDouble()) {
			frameInfo.index = value.toInt();
		}
	}

	if (objFrameInfo.contains("pts")) {
		auto value = objFrameInfo.value("pts");
		if (value.isDouble()) {
			frameInfo.pts = value.toInt();
		}
	}

	if (objFrameInfo.contains("frame_edit_stutas")) {
		auto value = objFrameInfo.value("frame_edit_stutas");
		if (value.isDouble()) {
			frameInfo.frameEditStutas = value.toInt();
		}
	}

	if (objFrameInfo.contains("v1")) {
		auto value = objFrameInfo.value("v1");
		if (value.isDouble()) {
			frameInfo.v1 = value.toVariant().toFloat();
		}
	}

	if (objFrameInfo.contains("v2")) {
		auto value = objFrameInfo.value("v2");
		if (value.isDouble()) {
			frameInfo.v2 = value.toVariant().toFloat();
		}
	}

	if (objFrameInfo.contains("v3")) {
		auto value = objFrameInfo.value("v3");
		if (value.isDouble()) {
			frameInfo.v3 = value.toVariant().toFloat();
		}
	}

	if (objFrameInfo.contains("image_path")) {
		auto value = objFrameInfo.value("image_path");
		if (value.isString()) {
			frameInfo.imagePath = value.toVariant().toString();
		}
	}

	if (objFrameInfo.contains("md5")) {
		auto value = objFrameInfo.value("md5");
		if (value.isString()) {
			frameInfo.md5 = value.toVariant().toString();
		}
	}

	if (objFrameInfo.contains("actual_p0")) {
		auto value = objFrameInfo.value("actual_p0");
		if (value.isObject()) {
			auto objActualP0 = value.toObject();
			decodeActualPoint(objActualP0, frameInfo.actualP0);
		}
	}

	if (objFrameInfo.contains("image_points")) {
		auto value = objFrameInfo.value("image_points");
		if (value.isArray()) {
			auto objImagePoints = value.toArray();
			decodeImagePoints(objImagePoints, frameInfo.imagePoints);
		}
	}

	if (objFrameInfo.contains("water_time_hhmmss")) {
		auto value = objFrameInfo.value("water_time_hhmmss");
		if (value.isString()) {
			frameInfo.timeHMS = value.toVariant().toString();
		}
	}

	if (objFrameInfo.contains("zoom_pairs")) {
		auto value = objFrameInfo.value("zoom_pairs");
		if (value.isArray()) {
			auto objZoomPairs = value.toArray();
			decodeZoomPairs(objZoomPairs, frameInfo.zoomRects);
		}
	}
	return 0;
}

int encodeFrameInfos(QJsonArray &objArray, QVector<VTFrameInfo> &frameInfos)
{
	for (auto n = 0; n < frameInfos.size(); n++) {
		QJsonObject objPoint;
		encodeFrameInfo(objPoint, frameInfos[n]);
		objArray.append(objPoint);
	}
	return 0;
}

int decodeFrameInfos(QJsonArray &objArray, QVector<VTFrameInfo> &frameInfos)
{
	auto size = objArray.size();
	for (auto i = 0; i < size; ++i) {
		auto value = objArray.at(i);
		if (value.isObject())
		{
			QJsonObject obj = value.toObject();
			VTFrameInfo frameInfo;
			decodeFrameInfo(obj, frameInfo);
			frameInfos.push_back(frameInfo);
		}
	}
	return 0;
}

static int enCodeCameraCalibration(QJsonObject &cameraCalibration, VTDedistortion &dedistortion)
{
	cameraCalibration.insert("has", dedistortion.has);
	cameraCalibration.insert("object_w", dedistortion.objW);
	cameraCalibration.insert("object_h", dedistortion.objH);
	cameraCalibration.insert("square_size", dedistortion.squareSize);

	auto count = 0;
	QJsonArray matrixs;
	for (auto i = 0; i < 3; i++)
		for (auto j = 0; j < 3; j++)
		{
			QJsonObject matrix;
			matrix.insert("index", count);
			matrix.insert("value", dedistortion.matrixs[i][j]);
			matrixs.append(matrix);
			count++;
		}

	count = 0;
	QJsonArray distCoeffs;
	for (auto n = 0; n < 5; n++)
	{
		QJsonObject distCoeff;
		distCoeff.insert("index", count);
		distCoeff.insert("value",dedistortion.distCoeffs[n]);
		distCoeffs.append(distCoeff);
		count++;
	}
	
	cameraCalibration.insert("matrix", matrixs);
	cameraCalibration.insert("distcoeff", distCoeffs);

	QJsonArray imagePath;
	for (auto n = 0; n < dedistortion.listImage.size(); n++)
	{
		auto str = dedistortion.listImage.at(n);
		imagePath.append(str);
	}
	cameraCalibration.insert("image_path", imagePath);
	return 0;
}

static int deCodeCameraCalibration(QJsonObject &cameraCalibration, VTDedistortion &dedistortion)
{
	if (cameraCalibration.contains("has")) {
		auto value = cameraCalibration.value("has");
		if (value.isDouble()) {
			dedistortion.has = value.toVariant().toInt();
		}
	}

	if (cameraCalibration.contains("object_w")) {
		auto value = cameraCalibration.value("object_w");
		if (value.isDouble()) {
			dedistortion.objW = value.toVariant().toInt();
		}
	}

	if (cameraCalibration.contains("object_h")) {
		auto value = cameraCalibration.value("object_h");
		if (value.isDouble()) {
			dedistortion.objH = value.toVariant().toInt();
		}
	}

	if (cameraCalibration.contains("square_size")) {
		auto value = cameraCalibration.value("square_size");
		if (value.isDouble()) {
			dedistortion.squareSize = value.toVariant().toInt();
		}
	}

	if (cameraCalibration.contains("matrix")) {
		auto value = cameraCalibration.value("matrix");
		if (value.isArray()) {
			auto matrixs = value.toArray();
			auto size = matrixs.size();
			for (auto i = 0; i < size; ++i) {
				auto value = matrixs.at(i);
				if (value.isObject()) {
					auto obj = value.toObject();
					if (obj.contains("index")) {
						auto value = obj.value("index");
						auto index = value.toInt();
						if (index >= 0 && index < 9) {
							if (obj.contains("value")) {
								auto value = obj.value("value");
								auto m = 0, n = 0;
								m = index / 3;
								n = index % 3;
								dedistortion.matrixs[m][n] = value.toDouble();
							}
						}
					}
				}
			}
		}
	}

	if (cameraCalibration.contains("distcoeff")) {
		auto value = cameraCalibration.value("distcoeff");
		if (value.isArray()) {
			auto distcoeffs = value.toArray();
			auto size = distcoeffs.size();
			for (auto i = 0; i < size; ++i) {
				auto value = distcoeffs.at(i);
				if (value.isObject()) {
					auto obj = value.toObject();
					if (obj.contains("index")) {
						auto value = obj.value("index");
						auto index = value.toInt();
						if (index >= 0 && index < 5) {
							if (obj.contains("value"))
							{
								auto value = obj.value("value");
								dedistortion.distCoeffs[index] = value.toDouble();
							}
						}
					}
				}
			}
		}
	}

	if (cameraCalibration.contains("image_path")) {
		auto value = cameraCalibration.value("image_path");
		if (value.isArray()) {
			auto imagePath = value.toArray();
			auto size = imagePath.size();
			for (auto n = 0; n < size; n++) {
				if (imagePath.at(n).isString()) {
					auto str = imagePath.at(n).toString();
					dedistortion.listImage.append(str);
				}
			}
		}
	}

	QJsonArray imagePath;
	for (auto n = 0; n < dedistortion.listImage.size(); n++) {
		auto str = dedistortion.listImage.at(n);
		imagePath.append(str);
	}
	cameraCalibration.insert("image_path", imagePath);

	return 0;
}

static int encodeLinePoints(QJsonArray &objArray, QVector<QVector<QPoint> > &linePoints)
{
	if (linePoints.size() == 0)
		return 0;
	for (auto i = 0; i < linePoints.size(); ++i)
	{
		QJsonArray objPoints;	
		if (linePoints[i].size() != 3)
			continue;
		for (auto j = 0; j < linePoints[i].size(); ++j)
		{
			QJsonObject objPoint;
			encodeImagePoint(objPoint, linePoints[i][j], j);
			objPoints.append(objPoint);
		}
		objArray.append(objPoints);
	}
	return 0;
}

static int decodeLinePoints(QJsonArray &objArray, QVector<QVector<QPoint> > &linePoints)
{
	for (auto i = 0; i < objArray.size(); ++i) {
		auto root = objArray.at(i);
		if (!root.isArray())
			continue;
		auto item = root.toArray();
		QVector<QPoint> points;
		points.push_back(QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT));
		points.push_back(QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT));
		points.push_back(QPoint(INVALID_VALUE_INT, INVALID_VALUE_INT));
		for (auto j = 0; j < item.size(); ++j) {
			auto value = item.at(j);
			if (value.isObject())
			{
				auto obj = value.toObject();
				QPoint tmp(INVALID_VALUE_INT, INVALID_VALUE_INT);
				auto index = -1;
				decodeImagePoint(obj, tmp, index);
				if(index >= 0 && index < points.size())
					points[index] = tmp;
			}
		}
		if (points.size() == 0)
			continue;
		linePoints[i] = points;
	}
	return 0;
}

void encodeRect(QJsonObject &objRect, QRect &rc)
{
	objRect.insert("x", rc.x());
	objRect.insert("y", rc.y());
	objRect.insert("w", rc.width());
	objRect.insert("h", rc.height());
}

void decodeRect(QJsonObject &objRect, QRect &rc)
{
	rc = QRect(0, 0, 0, 0);
	if (objRect.contains("x")) {
		auto value = objRect.value("x");
		if (value.isDouble()) {
			rc.setX(value.toVariant().toInt());
		}
	}

	if (objRect.contains("y")) {
		auto value = objRect.value("y");
		if (value.isDouble()) {
			rc.setY(value.toVariant().toInt());
		}
	}

	if (objRect.contains("w")) {
		auto value = objRect.value("w");
		if (value.isDouble()) {
			rc.setWidth(value.toVariant().toInt());
		}
	}

	if (objRect.contains("h")) {
		auto value = objRect.value("h");
		if (value.isDouble()) {
			rc.setHeight(value.toVariant().toInt());
		}
	}
}

static void encodeAppraisalStandards(QJsonArray& objArray, QVector<VTAppraisalStandard>& standards)
{
	if (standards.size() == 0)
		return;
	for (auto i = 0; i < standards.size(); ++i)
	{
		QJsonObject obj;
		obj.insert("id", standards[i].id);
		obj.insert("name", standards[i].name);
		objArray.append(obj);
	}
}

static void decodeAppraisalStandards(QJsonArray& objArray, QVector<VTAppraisalStandard>& standards)
{
	for (auto i = 0; i < objArray.size(); ++i) {
		auto root = objArray.at(i);
		if (!root.isObject())
			continue;
		auto item = root.toObject();
		VTAppraisalStandard standard;
		if (!item.contains("id") || !item.contains("name"))
			continue;
		auto value = item.value("id");
		if (value.isString()) {
			standard.id = value.toVariant().toString();
		}
		value = item.value("name");
		if (value.isString()) {
			standard.name = value.toVariant().toString();
		}
		standards.push_back(standard);
	}
}

int encodeProjectInfo(QJsonObject &objProjectInfo, VTProjectInfo &proInfo)
{
	//项目唯一ID
	objProjectInfo.insert("uid", proInfo.uuid);
	//项目名称
	objProjectInfo.insert("project_name", proInfo.projectName);
	//视频路径
	objProjectInfo.insert("video_path", proInfo.videoPath);
	//重建模式				
	objProjectInfo.insert("mode", proInfo.mode);
	//事故发生时间
	objProjectInfo.insert("accident_time", proInfo.accidentTime);
	//事故地点
	objProjectInfo.insert("accident_location", proInfo.accidentLocation);
	//车辆号牌
	objProjectInfo.insert("license_plate_umber", proInfo.licensePlateNumber);
	//车辆品牌
	objProjectInfo.insert("vehicle_brand", proInfo.vehicleBrand);
	//车辆类型
	objProjectInfo.insert("vehicle_type", proInfo.vehicleType);
	//车辆识别代码VIN
	objProjectInfo.insert("vehicle_vin", proInfo.vehicleVIN);
	//参照物描述
	objProjectInfo.insert("reference_describe_from", proInfo.referenceDesFrom);
	objProjectInfo.insert("reference_describe_to", proInfo.referenceDesTo);
	//委托人
	objProjectInfo.insert("client", proInfo.client);
	//委托事项
	objProjectInfo.insert("commitments", proInfo.commitments);
	//委托日期
	objProjectInfo.insert("date_of_commission", proInfo.dateOfCommission);
	//鉴定日期
	objProjectInfo.insert("appraisal_date", proInfo.appraisalDate);
	//鉴定地点
	objProjectInfo.insert("appraisal_location", proInfo.appraisalLocation);
	//鉴定材料
	objProjectInfo.insert("appraisal_material", proInfo.appraisalMaterial);
	//适应标准
	QJsonArray standards;
	encodeAppraisalStandards(standards, proInfo.appraisalStandards);
	objProjectInfo.insert("appraisal_standard", standards);
	//鉴定人
	objProjectInfo.insert("appraiser1", proInfo.appraiser1.name);
	objProjectInfo.insert("appraiser_id1", proInfo.appraiser1.id);
	objProjectInfo.insert("appraiser2", proInfo.appraiser2.name);
	objProjectInfo.insert("appraiser_id2", proInfo.appraiser2.id);

	objProjectInfo.insert("fps", proInfo.fps);
	objProjectInfo.insert("v01", proInfo.v01);
	objProjectInfo.insert("v02", proInfo.v02);
	objProjectInfo.insert("v03", proInfo.v03);
	objProjectInfo.insert("v11", proInfo.v11);
	objProjectInfo.insert("v12", proInfo.v12);
	objProjectInfo.insert("v13", proInfo.v13);

	objProjectInfo.insert("head_index", proInfo.headIndex);
	objProjectInfo.insert("tail_index0", proInfo.tailIndex0);
	objProjectInfo.insert("tail_index1", proInfo.tailIndex1);
	objProjectInfo.insert("create_time", proInfo.createTime);

	//水印时间信息
	QJsonObject objOCR;
	encodeOCR(objOCR, proInfo.firstImage);
	objProjectInfo.insert("ocr", objOCR);


	QJsonArray objActualPoints;
	encodeActualPoints(objActualPoints, proInfo.actualPoints);
	objProjectInfo.insert("actual_points", objActualPoints);

	QJsonArray objFrameInfos;
	encodeFrameInfos(objFrameInfos, proInfo.frameInfos);
	objProjectInfo.insert("frame_infos", objFrameInfos);

	QJsonObject CameraCalibration;
	enCodeCameraCalibration(CameraCalibration, proInfo.dedistortion);
	objProjectInfo.insert("camera_calibration", CameraCalibration);

	QJsonArray objLinePoints;
	encodeLinePoints(objLinePoints, proInfo.imageLines);
	objProjectInfo.insert("line_points", objLinePoints);

	//添加日志
	objProjectInfo.insert("log", g_logBrowser->toPlainText());
	return 0;
}

int decodeProjectInfo(QJsonObject &objProjectInfo, VTProjectInfo &proInfo)
{
	//项目唯一ID
	if (objProjectInfo.contains("uid")) {
		auto value = objProjectInfo.value("uid");
		if (value.isString()) {
			proInfo.uuid = value.toVariant().toString();
		}
	}
	//项目名称
	if (objProjectInfo.contains("project_name")) {
		auto value = objProjectInfo.value("project_name");
		if (value.isString()) {
			proInfo.projectName = value.toVariant().toString();
		}
	}
	//视频路径
	if (objProjectInfo.contains("video_path")) {
		auto value = objProjectInfo.value("video_path");
		if (value.isString()) {
			proInfo.videoPath = value.toVariant().toString();
		}
	}
	//标定方式			
	if (objProjectInfo.contains("mode")) {
		auto value = objProjectInfo.value("mode");
		if (value.isDouble()) {
			proInfo.mode = (VTCalibration)value.toVariant().toInt();
		}
	}
	//事故发生时间
	if (objProjectInfo.contains("accident_time")) {
		auto value = objProjectInfo.value("accident_time");
		if (value.isString()) {
			proInfo.accidentTime = value.toVariant().toString();
		}
	}
	//事故地点
	if (objProjectInfo.contains("accident_location")) {
		auto value = objProjectInfo.value("accident_location");
		if (value.isString()) {
			proInfo.accidentLocation = value.toVariant().toString();
		}
	}
	//车辆号牌
	if (objProjectInfo.contains("license_plate_umber")) {
		auto value = objProjectInfo.value("license_plate_umber");
		if (value.isString()) {
			proInfo.licensePlateNumber = value.toVariant().toString();
		}
	}
	//车辆品牌
	if (objProjectInfo.contains("vehicle_brand")) {
		auto value = objProjectInfo.value("vehicle_brand");
		if (value.isString()) {
			proInfo.vehicleBrand = value.toVariant().toString();
		}
	}

	//车辆类型
	if (objProjectInfo.contains("vehicle_type")) {
		auto value = objProjectInfo.value("vehicle_type");
		if (value.isString()) {
			proInfo.vehicleType = value.toVariant().toString();
		}
	}
	//车辆识别代码VIN
	if (objProjectInfo.contains("vehicle_vin")) {
		auto value = objProjectInfo.value("vehicle_vin");
		if (value.isString()) {
			proInfo.vehicleVIN = value.toVariant().toString();
		}
	}
	//参照物描述
	if (objProjectInfo.contains("reference_describe_from")) {
		auto value = objProjectInfo.value("reference_describe_from");
		if (value.isString()) {
			proInfo.referenceDesFrom = value.toVariant().toString();
		}
	}
	if (objProjectInfo.contains("reference_describe_to")) {
		auto value = objProjectInfo.value("reference_describe_to");
		if (value.isString()) {
			proInfo.referenceDesTo = value.toVariant().toString();
		}
	}
	//委托人
	if (objProjectInfo.contains("client")) {
		auto value = objProjectInfo.value("client");
		if (value.isString()) {
			proInfo.client = value.toVariant().toString();
		}
	}
	//委托事项
	if (objProjectInfo.contains("commitments")) {
		auto value = objProjectInfo.value("commitments");
		if (value.isString()) {
			proInfo.commitments = value.toVariant().toString();
		}
	}
	//委托日期
	if (objProjectInfo.contains("date_of_commission")) {
		auto value = objProjectInfo.value("date_of_commission");
		if (value.isString()) {
			proInfo.dateOfCommission = value.toVariant().toString();
		}
	}
	//鉴定日期
	if (objProjectInfo.contains("appraisal_date")) {
		auto value = objProjectInfo.value("appraisal_date");
		if (value.isString()) {
			proInfo.appraisalDate = value.toVariant().toString();
		}
	}
	//鉴定地点
	if (objProjectInfo.contains("appraisal_location")) {
		auto value = objProjectInfo.value("appraisal_location");
		if (value.isString()) {
			proInfo.appraisalLocation = value.toVariant().toString();
		}
	}
	//鉴定材料
	if (objProjectInfo.contains("appraisal_material")) {
		auto value = objProjectInfo.value("appraisal_material");
		if (value.isString()) {
			proInfo.appraisalMaterial = value.toVariant().toString();
		}
	}
	//适应标准
	if (objProjectInfo.contains("appraisal_standard")) {
		auto value = objProjectInfo.value("appraisal_standard");
		if (value.isArray()) {
			auto standards = value.toArray();
			decodeAppraisalStandards(standards, proInfo.appraisalStandards);
		}
	}
	//鉴定人
	if (objProjectInfo.contains("appraiser1")) {
		auto value = objProjectInfo.value("appraiser1");
		if (value.isString()) {
			proInfo.appraiser1.name = value.toVariant().toString();
		}
	}
	if (objProjectInfo.contains("appraiser2")) {
		auto value = objProjectInfo.value("appraiser2");
		if (value.isString()) {
			proInfo.appraiser2.name = value.toVariant().toString();
		}
	}
	if (objProjectInfo.contains("appraiser_id1")) {
		auto value = objProjectInfo.value("appraiser_id1");
		if (value.isString()) {
			proInfo.appraiser1.id = value.toVariant().toString();
		}
	}
	if (objProjectInfo.contains("appraiser_id2")) {
		auto value = objProjectInfo.value("appraiser_id2");
		if (value.isString()) {
			proInfo.appraiser2.id = value.toVariant().toString();
		}
	}

	if (objProjectInfo.contains("fps")) {
		auto value = objProjectInfo.value("fps");
		if (value.isDouble()) {
			proInfo.fps = value.toVariant().toInt();
		}
	}

	if (objProjectInfo.contains("v01")) {
		auto value = objProjectInfo.value("v01");
		if (value.isDouble()) {
			proInfo.v01 = value.toVariant().toFloat();
		}
	}
	if (objProjectInfo.contains("v02")) {
		auto value = objProjectInfo.value("v02");
		if (value.isDouble()) {
			proInfo.v02 = value.toVariant().toFloat();
		}
	}
	if (objProjectInfo.contains("v03")) {
		auto value = objProjectInfo.value("v03");
		if (value.isDouble()) {
			proInfo.v03 = value.toVariant().toFloat();
		}
	}

	if (objProjectInfo.contains("v11")) {
		auto value = objProjectInfo.value("v11");
		if (value.isDouble()) {
			proInfo.v11 = value.toVariant().toFloat();
		}
	}

	if (objProjectInfo.contains("v12")) {
		auto value = objProjectInfo.value("v12");
		if (value.isDouble()) {
			proInfo.v12 = value.toVariant().toFloat();
		}
	}

	if (objProjectInfo.contains("v13")) {
		auto value = objProjectInfo.value("v13");
		if (value.isDouble()) {
			proInfo.v13 = value.toVariant().toFloat();
		}
	}

	if (objProjectInfo.contains("head_index")) {
		auto value = objProjectInfo.value("head_index");
		if (value.isDouble()) {
			proInfo.headIndex = value.toVariant().toInt();
		}
	}

	if (objProjectInfo.contains("tail_index0")) {
		auto value = objProjectInfo.value("tail_index0");
		if (value.isDouble()) {
			proInfo.tailIndex0 = value.toVariant().toInt();
		}
	}

	if (objProjectInfo.contains("tail_index1")) {
		auto value = objProjectInfo.value("tail_index1");
		if (value.isDouble()) {
			proInfo.tailIndex1 = value.toVariant().toInt();
		}
	}

	if (objProjectInfo.contains("create_time")) {
		auto value = objProjectInfo.value("create_time");
		if (value.isDouble()) {
			proInfo.createTime = value.toVariant().toInt();
		}
	}

	if (objProjectInfo.contains("actual_points")) {
		auto value = objProjectInfo.value("actual_points");
		if (value.isArray()) {
			auto objActualPoints = value.toArray();
			decodeActualPoints(objActualPoints, proInfo.actualPoints);
		}
	}

	if (objProjectInfo.contains("frame_infos")) {
		auto value = objProjectInfo.value("frame_infos");
		if (value.isArray()) {
			auto objFrameInfos = value.toArray();
			decodeFrameInfos(objFrameInfos, proInfo.frameInfos);
		}
	}

	if (objProjectInfo.contains("camera_calibration")) {
		auto value = objProjectInfo.value("camera_calibration");
		if (value.isObject()) {
			auto cameraCalibration = value.toObject();
			deCodeCameraCalibration(cameraCalibration, proInfo.dedistortion);
		}
	}

	//水印时间信息
	if (objProjectInfo.contains("ocr")) {
		auto value = objProjectInfo.value("ocr");
		if (value.isObject()) {
			auto obj = value.toObject();
			if (!proInfo.firstImage)
			{
				proInfo.firstImage = std::make_shared<VTDecodeImage>();
			}
			decodeOCR(obj, proInfo.firstImage);
		}
	}


	if (objProjectInfo.contains("line_points")) {
		auto value = objProjectInfo.value("line_points");
		if (value.isArray()) {
			auto objLinePoints = value.toArray();
			decodeLinePoints(objLinePoints, proInfo.imageLines);
		}
	}

	//日志信息写入到g_log
	if (objProjectInfo.contains("log")) {
		auto value = objProjectInfo.value("log");
		if (value.isString()) {
			auto logs = value.toString();
			decodeLogs(logs);
		}
	}
	return 0;
}

QString actualPointstoJsonString(QVector<QPointF> &actualPoints)
{
	QJsonObject objActualPoints;
	QJsonArray objArray;
	encodeActualPoints(objArray, actualPoints);
	objActualPoints.insert("actual_points", objActualPoints);
	QJsonDocument document;
	document.setObject(objActualPoints);

	auto byteArray = document.toJson(QJsonDocument::Compact);
	return byteArray;
}

QString actualPointtoJsonString(QPointF point)
{
	QJsonObject objActualPoint;
	encodeActualPoint(objActualPoint, point);
	QJsonDocument document;
	document.setObject(objActualPoint);

	QByteArray byteArray = document.toJson(QJsonDocument::Compact);
	return byteArray;
}

void encodeWaterTime(QJsonObject &obj, VTWaterMark &waterMark)
{
	obj.insert("text", waterMark.text);
	QJsonObject rc;
	encodeRect(rc, waterMark.rect);
	obj.insert("rect", rc);
}

void decodeWaterTime(QJsonObject &obj, VTWaterMark &waterMark)
{
	if (obj.contains("text")) {
		QJsonValue value = obj.value("text");
		if (value.isString()) {
			waterMark.text = value.toVariant().toString();
		}
	}

	if (obj.contains("rect")) {
		QJsonValue value = obj.value("rect");
		if (value.isObject()) {
			QJsonObject obj = value.toObject();
			decodeRect(obj, waterMark.rect);
		}
	}
}

void encodeWaterTimes(QJsonArray &objArray, QVector<VTWaterMark> &waterMarks)
{
	for (auto n = 0; n < waterMarks.size(); n++) {
		QJsonObject obj;
		encodeWaterTime(obj, waterMarks[n]);
		objArray.append(obj);
	}
}

void decodeWaterTimes(QJsonArray &objArray, QVector<VTWaterMark> &waterMarks)
{
	auto size = objArray.size();
	for (auto i = 0; i < size; ++i) {
		auto value = objArray.at(i);
		if (value.isObject()) {
			auto obj = value.toObject();
			VTWaterMark waterMark;
			decodeWaterTime(obj, waterMark);
			waterMarks.push_back(waterMark);
		}
	}
}

void encodeOCR(QJsonObject &obj, std::shared_ptr<VTDecodeImage> decodeImage)
{
	if (!decodeImage)
		return;

	QJsonObject wmObj;
	encodeWaterTime(wmObj, decodeImage->waterMarkHMS);
	obj.insert("hhmmss", wmObj);

	QJsonArray arry;
	encodeWaterTimes(arry, decodeImage->waterMarks);
	obj.insert("others", arry);
}

void decodeOCR(QJsonObject &obj, std::shared_ptr<VTDecodeImage> decodeImage)
{
	if (!decodeImage)
		return;

	if (obj.contains("hhmmss")) {
		auto value = obj.value("hhmmss");
		if (value.isObject()) {
			auto objOthers = value.toObject();
			decodeWaterTime(objOthers, decodeImage->waterMarkHMS);
		}
	}

	if (obj.contains("others")) {
		auto value = obj.value("others");
		if (value.isArray()) {
			auto objOthers = value.toArray();
			decodeWaterTimes(objOthers, decodeImage->waterMarks);
		}
	}
}
int decodeLogs(QString logs) {
	auto list = logs.split("\n");
	for (auto&& item : list) {
		g_logBrowser->append(item);
	}
	return 0;
}