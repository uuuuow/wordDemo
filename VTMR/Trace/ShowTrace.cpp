#include "ShowTrace.h"
#include <QDEBUG>
#include <QMouseEvent>
#include "opencv2/opencv.hpp"
#include "PointsManage.h"
#include "qwt_text.h"

class Zoomer : public QwtPlotZoomer
{
public:
	Zoomer(int nFlag, QWidget *canvas) :
		QwtPlotZoomer(canvas),m_nFlag(nFlag)
	{
		setRubberBandPen(QColor(Qt::darkGreen));
		setTrackerMode(QwtPlotPicker::AlwaysOn);
	}

protected:
	virtual QwtText trackerTextF(const QPointF &pos) const
	{
		QString s = "";

		if (0 == m_nFlag)
		{
			int nSizeX = pointsX.size();

			for (int n = 0; n < nSizeX; n++)
			{
				QString strPX = "P1";
				switch (n)
				{
				case 0:
					strPX = "P1";
					break;
				case 1:
					strPX = "P1";
					break;
				case 2:
					strPX = "P2";
					break;
				case 3:
					strPX = "P3";
					break;
				case 4:
					strPX = "P4";
					break;
				default:
					strPX = "P1";
					break;
				}

				float fx = abs(pointsX[n].x() - pos.x());
				float fy = abs(pointsX[n].y() - pos.y());
				if (0 <= fx && fx <= 0.5 && 0 <= fy && fy <= 0.5)
				{
					s = QString("标定点%1(%2,%3)").arg(strPX).arg(pointsX[n].x()).arg(pointsX[n].y());
				}

			}

			int size = points.size();
			for (int n = 0; n < size; n++)
			{
				float fx = abs(points[n].x() - pos.x());
				float fy = abs(points[n].y() - pos.y());
				if (0 <= fx && fx <= 0.5 && 0 <= fy && fy <= 0.5)
				{
					s = QString("PTS:%1 (%2,%3)").arg(ptss[n]).arg(points[n].x()).arg(points[n].y());
				}

			}
		}
		else
		{
			int size = pointsV.size();
			for (int n = 0; n < size; n++)
			{
				float fx = abs(pointsV[n].x() - pos.x());
				float fy = abs(pointsV[n].y() - pos.y());
				if (0 <= fx && fx <= 0.5 && 0 <= fy && fy <= 0.5)
				{
					s = pointText[n];
				}

			}
		}
		

		QwtText text(s);
		text.setColor(Qt::white);

		QColor c = rubberBandPen().color();
		text.setBorderPen(QPen(c));
		text.setBorderRadius(6);
		c.setAlpha(170);
		text.setBackgroundBrush(c);

		return text;
	}

public:
	QPolygonF points;//P0待测点
	QPolygonF pointsX;//标定点

	QPolygonF pointsV;//P0待测点
	QMap<int, QString> pointText;//显示内容
	int m_nFlag = 0;//0--行驶轨迹，1-速度
	QVector<int64_t> ptss;
};

class CPlotWidget : public QwtPlot
{
public:
	CPlotWidget(QWidget *parent)
		:QwtPlot(parent)
	{

	}
	~CPlotWidget()
	{

	}
protected:
	void mousePressEvent(QMouseEvent *event)
	{
	}
};

