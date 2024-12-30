#include "AppraiserDialog.h"
#include <QFileDialog>
#include <QSqlQuery>
#include <QSqlError>
#include "Utils.h"
#include "Global.h"
#include <QDebug>
#include <QFileIconProvider>
#include "AddAppraiserDialog.h"
#include "PhotoDialog.h"
#include "SQLManager.h"

QMap<QString, VTAppraiserInfo> CAppraiserDialog::m_idAppraiserInfo;

CAppraiserDialog::CAppraiserDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowModality(Qt::WindowModal);
	setFixedSize(QSize(500, 320));


	connect(ui.BtnOk, SIGNAL(clicked()), this, SLOT(onOK()));
	connect(ui.BtnCancel, SIGNAL(clicked()), this, SLOT(onCancel()));
	connect(ui.BtnAdd, SIGNAL(clicked()), this, SLOT(onAdd()));
	connect(ui.BtnDel, SIGNAL(clicked()), ui.widget, SLOT(onDelete()));
	connect(ui.widget, SIGNAL(sigDeleteRow(int)), this, SLOT(onDelete(int)));


	CAppraiserDialog::loadAppraiser();
	for (auto &info : m_idAppraiserInfo)
		addTable(info);
}

CAppraiserDialog::~CAppraiserDialog()
{
}

void CAppraiserDialog::loadAppraiser()
{
	m_idAppraiserInfo.clear();

	QSqlQuery query;
	QString sql = QString("SELECT * FROM  main.%1").arg(SQL_APPRAISER_INFO_TABLE);
	query.exec(sql);
	if (!query.exec())
		qDebug() << QString("%1 执行失败：%2").arg(sql).arg(query.lastError().text());
	else {
		QVector<QString> keys;
		while (query.next()) {
			VTAppraiserInfo info;
			info.id = query.value(SQL_APPRAISER_INFO_ID).toString();
			info.name = query.value(SQL_APPRAISER_INFO_NAME).toString();
			info.photoPath = query.value(SQL_APPRAISER_INFO_PHOTO_PATH).toString();
			info.photoPathTmp = query.value(SQL_APPRAISER_INFO_PHOTO_PATH_TMP).toString();
			QFileInfo file(info.photoPath);
			if (!file.exists()) {
				qWarning() << info.photoPath << "photo is not exist,delete Id" << info.id << " appraiser" << info.name << "";
				keys.push_back(info.id);
				continue;
			}
			
			m_idAppraiserInfo.insert(info.id, info);
			qDebug() << QString("加载 appraiser:%1 appraiserID:%2 photoPath:%3").arg(info.name).arg(info.id).arg(info.photoPath);
		}

		for (auto key : keys)
			deleteSql(key);
	}
}

