#include "SQLManager.h"
#include "Global.h"
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "Global.h"

QSqlDatabase CSQLManager::database;
CSQLManager::CSQLManager(QObject *parent)
	: QObject(parent)
{
}

CSQLManager::~CSQLManager()
{
}

bool CSQLManager::initDataBase()
{
	database = QSqlDatabase::addDatabase("QSQLITE");
	auto db = g_appPath + VTMR_DB_PATH;
	database.setDatabaseName(db);
	database.setUserName("vtmr");
	database.setPassword("evvtmr2023");
	database.setConnectOptions("QSQLITE_ENABLE_REGEXP=1;QSQLITE_PASSPHRASE=evvtmr2023");
	if (!database.open()) {
		qDebug() << "Error: Failed to connect database." << database.lastError();
		return false;
	}
	return true;
}

bool CSQLManager::closDataBase()
{
	database.close();
	return true;
}

bool CSQLManager::commit(QSqlQuery& qsQuery)
{
	if (qsQuery.isActive()) {
		database.commit();
		return true;
	}
	else {
		qDebug() << "commit failed." << qsQuery.lastError();
		database.rollback();
		return false;
	}
}

void ClinetEditHistoryInsertSql(QString text)
{
	//创建表格
	QSqlQuery query;
	QString sql;
	sql = QString("CREATE TABLE IF NOT EXISTS main.%1(%2 text)")
		.arg(SQL_CLIENT_EDIT_HISTORY_TABLE)
		.arg(SQL_CLIENT_EDIT_TEXT);
	if (!query.exec(sql))
		qDebug() << QString("%1 执行失败：%2").arg(sql).arg(query.lastError().text());
	else
		CSQLManager::commit(query);

	auto find = false;
	for (auto n = 0; n < g_clientHistory.size(); ++n) {
		if (g_clientHistory[n] == text) {
			find = true;
			break;
		}
	}

	if (find)
		return;
	//插入数据
	sql = QString("INSERT INTO main.%1(%2) VALUES('%3')")
		.arg(SQL_CLIENT_EDIT_HISTORY_TABLE)
		.arg(SQL_CLIENT_EDIT_TEXT)
		.arg(text);
	if (!query.exec(sql))
		qDebug() << QString("%1 执行失败：%2").arg(sql).arg(query.lastError().text());
	else
		CSQLManager::commit(query);
	
	g_clientHistory.push_back(text);
}

void ClinetEditHistorySelectSql()
{
	QSqlQuery query;
	QString sql = QString("SELECT * FROM  main.%1").arg(SQL_CLIENT_EDIT_HISTORY_TABLE);
	query.exec(sql);
	if (!query.exec())
		qDebug() << QString("%1 执行失败：%2").arg(sql).arg(query.lastError().text());
	else {
		while (query.next()) {
			auto text = query.value(SQL_CLIENT_EDIT_TEXT).toString();
			auto find = false;
			for (int n = 0; n < g_clientHistory.size(); ++n) {
				if (g_clientHistory[n] == text) {
					find = true;
					break;
				}
			}

			if (!find)
				g_clientHistory.push_back(text);
		}
	}
}
