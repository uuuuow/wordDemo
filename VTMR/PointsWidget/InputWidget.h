#pragma once

#include <QWidget>
#include "ui_InputWidget.h"
#include "Utils.h"
class CPointItem;

class CInputWidget : public QWidget
{
	Q_OBJECT

public:
	CInputWidget(QWidget *parent = Q_NULLPTR);
	~CInputWidget();
	
	QVector<QPointF> &getActualPoints() { return m_actualPoints; };
	void setActualPoints(QVector<QPointF> &points) { m_actualPoints = points; };

	QListWidget *getListWidget() { return ui.listWidget; };
	void removeAllWidgetItem() {
		ui.listWidget->clear();
		for (int n = 0; n < 5; n++)
			m_itemList[n] = nullptr;
		for (int n = 0; n < 5; n++)
			m_widgetItemList[n] = nullptr;
	};

	void addItem(int index);

public:
	virtual void initActual(QVector<QPointF> &points) {};

protected:
	void paintEvent(QPaintEvent *e);

signals:
	void sigValueFChanged(int index, QPointF point);

public slots:
	void onValueFChanged(int index, QPointF point);

private:
	Ui::InputWidget ui;

protected:
	CPointItem * m_itemList[5] = {0};
	QListWidgetItem *m_widgetItemList[5] = { 0 };
	QVector<QPointF> m_actualPoints = {
		QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
		QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
		QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
		QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT),
		QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT)
	};
};
