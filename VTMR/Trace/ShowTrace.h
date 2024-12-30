#pragma once

#include <QWidget>
#include <QTabWidget>
#include "Global.h"
#include <map>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <qwt_plot_zoomer.h>
#include <QMutex>

using std::map;

class Zoomer;
class QwtPlot;
/*
*轨迹显示窗口
*/
class CShowTrace : public QWidget
{
	Q_OBJECT

public:
	CShowTrace(QWidget *parent = Q_NULLPTR);
	~CShowTrace();
	//画运行估计点
	void drawMPoints(QVector<QPointF> &p0s, QVector<QPointF> &pxs, QVector<int64_t> &ptss);
	//像素轨迹
	void drawMPixPoints(int width, int height, QVector<QPointF> &p0s, QVector<QPointF> &pxs, QVector<int64_t> &ptss);
	//画速度点
	void drawVPoints();
protected:
	virtual void resizeEvent(QResizeEvent* event);
private:
	map<cv::Point2d, QString> m_mapMarks;
	QTabWidget* m_tabWidget = nullptr;

	//
	QwtPlot *m_plotM = nullptr;
	Zoomer* m_zoomerM = nullptr;
	QwtPlotCurve *m_curveM0 = nullptr;
	QwtPlotCurve *m_curveMX = nullptr;
	//
	QwtPlot *m_plotMPix = nullptr;
	Zoomer* m_zoomerMPix = nullptr;
	QwtPlotCurve *m_curveMPix0 = nullptr;
	QwtPlotCurve *m_curveMPixX = nullptr;
	//
	QwtPlotCurve *m_curveV = nullptr;
	QwtPlot *m_plotV = nullptr;
	Zoomer* m_zoomerV = nullptr;
};
