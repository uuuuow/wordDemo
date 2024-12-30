#pragma once

#include <QDialog>
#include "ui_AppraisalStandardDialog.h"
#include "Utils.h"

class CAppraisalStandardDialog : public QDialog
{
	Q_OBJECT

public:
	CAppraisalStandardDialog(QWidget *parent = nullptr);
	~CAppraisalStandardDialog();
	VTAppraisalStandard getStantard();

	static void loadSql();
	static void insertSql(VTAppraisalStandard& info);
	static void deleteSql(VTAppraisalStandard& info);
	static QVector<VTAppraisalStandard> m_appraiserStandardTable;

private:
	Ui::AppraiserStandard ui;
};
