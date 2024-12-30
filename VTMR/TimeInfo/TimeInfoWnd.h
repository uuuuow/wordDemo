#pragma once

#include <QDialog>
#include "ui_TimeInfoWnd.h"

QT_BEGIN_NAMESPACE
class QSortFilterProxyModel;
class QTreeWidgetItem;
QT_END_NAMESPACE

class CTimeInfoWnd : public QDialog
{
	Q_OBJECT

public:
	CTimeInfoWnd(QWidget *parent = Q_NULLPTR);
	~CTimeInfoWnd();

	void onOCR();
	void onPTS();
	void onFPS();
	void onMedia();
	void onPoTrace();
private:
	// 截取的视频的开始时间和结束时间
	QVector<QString> vec;
	Ui::CTimeInfoWnd ui;
	QSortFilterProxyModel *m_modelPTS;
	QSortFilterProxyModel* m_modelWaterTime;
	QSortFilterProxyModel *m_modelMedia;
};
