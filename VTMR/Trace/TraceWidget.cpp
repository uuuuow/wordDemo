#include "TraceWidget.h"
#include <QTabWidget>


CTraceWidget::CTraceWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	setMouseTracking(true);
	resize(QSize(500, 500));

	m_moveChart = new QChart();

	//m_tabWidget = new QTabWidget(this);
	//m_tabWidget->setObjectName(QStringLiteral("tabWidget"));
	//m_tabWidget->setGeometry(QRect(0, 0, width(), height()));
	//m_tabWidget->addTab(m_moveChart, QString("行驶轨迹"));

	////////////////////////////////////////////////////////////////////////
	////速度
	//m_tabWidget->addTab(m_moveChart, QString("速度"));
	//m_tabWidget->setCurrentIndex(0);
}

CTraceWidget::~CTraceWidget()
{
}

void CTraceWidget::initViewMove()
{
	QSplineSeries *series1 = new QSplineSeries();//实例化一个QLineSeries对象
	series1->setColor(QColor(0, 100, 255));

	series1->append(QPointF(0, rand() % 200));
	series1->append(QPointF(30, rand() % 200));
	series1->append(QPointF(60, rand() % 200));
	series1->append(QPointF(90, rand() % 200));
	series1->append(QPointF(120, rand() % 200));

	series1->setName("线条1");

	series1->setVisible(true);
	series1->setPointLabelsFormat("(@xPoint,@yPoint)");
	series1->setPointLabelsVisible(true);

	m_moveChart->setTheme(QChart::ChartThemeLight);//设置白色主题
	m_moveChart->setDropShadowEnabled(true);//背景阴影    m_chart->setAutoFillBackground(true);  //设置背景自动填充
	m_moveChart->addSeries(series1);//添加系列到QChart上


	m_moveChart->setTitleBrush(QBrush(QColor(0, 0, 255)));//设置标题Brush
	m_moveChart->setTitleFont(QFont("微软雅黑"));//设置标题字体
	m_moveChart->setTitle("曲线图");


	//创建X轴和Y轴
	QValueAxis *axisX = new QValueAxis;
	axisX->setRange(0, 150);    //默认则坐标为动态计算大小的
	axisX->setLabelFormat("%dS");
	QValueAxis *axisY = new QValueAxis;
	axisY->setRange(0, 250);    //默认则坐标为动态计算大小的
	axisY->setTitleText("value值");

	m_moveChart->setAxisX(axisX, series1);
	m_moveChart->setAxisY(axisY, series1);
	//m_chart->createDefaultAxes();             //或者创建默认轴

	//修改说明样式
	m_moveChart->legend()->setVisible(true);
	m_moveChart->legend()->setAlignment(Qt::AlignBottom);//底部对齐
	m_moveChart->legend()->setBackgroundVisible(true);//设置背景是否可视
	m_moveChart->legend()->setAutoFillBackground(true);//设置背景自动填充
	m_moveChart->legend()->setColor(QColor(222, 233, 251));//设置颜色
	m_moveChart->legend()->setLabelColor(QColor(0, 100, 255));//设置标签颜色
	m_moveChart->legend()->setMaximumHeight(50);
	m_chartView = new QChartView(m_moveChart);
	m_chartView->setRenderHint(QPainter::Antialiasing);

	QVBoxLayout *vLayout = new QVBoxLayout(this);
	vLayout->addWidget(m_chartView);
}