CShowTrace::CShowTrace(QWidget *parent)
	: QWidget(parent)
{
	setMouseTracking(true);
	resize(QSize(500, 500));

	m_tabWidget = new QTabWidget(this);
	m_tabWidget->setObjectName(QStringLiteral("tabWidget"));
	m_tabWidget->setGeometry(QRect(0, 0, width(), height()));

	m_plotM = new CPlotWidget(m_tabWidget);
	m_zoomerM = new Zoomer(0, m_plotM->canvas());
	m_plotM->setMouseTracking(true);
	m_plotM->setAxisTitle(QwtPlot::xBottom, QString("X(m)"));
	m_plotM->setAxisTitle(QwtPlot::yLeft, QString("Y(m)"));
	m_plotM->setObjectName(QStringLiteral("tabM"));
	m_plotM->setCanvasBackground(Qt::white);
	m_plotM->setAxisMaxMajor(QwtPlot::xBottom, 10);
	m_plotM->setAxisMaxMajor(QwtPlot::yLeft, 10);
	m_plotM->insertLegend(new QwtLegend());
	QwtPlotGrid *grid = new QwtPlotGrid();
	grid->attach(m_plotM);
	m_curveM0 = new QwtPlotCurve();
	m_curveM0->setTitle("P0");
	//m_curveM0->setCurveAttribute(QwtPlotCurve::Fitted, false);
	m_curveM0->setPen(Qt::blue, 4);
	m_curveM0->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	QwtSymbol *symbolM0 = new QwtSymbol(QwtSymbol::Ellipse,
		QBrush(Qt::yellow), QPen(Qt::red, 2), QSize(8, 8));
	m_curveM0->setSymbol(symbolM0);
	m_curveM0->attach(m_plotM);

	m_curveMX = new QwtPlotCurve();
	m_curveMX->setTitle(QString("标定点"));
	m_curveMX->setPen(Qt::green, 4);
	m_curveMX->setStyle(QwtPlotCurve::Dots);
	m_curveMX->setRenderHint(QwtPlotItem::RenderAntialiased, false);
	QwtSymbol *symbolMX = new QwtSymbol(QwtSymbol::Ellipse,
		QBrush(Qt::green), QPen(Qt::red, 1), QSize(8, 8));
	m_curveMX->setSymbol(symbolMX);
	m_curveMX->attach(m_plotM);
	m_tabWidget->addTab(m_plotM, QString("行驶轨迹"));
	//////////////////////////////////////////////////////////////////////
	m_plotMPix = new CPlotWidget(m_tabWidget);
	m_zoomerMPix = new Zoomer(0, m_plotMPix->canvas());
	m_plotMPix->setMouseTracking(true);
	m_plotMPix->setAxisTitle(QwtPlot::xBottom, QString("X(像素)"));
	m_plotMPix->setAxisTitle(QwtPlot::yLeft, QString("Y(像素)"));
	m_plotMPix->setObjectName(QStringLiteral("tabMPix"));
	m_plotMPix->setCanvasBackground(Qt::white);
	m_plotMPix->insertLegend(new QwtLegend());
	QwtPlotGrid *gridPix = new QwtPlotGrid();
	gridPix->attach(m_plotMPix);
	m_curveMPix0 = new QwtPlotCurve();
	m_curveMPix0->setTitle("P0");
	m_curveMPix0->setPen(Qt::blue, 4);
	m_curveMPix0->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	QwtSymbol *symbolMPix0 = new QwtSymbol(QwtSymbol::Ellipse,
		QBrush(Qt::yellow), QPen(Qt::red, 2), QSize(8, 8));
	m_curveMPix0->setSymbol(symbolMPix0);
	m_curveMPix0->attach(m_plotMPix);

	m_curveMPixX = new QwtPlotCurve();
	m_curveMPixX->setTitle(QString("标定点"));
	m_curveMPixX->setPen(Qt::green, 4);
	m_curveMPixX->setStyle(QwtPlotCurve::Dots);
	m_curveMPixX->setRenderHint(QwtPlotItem::RenderAntialiased, false);
	QwtSymbol *symbolMPixX = new QwtSymbol(QwtSymbol::Ellipse,
		QBrush(Qt::green), QPen(Qt::red, 1), QSize(8, 8));
	m_curveMPixX->setSymbol(symbolMPixX);
	m_curveMPixX->attach(m_plotMPix);
	m_plotMPix->hide();

	//////////////////////////////////////////////////////////////////////
	//速度
	m_plotV = new CPlotWidget(m_tabWidget);
	m_zoomerV = new Zoomer(1, m_plotV->canvas());
	m_plotV->setAxisTitle(QwtPlot::xBottom, QString("PTS(ms)"));
	m_plotV->setAxisTitle(QwtPlot::yLeft, QString("时速(km/h)"));
	m_plotV->setObjectName(QStringLiteral("tabV"));
	m_plotV->setCanvasBackground(Qt::white);
	m_plotV->insertLegend(new QwtLegend());
	QwtPlotGrid *gridV = new QwtPlotGrid();
	gridV->attach(m_plotV);
	m_curveV = new QwtPlotCurve();
	m_curveV->setTitle("V");
	m_curveV->setPen(Qt::blue, 4);
	m_curveV->setRenderHint(QwtPlotItem::RenderAntialiased, false);
	QwtSymbol *symbolV = new QwtSymbol(QwtSymbol::Ellipse,
		QBrush(Qt::yellow), QPen(Qt::red, 2), QSize(8, 8));
	m_curveV->setSymbol(symbolV);
	m_curveV->attach(m_plotV);
	m_tabWidget->setCurrentIndex(0);
	m_tabWidget->addTab(m_plotV, QString("速度"));
}

