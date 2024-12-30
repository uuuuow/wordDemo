#pragma once

#include <QWidget>
#include "ui_TraceWidget.h"
#include <QtCharts/QtCharts>

class QTabWidget;


class CTraceWidget : public QWidget
{
	Q_OBJECT

public:
	CTraceWidget(QWidget *parent = Q_NULLPTR);
	~CTraceWidget();

	void initViewMove();

private:
	Ui::CTraceWidget ui;

	QTabWidget* m_tabWidget = nullptr;
	QChart *m_moveChart = nullptr;
	QChartView *m_chartView = nullptr;
};