void CAppraiserDialog::onOK()
{
	QDir dir;
	QString path;
	QString appraiserId;
	QString photoPath;

	path = g_appPath + APPRAISER_PATH;
	if (!dir.exists(path))
		dir.mkpath(path);
	
	loadAppraiser();

	for (auto &info : m_appraiserInfo) {
		appraiserId = info.id;
		photoPath = info.photoPath;
		auto iter = m_idAppraiserInfo.find(appraiserId);
		if (iter == m_idAppraiserInfo.end()) {
			auto fileInfo = QFileInfo(photoPath);
			auto fileSuffix = fileInfo.suffix();

			auto newPhotoPath = QString("%1/%2.%3")
				.arg(path)
				.arg(appraiserId)
				.arg(fileSuffix);

			auto newPhotoPathTmp = QString("%1/%2_tmp.png")
				.arg(path)
				.arg(appraiserId);
			if (!QFile::copy(photoPath, newPhotoPath))
				qDebug() << QString("鉴定人%1 证件号%2 拷贝证件照从%3到%4失败").arg(info.name).arg(appraiserId).arg(photoPath).arg(newPhotoPath);
			else {
				QImage image;
				image.load(newPhotoPath);
				auto w = image.width();
				auto h = image.height();
				auto tmpW = 200;
				auto tmpH = w == 0 ? 1 : h * tmpW / w;

				auto tmpImage = image.scaled(tmpW, tmpH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
				if (!tmpImage.save(newPhotoPathTmp)) {
					qDebug() << QString("保存缩略图失败，鉴定人%1 证件号%2 拷贝证件照从%3到%4 src w=%5 src h=%6 temp w=%7 temp h=%8")
						.arg(info.name).arg(appraiserId).arg(photoPath).arg(newPhotoPath)
						.arg(w).arg(h).arg(tmpW).arg(tmpH);
				}
			}
			info.photoPath = newPhotoPath;
			info.photoPathTmp = newPhotoPathTmp;
			insertSql(info);
		}
		else {
			auto change = false;
			if (info.name != iter.value().name)
				change = true;
			if (info.photoPath != iter.value().photoPath) {
				auto fileInfo = QFileInfo(info.photoPath);
				auto fileSuffix = fileInfo.suffix();

				auto newPhotoPath = QString("%1/%2.%3")
					.arg(path)
					.arg(appraiserId)
					.arg(fileSuffix);

				auto newPhotoPathTmp = QString("%1/%2_tmp.png")
					.arg(path)
					.arg(appraiserId);

				if (!QFile::copy(info.photoPath, newPhotoPath))
					qDebug() << QString("鉴定人%1 证件号%2 拷贝证件照从%3到%4失败").arg(info.name).arg(appraiserId).arg(photoPath).arg(newPhotoPath);
				else {
					QImage image;
					image.load(newPhotoPath);
					auto w = image.width();
					auto h = image.height();
					auto tmpW = 200;
					auto tmpH = h * tmpW / w;
					auto tmpImage = image.scaled(tmpW, tmpH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
					if (!tmpImage.save(newPhotoPathTmp))
					{
						qDebug() << QString("保存缩略图失败，鉴定人%1 证件号%2 拷贝证件照从%3到%4 src w=%5 src h=%6 temp w=%7 temp h=%8")
							.arg(info.name).arg(appraiserId).arg(photoPath).arg(newPhotoPath)
							.arg(w).arg(h).arg(tmpW).arg(tmpH);
					}
				}
				info.photoPath = newPhotoPath;
				info.photoPathTmp = newPhotoPathTmp;
				change = true;
			}
			if (change)
				updateSql(info);
		}
	}

	for (auto &appr : m_idAppraiserInfo) {
		auto find = false;
		for (auto & info : m_appraiserInfo) {
			if (info.id == appr.id) {
				find = true;
				break;
			}
		}
		if (!find) {
			deleteFile(appr);
			deleteSql(appr.id);
		}
	}
	loadAppraiser();

	QDialog::accept();
}

void CAppraiserDialog::onCancel()
{
	QDialog::accept();
}

void CAppraiserDialog::insertSql(VTAppraiserInfo &info)
{
	//创建表格
	QSqlQuery query;
	QString sql;
	sql = QString("CREATE TABLE IF NOT EXISTS main.%1(%2 text PRIMARY KEY, %3 text, %4 text, %5 text)")
		.arg(SQL_APPRAISER_INFO_TABLE).arg(SQL_APPRAISER_INFO_ID).arg(SQL_APPRAISER_INFO_NAME).arg(SQL_APPRAISER_INFO_PHOTO_PATH).arg(SQL_APPRAISER_INFO_PHOTO_PATH_TMP);
	if (!query.exec(sql))
		qDebug() << QString("%1 执行失败：%2").arg(sql).arg(query.lastError().text());
	else
		CSQLManager::commit(query);

	//插入数据
	sql = QString("INSERT INTO main.%1(%2, %3, %4, %5) VALUES('%6', '%7', '%8', '%9')")
		.arg(SQL_APPRAISER_INFO_TABLE)
		.arg(SQL_APPRAISER_INFO_ID).arg(SQL_APPRAISER_INFO_NAME).arg(SQL_APPRAISER_INFO_PHOTO_PATH).arg(SQL_APPRAISER_INFO_PHOTO_PATH_TMP)
		.arg(info.id).arg(info.name).arg(info.photoPath).arg(info.photoPathTmp);
	if (!query.exec(sql))
		qDebug() << QString("%1 执行失败：%2").arg(sql).arg(query.lastError().text());
	else
		CSQLManager::commit(query);
}

void CAppraiserDialog::deleteFile(VTAppraiserInfo &info)
{
	QFile filePhoto(info.photoPath);
	if (filePhoto.exists())
		filePhoto.remove();

	QFile filePhotoTmp(info.photoPathTmp);
	if (filePhotoTmp.exists())
		filePhotoTmp.remove();
}

void CAppraiserDialog::deleteSql(QString appraiserId)
{
	QSqlQuery query;
	QString sql = QString("DELETE FROM main.%1 WHERE %2 = '%3'").arg(SQL_APPRAISER_INFO_TABLE).arg(SQL_APPRAISER_INFO_ID).arg(appraiserId);
	if (!query.exec(sql))
		qDebug() << QString("%1 执行失败：%2").arg(sql).arg(query.lastError().text());
	else
		CSQLManager::commit(query);
}

void CAppraiserDialog::updateSql(VTAppraiserInfo &info)
{
	QSqlQuery query;
	QString sql = QString("UPDATE main.%1 SET %2 = '%3', %4 = '%5', %6 = '%7' WHERE %8 = '%9'")
		.arg(SQL_APPRAISER_INFO_TABLE)
		.arg(SQL_APPRAISER_INFO_NAME).arg(info.name)
		.arg(SQL_APPRAISER_INFO_PHOTO_PATH).arg(info.photoPath)
		.arg(SQL_APPRAISER_INFO_PHOTO_PATH_TMP).arg(info.photoPathTmp)
		.arg(SQL_APPRAISER_INFO_ID).arg(info.id);
	if (!query.exec(sql))
		qDebug() << QString("%1 执行失败：%2").arg(sql).arg(query.lastError().text());
	else
		CSQLManager::commit(query);
}

void CAppraiserDialog::addTable(VTAppraiserInfo &info)
{
	auto btn = new QPushButton(QString("预览"), this);
	connect(btn, &QPushButton::clicked,
		[=]() {
		auto row = -1;
		auto iter = m_previewBtn.begin();
		for (; iter != m_previewBtn.end(); iter++) {
			if (iter.value() == btn) {
				row = iter.key();
				break;
			}
		}
		if (row != -1) {
			auto path = m_appraiserInfo[row].photoPath;
			if (path.isEmpty())
				return;
			auto photoDialog = new CPhotoDialog("执业照", path, this);
			photoDialog->exec();
		}
	});

	auto row = ui.widget->addRowValue(30, QStringList() << info.name << info.id << "");
	ui.widget->setCellWidget(row, 2, btn);
	m_appraiserInfo.insert(row, info);
	m_previewBtn.insert(row, btn);
}

void CAppraiserDialog::onAdd()
{
	VTAppraiserInfo info;
	auto appraiserDialog = new CAddAppraiserDialog(this);
	if (QDialog::Accepted != appraiserDialog->exec())
		return;
	info = appraiserDialog->getInfo();
	if (info.name.isEmpty() || info.id.isEmpty() || info.photoPath.isEmpty())
		return;

	for (auto appr : m_appraiserInfo) {
		if (appr.id == info.id) {
			VTMessageBoxOk(QString("证件号 %1 已存在，不可重复添加！").arg(info.id), windowFlags(), QMessageBox::Information, this);
			return;
		}
	}
	
	addTable(info);
}

void CAppraiserDialog::onDelete(int row)
{
	auto iterBtn = m_previewBtn.find(row);
	if (iterBtn != m_previewBtn.end())
		m_previewBtn.erase(iterBtn);

	auto iterInfo = m_appraiserInfo.find(row);
	if (iterInfo != m_appraiserInfo.end())
		m_appraiserInfo.erase(iterInfo);
}
