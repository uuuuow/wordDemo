#pragma once

#include <QObject>
#include <QSqlDatabase>

/********************************************/
//APP信息表
#define SQL_APP_INFO_TABLE "app_info"
//注册码
#define SQL_APP_INFO_REGISTER_CODE "register_code"
/********************************************/

/********************************************/
//鉴定标准表
#define SQL_APPRAISAL_STANDARD_TABLE "appraisal_standard"
//标准号
#define SQL_APPRAISAL_STANDARD_ID "id"
//标准全称
#define SQL_APPRAISAL_STANDARD_NAME "name"
/********************************************/

/********************************************/
//鉴定人信息表
#define SQL_APPRAISER_INFO_TABLE "appraiser_info"
//鉴定人证件号
#define SQL_APPRAISER_INFO_ID "id"
//鉴定人名字
#define SQL_APPRAISER_INFO_NAME "name"
//鉴定人证件照路径
#define SQL_APPRAISER_INFO_PHOTO_PATH "photo_path"
//鉴定人证件照缩略图路径
#define SQL_APPRAISER_INFO_PHOTO_PATH_TMP "photo_path_tmp"
/********************************************/

/********************************************/
//委托人输入框历史内容表
#define SQL_CLIENT_EDIT_HISTORY_TABLE "client_edit_history"
//记录
#define SQL_CLIENT_EDIT_TEXT "edit_text"
/********************************************/

class CSQLManager : public QObject
{
	Q_OBJECT

public:
	CSQLManager(QObject *parent);
	~CSQLManager();

public:
	static bool initDataBase();
	static bool closDataBase();
	static bool commit(QSqlQuery& qsQuery);
private:
	static QSqlDatabase database;
};

extern void ClinetEditHistoryInsertSql(QString text);
extern void ClinetEditHistorySelectSql();