CShowTrace::~CShowTrace()
{
}

void CShowTrace::resizeEvent(QResizeEvent* event)
{
	if (m_tabWidget)
	{
		QSize size = event->size();
		int w = size.width();
		m_tabWidget->resize(w, 500);

	}
	QWidget::resizeEvent(event);
}

void CShowTrace::drawMPoints(QVector<QPointF> &p0s, QVector<QPointF> &pxs, QVector<int64_t> &ptss)
{
	m_tabWidget->removeTab(0);
	m_tabWidget->insertTab(0, m_plotM, QString("行驶轨迹"));
	m_plotM->show();
	m_plotMPix->hide();
	QPolygonF points;//P0待测点
	QSet<QPointF> p0_points;
	QPolygonF pointsX;//标定点

	float tempXMax = 0.0;
	float tempXMin = 0.0;
	float tempYMax = 0.0;
	float tempYMin = 0.0;

	bool flag = false;

	if (p0s.size() == 0)
		return;

	QVector<QPointF>::iterator iter1 = p0s.begin();
	for (; iter1 != p0s.end(); iter1++)
	{
		float x = iter1->x();
		float y = iter1->y();

		if (!flag)
		{
			flag = true;
			tempXMax = x;
			tempXMin = x;
			tempYMax = y;
			tempYMin = y;
		}
		else {
			if (x > tempXMax && INVALID_VALUE_FLOAT != tempXMax)
				tempXMax = x;
			if (x < tempXMin && INVALID_VALUE_FLOAT != tempXMin)
				tempXMin = x;
			if (y > tempYMax && INVALID_VALUE_FLOAT != tempYMax)
				tempYMax = y;
			if (y < tempYMin && INVALID_VALUE_FLOAT != tempYMin)
				tempYMin = y;
		}
		
		//points << *iter1;
		p0_points << *iter1;
	}

	qDebug() << "tempXMax=" << tempXMax
		<< " tempXMin=" << tempXMin
		<< " tempYMax=" << tempYMax
		<< " tempYMin=" << tempYMin;

	QVector<QPointF>::iterator iter2 = pxs.begin();
	for (; iter2 != pxs.end(); iter2++)
	{
		if (*iter2 == QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT))
			continue;
		float x = iter2->x();
		float y = iter2->y();

		if (x > tempXMax && INVALID_VALUE_FLOAT != x)
			tempXMax = x;
		if (x < tempXMin && INVALID_VALUE_FLOAT != x)
			tempXMin = x;
		if (y > tempYMax && INVALID_VALUE_FLOAT != y)
			tempYMax = y;
		if (y < tempYMin && INVALID_VALUE_FLOAT != y)
			tempYMin = y;		
		pointsX << *iter2;
	}

	qDebug() << "tempXMax=" << tempXMax
		<< " tempXMin=" << tempXMin
		<< " tempYMax=" << tempYMax
		<< " tempYMin=" << tempYMin;
	tempXMax += 1.0;
	tempXMin -= 1.0;
	tempYMax += 1.0;
	tempYMin -= 1.0;
	m_plotM->setAxisScale(QwtPlot::xBottom, tempXMin, tempXMax);
	m_plotM->setAxisScale(QwtPlot::yLeft, tempYMin, tempYMax);
	QVector<QPointF> tmp(p0_points.begin(), p0_points.end());
	std::sort(tmp.begin(), tmp.end(), [](QPointF& a, QPointF& b) {
		return a.x() == b.x() ? a.y() < a.y() : a.x() < b.x();
	});
	m_curveM0->setSamples(tmp);
	/////////////////////////////////////////////////
	m_curveMX->setSamples(pointsX);

	m_zoomerM->points = tmp;
	pointsX.clear();
	QVector<QPointF>::iterator iter3 = pxs.begin();
	for (; iter3 != pxs.end(); iter3++)
	{
		pointsX << *iter3;
	}
	m_zoomerM->pointsX = pointsX;
	m_zoomerM->ptss = ptss;
	m_plotM->replot();
}
namespace std {
	template <>
	struct hash<QPointF> {
		size_t operator()(const QPointF& point) const noexcept {
			return qHash(qMakePair(point.x(), point.y()));
		}
	};
}
void CShowTrace::drawMPixPoints(int width, int height, QVector<QPointF>& p0s, QVector<QPointF>& pxs, QVector<int64_t>& ptss)
{
	//m_curveM0->setCurveAttribute(QwtPlotCurve::Fitted, false);
	m_tabWidget->removeTab(0);
	m_tabWidget->insertTab(0, m_plotMPix, QString("行驶轨迹"));
	m_plotMPix->show();
	m_plotM->hide();
	QPolygonF points;//P0待测点
	QPolygonF pointsX;//标定点
	QSet <QPointF> p0_points;
	bool flag = false;

	if (p0s.size() == 0)
		return;
	QVector<QPointF>::iterator iter1 = p0s.begin();
	for (; iter1 != p0s.end(); iter1++)
	{
		int x = iter1->x();
		int y = iter1->y();
		p0_points.insert(QPoint(x, height - y));
		//points << QPoint(x, height - y);
	}

	QVector<QPointF>::iterator iter2 = pxs.begin();
	for (; iter2 != pxs.end(); iter2++)
	{
		if (*iter2 == QPointF(INVALID_VALUE_FLOAT, INVALID_VALUE_FLOAT))
			continue;
		float x = iter2->x();
		float y = iter2->y();
		pointsX << *iter2;
	}

	m_plotMPix->setAxisScale(QwtPlot::yLeft, 0, height, height / 10);
	m_plotMPix->setAxisScale(QwtPlot::xBottom, 0, width, width / 10);

	//qDebug() << "begin: " << points[0] << "end: " << points[points.length() - 1];
	//for (auto&& item : points) {
	//	qDebug() << "item: " << item;
	//}

	QVector<QPointF> tmp(p0_points.begin(), p0_points.end());
	std::sort(tmp.begin(), tmp.end(), [](QPointF& a, QPointF& b) {
		return a.x() == b.x() ? a.y() < a.y() : a.x() < b.x();
	});
	m_curveMPix0->setSamples(tmp);

	/////////////////////////////////////////////////
	m_curveMPixX->setSamples(pointsX);

	m_zoomerMPix->points = tmp;
	pointsX.clear();
	QVector<QPointF>::iterator iter3 = pxs.begin();
	for (; iter3 != pxs.end(); iter3++)
	{
		pointsX << *iter3;
	}
	m_zoomerMPix->pointsX = pointsX;
	m_zoomerMPix->ptss = ptss;
	m_plotMPix->replot();
}

