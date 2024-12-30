#include "TakenControl.h"

#include <QKeyEvent>

#include "Global.h"
#include <QTextEdit>

CTakenControl::CTakenControl(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	//设置焦点策略
	ui.btnPrevious->setFocusPolicy(Qt::NoFocus);
	ui.btnNext->setFocusPolicy(Qt::NoFocus);
	ui.btnWaterMark->setFocusPolicy(Qt::NoFocus);
	ui.LBSign->setFocusPolicy(Qt::NoFocus);
	ui.labelXY->setFocusPolicy(Qt::NoFocus);
	ui.labelVelocity->setFocusPolicy(Qt::NoFocus);
	ui.labelStatus->setFocusPolicy(Qt::NoFocus);
	ui.TEWaterMark->setFocusPolicy(Qt::ClickFocus);
	ui.labelTips->setFocusPolicy(Qt::NoFocus);
	
	connect(ui.btnPrevious, SIGNAL(clicked()), parent, SLOT(onPreviousFrame()));
	//connect(ui.btnHeadFrame, SIGNAL(clicked()), parent, SLOT(onHeadFrame()));
	connect(ui.btnNext, SIGNAL(clicked()), parent, SLOT(onNextFrame()));
	connect(ui.TEWaterMark, SIGNAL(timeChanged(const QTime&)), parent, SLOT(onWaterMarkChanged(const QTime&)));

	auto text = QString("未编辑");
	auto w = 0;
	w = ui.labelStatus->fontMetrics().boundingRect(text).width();
	if (!w)
		ui.labelStatus->setMinimumWidth(w);
	if (!w)ui.labelStatus->setMaximumWidth(w);

	text = QString("坐标(-,-)");
	w = ui.labelXY->fontMetrics().boundingRect(text).width();
	if (!w)
		ui.labelXY->setMinimumWidth(w);
	if (!w)
		ui.labelXY->setMaximumWidth(w);

	text = QString("速度: 0 km/h");
	w = ui.labelVelocity->fontMetrics().boundingRect(text).width();
	if (!w)
		ui.labelVelocity->setMinimumWidth(w);
	if (!w)
		ui.labelVelocity->setMaximumWidth(w);

	ui.btnWaterMark->hide();
	connect(this, SIGNAL(sigWaterMarkWnd(QPoint)), parent, SLOT(onWaterMarkWnd(QPoint)));
	connect(ui.btnWaterMark, &QPushButton::clicked, 
		[=]() {
		emit sigWaterMarkWnd(ui.btnWaterMark->pos());
	});

	setSign(false);
}

CTakenControl::~CTakenControl()
{
	disconnect(ui.btnPrevious, SIGNAL(clicked()), parent(), SLOT(onPreviousFrame()));
	disconnect(ui.btnNext, SIGNAL(clicked()), parent(), SLOT(onNextFrame()));
}

void CTakenControl::setSign(bool sign)
{
	QString strImage;
	if (sign)
		strImage = ":/DrawTools/images/DrawTools/sign2.png";
	else
		strImage = ":/DrawTools/images/DrawTools/sign1.png";
	QPixmap pixmap(strImage);
	pixmap.scaled(ui.LBSign->size(), Qt::KeepAspectRatio);
	ui.LBSign->setScaledContents(true);
	ui.LBSign->setPixmap(pixmap);
}


void CTakenControl::setFrames(int fps, qint64 pts, int index, int actualIndex)
{
	auto frames = g_proInfo.frameInfos.size();
	auto actualFrames = g_proInfo.packetDTSs.size();
	ui.labelTips->setText(QString("FPS %1 | PTS %2  | 第 %3(%4) 帧 / 共 %5(%6) 帧").arg(fps).arg(pts / 1000).arg(index).arg(actualIndex).arg(frames).arg(actualFrames));
}

void CTakenControl::setXY(QPoint point)
{
	auto text = QString("坐标(%1,%2)").arg(point.x()).arg(point.y());
	ui.labelXY->setText(text);
}

void CTakenControl::setStatus(int status)
{
	auto text = QString("未编辑");
	if(0 == status)
		text = QString("未编辑");
	else if (1 == status)
		text = QString("已编辑");
	else if (2 == status)
		text = QString("首帧");
	ui.labelStatus->setText(text);
}

void CTakenControl::setVelocity(QString text)
{
	ui.labelVelocity->setText(text);
}

void CTakenControl::setWaterMark(QTime time)
{
	ui.TEWaterMark->setTime(time);
}