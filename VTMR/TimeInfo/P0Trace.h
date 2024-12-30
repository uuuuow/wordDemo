#pragma once

#include <QChartView>
#include <QWheelEvent>

class CP0Trace  : public QChartView
{
	Q_OBJECT

public:
	CP0Trace(QWidget *parent);
	~CP0Trace();
	void setValue(QVector<QPointF>& points, QVector<int64_t>& ptss, int w, int h, bool scale = true);
protected:
	void wheelEvent(QWheelEvent* ev) override;
private:
	QVector<QPointF> m_points;
	QVector<int64_t> m_ptss;
	bool m_scale = true;
	int m_w = 1920;
	int m_h = 1080;
};