void CShowTrace::drawVPoints()
{
	float fv = 0.0;

	float tempXMax = 0.0;
	float tempXMin = 0.0;
	float tempYMax = 0.0;
	float tempYMin = 0.0;

	QPolygonF pointsTemp;

	float v = INVALID_VALUE_FLOAT;
	int64_t pts = 0;
	int size = g_proInfo.frameInfos.size();
	if (size == 0)
		return;

	QPolygonF points;
	float averageTime = 0.0;
	float fps = 0.0;
	QStringList strHMSList;
	CPointsManage::getVPoints(g_proInfo, points, strHMSList);

	if (points.size() == 0)
		return;
	if (points.size() > 2)
	{
		int n = 1;
		pointsTemp << points[0];
		for (; n < points.size() - 1; n++)
		{
			float y = points[n].ry();
			float x = (points[n - 1].rx() + points[n + 1].rx()) / 2;
			pointsTemp << QPointF(x, y);
		}
		pointsTemp << points[n];
		points = pointsTemp;
	}

	float fa = 0.0;

	int count = points.size();
	if (count >= 2)
	{
		float line[4] = { 0.0 };
		CvPoint* pointsX = (CvPoint*)malloc(count * sizeof(points[0]));
		for (int n = 0; n < count; n++)
		{
			pointsX[n].x = points[n].rx();
			pointsX[n].y = points[n].ry();
		}

		CvMat pointMat = cvMat(1, count, CV_32SC2, pointsX);
		cvFitLine(&pointMat, CV_DIST_L2, 1, 0.001, 0.001, line);
		if (line[0] != 0)
		{
			fa = line[1] * 1000 / (line[0] * 3.6);
			qDebug() << "line[0]=" << line[0] << " line[1]=" << line[1] << " line[2]=" << line[2] << " line[3]=" << line[3];
		}
		if (pointsX)
			free(pointsX);
	}

	QMap<int, QString> pointText;
	if (g_proInfo.timeMode == TIME_MODE_PTS)
	{
		m_plotV->setAxisTitle(QwtPlot::xBottom, QString("PTS(ms)\n加速度:%1 m/s2").arg(fa));

		VTFPS2AverageTime(g_proInfo, averageTime);
		averageTime /= 1000.0;

		for (int n = 0; n < points.size(); n++)
		{
			QString strPT = QString("PTS:%1 ms V:%2 km/h").arg(points[n].x()).arg(points[n].y());
			pointText[n] = strPT;
		}
	}
	else if (g_proInfo.timeMode == TIME_MODE_FPS)
	{
		m_plotV->setAxisTitle(QwtPlot::xBottom, QString("时间(ms)\n加速度:%1 m/s2").arg(fa));

		VTFPS2AverageTime(g_proInfo, averageTime);
		averageTime /= 1000.0;

		for (int n = 0; n < points.size(); n++)
		{
			points[n].setX(n * averageTime);
			QString strPT = QString("时间:%1 ms V:%2 km/h").arg(points[n].x()).arg(points[n].y());
			pointText[n] = strPT;
		}
	}
	else if (g_proInfo.timeMode == TIME_MODE_WATER_MARK)
	{
		m_plotV->setAxisTitle(QwtPlot::xBottom, QString("水印时间(ms)\n加速度:%1 m/s2").arg(fa));

		VTWT2AverageTime(g_proInfo, averageTime, fps);
		averageTime /= 1000.0;

		for (int n = 0; n < points.size(); n++)
		{
			points[n].setX(n * averageTime);
			QString strPT = QString("水印时间:%1 V:%2 km/h").arg(strHMSList[n]).arg(points[n].y());
			pointText[n] = strPT;
		}
	}

	float fvSum = 0.0;
	for (int n = 0; n < points.size(); n++)
	{
		float x = points[n].rx();
		float y = points[n].ry();

		if (n == 0)
		{
			tempXMax = x;
			tempXMin = x;
			tempYMax = y;
			tempYMin = y;
		}
		else
		{
			if (x > tempXMax && INVALID_VALUE_FLOAT != x)
				tempXMax = x;
			if (x < tempXMin && INVALID_VALUE_FLOAT != x)
				tempXMin = x;
			if (y > tempYMax && INVALID_VALUE_FLOAT != y)
				tempYMax = y;
			if (y < tempYMin && INVALID_VALUE_FLOAT != y)
				tempYMin = y;
		}
		fvSum += y;
	}

	tempXMax += averageTime ? averageTime : 40;
	tempXMin -= averageTime ? averageTime : 40;
	if (tempXMin < 0)
	{
		tempXMin = 0;
	}
	tempYMax += 1.0;
	tempYMin -= 1.0;

	m_plotV->setAxisScale(QwtPlot::xBottom, tempXMin, tempXMax);
	m_plotV->setAxisScale(QwtPlot::yLeft, tempYMin, tempYMax);
	QSet<QPointF> p0_points(points.begin(), points.end());
	QVector<QPointF> tmp(p0_points.begin(), p0_points.end());
	std::sort(tmp.begin(), tmp.end(), [](QPointF& a, QPointF& b) {
		return a.x() == b.x() ? a.y() < a.y() : a.x() < b.x();
	});
	m_curveMPix0->setSamples(tmp);
	qDebug() << QString("速度点") << tmp;

	m_curveV->setSamples(tmp);
	m_plotV->replot();
	m_zoomerV->pointsV = points;
	m_zoomerV->pointText = pointText;
}