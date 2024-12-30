#include "TimeInfoWnd.h"

#include <QAbstractItemModel>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <QStandardItemModel>
#include <QTime>
#include <QTreeWidgetItem>
#include "Global.h"
#include <QDebug>
#include <QButtonGroup>
#include "PointsManage.h"

CTimeInfoWnd::CTimeInfoWnd(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowTitle(QString("时间信息"));
	m_modelPTS = new QSortFilterProxyModel();
	m_modelWaterTime = new QSortFilterProxyModel();
	m_modelMedia = new QSortFilterProxyModel();
	ui.gbOCR->setMaximumWidth(400);
	ui.gbFPS->setMaximumWidth(400);
	ui.gbMedia->setMaximumWidth(400);
	ui.gbPTS->setMaximumWidth(400);

	ui.TVPts->setRootIsDecorated(false);
	ui.TVPts->setAlternatingRowColors(true);
	ui.TVPts->setModel(m_modelPTS);

	ui.TVWaterTime->setRootIsDecorated(false);
	ui.TVWaterTime->setAlternatingRowColors(true);
	ui.TVWaterTime->setModel(m_modelWaterTime);

	ui.TVMedia->setRootIsDecorated(false);
	ui.TVMedia->setAlternatingRowColors(true);
	ui.TVMedia->setModel(m_modelMedia);

	if (g_proInfo.timeMode == TIME_MODE_WATER_MARK)
		ui.CBOCR->setChecked(true);
	else if (g_proInfo.timeMode == TIME_MODE_PTS)
		ui.CBPTS->setChecked(true);
	else if (g_proInfo.timeMode == TIME_MODE_FPS)
		ui.CBFPS->setChecked(true);
	else
		ui.CBPTS->setChecked(true);

	QButtonGroup* group = new QButtonGroup(this);
	group->addButton(ui.CBOCR, 1);
	group->addButton(ui.CBPTS, 2);
	group->addButton(ui.CBFPS, 3);

	connect(ui.CBOCR, &QCheckBox::stateChanged, [=](int state) {
		if (state == Qt::Checked)
			g_proInfo.timeMode = TIME_MODE_WATER_MARK;
	});

	connect(ui.CBPTS, &QCheckBox::stateChanged, [=](int state) {
		if (state == Qt::Checked)
			g_proInfo.timeMode = TIME_MODE_PTS;
	});

	connect(ui.CBFPS, &QCheckBox::stateChanged, [=](int state) {
		if (state == Qt::Checked)
			g_proInfo.timeMode = TIME_MODE_FPS;
	});

	onOCR();
	onPTS();
	onFPS();
	onMedia();
	onPoTrace();
}

CTimeInfoWnd::~CTimeInfoWnd()
{
}

void CTimeInfoWnd::onOCR() 
{
	auto model = new QStandardItemModel(0, 2, this);
	model->setHeaderData(0, Qt::Horizontal, QString("水印时间"));
	model->setHeaderData(1, Qt::Horizontal, QString("帧数"));
	QVector<int> counts;
	QVector<QString> waterTimes;
	int count = 0, index;
	QString waterTime = "";
	for (auto& info : g_proInfo.frameInfos)
	{
		QString timeHMS = info.timeHMS;
		if (VTIsHHMMSS(timeHMS))
		{
			if (waterTime == "")
			{
				waterTime = timeHMS;
				count++;
			}
			else if (waterTime == timeHMS)
			{
				count++;
			}
			else
			{
				counts.push_back(count);
				waterTimes.push_back(waterTime);
				count = 1;
				waterTime = timeHMS;
			}
		}
	}
	if (count > 0)
	{
		counts.push_back(count);
		waterTimes.push_back(waterTime);
	}
	int size = counts.size();
	int duration = 0;
	int frames = 0;
	for (index = size - 1; index >= 0; --index)
	{
		model->insertRow(0);
		model->setData(model->index(0, 0), waterTimes[index]);
		model->setData(model->index(0, 1), counts[index]);

		frames += counts[index];
	}

	if (waterTimes.size() >= 2) {
		QTime begin, end;
		begin = QTime::fromString(waterTimes[0], "hh:mm:ss");
		end = QTime::fromString(waterTimes[waterTimes.size() - 1], "hh:mm:ss");
		duration = (end.hour() * 3600 + end.minute() * 60 + end.second()) -
			(begin.hour() * 3600 + begin.minute() * 60 + begin.second());
	}
	else if (waterTimes.size() == 1) {
		duration = 1;
	}
	model->insertRow(0);
	model->setData(model->index(0, 0), QString("总时长 %1 s").arg(duration));
	model->setData(model->index(0, 1), QString("总帧数 %1").arg(frames));
	m_modelWaterTime->setSourceModel(model);
}

