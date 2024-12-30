#include "StartUpDialog.h"
#include <QTimer>
#include <QPainter>
#include <QPainterPath>

CStartUpDialog::CStartUpDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);
	setWindowIcon(QIcon(":/VTMR/images/VTMR/vtmr.ico"));
	m_pixmap = QPixmap(":/StartUp/images/StartUp/startupbg.jpg");
	QSize scalesize(720, 420);
	m_pixmap = m_pixmap.scaled(scalesize, Qt::KeepAspectRatio);
	resize(m_pixmap.width(), m_pixmap.height());

	int x = (width() - ui.progressbar->width()) / 2;
	int y = (height() - ui.progressbar->height()) / 2;
	ui.progressbar->move(QPoint(x, y));

	m_timer = new QTimer(this);
	m_persent = 0;
	connect(m_timer, SIGNAL(timeout()), this, SLOT(updateProgressbar()));
	m_timer->start(150);
}

CStartUpDialog::~CStartUpDialog()
{
}

void CStartUpDialog::updateProgressbar()
{
	if (m_persent >= 100) {
		close();
	}
	else {
		m_persent += rand() % 5 + 3;
		if (m_persent >= 100)
			m_persent = 100;
	}
	
	ui.progressbar->setPersent(m_persent);
}

void CStartUpDialog::paintEvent(QPaintEvent *ev)
{
	QPainter painter(this);
	painter.drawPixmap(0, 0, m_pixmap);

	QFont font;
	font.setPointSize(20);
	font.setBold(false);
	font.setFamily("Microsoft YaHei");

	QFontMetrics metrics(font);
	QPainterPath path;
	QPen pen(QColor(255, 0, 0, 100));
	//QPen pen(QColor("#FFD4A9"));
	int penWidth = font.pointSize();
	if (penWidth > 6) {
		penWidth = 6;
	}
	pen.setWidth(penWidth);

	int len1 = metrics.boundingRect(QString("视频速度重建系统")).width();

	int w = width();
	int px1 = (len1 - w) / 2;
	if (px1 < 0) {
		px1 = -px1;
	}
	int py = 100;
	path.addText(px1, py, font, QString("视频速度重建系统"));

	painter.strokePath(path, pen);
	painter.drawPath(path);
	painter.fillPath(path, QBrush("#FFFFCC"));
	//painter.fillPath(path, QBrush("#80D1C8"));


	///////////////////////////////

	font.setPointSize(10);
	font.setBold(false);

	QFontMetrics metricsCompany(font);
	QPainterPath pathCompany;
	QPen penCompany(QColor(200, 0, 0, 100));
	//QPen penCompany(QColor("#FFFFCC"));
	penWidth = font.pointSize();
	if (penWidth > 6) {
		penWidth = 6;
	}
	penCompany.setWidth(penWidth);

	int len2 = metricsCompany.boundingRect(QString("单位：司法鉴定科学研究院")).width();
	int px2 = width() - len2 - 20;
	int py2 = height() - 60;

	pathCompany.addText(px2, py2, font, QString("单位：司法鉴定科学研究院"));
	painter.strokePath(pathCompany, penCompany);
	painter.drawPath(pathCompany);
	//painter.fillPath(pathCompany, QBrush("#00FFFF"));
	painter.fillPath(pathCompany, QBrush("#FFFFCC"));
	//painter.fillPath(pathCompany, QBrush("#FFD4A9"));


	int len3 = metricsCompany.boundingRect(QString("技术支持：上海弘目智能科技有限公司")).width();
	int px3 = width() - len3 - 20;
	int py3 = height() - 20;

	pathCompany.addText(px3, py3, font, QString("技术支持：上海弘目智能科技有限公司"));
	painter.strokePath(pathCompany, penCompany);
	painter.drawPath(pathCompany);

	QDialog::paintEvent(ev);
}
