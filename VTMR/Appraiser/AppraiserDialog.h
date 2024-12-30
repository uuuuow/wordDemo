#pragma once

#include <QDialog>
#include "ui_AppraiserDialog.h"
#include <QImage>
#include <QMap>
#include "Utils.h"

class CAppraiserDialog : public QDialog
{
	Q_OBJECT

public:
	CAppraiserDialog(QWidget *parent = Q_NULLPTR);
	~CAppraiserDialog();
	static void insertSql(VTAppraiserInfo &info);
	static void deleteSql(QString appraiserId);
	static void updateSql(VTAppraiserInfo &info);
	void addTable(VTAppraiserInfo &info);
	void deleteFile(VTAppraiserInfo &info);

	static void loadAppraiser();
	static QMap<QString, VTAppraiserInfo> &getInfo() { return m_idAppraiserInfo; }
public slots:
	void onOK();
	void onCancel();
	void onAdd();
	void onDelete(int row);
private:
	Ui::AppraiserDialog ui;

	QMap<int, VTAppraiserInfo> m_appraiserInfo;
	QMap<int, QPushButton *> m_previewBtn;
	static QMap<QString, VTAppraiserInfo> m_idAppraiserInfo;
};
