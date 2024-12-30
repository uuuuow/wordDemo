#include "AppraisalStandardDialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include "SQLManager.h"
#include "Global.h"

QVector<VTAppraisalStandard> CAppraisalStandardDialog::m_appraiserStandardTable;
CAppraisalStandardDialog::CAppraisalStandardDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowTitle("鉴定标准");
	connect(this, SIGNAL(), this, SLOT());
	connect(ui.btnAdd, &QPushButton::clicked,
		[=]() {
			if (ui.LEId->text().isEmpty()) {
				VTMessageBoxOk(QString("标准号不能为空"), windowFlags(), QMessageBox::Information, this);
				return;
			}else if (ui.LEName->text().isEmpty()) {
				VTMessageBoxOk(QString("标准全称不能为空"), windowFlags(), QMessageBox::Information, this);
				return;
			}
			QDialog::accept();
		});
	connect(ui.btnCancel, &QPushButton::clicked,
		[=]() {
			QDialog::reject();
		});
}

CAppraisalStandardDialog::~CAppraisalStandardDialog()
{}

VTAppraisalStandard CAppraisalStandardDialog::getStantard()
{
	VTAppraisalStandard standard;
	standard.id = ui.LEId->text();
	standard.name = ui.LEName->text();
	return standard;
}

void CAppraisalStandardDialog::loadSql()
{
	QString regCode;
	QSqlQuery query;
	QString sql = QString("SELECT * FROM  main.%1").arg(SQL_APPRAISAL_STANDARD_TABLE);
	query.exec(sql);
	if (!query.exec())
		qDebug() << QString("%1 执行失败：%2").arg(sql).arg(query.lastError().text());
	else {
		VTAppraisalStandard standard;
		while (query.next()) {
			standard.id = query.value(SQL_APPRAISAL_STANDARD_ID).toString();
			standard.name = query.value(SQL_APPRAISAL_STANDARD_NAME).toString();
			m_appraiserStandardTable.push_back(standard);
		}
	}

	if (m_appraiserStandardTable.isEmpty()) {
		for (auto& standard : g_appraisalStandardTable) {
			m_appraiserStandardTable.push_back(standard);
			insertSql(standard);
		}
	}
}

void CAppraisalStandardDialog::insertSql(VTAppraisalStandard& info)
{
	//创建表格
	QSqlQuery query;
	QString sql;
	sql = QString("CREATE TABLE IF NOT EXISTS main.%1(%2 text PRIMARY KEY, %3 text)")
		.arg(SQL_APPRAISAL_STANDARD_TABLE)
		.arg(SQL_APPRAISAL_STANDARD_ID).arg(SQL_APPRAISAL_STANDARD_NAME);
	if (!query.exec(sql))
		qDebug() << QString("%1 执行失败：%2").arg(sql).arg(query.lastError().text());
	else
		CSQLManager::commit(query);

	//插入数据
	sql = QString("INSERT INTO main.%1(%2, %3) VALUES('%4', '%5')")
		.arg(SQL_APPRAISAL_STANDARD_TABLE)
		.arg(SQL_APPRAISAL_STANDARD_ID).arg(SQL_APPRAISAL_STANDARD_NAME)
		.arg(info.id).arg(info.name);
	if (!query.exec(sql))
		qDebug() << QString("%1 执行失败：%2").arg(sql).arg(query.lastError().text());
	else
		CSQLManager::commit(query);
}

void CAppraisalStandardDialog::deleteSql(VTAppraisalStandard& info)
{
	QSqlQuery query;
	QString sql = QString("DELETE FROM main.%1 WHERE %2 = '%3'")
		.arg(SQL_APPRAISAL_STANDARD_TABLE)
		.arg(SQL_APPRAISAL_STANDARD_ID).arg(info.id);
	if (!query.exec(sql))
		qDebug() << QString("%1 执行失败：%2").arg(sql).arg(query.lastError().text());
	else
		CSQLManager::commit(query);
}