void CTimeInfoWnd::onPTS()
{
	int index;
	auto model = new QStandardItemModel(0, 3, this);
	model->setHeaderData(0, Qt::Horizontal, QString("帧号"));
	model->setHeaderData(1, Qt::Horizontal, QString("PTS（毫秒）"));
	model->setHeaderData(2, Qt::Horizontal, QString("间隔（毫秒）"));

	auto size = g_proInfo.frameInfos.size();
	int64_t interval = 0;
	vector<int64_t> intervals;
	for (index = 0; index < size; ++index)
	{
		if (index == 0)
			interval = g_proInfo.frameInfos[index].pts;
		intervals.push_back((g_proInfo.frameInfos[index].pts - interval) / 1000);
		interval = g_proInfo.frameInfos[index].pts;
	}
	for (index = size - 1; index >= 0; --index)
	{
		QString tmp = VTMillisecondsToTimeFormat(g_proInfo.frameInfos[index].pts / 1000);
		if (index == size - 1 || index == 0) {
			vec.emplaceBack(tmp);
		}
		model->insertRow(0);
		model->setData(model->index(0, 0), g_proInfo.frameInfos[index].index);
		model->setData(model->index(0, 1), tmp);
		model->setData(model->index(0, 2), intervals[index]);
	}
	m_modelPTS->setSourceModel(model);
}

void CTimeInfoWnd::onFPS()
{
	QString text = QString("平均帧率：%1").arg(g_proInfo.fps);
	ui.LBFps->setText(text);
}

void CTimeInfoWnd::onMedia()
{
	auto model = new QStandardItemModel(0, 2, this);
	model->setHeaderData(0, Qt::Horizontal, QString("水印时间"));
	model->setHeaderData(1, Qt::Horizontal, QString("帧数"));
	int64_t dts = -1;
	int count = 0, index;
	QVector<int> counts;
	QVector<int64_t> dtss;
	auto packetDTSs = g_proInfo.packetDTSs;
	int duration = g_proInfo.duration / 1000000;
	int64_t startTime = g_proInfo.startTime;

	for (auto& pktDts : packetDTSs)
	{
		if (dts == -1)
		{
			dts = (pktDts - startTime) / 1000000;
			count++;
		}
		else if (dts == (pktDts - startTime) / 1000000)
		{
			count++;
		}
		else
		{
			counts.push_back(count);
			dtss.push_back(dts);
			count = 1;
			dts = (pktDts - startTime) / 1000000;
		}
	} 
	if (count > 0)
	{
		counts.push_back(count);
		dtss.push_back(dts);
	}
	int size = counts.size();
	// 求出相差多少秒
	bool isBeginRender = false;
	QString endTime = "60:60:60";
	int differenceInSeconds = 0;
	if (vec.size() == 2) {
		endTime = vec[0];
		auto beginTime = vec[1];
		QTime time1 = QTime::fromString(beginTime, "hh:mm:ss.zzz");
		QTime time2 = QTime::fromString(endTime, "hh:mm:ss.zzz");
		differenceInSeconds = time1.secsTo(time2);
	}
	for (index = size - 1; index >= 0; --index)
	{
		auto tmp = VTSecondsToTimeFormat(dtss[index]);
		if (!isBeginRender && endTime != "60:60:60" && endTime.mid(0, 8) == tmp) {
			isBeginRender = true;
		}
		model->insertRow(0);
		model->setData(model->index(0, 0), tmp);
		model->setData(model->index(0, 1), counts[index]);
		if (isBeginRender && differenceInSeconds >= 0) {
			// 字体标红
			model->itemFromIndex(model->index(0, 0))->setForeground(Qt::red);
			model->itemFromIndex(model->index(0, 1))->setForeground(Qt::red);
			differenceInSeconds--;
		}
	}
	model->insertRow(0);
	model->setData(model->index(0, 0), QString("总时长 %1 s").arg(duration));
	model->setData(model->index(0, 1), QString("总帧数 %1").arg(packetDTSs.size()));
	m_modelMedia->setSourceModel(model);
}

void CTimeInfoWnd::onPoTrace()
{
	QVector<QPointF> p0s;
	QVector<int64_t> ptss;
	CPointsManage::getMPoints(g_proInfo, p0s, ptss);

	auto width = 1920, height = 1080;
	if (g_proInfo.frameInfos.size() > 0) {
		if (g_proInfo.firstImage && g_proInfo.firstImage->image) {
			width = g_proInfo.firstImage->image->width();
			height = g_proInfo.firstImage->image->height();
		}
	}

	//ui.P0Trace->setValue(p0s, ptss, width, height);
}